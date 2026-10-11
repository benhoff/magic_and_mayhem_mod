// Unchanged mapped PE32 entry and independent state model; no installed hook.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../reconstruction/rendering/glyph_backend_state.h"
#include <array>
extern "C" {
alignas(16) unsigned char mnm_glyph_seed_fx[512],mnm_glyph_host_fx[512],mnm_glyph_before_fx[512],mnm_glyph_after_fx[512];
std::uint32_t mnm_glyph_saved_sp,mnm_glyph_call_sp,mnm_glyph_function=0x581ec0;
std::uint32_t mnm_glyph_seed_flags,mnm_glyph_after_flags,mnm_glyph_gpr[8],mnm_glyph_arguments[4],mnm_glyph_guards[4];
void mnm_glyph_probe(std::uint32_t,std::uint32_t,std::int32_t,std::int32_t,int,int,int);
}
static std::uint16_t half(const void *p){std::uint16_t n;std::memcpy(&n,p,2);return n;}
static void seed(unsigned variant,unsigned active){
    alignas(16) unsigned char host[512];const double a=1.25,b=-2.0;
    __asm__ volatile("fxsave %0\n\tfninit\n\tfxsave %1":"=m"(host),"=m"(mnm_glyph_seed_fx)::"memory");
    if(active) __asm__ volatile("fldl %0"::"m"(a):"memory");
    if(active>1) __asm__ volatile("fldl %0"::"m"(b):"memory");
    __asm__ volatile("fxsave %0\n\tfxrstor %1":"=m"(mnm_glyph_seed_fx):"m"(host):"memory");
    const unsigned precisions[]={0,2,3};const std::uint16_t cw=std::uint16_t(0x7f|(precisions[variant/4]<<8)|((variant%4)<<10));
    std::memcpy(mnm_glyph_seed_fx,&cw,2);
    auto status=half(mnm_glyph_seed_fx+2);if(variant&1)status|=0x0220;std::memcpy(mnm_glyph_seed_fx+2,&status,2);
    const std::uint32_t mxcsr=0x1f80|((variant%4)<<13);std::memcpy(mnm_glyph_seed_fx+24,&mxcsr,4);
    for(unsigned i=0;i<128;++i)mnm_glyph_seed_fx[160+i]=static_cast<unsigned char>(i*37+variant);
    const std::uint32_t flags[]={0x202,0xed7,0x602,0xa97};mnm_glyph_seed_flags=flags[variant%4];
}
int main(int argc,char **argv)try{
    if(argc!=4)throw std::runtime_error("PE corpus output");
    map_image(read(argv[1]));
    if(std::memcmp(reinterpret_cast<void*>(0x581ec0),"\x83\xec\x28\x8b\x44\x24\x30",7)||
       *reinterpret_cast<std::uint32_t*>(0x5c7860)!=0x3c820821||*reinterpret_cast<std::uint32_t*>(0x5c5bb0)!=0||
       std::memcmp(reinterpret_cast<void*>(0x59bee0),"\x55\x8b\xec\x83\xc4\xf4",6))throw std::runtime_error("entry/constants");
    auto input=read(argv[2]);if(input.size()<12||std::memcmp(input.data(),"MNMGLS01",8))throw std::runtime_error("corpus header");
    std::ofstream output(argv[3],std::ios::binary);output.write("MNMGLSR1",8);std::uint32_t recordBytes=112;output.write(reinterpret_cast<char*>(&recordBytes),4);
    std::size_t at=12;
    for(unsigned n=0;n<u32(input,8);++n){
        const auto bytes=u32(input,at),size=u32(input,at+4),w=u32(input,at+8),h=u32(input,at+12),stride=u32(input,at+16);
        if(bytes!=328+size||bytes>input.size()-at||!w||!h||w>256||h>256||stride<w||stride>260)throw std::runtime_error("corpus extent");
        Bytes frame(input.begin()+at+328,input.begin()+at+bytes);const auto source=frame;
        std::vector<std::uint16_t> pixels(64+stride*h,0xa55a);for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)pixels[32+y*stride+x]=std::uint16_t((y*w+x)*1273+113);const auto before=pixels;
        global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()+32));global(0x6a2dc8,stride);global(0x6e1f88,0);
        *reinterpret_cast<std::uint16_t*>(0x6e1f7c)=0xf800;*reinterpret_cast<std::uint16_t*>(0x6e1f7e)=0x07e0;*reinterpret_cast<std::uint16_t*>(0x6e1f80)=0x001f;
        for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(input,at+28+i*4));
        std::memcpy(reinterpret_cast<void*>(0x6f5dac),input.data()+at+72,256);const auto cold=u32(input,at+60),variant=u32(input,at+64),active=u32(input,at+68);global(0x6f5eac,cold?0:0x3f800000);
        if(cold>1||variant>11||active>2)throw std::runtime_error("state seeds");
        seed(variant,active);
        mnm_glyph_probe(mnm_glyph_function,reinterpret_cast<std::uintptr_t>(frame.data()),std::int32_t(u32(input,at+20)),std::int32_t(u32(input,at+24)),u32(input,at+44),u32(input,at+48),u32(input,at+52));
        MnmGlyphStateInput request{frame.data(),size,std::uint32_t(reinterpret_cast<std::uintptr_t>(frame.data())),0x6f5dac,mnm_glyph_call_sp-4,mnm_glyph_seed_flags,std::int32_t(u32(input,at+28)),std::int32_t(u32(input,at+32)),std::int32_t(u32(input,at+36)),std::int32_t(u32(input,at+40)),std::int32_t(u32(input,at+20)),std::int32_t(u32(input,at+24)),u32(input,at+44),u32(input,at+48),u32(input,at+52),cold,variant%4};
        MnmGlyphState state{};if(mnm_glyph_backend_state(&request,&state))throw std::runtime_error("model refusal");
        std::uint32_t row[28]={};
        if(mnm_glyph_gpr[0]==state.eax&&mnm_glyph_gpr[1]==state.ecx&&mnm_glyph_gpr[2]==state.edx)row[0]|=1;
        if(!std::memcmp(mnm_glyph_arguments,state.arguments,16))row[0]|=2;
        if(mnm_glyph_gpr[3]==0xb1b2b3b4&&mnm_glyph_gpr[5]==0xe1e2e3e4&&mnm_glyph_gpr[6]==0x51525354&&mnm_glyph_gpr[7]==0x71727374)row[0]|=4;
        const std::uint32_t guards[]={0xaa55aa55,0x55aa55aa,0x2468ace0,0x13579bdf};if(mnm_glyph_gpr[4]==mnm_glyph_call_sp+16&&!std::memcmp(guards,mnm_glyph_guards,16))row[0]|=8;
        if((mnm_glyph_after_flags&0xcd5)==state.flags)row[0]|=16;
        std::uint32_t table[64];if(cold)mnm_glyph_coverage_bits(variant%4,table);else std::memcpy(table,input.data()+at+72,256);
        if(!std::memcmp(table,reinterpret_cast<void*>(0x6f5dac),256)&&*reinterpret_cast<std::uint32_t*>(0x6f5eac)==(cold?0:0x3f800000))row[0]|=32;
        bool intact=frame==source;for(unsigned i=0;i<pixels.size();++i)if((i<32||i>=32+stride*h||(i-32)%stride>=w)&&pixels[i]!=before[i])intact=false;if(intact)row[0]|=64;
        if(!std::memcmp(mnm_glyph_before_fx+160,mnm_glyph_after_fx+160,128)&&!std::memcmp(mnm_glyph_before_fx+24,mnm_glyph_after_fx+24,4))row[0]|=128;
        bool kept=half(mnm_glyph_before_fx)==half(mnm_glyph_after_fx)&&mnm_glyph_before_fx[4]==mnm_glyph_after_fx[4]&&((half(mnm_glyph_before_fx+2)^half(mnm_glyph_after_fx+2))&0x3800)==0;
        for(unsigned i=0;i<active;++i)kept=kept&&!std::memcmp(mnm_glyph_before_fx+32+i*16,mnm_glyph_after_fx+32+i*16,10);
        row[1]=kept;row[2]=half(mnm_glyph_before_fx+2);row[3]=half(mnm_glyph_after_fx+2);row[4]=state.branch;row[5]=state.visible_rows;row[6]=state.opaque_pixels;row[7]=table[63];
        std::memcpy(row+8,mnm_glyph_gpr,12);row[11]=state.eax;row[12]=state.ecx;row[13]=state.edx;std::memcpy(row+14,mnm_glyph_arguments,16);std::memcpy(row+18,state.arguments,16);row[22]=mnm_glyph_after_flags&0xcd5;row[23]=state.flags;row[24]=half(mnm_glyph_after_fx);row[25]=mnm_glyph_after_fx[4];row[26]=mnm_glyph_before_fx[4];row[27]=mnm_glyph_call_sp;
        output.write(reinterpret_cast<char*>(row),sizeof(row));at+=bytes;
    }
    if(at!=input.size()||!output)throw std::runtime_error("output extent");
    std::cout<<u32(input,8)<<" unchanged glyph caller-state executions\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
