#include "blit.hpp"
#include <QGuiApplication>
#include <QColor>
#include <iostream>
#include <random>
#include <stdexcept>

using namespace mnm::render;
static void require(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
template<class F> static void rejected(F action){bool failed=false;try{action();}catch(const std::runtime_error&){failed=true;}require(failed,"Invalid surface command accepted");}
static void cpuCopy(const Image& src,Image& dst,Rect r,int x,int y,std::optional<std::uint32_t> key){
    for(int sy=r.top;sy<r.bottom;++sy)for(int sx=r.left;sx<r.right;++sx){
        const auto pixel=src.pixels[sy*src.width+sx];if(key && pixel==*key)continue;
        dst.pixels[(y+sy-r.top)*dst.width+x+sx-r.left]=pixel;
    }
}
static void checkPresentation(const QImage& actual,const Image& native,PixelFormat format,const std::vector<Rgb>& palette){
    require(actual.size()==QSize(native.width,native.height),"Presentation dimensions changed");
    for(int y=0;y<native.height;++y)for(int x=0;x<native.width;++x){
        const auto pixel=native.pixels[y*native.width+x];Rgb expected;
        if(format.bits==8)expected=palette[pixel];
        else {
            std::uint8_t values[3];
            for(unsigned i=0;i<3;++i){const auto mask=format.masks[i],low=mask&(~mask+1);values[i]=std::uint8_t(((pixel&mask)/low)*255/(mask/low));}
            if(format.bits==16 && format.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}){
                const auto r=(pixel>>11)&31,g=(pixel>>5)&63,b=pixel&31;
                values[0]=(r<<3)|(r>>2);values[1]=(g<<2)|(g>>4);values[2]=(b<<3)|(b>>2);
            }
            expected={values[0],values[1],values[2]};
        }
        require(actual.pixelColor(x,y)==QColor(expected.red,expected.green,expected.blue,255),"GPU palette/mask presentation differs from CPU");
    }
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try {
        GlBlitter renderer;std::mt19937 rng(0x50414c);unsigned operations=0;
        for(unsigned bits:{8u,16u,24u,32u}){
            PixelFormat f{bits,bits==8?std::array<std::uint32_t,3>{}:bits==16?
                std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};
            const auto mask=bits==32?UINT32_MAX:((std::uint32_t{1}<<bits)-1);
            Image a{5,4,std::vector<std::uint32_t>(20)},b{9,6,std::vector<std::uint32_t>(54)};
            for(auto& pixel:a.pixels)pixel=rng()&mask;
            for(auto& pixel:b.pixels)pixel=rng()&mask;
            const auto sa=renderer.create(a,f),sb=renderer.create(b,f);
            std::vector<Rgb> palette(256);for(auto& color:palette)color={std::uint8_t(rng()),std::uint8_t(rng()),std::uint8_t(rng())};
            if(bits==8){renderer.setPalette(sa,0,std::vector<Rgb>(256));renderer.setPalette(sb,0,palette);}
            for(unsigned step=0;step<48;++step){
                if(step%3==0){
                    Image patch{2,2,{std::uint32_t(rng()&mask),std::uint32_t(rng()&mask),std::uint32_t(rng()&mask),std::uint32_t(rng()&mask)}};
                    const int x=int(rng()%4),y=int(rng()%3);renderer.update(sa,x,y,patch);
                    cpuCopy(patch,a,{0,0,2,2},x,y,std::nullopt);
                }else {
                    const Rect r{1,1,5,4};const int x=int(rng()%6),y=int(rng()%4);
                    const std::optional<std::uint32_t> key=step%2?std::optional<std::uint32_t>(a.pixels[6]):std::nullopt;
                    const auto before=renderer.stats();renderer.copy(sa,sb,r,x,y,key);
                    const auto after=renderer.stats();
                    require(after.uploads==before.uploads && after.nativeReadbacks==before.nativeReadbacks,"Copy reuploaded or read back native surfaces");
                    cpuCopy(a,b,r,x,y,key);
                }
                require(renderer.read(sa).pixels==a.pixels && renderer.read(sb).pixels==b.pixels,"Ordered surface sequence differs from CPU");
                if(step%11==0)checkPresentation(renderer.present(sb),b,f,palette);
                ++operations;
            }
            if(bits==8){
                const auto oldPixels=renderer.read(sb).pixels;
                const auto before=renderer.stats();
                palette[0]={1,2,3};renderer.setPalette(sb,0,{palette[0]});
                require(renderer.stats().uploads==before.uploads,"Palette update uploaded native pixels");
                require(renderer.read(sb).pixels==oldPixels,"Palette update changed indices");checkPresentation(renderer.present(sb),b,f,palette);
            }
            rejected([&]{renderer.copy(sa,sb,{0,0,5,4},2147483647,0);});
            rejected([&]{renderer.copy(sa,sa,{0,0,1,1},0,0);});
            rejected([&]{renderer.update(sb,8,5,Image{2,2,{0,0,0,0}});});
            require(renderer.read(sb).pixels==b.pixels,"Rejected command changed destination");
            renderer.destroy(sa);require(renderer.read(sb).pixels==b.pixels,"Destroying source lost destination");
            rejected([&]{renderer.read(sa);});renderer.destroy(sb);
            const auto replacement=renderer.create(Image{1,1,{0}},f);require(replacement!=sa && replacement!=sb,"Destroyed surface ID reused");renderer.destroy(replacement);
        }
        // Duplicate palette colors must keep distinct source-key index behavior.
        const PixelFormat indexed{8,{}};Image src{3,1,{1,2,3}},dst{3,1,{4,4,4}};
        const auto s=renderer.create(src,indexed),d=renderer.create(dst,indexed);
        renderer.setPalette(s,1,{{255,0,0},{255,0,0},{0,255,0}});
        renderer.setPalette(d,1,{{0,0,255},{255,255,0},{0,255,0},{255,255,255}});
        renderer.copy(s,d,{0,0,3,1},0,0,1);
        require(renderer.read(d).pixels==std::vector<std::uint32_t>{4,2,3},"Duplicate RGB colors collapsed indices");
        auto image=renderer.present(d);require(image.pixelColor(0,0)==QColor(Qt::white) && image.pixelColor(1,0)==QColor(Qt::yellow),"Destination palette ignored");
        renderer.setPalette(d,2,{{12,34,56}});image=renderer.present(d);
        require(image.pixelColor(1,0)==QColor(12,34,56) && renderer.read(d).pixels[1]==2,"Palette cycling changed native content");
        const auto rgb=renderer.create(Image{1,1,{0}},PixelFormat{32,{0xff0000,0xff00,0xff}});
        rejected([&]{renderer.copy(s,rgb,{0,0,1,1},0,0);});rejected([&]{renderer.setPalette(rgb,0,{{1,2,3}});});
        rejected([&]{renderer.setPalette(d,255,{{1,2,3},{4,5,6}});});rejected([&]{renderer.copy(s,d,{0,0,1,1},0,0,256);});
        rejected([&]{renderer.create(Image{1,1,{0}},PixelFormat{16,{0xf801,0x7e0,0x1f}});});
        {GlBlitter other;rejected([&]{other.read(d);});}
        renderer.destroy(rgb);renderer.destroy(s);renderer.destroy(d);
        for(auto format:{PixelFormat{16,{0x7c00,0x3e0,0x1f}},PixelFormat{32,{0xff,0xff00,0xff0000}}}){
            Image pixels{2,1,{format.bits==16?0xffffu:0xabcdef01u,format.bits==16?0x8000u:0xff000000u}};
            const auto id=renderer.create(pixels,format);
            checkPresentation(renderer.present(id),pixels,format,{});require(renderer.read(id).pixels==pixels.pixels,"RGB presentation discarded unused native bits");
            renderer.destroy(id);
        }
        std::vector<SurfaceId> tiny;for(unsigned i=0;i<64;++i)tiny.push_back(renderer.create(Image{1,1,{0}},indexed));
        rejected([&]{renderer.create(Image{1,1,{0}},indexed);});for(auto id:tiny)renderer.destroy(id);
        require(renderer.stats().surfaces==0 && renderer.stats().pixels==0,"Surface resources leaked");
        std::cout<<operations<<" ordered updates/copies match; palette cycling, presentation and lifetimes pass\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
