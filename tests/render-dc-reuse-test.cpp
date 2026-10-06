#include "bitmap_dc.hpp"
#include "../reconstruction/rendering/surface_bitmap.hpp"
#include <QGuiApplication>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned refusals=0;
template<class Action>static void refused(Action&& action){bool caught=false;try{action();}catch(const std::runtime_error&){caught=true;}require(caught,"Expected explicit refusal");++refusals;}
static std::vector<std::uint8_t> bytes(const std::string& name){std::ifstream f(name,std::ios::binary);require(bool(f),"Missing bitmap");return {std::istreambuf_iterator<char>(f),{}};}
static BitmapDcState::Palette palette(unsigned style){BitmapDcState::Palette out{};for(unsigned i=0;i<256;++i){unsigned r=i,g=i,b=i;if(style==2)r=g=b=255-i;if(style==3){r=((i>>5)&7)*255/7;g=((i>>2)&7)*255/7;b=(i&3)*85;if(i==255)r=g=b=0;}out[i]={std::uint8_t(r),std::uint8_t(g),std::uint8_t(b)};}return out;}
static std::vector<Rgb> change(){std::vector<Rgb> out;for(unsigned i=64;i<96;++i)out.push_back({std::uint8_t(255-i),std::uint8_t(i*5),std::uint8_t(i*7)});return out;}
static void table(std::ofstream& out,const BitmapDcState& state,unsigned bits){
    BitmapDcState::Palette colors{};if(bits==8)colors=state.palette();
    for(auto c:colors){std::uint32_t rgb=(std::uint32_t(c.red)<<16)|(std::uint32_t(c.green)<<8)|c.blue;out.write(reinterpret_cast<const char*>(&rgb),4);}
}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
    require(argc==4,"Expected manifest/assets/output");std::ifstream in(argv[1]);std::ofstream out(argv[3],std::ios::binary);require(bool(in)&&bool(out),"Files missing");BitmapDcState::Palette defaults{};unsigned value;
    for(auto& c:defaults){require(bool(in>>value)&&value<=0xffffff,"Default context input");c={std::uint8_t(value>>16),std::uint8_t(value>>8),std::uint8_t(value)};}
    GlBlitter gl;unsigned id,bits,initial,action,clip,cases=0,phases=0;std::string asset;
    while(in>>id>>asset>>bits>>initial>>action>>clip){
        auto parsed=mnm::reconstruction::parseSurfaceBitmap(bytes(std::string(argv[2])+"/"+asset+".bmp"));require(bool(parsed),"Invalid asset");const auto& dib=*parsed;
        BitmapDcState state(8,6,bits,bits==8?std::optional(defaults):std::nullopt);if(initial)state.bindPalette(palette(1));
        PixelFormat format{bits,bits==8?std::array<std::uint32_t,3>{}:bits==16?std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};
        const auto target=gl.create({8,6,std::vector<std::uint32_t>(48,bits==8?19:bits==16?0x2bab:0x556677)},format);
        refused([&]{state.release();});refused([&]{state.reload(gl,target,dib);});refused([&]{state.regions();});
        for(unsigned phase=0;phase<3;++phase){
            if(phase==1){
                if(action==1&&initial){const auto reversed=palette(2);state.updateBoundPalette(64,{reversed.begin()+64,reversed.begin()+96});}
                if(action==2)state.bindPalette(palette(3));
                if(action==3)state.bindPalette(std::nullopt);
            }
            state.acquire();refused([&]{state.acquire();});require(!state.regions(),"Clip persisted across acquire");
            std::uint32_t head[3]={id,phase,bits};out.write(reinterpret_cast<const char*>(head),sizeof(head));table(out,state,bits);
            if(phase==0){
                if(action==4)state.setDcColors(64,change());
                if(action==5)state.bindPalette(palette(3));
                if(action==6&&initial){const auto reversed=palette(2);state.updateBoundPalette(64,{reversed.begin()+64,reversed.begin()+96});}
                if(clip)state.selectRegions(std::vector<Rect>{{2,1,6,5}});
            }
            if(phase==2)state.selectRegions(std::nullopt);
            table(out,state,bits);state.reload(gl,target,dib);const auto native=gl.read(target);const auto rgb=gl.present(target);out.write(reinterpret_cast<const char*>(native.pixels.data()),192);
            for(int y=0;y<6;++y)for(int x=0;x<8;++x){auto color=rgb.pixel(x,y)&0xffffff;out.write(reinterpret_cast<const char*>(&color),4);}
            state.release();++phases;
        }
        const auto known=gl.read(target);gl.invalidateContents(target);state.acquire();state.selectRegions(std::vector<Rect>{});const auto uploads=gl.stats().uploads;state.reload(gl,target,dib);require(gl.stats().uploads==uploads,"Empty region wrote pixels");refused([&]{gl.read(target);});refused([&]{gl.presentGpu(target);});
        refused([&]{state.selectRegions(std::vector<Rect>{{0,0,2,2},{-1,0,1,1}});});require(state.regions()&&state.regions()->empty(),"Rejected clip mutated state");refused([&]{state.selectRegions(std::vector<Rect>(33,{0,0,1,1}));});
        state.release();state.acquire();state.reload(gl,target,dib);require(gl.read(target).pixels==known.pixels,"Fresh acquire did not restore unmasked reload");state.release();gl.destroy(target);++cases;
    }
    require(in.eof()&&cases==192&&phases==576,"Case/phase count");
    BitmapDcState missing(8,6,8);refused([&]{missing.acquire();});missing.bindPalette(palette(1));missing.acquire();refused([&]{missing.setDcColors(255,{{0,0,0},{1,1,1}});});missing.release();missing.bindPalette(std::nullopt);refused([&]{missing.acquire();});refused([&]{missing.updateBoundPalette(0,{{0,0,0}});});
    BitmapDcState rgb(8,6,32);refused([&]{rgb.bindPalette(palette(1));});rgb.acquire();refused([&]{rgb.palette();});refused([&]{rgb.setDcColors(0,{{0,0,0}});});rgb.release();refused([&]{BitmapDcState invalid(0,6,8,defaults);});
    require(gl.stats().surfaces==0,"Surface leak");out.close();require(bool(out),"Output failure");std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"phases\":"<<phases<<",\"refusals\":"<<refusals<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
