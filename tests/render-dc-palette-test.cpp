#include "blit.hpp"
#include "surface_copy.hpp"
#include "../reconstruction/rendering/surface_bitmap.hpp"
#include <QGuiApplication>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned refusals=0,knownChecks=0;
template<class Action>static void refused(Action&& action){bool caught=false;try{action();}catch(const std::runtime_error&){caught=true;}require(caught,"Expected refusal");++refusals;}
static std::vector<std::uint8_t> bytes(const std::string& name){std::ifstream f(name,std::ios::binary);require(bool(f),"Missing bitmap input");return {std::istreambuf_iterator<char>(f),{}};}
static std::vector<Rgb> colors(unsigned style,const DibInput& dib){
    std::vector<Rgb> out;for(unsigned i=0;i<256;++i){unsigned r=0,g=0,b=0;
        if(style==1)r=g=b=i;else if(style==2)r=g=b=255-i;
        else if(style==3){r=((i>>5)&7)*255/7;g=((i>>2)&7)*255/7;b=(i&3)*85;if(i==255)r=g=b=0;}
        else{require(style==4&&dib.palette.size()==1024,"Exact source palette");r=dib.palette[i*4+2];g=dib.palette[i*4+1];b=dib.palette[i*4];}
        out.push_back({std::uint8_t(r),std::uint8_t(g),std::uint8_t(b)});
    }return out;
}
static std::optional<std::vector<Rect>> regions(unsigned code){
    if(code==0)return std::nullopt;
    if(code==1)return std::vector<Rect>{{2,1,6,5}};
    if(code==2)return std::vector<Rect>{{0,0,3,2},{5,3,8,6}};
    if(code==3)return std::vector<Rect>{};
    require(code==4,"Region code");return std::vector<Rect>{{2,1,6,5},{4,0,8,3}};
}
static ClipperState clipper(unsigned code){
    if(code==0)return {};
    if(code==3)return {true,std::nullopt};
    if(code==4)return {true,std::vector<Rect>{}};
    return {true,regions(code)};
}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
    require(argc==4,"Expected manifest, assets and pixel output");std::ifstream input(argv[1]);std::ofstream output(argv[3],std::ios::binary);require(bool(input)&&bool(output),"Files unavailable");GlBlitter gl;
    unsigned id,bits,style,ddclip,gdiclip,cases=0;std::string asset;
    while(input>>id>>asset>>bits>>style>>ddclip>>gdiclip){
        auto parsed=mnm::reconstruction::parseSurfaceBitmap(bytes(std::string(argv[2])+"/"+asset+".bmp"));require(bool(parsed),"Invalid bitmap");const auto& dib=*parsed;
        PixelFormat format{bits,bits==8?std::array<std::uint32_t,3>{}:bits==16?std::array<std::uint32_t,3>{0xf800,0x7e0,0x1f}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};
        const auto target=gl.create({8,6,std::vector<std::uint32_t>(48,bits==8?19:bits==16?0x2bab:0x556677)},format);
        if(bits==8){require(style!=0,"Absent palette excluded from native conversion");auto palette=colors(style,dib);
            gl.setPalette(target,0,{palette.begin(),palette.begin()+128});const auto before=gl.read(target);refused([&]{gl.reloadDib(target,dib);});require(gl.read(target).pixels==before.pixels,"Partial palette refusal wrote pixels");
            gl.setPalette(target,128,{palette.begin()+128,palette.end()});
        }
        gl.setClipper(target,clipper(ddclip));auto dc=regions(gdiclip);gl.reloadDib(target,dib,dc);const auto native=gl.read(target);const auto rgb=gl.present(target);
        std::uint32_t head[2]={id,bits};output.write(reinterpret_cast<const char*>(head),sizeof(head));output.write(reinterpret_cast<const char*>(native.pixels.data()),192);
        for(int y=0;y<6;++y)for(int x=0;x<8;++x){auto value=rgb.pixel(x,y)&0xffffff;output.write(reinterpret_cast<const char*>(&value),4);}
        gl.invalidateContents(target);const auto uploads=gl.stats().uploads;
        refused([&]{gl.reloadDib(target,dib,std::vector<Rect>{{0,0,2,2},{-1,0,1,1}});});require(gl.stats().uploads==uploads,"Malformed later DC region partially wrote");
        refused([&]{gl.reloadDib(target,dib,std::vector<Rect>(33,{0,0,1,1}));});
        gl.reloadDib(target,dib,dc);const auto decoded=decodeDibRgb(dib);bool full=true;
        const auto sample=gl.create({1,1,{0}},format);
        for(int y=0;y<6;++y)for(int x=0;x<8;++x){bool yes=x<decoded.width&&y<decoded.height;
            if(dc){bool included=false;for(auto r:*dc)included=included||(r.left<=x&&x<r.right&&r.top<=y&&y<r.bottom);yes=yes&&included;}
            if(yes){gl.copy(target,sample,{x,y,x+1,y+1},0,0);require(gl.read(sample).pixels[0]==native.pixels[std::size_t(y)*8+x],"Known region native differs");++knownChecks;}
            else{full=false;refused([&]{gl.copy(target,sample,{x,y,x+1,y+1},0,0);});}
        }
        if(full)require(gl.read(target).pixels==native.pixels,"Full reload differs");else{refused([&]{gl.read(target);});refused([&]{gl.present(target);});refused([&]{gl.presentGpu(target);});}
        gl.destroy(sample);gl.destroy(target);++cases;
    }
    require(input.eof()&&cases>0,"Manifest failure");
    auto black=*mnm::reconstruction::parseSurfaceBitmap(bytes(std::string(argv[2])+"/indexed8.bmp"));
    std::fill(black.palette.begin(),black.palette.end(),0);std::fill(black.pixels.begin(),black.pixels.end(),0);
    const auto a=gl.create({8,6,std::vector<std::uint32_t>(48)},PixelFormat{8,{}});
    const auto b=gl.create({8,6,std::vector<std::uint32_t>(48)},PixelFormat{8,{}});
    gl.setPalette(a,0,colors(1,black));gl.setPalette(b,0,colors(2,black));
    gl.reloadDib(a,black);require(gl.read(a).pixels==std::vector<std::uint32_t>(48,0),"Initial nearest black");
    gl.setPalette(a,0,{{255,255,255}});gl.reloadDib(a,black);
    require(gl.read(a).pixels==std::vector<std::uint32_t>(48,1),"Partial palette update ignored by reload");
    require((gl.present(a).pixel(0,0)&0xffffff)==0x010101,"Partial palette presentation differs");
    gl.reloadDib(b,black);require(gl.read(b).pixels==std::vector<std::uint32_t>(48,255),"Reverse palette nearest black");
    gl.swapContents(a,b);gl.reloadDib(a,black);gl.reloadDib(b,black);
    require(gl.read(a).pixels==std::vector<std::uint32_t>(48,1)&&gl.read(b).pixels==std::vector<std::uint32_t>(48,255),"Storage swap moved palette translation state");
    gl.destroy(a);gl.destroy(b);
require(gl.stats().surfaces==0,"Surface leak");output.close();require(bool(output),"Output failure");
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"refusals\":"<<refusals<<",\"known_pixel_checks\":"<<knownChecks<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
