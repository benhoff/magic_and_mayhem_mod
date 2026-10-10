// Private mapped, unchanged NoCD advance/draw routines. Never enters Win32.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
int main(int argc,char **argv) try {
  if (argc!=4) throw std::runtime_error("Expected PE, corpus, output");
  map_image(read(argv[1]));
  if (std::memcmp(reinterpret_cast<void *>(0x4a58e0),"\x51\x8a\x54\x24\x08",5) ||
      std::memcmp(reinterpret_cast<void *>(0x4a59e0),"\x53\x8b\x5c\x24\x08",5) ||
      std::strcmp(reinterpret_cast<const char *>(0x5e0578),"!?:;"))
    throw std::runtime_error("Original text signature/table changed");
  auto input=read(argv[2]);
  if (input.size()<12 || std::memcmp(input.data(),"MNMTXT01",8)) throw std::runtime_error("Text corpus header");
  std::ofstream output(argv[3],std::ios::binary);
  auto word=[&](std::uint32_t v) {output.write(reinterpret_cast<const char *>(&v),4);};
  std::size_t at=12;
  for (unsigned n=0;n<u32(input,8);++n) {
    const auto size=u32(input,at),fontSize=u32(input,at+4),count=u32(input,at+8),draw=u32(input,at+12);
    const auto w=u32(input,at+48),h=u32(input,at+52);
    Bytes font(input.begin()+at+56,input.begin()+at+56+fontSize);const auto source=font;
    const auto rows=u32(font,16),glyphs=u32(font,12),metrics=40u+(u32(font,32)?768u:0u);
    const auto table=metrics+rows*u32(font,36)*8,base=table+glyphs*4;
    if (rows>128 || size!=56+fontSize+rows*4+count*4 || w>256 || h>64)
      throw std::runtime_error("Text extent");
    std::array<std::uint32_t,144> object{};
    object[0]=reinterpret_cast<std::uintptr_t>(font.data());
    object[2]=reinterpret_cast<std::uintptr_t>(font.data()+metrics);
    object[3]=reinterpret_cast<std::uintptr_t>(font.data()+table);
    object[4]=reinterpret_cast<std::uintptr_t>(font.data()+base);
    object[0x218/4]=u32(input,at+32);object[0x21c/4]=u32(input,at+36);
    object[0x230/4]=u32(input,at+24);object[0x224/4]=u32(input,at+28);
    for (unsigned r=0;r<rows;++r) object[6+r]=u32(input,at+56+fontSize+r*4);
    global(0x6e1ed4,u32(input,at+20));
    std::vector<std::uint16_t> pixels(64+(w+2)*h,0xa55a);
    for (unsigned y=0;y<h;++y) for (unsigned x=0;x<w;++x) pixels[32+y*(w+2)+x]=std::uint16_t((y*w+x)*1273+113);
    const auto before=pixels;
    global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()+32));global(0x6a2dc8,w+2);
    global(0x6e0008,0);global(0x6cbb6c,u32(input,at+44));global(0x6a49b8,w);global(0x656618,h);
    global(0x6e1f88,0);
    *reinterpret_cast<std::uint16_t *>(0x6e1f7c)=0xf800;
    *reinterpret_cast<std::uint16_t *>(0x6e1f7e)=0x07e0;
    *reinterpret_cast<std::uint16_t *>(0x6e1f80)=0x001f;
    for (unsigned i=0;i<64;++i) reinterpret_cast<float *>(0x6f5dac)[i]=i/64.f;
    *reinterpret_cast<float *>(0x6f5eac)=1;
    for (unsigned i=0;i<count;++i) {
      const auto byte=u32(input,at+56+fontSize+rows*4+i*4);
      std::uint32_t result=0;
      if (draw) {
        using Draw=void(__attribute__((thiscall)) *)(void *,unsigned,int,int,int);
        reinterpret_cast<Draw>(0x4a59e0)(object.data(),byte,173,91,237);
      } else {
        using Advance=int(__attribute__((thiscall)) *)(void *,unsigned,unsigned);
        result=reinterpret_cast<Advance>(0x4a58e0)(object.data(),byte,u32(input,at+16));
      }
      word(result);word(object[0x218/4]);word(object[0x21c/4]);
      for (unsigned r=0;r<rows;++r) word(object[6+r]);
    }
    if (font!=source) throw std::runtime_error("Original text modified font");
    for (unsigned i=0;i<pixels.size();++i) {
      const bool guard=i<32 || i>=32+(w+2)*h || (i-32)%(w+2)>=w;
      if (guard && pixels[i]!=before[i]) throw std::runtime_error("Original text wrote guard");
    }
    if (draw) for (unsigned y=0;y<h;++y) output.write(reinterpret_cast<const char *>(pixels.data()+32+y*(w+2)),w*2);
    at+=size;
  }
  if (at!=input.size() || !output) throw std::runtime_error("Text output/input extent");
  std::cout<<"Original byte state, font guards and padded canvas guards pass\n";
  return 0;
} catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
