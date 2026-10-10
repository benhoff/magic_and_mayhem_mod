// Independent original execution versus the actual native route/RET16 entry.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
extern "C" {
#include "../runtime/scene/glyph_route.h"
alignas(16) unsigned char mnm_glyph_seed_fx[512],mnm_glyph_host_fx[512],mnm_glyph_before_fx[512],mnm_glyph_after_fx[512];
std::uint32_t mnm_glyph_saved_sp,mnm_glyph_call_sp,mnm_glyph_function;
std::uint32_t mnm_glyph_seed_flags,mnm_glyph_after_flags,mnm_glyph_gpr[8],mnm_glyph_arguments[4],mnm_glyph_guards[4];
std::uint32_t mnm_glyph_pinned_original=0x581ec0,mnm_glyph_original_calls;
void mnm_glyph_counted_original();
void (*mnm_glyph_original)()=mnm_glyph_counted_original;
void mnm_glyph_enter();
void mnm_glyph_probe(std::uint32_t,std::uint32_t,std::int32_t,std::int32_t,int,int,int);
}
static std::uint16_t half(const void *p){std::uint16_t n;std::memcpy(&n,p,2);return n;}
static void seed(unsigned variant,unsigned active){
    alignas(16) unsigned char host[512];const double a=1.25,b=-2.0;
    __asm__ volatile("fxsave %0\n\tfninit\n\tfxsave %1":"=m"(host),"=m"(mnm_glyph_seed_fx)::"memory");
    if(active)__asm__ volatile("fldl %0"::"m"(a):"memory");
    if(active>1)__asm__ volatile("fldl %0"::"m"(b):"memory");
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
       *reinterpret_cast<std::uint64_t*>(0x5c5390)!=0x3ff0000000000000ull||
       std::memcmp(reinterpret_cast<void*>(0x59bee0),"\x55\x8b\xec\x83\xc4\xf4",6))throw std::runtime_error("entry/constants");
    auto input=read(argv[2]);if(input.size()<12||std::memcmp(input.data(),"MNMGLS01",8))throw std::runtime_error("corpus header");
    std::ofstream output(argv[3],std::ios::binary);output.write("MNMGLBK1",8);std::uint32_t recordBytes=64;output.write(reinterpret_cast<char*>(&recordBytes),4);
    std::size_t at=12;std::uint64_t words=0;
    for(unsigned n=0;n<u32(input,8);++n){
        const auto bytes=u32(input,at),size=u32(input,at+4),w=u32(input,at+8),h=u32(input,at+12),stride=u32(input,at+16);
        if(bytes!=328+size||bytes>input.size()-at||!w||!h||w>256||h>256||stride<w||stride>260)throw std::runtime_error("corpus extent");
        Bytes frame(input.begin()+at+328,input.begin()+at+bytes);const auto source=frame;
        std::vector<std::uint16_t> pixels(64+stride*h,0xa55a);for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)pixels[32+y*stride+x]=std::uint16_t((y*w+x)*1273+113);const auto before=pixels;
        global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()+32));global(0x6a2dc8,stride);global(0x6e1f88,0);
        *reinterpret_cast<std::uint16_t*>(0x6e1f7c)=0xf800;*reinterpret_cast<std::uint16_t*>(0x6e1f7e)=0x07e0;*reinterpret_cast<std::uint16_t*>(0x6e1f80)=0x001f;
        for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(input,at+28+i*4));
        const auto cold=u32(input,at+60),variant=u32(input,at+64),active=u32(input,at+68);
        if(cold>1||variant>11||active>2)throw std::runtime_error("state seeds");
        auto call=[&](std::uint32_t function){
            mnm_glyph_function=function;
            mnm_glyph_probe(function,reinterpret_cast<std::uintptr_t>(frame.data()),std::int32_t(u32(input,at+20)),std::int32_t(u32(input,at+24)),u32(input,at+44),u32(input,at+48),u32(input,at+52));
        };
        for(unsigned mode=0;mode<(n<48?5u:1u);++mode){
        if(mode==4&&cold)continue;
        pixels=before;
        seed(variant,active);
        if(mode==3)mnm_glyph_seed_fx[0]&=0xfe; // finite draws are safe with IE unmasked
        std::memcpy(reinterpret_cast<void*>(0x6f5dac),input.data()+at+72,256);global(0x6f5eac,cold?0:0x3f800000);
        if(mode==4)global(0x6f5dac+63*4,0x3f800001);
        call(mnm_glyph_pinned_original);
        const auto originalPixels=pixels;
        unsigned char originalFx[512];std::memcpy(originalFx,mnm_glyph_after_fx,512);
        std::uint32_t originalGpr[8],originalArgs[4],originalGuards[4],originalTable[64];
        std::memcpy(originalGpr,mnm_glyph_gpr,32);std::memcpy(originalArgs,mnm_glyph_arguments,16);std::memcpy(originalGuards,mnm_glyph_guards,16);std::memcpy(originalTable,reinterpret_cast<void*>(0x6f5dac),256);
        const auto originalFlags=mnm_glyph_after_flags;
        pixels=before;std::memcpy(reinterpret_cast<void*>(0x6f5dac),input.data()+at+72,256);
        if(mode==4)global(0x6f5dac+63*4,0x3f800001);
        MnmGlyphRouteContext context{};
        context.backend.state={frame.data(),size,std::uint32_t(reinterpret_cast<std::uintptr_t>(frame.data())),0x6f5dac,1,mnm_glyph_seed_flags,std::int32_t(u32(input,at+28)),std::int32_t(u32(input,at+32)),std::int32_t(u32(input,at+36)),std::int32_t(u32(input,at+40)),0,0,0,0,0,cold,variant%4};
        context.backend.pixels=pixels.data()+32;context.backend.width=w;context.backend.height=h;context.backend.stride=stride;context.backend.coverage=reinterpret_cast<std::uint32_t*>(0x6f5dac);
        if(mode==2)context.backend.state.frame=nullptr;
        mnm_glyph_route_context=mode==1?nullptr:&context;mnm_glyph_original_calls=0;
        call(reinterpret_cast<std::uintptr_t>(mnm_glyph_enter));
        std::uint32_t row[16]={};
        if(pixels==originalPixels)row[0]|=1;
        if(!std::memcmp(originalTable,reinterpret_cast<void*>(0x6f5dac),256))row[0]|=2;
        if(!std::memcmp(originalGpr,mnm_glyph_gpr,32))row[0]|=4;
        if(!std::memcmp(originalArgs,mnm_glyph_arguments,16)&&!std::memcmp(originalGuards,mnm_glyph_guards,16))row[0]|=8;
        if((originalFlags&0xcd5)==(mnm_glyph_after_flags&0xcd5))row[0]|=16;
        bool fp=half(originalFx)==half(mnm_glyph_after_fx)&&half(originalFx+2)==half(mnm_glyph_after_fx+2)&&originalFx[4]==mnm_glyph_after_fx[4];
        for(unsigned i=0;i<8;++i)if(originalFx[4]&(1u<<(((half(originalFx+2)>>11)+i)&7)))fp=fp&&!std::memcmp(originalFx+32+i*16,mnm_glyph_after_fx+32+i*16,10);
        if(fp)row[0]|=32;
        if(!std::memcmp(originalFx+24,mnm_glyph_after_fx+24,8)&&!std::memcmp(originalFx+160,mnm_glyph_after_fx+160,128))row[0]|=64;
        bool intact=frame==source;for(unsigned i=0;i<pixels.size();++i)if((i<32||i>=32+stride*h||(i-32)%stride>=w)&&pixels[i]!=before[i])intact=false;if(intact)row[0]|=128;
        row[1]=context.handled;row[2]=context.forwarded;row[3]=mnm_glyph_original_calls;row[4]=half(originalFx+2);row[5]=half(mnm_glyph_after_fx+2);row[6]=stride*h;row[7]=active;row[8]=cold;row[9]=variant;
        row[10]=originalGpr[4];row[11]=mnm_glyph_gpr[4];row[12]=originalFlags&0xcd5;row[13]=mnm_glyph_after_flags&0xcd5;
        row[14]=mode;row[15]=n;
        output.write(reinterpret_cast<char*>(row),sizeof(row));words+=stride*h;
        }
        at+=bytes;
    }
    mnm_glyph_route_context=nullptr;
    if(at!=input.size()||!output)throw std::runtime_error("output extent");
    std::cout<<u32(input,8)<<" original/native entry executions, "<<words<<" canvas WORDs\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
