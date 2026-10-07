#include "surface_backend.hpp"
#include "dib.hpp"
#include <algorithm>
#include <atomic>
#include <limits>

namespace mnm::render {
struct SurfaceBackend::PaletteObject {
    PaletteId id;Palette colors;unsigned references=1;
};
struct SurfaceBackend::Entry {
    unsigned references=1;bool primary=false,lost=false;
    std::optional<SurfaceId> back,front;
    std::shared_ptr<PaletteObject> palette;
    int width,height;
    PixelFormat format;
    std::uint32_t dcToken;
    SurfaceAccessState access;
    BitmapDcState dc;
    std::optional<Palette> defaults,binding;
    ClipperState clipper;
    std::optional<std::uint32_t> key;
    std::optional<Image> locked;
    Entry(const Image& image,PixelFormat f,unsigned caps,std::uint32_t storage,
          std::uint32_t token,std::optional<Palette> context,std::optional<std::int32_t> pitch)
        :width(image.width),height(image.height),format(f),dcToken(token),
         access(image.width,image.height,f.bits,caps,storage,token,f.masks,pitch),
         dc(image.width,image.height,f.bits,context),defaults(std::move(context)){}
};
SurfaceBackend::SurfaceBackend()=default;
SurfaceBackend::~SurfaceBackend()=default; // Whole owner teardown also drops poisoned leases.
SurfaceBackend::Entry& SurfaceBackend::entry(SurfaceId id){
    gl_.stats();auto at=entries_.find(id);
    if(at==entries_.end() || !at->second->references)throw std::runtime_error("Unknown or foreign owned surface");
    return *at->second;
}
const SurfaceBackend::Entry& SurfaceBackend::entry(SurfaceId id) const {
    gl_.stats();auto at=entries_.find(id);
    if(at==entries_.end() || !at->second->references)throw std::runtime_error("Unknown or foreign owned surface");
    return *at->second;
}
void SurfaceBackend::idle(const Entry& e){
    if(e.access.borrowed())throw std::runtime_error("Owned surface is borrowed or poisoned");
}
void SurfaceBackend::available(const Entry& e){
    idle(e);if(e.lost)throw std::runtime_error("Owned surface contents are lost");
}
void SurfaceBackend::raster(const Entry& e){
    if(e.lost)throw std::runtime_error("Owned DC raster target is lost");
    if(!e.access.dcActive() || !e.access.dcMatchesStorage() || e.locked || e.access.poisoned())
        throw std::runtime_error("Owned DC raster requires an active unpoisoned DC without a CPU lease");
}
SurfaceId SurfaceBackend::create(const Image& image,PixelFormat format,unsigned caps,std::optional<Palette> defaults,bool primary,std::optional<std::int32_t> rowPitch){
    validateSurfaceFormat(format);
    if(nextToken_>std::numeric_limits<std::uint32_t>::max()-2)
        throw std::runtime_error("Owned surface tokens exhausted");
    auto e=std::make_unique<Entry>(image,format,caps,nextToken_,nextToken_+1,std::move(defaults),rowPitch);
    // Sentinel values are never issued, and token identities never wrap/reuse.
    if(nextToken_==0xabababaa || nextToken_==0xabababab)throw std::runtime_error("Owned token sentinel reached");
    e->primary=primary;auto id=gl_.create(image,format);
    try {if(e->defaults)gl_.setPalette(id,0,{e->defaults->begin(),e->defaults->end()});entries_.emplace(id,std::move(e));}
    catch(...){gl_.destroy(id);throw;}
    nextToken_+=2;return id;
}
SurfaceId SurfaceBackend::createRows(const PixelRows& rows,PixelFormat format,unsigned caps,std::optional<Palette> defaults){
    return create(unpackPixelRows(rows,format),format,caps,std::move(defaults),false,rows.pitch);
}
void SurfaceBackend::updateRows(SurfaceId id,int x,int y,const PixelRows& rows){update(id,x,y,unpackPixelRows(rows,entry(id).format));}
void SurfaceBackend::readRows(SurfaceId id,PixelRows& rows){packPixelRows(read(id),entry(id).format,rows);}
void SurfaceBackend::writeLockedRows(SurfaceId id,int x,int y,const PixelRows& rows){writeLocked(id,x,y,unpackPixelRows(rows,entry(id).format));}
void SurfaceBackend::destroy(SurfaceId id){
    if(entry(id).references!=1)throw std::runtime_error("Use release for retained surface aliases");
    (void)release(id);
}
SurfaceId SurfaceBackend::alias(SurfaceId id){auto& e=entry(id);if(e.references==1024)throw std::runtime_error("Surface reference budget exceeded");++e.references;return id;}
unsigned SurfaceBackend::references(SurfaceId id) const{return entry(id).references;}
void SurfaceBackend::retire(SurfaceId id){
    auto& e=*entries_.at(id);const auto back=e.back;gl_.destroy(id);entries_.erase(id);
    if(back){auto& b=*entries_.at(*back);b.front.reset();if(!b.references)retire(*back);}
}
unsigned SurfaceBackend::release(SurfaceId id){
    auto& e=entry(id);
    if(e.references==1){idle(e);if(e.back){const auto& b=*entries_.at(*e.back);if(!b.references)idle(b);}}
    const auto remaining=--e.references;
    if(!remaining && !e.front)retire(id);
    return remaining;
}
SurfaceBackend::PaletteId SurfaceBackend::createPalette(const Palette& colors){
    gl_.stats();if(paletteObjects()>=256)throw std::runtime_error("Palette object budget exceeded");
    static std::atomic<PaletteId> serial{1};auto id=serial.load();
    do {if(!id || id==std::numeric_limits<PaletteId>::max())throw std::runtime_error("Palette identity exhausted");}
    while(!serial.compare_exchange_weak(id,id+1));
    auto p=std::make_shared<PaletteObject>();p->id=id;p->colors=colors;palettes_.emplace(id,std::move(p));return id;
}
std::shared_ptr<SurfaceBackend::PaletteObject> SurfaceBackend::paletteObject(PaletteId id) const{
    gl_.stats();auto at=palettes_.find(id);if(at==palettes_.end())throw std::runtime_error("Unknown or foreign palette object");return at->second;
}
unsigned SurfaceBackend::retainPalette(PaletteId id){gl_.stats();auto p=paletteObject(id);if(p->references==1024)throw std::runtime_error("Palette reference budget exceeded");return ++p->references;}
unsigned SurfaceBackend::releasePalette(PaletteId id){gl_.stats();auto p=paletteObject(id);const auto count=--p->references;if(!count)palettes_.erase(id);return count;}
std::size_t SurfaceBackend::paletteObjects() const{
    gl_.stats();std::vector<PaletteId> ids;for(const auto& pair:palettes_)ids.push_back(pair.first);
    for(const auto& pair:entries_)if(pair.second->palette)ids.push_back(pair.second->palette->id);
    std::sort(ids.begin(),ids.end());return std::unique(ids.begin(),ids.end())-ids.begin();
}
void SurfaceBackend::bindPaletteObject(SurfaceId id,std::optional<PaletteId> palette){
    auto& e=entry(id);std::shared_ptr<PaletteObject> object;if(palette)object=paletteObject(*palette);
    bindPalette(id,object?std::optional<Palette>(object->colors):std::nullopt);e.palette=std::move(object);
}
void SurfaceBackend::propagatePalette(const std::shared_ptr<PaletteObject>& p,unsigned first,const std::vector<Rgb>& colors){
    if(colors.empty() || first>=256 || colors.size()>256-first)throw std::runtime_error("Palette update outside table");
    std::copy(colors.begin(),colors.end(),p->colors.begin()+first);
    for(auto& pair:entries_){auto& e=*pair.second;if(e.palette!=p)continue;
        e.dc.updateBoundPalette(first,colors);std::copy(colors.begin(),colors.end(),e.binding->begin()+first);
        if(!e.access.dcActive())gl_.setPalette(pair.first,first,colors);
    }
}
void SurfaceBackend::updatePaletteObject(PaletteId id,unsigned first,const std::vector<Rgb>& colors){gl_.stats();propagatePalette(paletteObject(id),first,colors);}
std::uint32_t SurfaceBackend::getPaletteObject(SurfaceId id,PaletteId& output){
    auto& e=entry(id);if(e.lost)return 0x887601c2;
    if(!e.palette)return 0x8876023c; // DDERR_NOPALETTEATTACHED.
    if(e.palette->references==1024)throw std::runtime_error("Palette reference budget exceeded");
    palettes_.emplace(e.palette->id,e.palette);++e.palette->references;output=e.palette->id;return 0;
}
std::optional<SurfaceBackend::PaletteId> SurfaceBackend::paletteIdentity(SurfaceId id) const{const auto& p=entry(id).palette;return p?std::optional<PaletteId>(p->id):std::nullopt;}
void SurfaceBackend::attachBackBuffer(SurfaceId front,SurfaceId back){
    auto& f=entry(front);auto& b=entry(back);idle(f);idle(b);
    if(front==back || f.back || f.front || b.front || b.back || f.width!=b.width || f.height!=b.height || f.format.bits!=b.format.bits || f.format.masks!=b.format.masks)
        throw std::runtime_error("Unsupported two-buffer attachment");
    f.back=back;b.front=front;f.primary=true;
}
SurfaceId SurfaceBackend::getBackBuffer(SurfaceId front){
    const auto id=entry(front).back;if(!id)throw std::runtime_error("No attached back buffer");
    auto& b=*entries_.at(*id);if(b.references==1024)throw std::runtime_error("Surface reference budget exceeded");++b.references;return *id;
}
std::uint32_t SurfaceBackend::flip(SurfaceId front,std::uint32_t flags){
    auto& f=entry(front);if(!f.back || (flags!=0 && flags!=1))throw std::runtime_error("Unsupported two-buffer flip");
    auto& b=*entries_.at(*f.back);if(f.lost || b.lost)return 0x887601c2;
    if(f.access.poisoned() || b.access.poisoned())throw std::runtime_error("Flip after mapping debt is unvalidated");
    if((f.access.storageDcActive() && f.access.mapBalance()!=1) ||
       (b.access.storageDcActive() && b.access.mapBalance()!=1))
        throw std::runtime_error("Flip after unmatched DC Unlock is unvalidated");
    gl_.swapContents(front,*f.back);
    f.access.exchangeStorageLease(b.access);std::swap(f.locked,b.locked);
    for(const auto id:{front,*f.back}){const auto& e=*entries_.at(id);if(e.format.bits!=8)continue;
        if(e.access.dcActive()){const auto& p=e.dc.palette();gl_.setPalette(id,0,{p.begin(),p.end()});}
        else installBinding(id,e);
    }
    return f.access.storageBorrowed()?SurfaceAccessState::busy:0;
}
void SurfaceBackend::markLost(SurfaceId id){auto& e=entry(id);idle(e);gl_.invalidateContents(id);e.lost=true;}
std::uint32_t SurfaceBackend::isLost(SurfaceId id) const{return entry(id).lost?0x887601c2:0;}
std::uint32_t SurfaceBackend::restore(SurfaceId id,std::optional<Mode> mode){
    auto& e=entry(id);idle(e);
    if(e.primary && e.lost){
        if(!mode)throw std::runtime_error("Primary Restore requires display context");
        if(mode->width!=e.width || mode->height!=e.height || mode->format.bits!=e.format.bits || mode->format.masks!=e.format.masks)return 0x8876024b;
    }
    if(e.lost){gl_.invalidateContents(id);e.lost=false;}
    return 0;
}
std::optional<std::uint32_t> SurfaceBackend::sourceKey(SurfaceId id) const{return entry(id).key;}
void SurfaceBackend::update(SurfaceId id,int x,int y,const Image& patch){available(entry(id));gl_.update(id,x,y,patch);}
std::uint32_t SurfaceBackend::lock(SurfaceId id,Descriptor& output){
    auto& e=entry(id);
    if(e.lost)return 0x887601c2;
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
    if(e.lost)return 0x887601c2;
    if(e.access.dcActive())return e.access.acquireDc(output);
    if(e.access.poisoned())throw std::runtime_error("DC reacquisition after mapping debt is unvalidated");
    if(e.access.storageDcActive())throw std::runtime_error("DC reacquisition on foreign flipped storage is unvalidated");
    e.dc.acquire();
    try {if(e.format.bits==8){const auto& p=e.dc.palette();gl_.setPalette(id,0,{p.begin(),p.end()});}return e.access.acquireDc(output);}
    catch(...){e.dc.release();throw;}
}
std::uint32_t SurfaceBackend::releaseDc(SurfaceId id,std::uint32_t token){
    auto& e=entry(id);
    if(!e.access.dcActive() || token!=e.dcToken || !e.access.dcMatchesStorage())return e.access.releaseDc(token);
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
    e.palette.reset();e.dc.bindPalette(palette);e.binding=std::move(palette);if(!e.access.dcActive())installBinding(id,e);
}
void SurfaceBackend::updatePalette(SurfaceId id,unsigned first,const std::vector<Rgb>& colors){
    auto& e=entry(id);if(e.palette){propagatePalette(e.palette,first,colors);return;}e.dc.updateBoundPalette(first,colors);
    std::copy(colors.begin(),colors.end(),e.binding->begin()+first);
    if(!e.access.dcActive())gl_.setPalette(id,first,colors);
}
void SurfaceBackend::reloadDib(SurfaceId id,const DibInput& dib){auto& e=entry(id);raster(e);e.dc.reload(gl_,id,dib);}
void SurfaceBackend::setClipper(SurfaceId id,const ClipperState& clip){auto& e=entry(id);gl_.setClipper(id,clip);e.clipper=clip;}
void SurfaceBackend::setSourceKey(SurfaceId id,std::optional<std::uint32_t> key){
    auto& e=entry(id);const auto maximum=e.format.bits==32?UINT32_MAX:(std::uint32_t{1}<<e.format.bits)-1;
    if(key && *key>maximum)throw std::runtime_error("Owned source key exceeds native storage width");
    e.access.setSourceKey(key);e.key=key;
}
SurfaceCopyResult SurfaceBackend::copy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest& request){
    auto& s=entry(source);auto& d=entry(destination);
    if(s.format.bits!=d.format.bits || s.format.masks!=d.format.masks)throw std::runtime_error("Owned draw requires identical native formats");
    if(request.sourceBusy || request.destinationBusy)throw std::runtime_error("Owned admission cannot use supplied busy flags");
    if(s.lost || d.lost)return {0x887601c2,0};
    auto r=request;r.sourceBusy=s.access.borrowed();r.destinationBusy=d.access.borrowed();
    // An explicitly empty Blt clip list executes no driver draw, even borrowed.
    if(r.api==SurfaceCopyApi::Blt && d.clipper.attached && d.clipper.regions && d.clipper.regions->empty())
        r.sourceBusy=r.destinationBusy=false;
    const auto keyFlag=r.api==SurfaceCopyApi::BltFast?1u:0x8000u;
    const auto wait=r.api==SurfaceCopyApi::BltFast?0x10u:0x1000000u;
    if(r.flags & ~(wait|keyFlag))return gl_.surfaceCopy(source,destination,r);
    const bool keyed=(r.flags&keyFlag)!=0;
    if(!keyed)return gl_.surfaceCopy(source,destination,r);
    r.flags=s.key?(r.flags&wait):keyFlag;
    const auto plan=planSurfaceCopy(s.width,s.height,d.width,d.height,d.clipper,r);
    // Valid, idle keyed Blt rejects a missing key before NOCLIPLIST. Keep
    // malformed/busy combinations at their separately captured planner boundary.
    if(!s.key && r.api==SurfaceCopyApi::Blt && plan.hresult==surfaceStatus::noClipList){
        const auto bare=planSurfaceCopy(s.width,s.height,d.width,d.height,{},r);
        if(bare.hresult==surfaceStatus::invalidArgument)return {bare.hresult,0};
    }
    if(source==destination && !s.key){
        if(r.api==SurfaceCopyApi::Blt)return {plan.hresult,unsigned(plan.pieces.size())};
        r.flags=request.flags&wait; // Measured missing-key BltFast is opaque.
    }
    return gl_.surfaceCopy(source,destination,r,s.key?std::optional<std::uint32_t>(*s.key):std::nullopt);
}
SurfaceCopyResult SurfaceBackend::fill(SurfaceId id,std::optional<Rect> rectangle,std::uint32_t color,std::uint32_t flags){
    auto& e=entry(id);
    if(flags!=0x400 && flags!=0x1000400)throw std::runtime_error("Unvalidated owned fill format/flags");
    if(e.access.poisoned())throw std::runtime_error("Fill after mapping debt is unvalidated");
    color&=e.format.bits==8?255u:e.format.masks[0]|e.format.masks[1]|e.format.masks[2];
    const auto r=rectangle.value_or(Rect{0,0,e.width,e.height});
    if(r.left>=r.right || r.top>=r.bottom)return {surfaceStatus::invalidRect,0};
    if(e.clipper.attached && !e.clipper.regions)return {surfaceStatus::noClipList,0};
    if(!e.clipper.attached && (r.left<0 || r.top<0 || r.right>e.width || r.bottom>e.height))return {surfaceStatus::invalidRect,0};
    const std::vector<Rect> full{{0,0,e.width,e.height}};
    const auto& regions=e.clipper.attached?*e.clipper.regions:full;unsigned pieces=0;
    for(auto clip:regions){Rect p{std::max(r.left,clip.left),std::max(r.top,clip.top),std::min(r.right,clip.right),std::min(r.bottom,clip.bottom)};
        if(p.left>=p.right || p.top>=p.bottom)continue;
        gl_.update(id,p.left,p.top,{p.right-p.left,p.bottom-p.top,std::vector<std::uint32_t>(std::size_t(p.right-p.left)*(p.bottom-p.top),color)});
        if(e.locked)for(int y=p.top;y<p.bottom;++y)
            std::fill(e.locked->pixels.begin()+std::size_t(y)*e.width+p.left,e.locked->pixels.begin()+std::size_t(y)*e.width+p.right,color);
        ++pieces;
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
