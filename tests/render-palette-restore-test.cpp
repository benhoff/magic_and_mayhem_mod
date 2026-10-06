#include "blit.hpp"
#include <QGuiApplication>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<Rgb> colors(std::istream& input){std::vector<Rgb> out;for(unsigned i=0;i<256;++i){std::uint32_t v=0;require(bool(input>>v)&&v<=0xffffff,"Palette input");out.push_back({std::uint8_t(v),std::uint8_t(v>>8),std::uint8_t(v>>16)});}return out;}
static Image indices(std::istream& input){Image out{8,6,{}};for(unsigned i=0;i<48;++i){std::uint32_t v=0;require(bool(input>>v)&&v<256,"Index input");out.pixels.push_back(v);}return out;}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
 require(argc==2,"Expected captured input file");std::ifstream input(argv[1]);require(bool(input),"Open inputs");GlBlitter gl;unsigned cases=0,refusals=0,pixelChecks=0,indexChecks=0;
 unsigned id,phase;
 while(input>>id>>phase){require(id<4&&(phase==6||phase==8||phase==9),"Case input");auto initialColors=colors(input),reloadColors=colors(input),targetColors=colors(input);auto initial=indices(input),reload=indices(input),target=indices(input);
  auto surface=gl.create(initial,{8,{}});gl.setPalette(surface,0,initialColors);gl.invalidateContents(surface);
  auto refused=[&](auto action){bool caught=false;try{action();}catch(const std::runtime_error&){caught=true;}require(caught,"Unknown indexed contents admitted");++refusals;};
  refused([&]{gl.read(surface);});refused([&]{gl.present(surface);});
  // Applying verified palette state never defines lost index contents.
  gl.setPalette(surface,0,reloadColors);refused([&]{gl.read(surface);});refused([&]{gl.present(surface);});
  gl.update(surface,0,0,{8,1,{reload.pixels.begin(),reload.pixels.begin()+8}});refused([&]{gl.read(surface);});
  gl.update(surface,0,0,reload);require(gl.read(surface).pixels==reload.pixels,"Reload index mismatch");++indexChecks;
  auto checkRgb=[&](const Image& expected,const std::vector<Rgb>& palette){auto image=gl.present(surface);require(image.width()==8&&image.height()==6,"Presentation size");for(unsigned i=0;i<48;++i){auto rgb=palette[expected.pixels[i]];require(image.pixel(int(i%8),int(i/8))==qRgb(rgb.red,rgb.green,rgb.blue),"Indexed RGB mismatch");++pixelChecks;}};
  checkRgb(reload,reloadColors);gl.setPalette(surface,0,targetColors);require(gl.read(surface).pixels==target.pixels,"Palette update changed indices");++indexChecks;checkRgb(target,targetColors);gl.destroy(surface);++cases;
 }
 require(input.eof()&&cases==9&&gl.stats().surfaces==0,"Input count/cleanup");std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"unknown_refusals\":"<<refusals<<",\"index_checks\":"<<indexChecks<<",\"rgb_pixel_checks\":"<<pixelChecks<<"}\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
