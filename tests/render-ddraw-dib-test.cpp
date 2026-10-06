#include "blit.hpp"
#include "surface_copy.hpp"
#include "../reconstruction/rendering/surface_bitmap.hpp"
#include "../reconstruction/rendering/surface_retry.hpp"
#include <QGuiApplication>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace mnm::render;
using namespace mnm::reconstruction;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned refusals=0;
template<class Action>static void refused(Action&& action){bool caught=false;try{action();}catch(const std::runtime_error&){caught=true;}require(caught,"Expected explicit refusal");++refusals;}
static std::vector<std::uint8_t> bytes(const std::string& path){std::ifstream f(path,std::ios::binary);require(bool(f),"Missing asset input");return {std::istreambuf_iterator<char>(f),{}};}
struct Recovery:SurfaceRetryAdapter {
    GlBlitter& gl;std::array<SurfaceId,3> ids;std::vector<std::uint8_t> asset;
    unsigned draws=0,cursorReloads=0,callbacks=0;bool missing=false,failRestore=false;
    Recovery(GlBlitter& owner,std::array<SurfaceId,3> surfaces,std::vector<std::uint8_t> input):gl(owner),ids(surfaces),asset(std::move(input)){}
    std::uint32_t draw(bool,bool,std::uint32_t,std::uint16_t) override {return draws++?0:0x887601c2u;}
    std::uint32_t restore(RecoverySurface s) override {
        if(failRestore)return 0x80004005u;
        gl.invalidateContents(ids[unsigned(s)-1]);return 0;
    }
    std::uint32_t setSourceKey(RecoverySurface,std::uint16_t) override {return 0x80004005u;}
    void report(std::uint32_t) override {}
    void reload(RecoverySurface) override {++callbacks;}
    void reloadCursorBitmap() override {
        ++cursorReloads;
        auto reader=[&](std::string_view path)->std::optional<std::vector<std::uint8_t>>{require(path=="bitmaps\\cursors.bmp","Cursor path");return missing?std::nullopt:std::optional(asset);};
        auto upload=[&](const DibInput& dib){gl.reloadDib(ids[2],dib);return 0;};
        require(mnm::reconstruction::reloadCursorBitmap(reader,upload)==0,"Sentinel result");
    }
};
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
    require(argc==4,"Expected manifest, asset directory and pixel output");std::ifstream input(argv[1]);require(bool(input),"Missing case manifest");std::ofstream output(argv[3],std::ios::binary);require(bool(output),"Cannot write outputs");GlBlitter gl;unsigned cases=0;std::uint32_t id,w,h,bits;std::string name;
    const PixelFormat rgb{32,{0xff0000,0xff00,0xff}};
    while(input>>id>>name>>w>>h>>bits){
        require(bits==16 || bits==32,"Target format");const PixelFormat format{bits,bits==16?std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:rgb.masks};
        auto bitmap=parseSurfaceBitmap(bytes(std::string(argv[2])+"/"+name+".bmp"));require(bool(bitmap),"Expected valid original input");
        Image initial{int(w),int(h),std::vector<std::uint32_t>(std::size_t(w)*h,bits==16?0x2bab:0x556677)};
        const auto target=gl.create(initial,format);gl.reloadDib(target,*bitmap);const auto native=gl.read(target);const auto presented=gl.present(target);
        std::uint32_t head[4]={id,w,h,bits};output.write(reinterpret_cast<const char*>(head),sizeof(head));output.write(reinterpret_cast<const char*>(native.pixels.data()),std::streamsize(native.pixels.size()*4));
        for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){auto p=presented.pixel(int(x),int(y))&0xffffff;output.write(reinterpret_cast<const char*>(&p),4);}
        gl.invalidateContents(target);refused([&]{gl.read(target);});refused([&]{gl.present(target);});gl.reloadDib(target,*bitmap);
        const auto decoded=decodeDibRgb(*bitmap);const int cw=std::min(int(w),decoded.width),ch=std::min(int(h),decoded.height);
        const auto check=gl.create({cw,ch,std::vector<std::uint32_t>(std::size_t(cw)*ch)},format);gl.copy(target,check,{0,0,cw,ch},0,0);
        const auto recovered=gl.read(check);for(int y=0;y<ch;++y)for(int x=0;x<cw;++x)require(recovered.pixels[std::size_t(y)*cw+x]==native.pixels[std::size_t(y)*w+x],"Defined reload region differs");gl.destroy(check);
        if(cw<int(w)||ch<int(h)){refused([&]{gl.read(target);});refused([&]{gl.present(target);});}
        else require(gl.read(target).pixels==native.pixels,"Complete reload differs");
        gl.destroy(target);++cases;
    }
    require(input.eof()&&cases==72,"Case count");
    auto inputBytes=bytes(std::string(argv[2])+"/indexed8.bmp");const auto good=*parseSurfaceBitmap(inputBytes);
    auto target=gl.create({8,6,std::vector<std::uint32_t>(48,0x556677)},rgb);gl.invalidateContents(target);
    auto malformed=good;malformed.pixels.resize(1);refused([&]{gl.reloadDib(target,malformed);});refused([&]{gl.read(target);});
    for(unsigned at:{0u,12u,14u,16u,32u,36u}){auto unsupported=good;unsupported.header[at]^=1;refused([&]{gl.reloadDib(target,unsupported);});}
    auto usage=good;usage.usage=1;refused([&]{gl.reloadDib(target,usage);});
    auto palette=good;palette.palette.pop_back();refused([&]{gl.reloadDib(target,palette);});
    for(unsigned at:{4u,8u}){auto dimensions=good;dimensions.header[at+3]=0x80;refused([&]{gl.reloadDib(target,dimensions);});}
    gl.setClipper(target,{true,std::vector<Rect>{{0,0,8,6}}});gl.reloadDib(target,good);refused([&]{gl.reloadDib(target,good,std::vector<Rect>{{-1,0,8,6}});});gl.destroy(target);
    for(const auto format:std::array<PixelFormat,3>{{{8,{}},{16,{0x7c00,0x3e0,0x1f}},{32,{0xff,0xff00,0xff0000}}}}){auto incompatible=gl.create({8,6,std::vector<std::uint32_t>(48)},format);refused([&]{gl.reloadDib(incompatible,good);});gl.destroy(incompatible);}
    refused([&]{gl.reloadDib(target,good);});
    unsigned compositions=0;
    for(unsigned depth:{16u,32u})for(unsigned wrapper=0;wrapper<4;++wrapper)for(unsigned mode=0;mode<3;++mode){
        const PixelFormat recoveryFormat{depth,depth==16?std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:rgb.masks};
        std::array<SurfaceId,3> surfaces;for(auto& surface:surfaces)surface=gl.create({8,6,std::vector<std::uint32_t>(48,depth==16?0x2bab:0x556677)},recoveryFormat);gl.invalidateContents(surfaces[2]);
        Recovery a(gl,surfaces,inputBytes);a.missing=mode==1;a.failRestore=mode==2;
        SurfaceRetryInput config;config.wrapper=SurfaceWrapper(wrapper);config.sourceRoute=config.destinationRoute=SurfaceReloadRoute::CursorBitmap;config.keyEnabled=true;
        const auto result=runSurfaceRetry(config,a,3);require(result.returned&&!result.budgetExhausted,"Recovery result");
        const unsigned reloads=mode==2?0:wrapper<2?1:2;require(a.cursorReloads==reloads,"Global cursor dispatch count");
        if(mode==0){auto expected=decodeDibRgb(good);if(depth==16)for(auto& p:expected.pixels)p=((p>>19)&31)<<11|((p>>10)&63)<<5|((p>>3)&31);require(gl.read(surfaces[2]).pixels==expected.pixels,"Global cursor pixels");}else refused([&]{gl.read(surfaces[2]);});
        if(mode!=2){refused([&]{gl.read(surfaces[1]);});if(wrapper>=2)refused([&]{gl.read(surfaces[0]);});}
        for(auto surface:surfaces)gl.destroy(surface);
        ++compositions;
    }
    {Recovery a(gl,{0,0,0},inputBytes);SurfaceRetryInput c;c.sourceReload=true;c.sourceRoute=SurfaceReloadRoute::CursorBitmap;refused([&]{runSurfaceRetry(c,a,1);});require(a.draws==0,"Ambiguous route performed draw");c.sourceReload=false;c.sourceRoute=SurfaceReloadRoute(99);refused([&]{runSurfaceRetry(c,a,1);});require(a.draws==0,"Invalid route performed draw");}
    struct Unbound:SurfaceRetryAdapter{std::uint32_t draw(bool,bool,std::uint32_t,std::uint16_t) override{return 0x887601c2u;}std::uint32_t restore(RecoverySurface)override{return 0;}std::uint32_t setSourceKey(RecoverySurface,std::uint16_t)override{return 0;}void reload(RecoverySurface)override{}void report(std::uint32_t)override{}} unbound;
    SurfaceRetryInput c;c.sourceRoute=SurfaceReloadRoute::CursorBitmap;refused([&]{runSurfaceRetry(c,unbound,1);});
    auto underflow=inputBytes;for(unsigned i=0;i<4;++i)underflow[2+i]=0;refused([&]{parseSurfaceBitmap(underflow);});
    require(gl.stats().surfaces==0,"Leaked native surface");output.close();require(bool(output),"Pixel output failure");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"compositions\":"<<compositions<<",\"refusals\":"<<refusals<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
