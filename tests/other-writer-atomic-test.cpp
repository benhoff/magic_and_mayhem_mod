#include "../renderer/canvas_sequence.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
namespace {
unsigned refusals=0;
render::Image initial(int w,int h) {
  render::Image result{w,h,{}};
  for(int i=0;i<w*h;++i)result.pixels.push_back(std::uint16_t(113+i*1273));
  return result;
}
void setup(render::CanvasSequence &canvas,const render::Image &image,int missing=-1) {
  canvas.create(1,image.width,image.height);
  for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x)
    if(y*image.width+x!=missing)canvas.fill(1,{x,y,x+1,y+1},image.pixels[y*image.width+x]);
}
template<class F> void refuses(render::CanvasSequence &canvas,const render::Image &image,int missing,F operation) {
  bool refused=false;try{operation();}catch(const std::exception &){refused=true;}
  if(!refused)throw std::runtime_error("Unclosed writer admitted");
  if(missing>=0){const int x=missing%image.width,y=missing/image.width;canvas.fill(1,{x,y,x+1,y+1},image.pixels[missing]);}
  if(canvas.read(1).pixels!=image.pixels)throw std::runtime_error("Late writer refusal changed earlier pixels");
  ++refusals;
}
assets::SpriteFrame sprite(unsigned w,unsigned h,bool indexed=false) {
  assets::SpriteFrame f;f.width=w;f.height=h;f.opaqueMask.assign(w*h,1);
  if(indexed)f.pixels=std::vector<std::uint8_t>(w*h,1);
  else f.pixels=std::vector<std::uint16_t>(w*h,0x4210);
  return f;
}
}
int main() try {
  const std::array<std::uint16_t,256> palette=[] {std::array<std::uint16_t,256> p{};p.fill(0x4210);return p;}();
  for(bool indexed:{false,true})for(unsigned mode:{1u,2u,4u}){
    const auto image=initial(4,2);render::CanvasSequence canvas;setup(canvas,image,7);
    refuses(canvas,image,7,[&]{canvas.raster(1,sprite(4,2,indexed),0,0,{0,0,4,2},mode,1,palette);});
  }
  {
    const auto image=initial(4,2);render::CanvasSequence canvas;setup(canvas,image,7);
    refuses(canvas,image,7,[&]{canvas.addRgb(1,{0,0,4,2},{1,2,3});});
  }
  for(bool indexed:{false,true}){
    const auto image=initial(6,2);render::CanvasSequence canvas;setup(canvas,image,11);
    std::array<int,16> shifts;shifts.fill(2);
    refuses(canvas,image,11,[&]{canvas.raster(1,sprite(4,2,indexed),0,0,{0,0,6,2},3,1,palette,shifts,2);});
  }
  for(bool indexed:{false,true})for(bool invalidOffset:{false,true}){
    const auto image=initial(5,2);render::CanvasSequence canvas;setup(canvas,image);
    std::array<int,16> shifts;shifts.fill(1);shifts[1]=invalidOffset?-1:2;
    refuses(canvas,image,-1,[&]{canvas.raster(1,sprite(4,2,indexed),0,0,{0,0,5,2},3,1,palette,shifts,2);});
  }
  for(bool indexed:{false,true}){
    const auto image=initial(8,8);render::CanvasSequence canvas;setup(canvas,image,3*8+4);
    refuses(canvas,image,3*8+4,[&]{canvas.raster(1,sprite(4,4,indexed),0,0,{0,0,8,8},5,1,palette);});
    const auto small=initial(5,8);render::CanvasSequence bounded;setup(bounded,small);
    refuses(bounded,small,-1,[&]{bounded.raster(1,sprite(4,4,indexed),0,0,{0,0,8,8},5,1,palette);});
  }
  {
    const auto image=initial(16,16);render::CanvasSequence canvas;setup(canvas,image);
    std::vector<std::uint16_t> colours(16,0x4210);std::vector<unsigned char> hidden(16);hidden[15]=1;
    refuses(canvas,image,-1,[&]{canvas.terrainMap(1,8,3,4,4,2,2,colours,hidden);});
  }
  {
    const auto image=initial(16,16);render::CanvasSequence canvas;setup(canvas,image);
    canvas.create(2,8,6);canvas.fill(2,{0,0,8,6},123);
    std::vector<std::uint16_t> colours(32,0x4210);std::vector<unsigned char> hidden(32);hidden[28]=1;
    refuses(canvas,image,-1,[&]{canvas.terrainMap(1,8,0,4,8,2,4,colours,hidden,2);});
  }
  // Transparencies and clipped rows do not acquire unused background/sample
  // dependencies; a fully opaque source defines an initially undefined canvas.
  render::CanvasSequence opaque;opaque.create(1,4,2);opaque.raster(1,sprite(4,2),0,0,{0,0,4,2},0,1,palette);
  for(auto p:opaque.read(1).pixels)if(p!=0x4210)throw std::runtime_error("Opaque source initialization");
  const auto image=initial(5,2);render::CanvasSequence cropped;setup(cropped,image);
  std::array<int,16> shifts;shifts.fill(1);shifts[1]=-1;
  cropped.raster(1,sprite(4,2),0,0,{0,0,5,1},3,1,palette,shifts,2);
  auto expected=image;for(int i=0;i<4;++i)expected.pixels[i]=image.pixels[i+1];
  if(cropped.read(1).pixels!=expected.pixels)throw std::runtime_error("Clipped wave reads unused row");
  std::cout<<"{\"success\":true,\"atomic_refusals\":"<<refusals<<",\"positive_cases\":2}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
