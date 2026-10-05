#include <cstdint>
#include <cstring>
#include <stdexcept>
using u8=std::uint8_t;using u32=std::uint32_t;using i32=std::int32_t;
#include "../runtime/render/frame_pixels.h"
static void check(bool value){if(!value)throw std::runtime_error("Pixel conversion failed");}
int main(){
    u8 out[16]={0};const u8 expected[16]={255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255};
    const u8 rgb565[12]={0,0xf8,0xe0,7,0,0,0x1f,0,0xff,0xff,0,0};
    check(render_pixels(out,2,2,rgb565,6,16,0xf800,0x7e0,0x1f,nullptr));check(!std::memcmp(out,expected,16));
    const u8 flipped[12]={0x1f,0,0xff,0xff,0,0,0,0xf8,0xe0,7,0,0};
    check(render_pixels(out,2,2,flipped+6,-6,16,0xf800,0x7e0,0x1f,nullptr));check(!std::memcmp(out,expected,16));
    const u8 rgb24[12]={0,0,255,0,255,0,255,0,0,255,255,255};
    check(render_pixels(out,2,2,rgb24,6,24,0xff0000,0xff00,0xff,nullptr));check(!std::memcmp(out,expected,16));
    const u8 rgb32[16]={0,0,255,0,0,255,0,0,255,0,0,0,255,255,255,0};
    check(render_pixels(out,2,2,rgb32,8,32,0xff0000,0xff00,0xff,nullptr));check(!std::memcmp(out,expected,16));
    u8 palette[1024]={0};for(int i=0;i<4;++i)std::memcpy(palette+i*4,expected+i*4,4);
    const u8 indexed[4]={0,1,2,3};
    check(render_pixels(out,2,2,indexed,2,8,0,0,0,palette));check(!std::memcmp(out,expected,16));
    check(!render_pixels(out,2,2,indexed,2,8,0,0,0,nullptr));
    check(!render_pixels(out,2,2,rgb32,3,32,0xff0000,0xff00,0xff,nullptr));
    check(!render_pixels(out,2,2,rgb32,8,32,0xff0000,0xff00,0xff00,nullptr));
    check(!render_pixels(out,2,2,rgb32,8,32,0xfa0000,0xff00,0xff,nullptr));
    check(!render_pixels(out,2049,1,rgb32,8196,32,0xff0000,0xff00,0xff,nullptr));
    // Exhaust the native RGB565 domain against the original floor-scaling
    // formula, including channels that differ from bit-replication rounding.
    u8 row[2048*2],rgba[2048*4];
    for(u32 base=0;base<65536;base+=2048){
        for(u32 x=0;x<2048;++x){row[x*2]=u8(base+x);row[x*2+1]=u8((base+x)>>8);}
        check(render_pixels(rgba,2048,1,row,4096,16,0xf800,0x7e0,0x1f,nullptr));
        for(u32 x=0;x<2048;++x){u32 value=base+x;
            check(rgba[x*4]==((value>>11)*255/31) && rgba[x*4+1]==(((value>>5)&63)*255/63) &&
                  rgba[x*4+2]==((value&31)*255/31) && rgba[x*4+3]==255);}
    }
}
