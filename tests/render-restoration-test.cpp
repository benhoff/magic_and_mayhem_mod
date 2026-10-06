#include "blit.hpp"
#include "surface_copy.hpp"
#include "../reconstruction/rendering/surface_retry.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
// Test-only adapter composes recovered callbacks with native validity policy.
// Reload bytes are explicitly supplied fixture inputs, not invented Restore output.
struct RecoveryAdapter:mnm::reconstruction::SurfaceRetryAdapter {
    GlBlitter& gl;SurfaceId id;Image reloadImage;bool first=true;
    RecoveryAdapter(GlBlitter& g,SurfaceId s,Image image):gl(g),id(s),reloadImage(std::move(image)){}
    std::uint32_t draw(bool,bool fill,std::uint32_t,std::uint16_t color) override {
        if(first){first=false;return 0x887601c2u;}
        if(!fill)throw std::runtime_error("Only selected fill adapter tested");
        gl.update(id,0,0,{reloadImage.width,reloadImage.height,std::vector<std::uint32_t>(reloadImage.pixels.size(),color)});return 0;
    }
    std::uint32_t restore(mnm::reconstruction::RecoverySurface) override {gl.invalidateContents(id);return 0;}
    std::uint32_t setSourceKey(mnm::reconstruction::RecoverySurface,std::uint16_t) override {return 0x80004005u;}
    void reload(mnm::reconstruction::RecoverySurface) override {gl.update(id,0,0,reloadImage);}
    void report(std::uint32_t) override {}
};
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try {
        GlBlitter gl;unsigned checks=0;
        const auto check=[&](bool ok){if(!ok)throw std::runtime_error("Restoration validity mismatch");++checks;};
        const auto rejected=[&](auto action){bool failed=false;try{action();}catch(const std::runtime_error&){failed=true;}check(failed);};
        const PixelFormat format{16,{0xf800,0x7e0,31}};
        Image original{4,3,{1,2,3,4,5,6,7,8,9,10,11,12}};
        const auto source=gl.create(original,format),dest=gl.create({4,3,std::vector<std::uint32_t>(12,100)},format);
        const auto before=gl.stats();gl.invalidateContents(dest);const auto after=gl.stats();
        check(before.uploads==after.uploads && before.nativeReadbacks==after.nativeReadbacks);
        rejected([&]{gl.read(dest);});rejected([&]{gl.present(dest);});rejected([&]{gl.presentGpu(dest);});
        rejected([&]{gl.copy(dest,source,{0,0,4,3},0,0);});
        gl.update(dest,0,0,{2,1,{21,22}});rejected([&]{gl.read(dest);});
        gl.copy(dest,source,{0,0,2,1},0,0);original.pixels[0]=21;original.pixels[1]=22;
        check(gl.read(source).pixels==original.pixels);
        rejected([&]{gl.copy(dest,source,{2,0,4,1},0,0);});
        rejected([&]{gl.copy(source,dest,{0,0,4,3},0,0,1);});
        const auto mask=gl.create({4,3,std::vector<std::uint32_t>(12,1)},{8,{}});gl.invalidateContents(mask);
        rejected([&]{gl.copy(source,dest,{0,0,4,3},0,0,std::nullopt,mask);});
        gl.copy(source,dest,{0,0,4,3},0,0);check(gl.read(dest).pixels==original.pixels);
        rejected([&]{gl.copy(source,dest,{0,0,4,3},0,0,std::nullopt,mask);});gl.destroy(mask);
        gl.invalidateContents(source);gl.swapContents(source,dest);
        check(gl.read(source).pixels==original.pixels);rejected([&]{gl.read(dest);});
        gl.update(dest,0,0,{4,1,{31,32,33,34}});rejected([&]{gl.read(dest);});
        gl.update(dest,0,1,{4,2,{41,42,43,44,51,52,53,54}});
        check(gl.read(dest).pixels==std::vector<std::uint32_t>({31,32,33,34,41,42,43,44,51,52,53,54}));
        gl.setClipper(dest,{true,std::vector<Rect>{{1,1,3,2}}});gl.invalidateContents(dest);
        SurfaceCopyRequest request;request.source=request.destination={0,0,4,3};
        check(gl.surfaceCopy(source,dest,request).pieces==1);rejected([&]{gl.read(dest);});
        gl.copy(dest,source,{1,1,3,2},1,1);check(gl.read(source).pixels==original.pixels);
        request.source=request.destination={0,0,1,1};gl.setClipper(dest,{});
        rejected([&]{gl.surfaceCopy(dest,dest,request);});
        gl.update(dest,0,0,original);check(gl.read(dest).pixels==original.pixels);
        using namespace mnm::reconstruction;
        SurfaceRetryInput retry;retry.wrapper=SurfaceWrapper::PartialFill;retry.keyEnabled=true;retry.color=0x12345678;
        {RecoveryAdapter adapter(gl,dest,original);const auto result=runSurfaceRetry(retry,adapter,16);check(result.returned && result.draws==1);rejected([&]{gl.read(dest);});}
        retry.wrapper=SurfaceWrapper::FullFill;
        {RecoveryAdapter adapter(gl,dest,original);const auto result=runSurfaceRetry(retry,adapter,16);check(result.returned && result.draws==2);check(gl.read(dest).pixels==std::vector<std::uint32_t>(12,0x5678));}
        retry.wrapper=SurfaceWrapper::PartialFill;retry.destinationReload=true;
        {RecoveryAdapter adapter(gl,dest,original);runSurfaceRetry(retry,adapter,16);check(gl.read(dest).pixels==original.pixels);}
        retry.wrapper=SurfaceWrapper::FullFill;retry.destinationReload=false;
        {RecoveryAdapter adapter(gl,dest,original);const auto result=runSurfaceRetry(retry,adapter,1);check(result.budgetExhausted && !result.returned);rejected([&]{gl.read(dest);});}
        gl.destroy(source);gl.destroy(dest);check(gl.stats().surfaces==0);
        rejected([&]{gl.invalidateContents(dest);});
        std::cout<<"{\"success\":true,\"checks\":"<<checks<<"}\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
