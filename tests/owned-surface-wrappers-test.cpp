#include "../reconstruction/rendering/owned_surface_wrappers.hpp"
#include <QGuiApplication>
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::render;
using namespace mnm::reconstruction;
static unsigned checks=0,refusals=0,copies=0,fills=0,reloads=0,bars=0;
static void require(bool ok,const char* why) {++checks; if(!ok) throw std::runtime_error(why);}
template<class F> static void refuses(F&& action) {
    bool caught=false;
    try {action();} catch(const std::exception&) {caught=true;}
    require(caught,"Expected bounded refusal"); ++refusals;
}
static PixelFormat rgb(unsigned bits) {
    return {bits,bits==8 ? std::array<std::uint32_t,3>{} : bits==16
        ? std::array<std::uint32_t,3>{0xf800,0x7e0,31}
        : std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};
}
static Image plane(unsigned value=0) {return {8,6,std::vector<std::uint32_t>(48,value)};}
static bool equal(SurfaceRectangle32 a,SurfaceRectangle32 b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
static std::vector<std::uint8_t> bitmap(unsigned width=8,unsigned height=6) {
    const unsigned stride=(width+3)&~3u,offset=54+1024;
    std::vector<std::uint8_t> bytes(offset+stride*height);
    const auto word=[&](unsigned at,unsigned value,unsigned count=4) {
        for(unsigned n=0;n<count;++n) bytes[at+n]=std::uint8_t(value>>(n*8));
    };
    bytes[0]='B'; bytes[1]='M'; word(2,unsigned(bytes.size())); word(10,offset);
    word(14,40); word(18,width); word(22,height); word(26,1,2); word(28,8,2);
    word(34,stride*height);
    for(unsigned n=0;n<256;++n) for(unsigned k=0;k<3;++k) bytes[54+n*4+k]=std::uint8_t(n);
    for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x)
        bytes[offset+y*stride+x]=std::uint8_t(1+y*width+x);
    return bytes;
}
static SurfaceBackend::Palette grayscale() {
    SurfaceBackend::Palette p{};
    for(unsigned n=0;n<256;++n) p[n]={std::uint8_t(n),std::uint8_t(n),std::uint8_t(n)};
    return p;
}
static void copyControls(SurfaceBackend& b) {
    Image input=plane();
    for(unsigned n=0;n<48;++n) input.pixels[n]=0x1000+n;
    input.pixels[9]=0x2222;
    const auto source=b.create(input,rgb(16)),destination=b.create(plane(0x9999),rgb(16));
    b.setSourceKey(source,0x2222);
    for(unsigned kind=0;kind<5;++kind) for(bool fast:{false,true})
        for(bool noWait:{false,true}) for(bool primary:{false,true}) {
            b.update(destination,0,0,plane(0x9999));
            const bool explicitRect=kind==4,keyed=kind>=2;
            SurfaceRectangle32 dest=explicitRect ? SurfaceRectangle32{1,1,4,3}
                                                : SurfaceRectangle32{91,92,93,94};
            const auto initial=dest;
            const auto counters=b.stats();
            const auto out=copyOwnedSurfaceWrapper(b,SurfaceCopyWrapper(kind),source,destination,
                                                  {1,1,4,3},1,1,dest,{fast,noWait,primary,1,1});
            const auto after=b.stats();
            require(out.called && out.result.hresult==0,"Owned wrapper copy result");
            require(after.uploads==counters.uploads && after.nativeReadbacks==counters.nativeReadbacks,
                    "Copy used ordinary upload/readback");
            const int shift=!fast && primary ? 1 : 0;
            auto expected=plane(0x9999);
            for(int y=0;y<2;++y) for(int x=0;x<3;++x) {
                const auto value=input.pixels[std::size_t(y+1)*8+x+1];
                if(!keyed || value!=0x2222) expected.pixels[std::size_t(y+1+shift)*8+x+1+shift]=value;
            }
            require(b.read(destination).pixels==expected.pixels,"Copy destination pixels");
            require(equal(dest,explicitRect ? SurfaceRectangle32{1+shift,1+shift,4+shift,3+shift} : initial),
                    "Destination mutation boundary");
            ++copies;
        }
    SurfaceRectangle32 dest{0,0,3,2};
    const auto before=b.read(destination).pixels;
    for(auto kind:{SurfaceCopyWrapper::opaqueGuarded,SurfaceCopyWrapper::opaqueRecovering}) {
        require(!copyOwnedSurfaceWrapper(b,kind,std::nullopt,destination,{0,0,3,2},0,0,dest,{}).called,
                "Missing source did not bypass guarded wrapper");
        require(b.read(destination).pixels==before,"Guarded bypass changed pixels");
    }
    refuses([&]{copyOwnedSurfaceWrapper(b,SurfaceCopyWrapper::keyedRectangle,std::nullopt,destination,
                                        {0,0,3,2},0,0,dest,{});});
    refuses([&]{planSurfaceWrapperCopy(SurfaceCopyWrapper(99),{},0,0,dest,{});});
    const auto unchanged=dest;
    refuses([&]{planSurfaceWrapperCopy(SurfaceCopyWrapper::keyedExplicit,{},0,0,dest,{},false,true);});
    require(equal(dest,unchanged),"Rejected missing keyed input changed geometry");
    dest={0,0,5,4};
    refuses([&]{copyOwnedSurfaceWrapper(b,SurfaceCopyWrapper::keyedExplicit,source,destination,
                                        {0,0,3,2},0,0,dest,{false,false,true,1,1});});
    require(equal(dest,{1,1,6,5}),"Explicit geometry was not forwarded before backend refusal");
    require(b.read(destination).pixels==before,"Stretch refusal changed pixels");
    const auto alias=b.alias(source);
    dest={0,0,2,1};
    const auto copied=copyOwnedSurfaceWrapper(b,SurfaceCopyWrapper::opaqueGuarded,alias,source,
                                             {0,0,2,1},5,4,dest,{true,false,false,0,0});
    require(copied.called && !copied.result.hresult,"Canonical alias copy result");
    auto expected=input.pixels; expected[37]=input.pixels[0]; expected[38]=input.pixels[1];
    require(b.read(source).pixels==expected,"Alias source identity");
    require(b.release(alias)==1,"Alias reference retention");
    b.destroy(destination); b.destroy(source);
}
static void fillControls(SurfaceBackend& b) {
    const auto id=b.create(plane(7),rgb(16));
    std::vector<std::uint32_t> reports;
    OwnedFillRecovery options;
    options.bindings.report=[&](auto h){reports.push_back(h);};
    auto out=fillOwnedRectangle(b,id,Rect{1,1,4,3},0xfeed1234,options);
    require(out.returned && out.draws==1 && !out.budgetExhausted && reports.empty(),"Partial fill return");
    auto expected=plane(7);
    for(unsigned y=1;y<3;++y) for(unsigned x=1;x<4;++x) expected.pixels[y*8+x]=0x1234;
    require(b.read(id).pixels==expected.pixels,"Partial fill low-WORD pixels"); ++fills;
    b.markLost(id);
    options.keyEnabled=true; options.key=0x3456;
    out=fillOwnedRectangle(b,id,Rect{1,1,4,3},0xfeed1234,options);
    require(out.returned && out.draws==1 && !b.isLost(id),"Partial fill restore scheduling");
    require(b.sourceKey(id)==0x3456 && reports==std::vector<std::uint32_t>{0x887601c2},"Recovery key/report");
    refuses([&]{b.read(id);}); ++fills;
    reports.clear(); b.markLost(id);
    out=fillOwnedSurface(b,id,0xfeed1234,options);
    require(out.returned && out.draws==2 && !out.budgetExhausted,"Full fill restore/redraw");
    require(reports==std::vector<std::uint32_t>{0x887601c2,0,0},"Full fill report order");
    require(b.read(id).pixels==plane(0x1234).pixels,"Full fill defines restored pixels"); ++fills;
    b.markLost(id); reports.clear();
    out=fillOwnedSurface(b,id,0,options,1);
    require(out.budgetExhausted && !out.returned && out.draws==1,"Full fill budget manufactured return");
    refuses([&]{b.read(id);}); ++fills;
    const auto count=reports.size();
    refuses([&]{fillOwnedSurface(b,id,0,options,0);});
    require(reports.size()==count,"Invalid budget performed callbacks");
    fillOwnedSurface(b,id,0x1234);
    require(b.read(id).pixels==plane(0x1234).pixels,"Recovery after budget stop"); ++fills;
    b.destroy(id);
}
static void reloadControls(SurfaceBackend& b) {
    auto bytes=bitmap();
    refuses([&]{OwnedSurfaceBitmap bad(std::vector<std::uint8_t>{});});
    refuses([&]{loadOwnedCursorBitmap({});});
    require(!loadOwnedCursorBitmap([](auto){return std::optional<std::vector<std::uint8_t>>{};}),"Missing asset result");
    auto resource=loadOwnedCursorBitmap([&](std::string_view path){
        require(path=="bitmaps\\cursors.bmp","Fixed cursor asset path"); return std::optional(bytes);
    });
    require(bool(resource),"Cursor resource loaded");
    auto callback=resource->callback(); resource.reset(); bytes.clear();
    for(unsigned bits:{8u,16u,32u}) {
        const auto id=b.create(plane(),rgb(bits),0x840,bits==8 ? std::optional(grayscale()) : std::nullopt);
        callback(b,id);
        auto expected=plane();
        for(unsigned y=0;y<6;++y) for(unsigned x=0;x<8;++x) {
            const auto value=1+(5-y)*8+x;
            expected.pixels[y*8+x]=bits==8 ? value : bits==16
                ? ((value>>3)<<11)|((value>>2)<<5)|(value>>3) : value*0x010101;
        }
        require(b.read(id).pixels==expected.pixels,"Owned resource callback pixels");
        require(!b.borrowed(id) && b.mapBalance(id)==0,"Bitmap reload leaked DC");
        b.destroy(id); ++reloads;
    }
    const auto incompatible=b.create(plane(19),{16,{0x7c00,0x3e0,31}});
    refuses([&]{callback(b,incompatible);});
    require(!b.borrowed(incompatible) && b.mapBalance(incompatible)==0,"Rejected reload leaked DC");
    require(b.read(incompatible).pixels==plane(19).pixels,"Rejected reload mutated pixels");
    b.destroy(incompatible);
    OwnedSurfaceBitmap origin(bitmap());
    const OwnedSurfaceBitmap moved(std::move(origin));
    refuses([&]{origin.callback();});
    const auto movedTarget=b.create(plane(),rgb(32));
    refuses([&]{origin.reload(b,movedTarget);});
    moved.reload(b,movedTarget);
    require(b.read(movedTarget).pixels[0]==41*0x010101,"Moved resource lost owned input");
    b.destroy(movedTarget);
    const auto cursor=b.create(plane(),rgb(32)),trigger=b.create(plane(),rgb(32));
    b.markLost(trigger);
    OwnedFillRecovery options; options.route=SurfaceReloadRoute::CursorBitmap;
    options.bindings.cursor=callback; options.bindings.cursorSurface=cursor;
    const auto result=fillOwnedRectangle(b,trigger,Rect{0,0,2,2},0,options);
    require(result.returned && !b.isLost(trigger),"Cursor recovery scheduling");
    refuses([&]{b.read(trigger);});
    require(b.read(cursor).pixels[0]==41*0x010101,"Global cursor target differed from trigger");
    b.destroy(cursor); b.destroy(trigger); ++reloads;
    const OwnedSurfaceBitmap full(bitmap()),partial(bitmap(4,3));
    const auto p=b.createPalette(grayscale());
    const auto target=b.create(plane(99),rgb(8),0x840,grayscale());
    const auto peer=b.create(plane(77),rgb(8),0x840,grayscale());
    b.bindPaletteObject(target,p); b.bindPaletteObject(peer,p); b.markLost(target);
    b.updatePaletteObject(p,200,{{1,2,3}});
    refuses([&]{restoreOwnedIndexedBitmap(b,target,p+1,full);});
    require(b.isLost(target)!=0,"Wrong palette recovery changed loss state");
    require(!restoreOwnedIndexedBitmap(b,target,p,full),"Indexed explicit recovery result");
    auto expected=plane();
    for(unsigned y=0;y<6;++y) for(unsigned x=0;x<8;++x) expected.pixels[y*8+x]=1+(5-y)*8+x;
    require(b.read(target).pixels==expected.pixels,"Indexed restored bytes");
    require(b.paletteIdentity(target)==p && b.paletteIdentity(peer)==p,"Shared palette identity changed");
    std::uint32_t dc=0;
    require(!b.acquireDc(target,dc),"Inspect recovered palette");
    const auto color=b.dcPalette(target)[200];
    require(color.red==1 && color.green==2 && color.blue==3,"Palette update during loss was lost");
    require(!b.releaseDc(target,dc),"Palette inspection release"); ++reloads;
    b.markLost(target); require(!restoreOwnedIndexedBitmap(b,target,p,partial),"Partial recovery result");
    refuses([&]{b.read(target);});
    fillOwnedSurface(b,target,0); require(!b.borrowed(target),"Partial reload leaked lease"); ++reloads;
    const auto primary=b.create(plane(99),rgb(8),0x840,grayscale(),true);
    b.bindPaletteObject(primary,p); b.markLost(primary);
    require(restoreOwnedIndexedBitmap(b,primary,p,full,SurfaceBackend::Mode{9,6,rgb(8)})==0x8876024b,
            "Primary mode mismatch result");
    require(b.isLost(primary)!=0 && b.paletteIdentity(primary)==p,"Rejected primary mode mutated ownership");
    require(!restoreOwnedIndexedBitmap(b,primary,p,full,SurfaceBackend::Mode{8,6,rgb(8)}),
            "Primary matching-mode recovery result");
    require(b.read(primary).pixels==expected.pixels,"Primary recovered index pixels"); ++reloads;
    b.destroy(primary);
    b.destroy(target); b.destroy(peer); require(b.releasePalette(p)==0,"Palette reference leak");
    auto malformed=bitmap(); malformed[30]=1;
    refuses([&]{OwnedSurfaceBitmap invalid(malformed);});
}
static void barControls(SurfaceBackend& b) {
    for(unsigned format=0;format<2;++format) for(unsigned width:{2u,4u,8u,128u})
        for(unsigned height:{2u,5u,75u,128u}) for(unsigned fill:{0u,(height-2)/2,height-2}) {
            const auto expected=makeVerticalBarImage(width,height,fill,format);
            const auto id=createOwnedVerticalBar(b,width,height,fill,format);
            require(b.read(id).pixels==expected.pixels,"Owned bar upload");
            require(expected.pixels.front()==0 && expected.pixels.back()==0,"Bar border pixels");
            b.destroy(id); ++bars;
        }
    for(unsigned format=0;format<2;++format) {
        const auto image=makeVerticalBarImage(4,5,3,format);
        require(image.pixels[5]==(format ? 0x7fe0 : 0xffe0),"Bar gradient first row");
        require(image.pixels[9]==(format ? 0x7ea0 : 0xfd40),"Bar gradient middle row");
        require(image.pixels[13]==(format ? 0x7d40 : 0xfa80),"Bar gradient last row");
    }
    for(const auto args:std::array<std::array<unsigned,4>,6>{{{1,5,0,0},{4,1,0,0},
        {129,5,0,0},{4,129,0,0},{4,5,4,0},{4,5,0,2}}})
        refuses([&]{createOwnedVerticalBar(b,args[0],args[1],args[2],args[3]);});
}
int main(int argc,char** argv) {
    QGuiApplication app(argc,argv);
    try {
        SurfaceBackend backend;
        copyControls(backend); fillControls(backend); reloadControls(backend); barControls(backend);
        require(backend.stats().surfaces==0 && backend.paletteObjects()==0,"Owned resources leaked");
        std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"refusals\":"<<refusals
                 <<",\"copy_cases\":"<<copies<<",\"fill_cases\":"<<fills
                 <<",\"reload_cases\":"<<reloads<<",\"bar_cases\":"<<bars<<"}\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n'; return 1;}
}
