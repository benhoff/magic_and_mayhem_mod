#include "surface_backend.hpp"
#include "dib.hpp"
#include <algorithm>
#include <limits>

namespace mnm::render {
struct SurfaceBackend::Entry {
    int width,height;
    PixelFormat format;
    std::uint32_t dcToken;
    SurfaceAccessState access;
    BitmapDcState dc;
    std::optional<Palette> defaults,binding;
    ClipperState clipper;
    std::optional<std::uint16_t> key;
    std::optional<Image> locked;
    Entry(const Image& image,PixelFormat f,unsigned caps,std::uint32_t storage,
          std::uint32_t token,std::optional<Palette> context)
        :width(image.width),height(image.height),format(f),dcToken(token),
         access(image.width,image.height,f.bits,caps,storage,token),
         dc(image.width,image.height,f.bits,context),defaults(std::move(context)){}
};
SurfaceBackend::SurfaceBackend()=default;
SurfaceBackend::~SurfaceBackend()=default; // Whole owner teardown also drops poisoned leases.
SurfaceBackend::Entry& SurfaceBackend::entry(SurfaceId id){
    gl_.stats();auto at=entries_.find(id);
    if(at==entries_.end())throw std::runtime_error("Unknown or foreign owned surface");
    return *at->second;
}
const SurfaceBackend::Entry& SurfaceBackend::entry(SurfaceId id) const {
    gl_.stats();auto at=entries_.find(id);
    if(at==entries_.end())throw std::runtime_error("Unknown or foreign owned surface");
    return *at->second;
}
void SurfaceBackend::available(const Entry& e){
    if(e.access.borrowed())throw std::runtime_error("Owned surface is borrowed or poisoned");
}
void SurfaceBackend::raster(const Entry& e){
    if(!e.access.dcActive() || e.locked || e.access.poisoned())
        throw std::runtime_error("Owned DC raster requires an active unpoisoned DC without a CPU lease");
}
SurfaceId SurfaceBackend::create(const Image& image,PixelFormat format,unsigned caps,std::optional<Palette> defaults){
    const std::array<std::uint32_t,3> masks=format.bits==8?std::array<std::uint32_t,3>{}:
        format.bits==16?std::array<std::uint32_t,3>{0xf800,0x7e0,31}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff};
    if((format.bits!=8 && format.bits!=16 && format.bits!=32) || format.masks!=masks)
        throw std::runtime_error("Owned backend requires indexed8/canonical RGB565/RGB32");
    if(nextToken_>std::numeric_limits<std::uint32_t>::max()-2)
        throw std::runtime_error("Owned surface tokens exhausted");
    auto e=std::make_unique<Entry>(image,format,caps,nextToken_,nextToken_+1,std::move(defaults));
    // Sentinel values are never issued, and token identities never wrap/reuse.
    if(nextToken_==0xabababaa || nextToken_==0xabababab)throw std::runtime_error("Owned token sentinel reached");
    auto id=gl_.create(image,format);
    try {if(e->defaults)gl_.setPalette(id,0,{e->defaults->begin(),e->defaults->end()});entries_.emplace(id,std::move(e));}
    catch(...){gl_.destroy(id);throw;}
    nextToken_+=2;return id;
}
void SurfaceBackend::destroy(SurfaceId id){available(entry(id));gl_.destroy(id);entries_.erase(id);}
void SurfaceBackend::update(SurfaceId id,int x,int y,const Image& patch){available(entry(id));gl_.update(id,x,y,patch);}
std::uint32_t SurfaceBackend::lock(SurfaceId id,Descriptor& output){
    auto& e=entry(id);
    if(e.access.borrowed())return e.access.lock(output);
    // Unknown contents refuse before changing admission or the output descriptor.
    auto pixels=gl_.read(id);auto result=e.access.lock(output);
    if(!result)e.locked=std::move(pixels);
    return result;
}
const Image& SurfaceBackend::lockedPixels(SurfaceId id) const {
    const auto& e=entry(id);if(!e.locked)throw std::runtime_error("No owned CPU pixel lease");return *e.locked;
}
void SurfaceBackend::writeLocked(SurfaceId id,int x,int y,const Image& patch){
    auto& e=entry(id);
    if(!e.locked || e.access.dcActive() || e.access.poisoned())throw std::runtime_error("Unsupported CPU/DC write interleaving");
    gl_.update(id,x,y,patch);
    for(int row=0;row<patch.height;++row)
        std::copy_n(patch.pixels.begin()+std::size_t(row)*patch.width,patch.width,
                    e.locked->pixels.begin()+std::size_t(row+y)*e.width+x);
}
std::uint32_t SurfaceBackend::unlock(SurfaceId id){auto& e=entry(id);auto result=e.access.unlock();if(!result)e.locked.reset();return result;}
void SurfaceBackend::installBinding(SurfaceId id,const Entry& e){
    if(e.format.bits!=8)return;
    const auto* colors=e.binding?&*e.binding:e.defaults?&*e.defaults:nullptr;
    if(colors)gl_.setPalette(id,0,{colors->begin(),colors->end()});
}
std::uint32_t SurfaceBackend::acquireDc(SurfaceId id,std::uint32_t& output){
    auto& e=entry(id);
    if(e.access.dcActive())return e.access.acquireDc(output);
    if(e.access.poisoned())throw std::runtime_error("DC reacquisition after mapping debt is unvalidated");
    e.dc.acquire();
    try {if(e.format.bits==8){const auto& p=e.dc.palette();gl_.setPalette(id,0,{p.begin(),p.end()});}return e.access.acquireDc(output);}
    catch(...){e.dc.release();throw;}
}
std::uint32_t SurfaceBackend::releaseDc(SurfaceId id,std::uint32_t token){
    auto& e=entry(id);
    if(!e.access.dcActive() || token!=e.dcToken)return e.access.releaseDc(token);
    if(e.access.mapBalance()<=-64)throw std::runtime_error("Owned DC release budget exceeded");
    installBinding(id,e);const auto result=e.access.releaseDc(token);e.dc.release();return result;
}
const SurfaceBackend::Palette& SurfaceBackend::dcPalette(SurfaceId id) const {return entry(id).dc.palette();}
const std::optional<std::vector<Rect>>& SurfaceBackend::dcRegions(SurfaceId id) const {return entry(id).dc.regions();}
void SurfaceBackend::selectDcRegions(SurfaceId id,std::optional<std::vector<Rect>> regions){raster(entry(id));entry(id).dc.selectRegions(std::move(regions));}
void SurfaceBackend::setDcColors(SurfaceId id,unsigned first,const std::vector<Rgb>& colors){
    auto& e=entry(id);raster(e);e.dc.setDcColors(first,colors);gl_.setPalette(id,first,colors);
}
void SurfaceBackend::bindPalette(SurfaceId id,std::optional<Palette> palette){
    auto& e=entry(id);
    if(e.format.bits!=8)throw std::runtime_error("Palette binding requires indexed8");
    if(!palette && !e.defaults)throw std::runtime_error("Palette detach requires explicit default context");
    e.dc.bindPalette(palette);e.binding=std::move(palette);if(!e.access.dcActive())installBinding(id,e);
}
void SurfaceBackend::updatePalette(SurfaceId id,unsigned first,const std::vector<Rgb>& colors){
    auto& e=entry(id);e.dc.updateBoundPalette(first,colors);
    std::copy(colors.begin(),colors.end(),e.binding->begin()+first);
    if(!e.access.dcActive())gl_.setPalette(id,first,colors);
}
void SurfaceBackend::reloadDib(SurfaceId id,const DibInput& dib){auto& e=entry(id);raster(e);e.dc.reload(gl_,id,dib);}
void SurfaceBackend::setClipper(SurfaceId id,const ClipperState& clip){auto& e=entry(id);gl_.setClipper(id,clip);e.clipper=clip;}
void SurfaceBackend::setSourceKey(SurfaceId id,std::optional<std::uint16_t> key){
    auto& e=entry(id);if(e.format.bits!=16)throw std::runtime_error("Owned source-key contract requires RGB565");e.key=key;
}
SurfaceCopyResult SurfaceBackend::copy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest& request){
    auto& s=entry(source);auto& d=entry(destination);
    if(s.format.bits!=16 || d.format.bits!=16)throw std::runtime_error("Owned draw contract requires RGB565");
    if(request.sourceBusy || request.destinationBusy)throw std::runtime_error("Owned admission cannot use supplied busy flags");
    auto r=request;r.sourceBusy=s.access.borrowed();r.destinationBusy=d.access.borrowed();
    const auto keyFlag=r.api==SurfaceCopyApi::BltFast?1u:0x8000u;
    const auto wait=r.api==SurfaceCopyApi::BltFast?0x10u:0x1000000u;
    if(r.flags & ~(wait|keyFlag))return gl_.surfaceCopy(source,destination,r);
    const bool keyed=(r.flags&keyFlag)!=0;
    if(!keyed)return gl_.surfaceCopy(source,destination,r);
    r.flags=s.key?(r.flags&wait):keyFlag;
    const auto plan=planSurfaceCopy(s.width,s.height,d.width,d.height,d.clipper,r);
    if(source==destination && s.key)throw std::runtime_error("Keyed overlap awaits independent comparison");
    for(const auto& p:plan.pieces)gl_.copy(source,destination,p.source,p.x,p.y,s.key);
    return {plan.hresult,unsigned(plan.pieces.size())};
}
SurfaceCopyResult SurfaceBackend::fill(SurfaceId id,std::optional<Rect> rectangle,std::uint16_t color,std::uint32_t flags){
    auto& e=entry(id);
    if(e.format.bits!=16 || (flags!=0x400 && flags!=0x1000400))throw std::runtime_error("Unvalidated owned fill format/flags");
    const auto r=rectangle.value_or(Rect{0,0,e.width,e.height});
    if(r.left>=r.right || r.top>=r.bottom)return {surfaceStatus::invalidRect,0};
    if(e.clipper.attached && !e.clipper.regions)return {surfaceStatus::noClipList,0};
    if(!e.clipper.attached && (r.left<0 || r.top<0 || r.right>e.width || r.bottom>e.height))return {surfaceStatus::invalidRect,0};
    if(e.access.borrowed())return {surfaceStatus::busy,0};
    const std::vector<Rect> full{{0,0,e.width,e.height}};
    const auto& regions=e.clipper.attached?*e.clipper.regions:full;unsigned pieces=0;
    for(auto clip:regions){Rect p{std::max(r.left,clip.left),std::max(r.top,clip.top),std::min(r.right,clip.right),std::min(r.bottom,clip.bottom)};
        if(p.left>=p.right || p.top>=p.bottom)continue;
        gl_.update(id,p.left,p.top,{p.right-p.left,p.bottom-p.top,std::vector<std::uint32_t>(std::size_t(p.right-p.left)*(p.bottom-p.top),color)});++pieces;
    }
    return {0,pieces};
}
Image SurfaceBackend::read(SurfaceId id){available(entry(id));return gl_.read(id);}
QImage SurfaceBackend::present(SurfaceId id){available(entry(id));return gl_.present(id);}
GpuFrame SurfaceBackend::presentGpu(SurfaceId id){available(entry(id));return gl_.presentGpu(id);}
Image SurfaceBackend::readDc(SurfaceId id){raster(entry(id));return gl_.read(id);}
QImage SurfaceBackend::presentDc(SurfaceId id){raster(entry(id));return gl_.present(id);}
void SurfaceBackend::invalidateContents(SurfaceId id){available(entry(id));gl_.invalidateContents(id);}
bool SurfaceBackend::borrowed(SurfaceId id) const {return entry(id).access.borrowed();}
bool SurfaceBackend::poisoned(SurfaceId id) const {return entry(id).access.poisoned();}
int SurfaceBackend::mapBalance(SurfaceId id) const {return entry(id).access.mapBalance();}
RenderStats SurfaceBackend::stats() const{return gl_.stats();}
Driver SurfaceBackend::driver() const{return gl_.driver();}
}
