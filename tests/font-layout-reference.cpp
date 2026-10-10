// Unchanged pinned original higher text entry calls in a private mapping.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
int main(int argc,char **argv) try {
  if(argc!=4)throw std::runtime_error("Expected PE, corpus, output");
  map_image(read(argv[1]));
  if(std::memcmp(reinterpret_cast<void *>(0x4a5ce0),"\x81\xec\x20\x08\x00\x00",6) ||
     std::memcmp(reinterpret_cast<void *>(0x4a6190),"\x81\xec\xac\x01\x00\x00",6) ||
     std::memcmp(reinterpret_cast<void *>(0x4a6630),"\x81\xec\xac\x01\x00\x00",6))
    throw std::runtime_error("Original higher text entry signature");
  const auto input=read(argv[2]);
  if(input.size()<16 || std::memcmp(input.data(),"MNMLAY02",8))throw std::runtime_error("Layout corpus header");
  std::ofstream output(argv[3],std::ios::binary);
  auto put=[&](std::uint32_t v){output.write(reinterpret_cast<const char *>(&v),4);};
  std::size_t at=16;std::vector<Bytes> fonts;
  for(unsigned f=0;f<u32(input,12);++f){const auto bytes=u32(input,at);at+=4;fonts.emplace_back(input.begin()+at,input.begin()+at+bytes);at+=bytes;}
  for(unsigned c=0;c<u32(input,8);++c) {
    const auto size=u32(input,at),fontId=u32(input,at+4),textSize=u32(input,at+8),kind=u32(input,at+12);
    auto &font=fonts.at(fontId);const auto source=font;
    const auto rows=u32(font,16),glyphs=u32(font,12),metrics=40u+(u32(font,32)?768u:0u);
    const auto table=metrics+rows*u32(font,36)*8,base=table+glyphs*4;
    const auto w=u32(input,at+64),h=u32(input,at+68),textAt=at+72+rows*4;
    if(rows>128 || size!=72+rows*4+textSize || w>512 || h>256 || !textSize || textSize>65536)
      throw std::runtime_error("Layout fixture extent");
    // Guard all borrowed source bytes, both active row state and line storage.
    Bytes text(textSize+64,0xa5);std::memcpy(text.data()+32,input.data()+textAt,textSize);const auto textBefore=text;
    std::array<std::uint32_t,0xe3c/4+32> storage{};storage.fill(0xa55aa55a);
    auto *object=storage.data()+16;std::fill_n(object,0xe3c/4,0);
    object[0]=reinterpret_cast<std::uintptr_t>(font.data());object[2]=reinterpret_cast<std::uintptr_t>(font.data()+metrics);
    object[3]=reinterpret_cast<std::uintptr_t>(font.data()+table);object[4]=reinterpret_cast<std::uintptr_t>(font.data()+base);
    object[0x218/4]=u32(input,at+28);object[0x21c/4]=u32(input,at+32);object[0x220/4]=u32(input,at+36);
    object[0x230/4]=u32(input,at+20);object[0x224/4]=u32(input,at+24);
    for(unsigned r=0;r<rows;++r)object[6+r]=u32(input,at+72+r*4);
    std::array<std::uint32_t,4> rect{};for(unsigned k=0;k<4;++k)rect[k]=u32(input,at+48+k*4);
    global(0x6e1ed4,u32(input,at+16));
    std::vector<std::uint16_t> pixels(64+(w+2)*h,0xa55a);
    for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)pixels[32+y*(w+2)+x]=std::uint16_t((y*w+x)*1273+113);
    const auto before=pixels;
    global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()+32));global(0x6a2dc8,w+2);
    global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,w);global(0x656618,h);global(0x6e1f88,0);
    *reinterpret_cast<std::uint16_t *>(0x6e1f7c)=0xf800;*reinterpret_cast<std::uint16_t *>(0x6e1f7e)=0x07e0;
    *reinterpret_cast<std::uint16_t *>(0x6e1f80)=0x001f;
    for(unsigned k=0;k<64;++k)reinterpret_cast<float *>(0x6f5dac)[k]=k/64.f;
    *reinterpret_cast<float *>(0x6f5eac)=1;
    std::uint32_t consumed=0xffffffffu,result;
    if(kind==0) {
      using Draw=unsigned(__attribute__((thiscall)) *)(void *,void *,void *,int,int,int,unsigned,void *);
      result=reinterpret_cast<Draw>(0x4a5ce0)(object,text.data()+32,rect.data(),173,91,237,u32(input,at+44),&consumed);
    }else if(kind==1) {
      using Draw=unsigned(__attribute__((thiscall)) *)(void *,void *,void *,unsigned,int,int,int,unsigned,void *);
      result=reinterpret_cast<Draw>(0x4a6190)(object,text.data()+32,rect.data(),u32(input,at+40),173,91,237,u32(input,at+44),&consumed);
    }else {
      using Measure=unsigned(__attribute__((thiscall)) *)(void *,void *,void *,unsigned,unsigned);
      result=reinterpret_cast<Measure>(0x4a6630)(object,text.data()+32,rect.data(),0,u32(input,at+44));
    }
    if(font!=source || text!=textBefore)throw std::runtime_error("Original higher text modified source");
    for(unsigned k=0;k<16;++k)if(storage[k]!=0xa55aa55a || storage[16+0xe3c/4+k]!=0xa55aa55a)
      throw std::runtime_error("Original higher text object guard");
    for(unsigned i=0;i<pixels.size();++i) {
      const bool guard=i<32 || i>=32+(w+2)*h || (i-32)%(w+2)>=w;
      if(guard && pixels[i]!=before[i])throw std::runtime_error("Original higher text canvas guard");
    }
    put(result);put(object[0x218/4]);put(object[0x21c/4]);
    put(consumed==0xffffffffu ? consumed : consumed-reinterpret_cast<std::uintptr_t>(text.data()+32));
    for(unsigned r=0;r<rows;++r)put(object[6+r]);
    const auto lines=object[0xe38/4];if(lines>256)throw std::runtime_error("Original higher text line extent");
    put(lines);
    for(unsigned l=0;l<lines;++l) {
      const auto off=0x238/4+l*3;put(object[off]);put(object[off+1]);
      put(object[off+2]-reinterpret_cast<std::uintptr_t>(text.data()+32));
    }
    for(unsigned y=0;y<h;++y)output.write(reinterpret_cast<const char *>(pixels.data()+32+y*(w+2)),w*2);
    at+=size;
  }
  if(at!=input.size() || !output)throw std::runtime_error("Layout output/input extent");
  std::cout<<"Original higher text state, source/object and canvas guards pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
