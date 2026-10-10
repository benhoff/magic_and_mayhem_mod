#include "../compat/legacy/font_layout.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
using Bytes=std::vector<std::uint8_t>;
std::uint32_t word(const Bytes &b,std::size_t at) {
  if(at>b.size() || b.size()-at<4)throw std::runtime_error("Layout extent");
  return b[at]|std::uint32_t(b[at+1])<<8|std::uint32_t(b[at+2])<<16|std::uint32_t(b[at+3])<<24;
}
int main(int argc,char **argv) try {
  if(argc!=3)throw std::runtime_error("Expected corpus, output");
  std::ifstream stream(argv[1],std::ios::binary);const Bytes b{std::istreambuf_iterator<char>(stream),{}};
  if(b.size()<16 || std::memcmp(b.data(),"MNMLAY02",8))throw std::runtime_error("Layout corpus header");
  std::ofstream out(argv[2],std::ios::binary);
  auto put=[&](std::uint32_t v){for(unsigned k=0;k<4;++k)out.put(char(v>>(k*8)));};
  std::size_t at=16;
  std::vector<std::unique_ptr<mnm::compat::FontLayout>> fonts;std::vector<unsigned> rowCounts;
  for(unsigned f=0;f<word(b,12);++f){
    const auto bytes=word(b,at);at+=4;Bytes raw(b.begin()+at,b.begin()+at+bytes);at+=bytes;
    auto decoded=mnm::assets::decodeSft(raw);
    if(auto *e=std::get_if<mnm::assets::SftError>(&decoded))throw std::runtime_error(e->detail);
    rowCounts.push_back(word(raw,16));
    fonts.push_back(std::make_unique<mnm::compat::FontLayout>(std::get<mnm::assets::SftFont>(std::move(decoded))));
  }
  for(unsigned c=0;c<word(b,8);++c) {
    const auto size=word(b,at),fontId=word(b,at+4),textSize=word(b,at+8);
    const auto &font=*fonts.at(fontId);
    mnm::reconstruction::rendering::FontLayoutInput in;
    in.kind=static_cast<mnm::reconstruction::rendering::FontLayoutKind>(word(b,at+12));
    in.punctuationMode=std::int32_t(word(b,at+16));in.cursor.tracking=std::int32_t(word(b,at+20));
    in.cursor.tabAdvance=std::int32_t(word(b,at+24));in.cursor.x=std::int32_t(word(b,at+28));
    in.cursor.y=std::int32_t(word(b,at+32));in.lineHeight=std::int32_t(word(b,at+36));
    in.firstX=std::int32_t(word(b,at+40));in.flags=word(b,at+44);
    in.bounds={std::int32_t(word(b,at+48)),std::int32_t(word(b,at+52)),std::int32_t(word(b,at+56)),std::int32_t(word(b,at+60))};
    const auto rows=rowCounts.at(fontId),w=word(b,at+64),h=word(b,at+68);
    for(unsigned r=0;r<rows;++r)in.cursor.trailing.push_back(std::int32_t(word(b,at+72+r*4)));
    const auto textAt=at+72+rows*4;
    in.text=Bytes(b.begin()+textAt,b.begin()+textAt+textSize);
    mnm::render::CanvasSequence canvas;canvas.create(1,w,h);
    mnm::render::Image image{int(w),int(h),{}};
    for(unsigned i=0;i<w*h;++i)image.pixels.push_back(std::uint16_t(i*1273+113));
    canvas.update(1,0,0,image);
    std::array<float,64> table{};for(unsigned k=0;k<64;++k)table[k]=k/64.f;
    mnm::reconstruction::rendering::FontLayoutResult result;
    try{result=font.draw(canvas,1,in,{0,0,int(w),int(h)},{173,91,237},table);}
    catch(const std::exception &e){throw std::runtime_error("Case "+std::to_string(c)+": "+e.what());}
    put(result.width);put(result.cursor.x);put(result.cursor.y);put(result.consumed.value_or(0xffffffffu));
    for(auto r:result.cursor.trailing)put(r);
    put(result.lines.size());for(auto l:result.lines){put(l.x);put(l.count);put(l.source);}
    for(auto p:canvas.read(1).pixels){out.put(char(p));out.put(char(p>>8));}
    at+=size;
  }
  if(at!=b.size() || !out)throw std::runtime_error("Layout output/input extent");
  std::cout<<"Native line state and owned text canvases pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
