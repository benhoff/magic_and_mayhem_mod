// Independent unchanged PE32 replay of retained live shadow/takeover inputs.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
extern "C" {
#include "../reconstruction/rendering/glyph_backend.h"
alignas(16) unsigned char mnm_glyph_seed_fx[512],mnm_glyph_host_fx[512],mnm_glyph_before_fx[512],mnm_glyph_after_fx[512];
std::uint32_t mnm_glyph_saved_sp,mnm_glyph_call_sp,mnm_glyph_function;
std::uint32_t mnm_glyph_seed_flags,mnm_glyph_after_flags,mnm_glyph_gpr[8],mnm_glyph_arguments[4],mnm_glyph_guards[4];
void mnm_glyph_probe(std::uint32_t,std::uint32_t,std::int32_t,std::int32_t,int,int,int);
}
static unsigned half(const unsigned char* p){return p[0]|unsigned(p[1])<<8;}
static bool fpEqual(const unsigned char* a,const unsigned char* b){
    if(std::memcmp(a,b,5)||std::memcmp(a+24,b+24,8)||std::memcmp(a+160,b+160,128))return false;
    auto top=(half(a+2)>>11)&7;
    for(unsigned i=0;i<8;++i)if(a[4]&(1u<<((top+i)&7)))if(std::memcmp(a+32+i*16,b+32+i*16,10))return false;
    return true;
}
int main(int argc,char** argv)try{
    if(argc<3)throw std::runtime_error("PE samples...");
    map_image(read(argv[1]));
    if(std::memcmp(reinterpret_cast<void*>(0x581ec0),"\x83\xec\x28\x8b\x44\x24\x30",7))throw std::runtime_error("original glyph signature");
    std::uint64_t words=0;
    for(int arg=2;arg<argc;++arg){
        auto input=read(argv[arg]);if(input.size()<256||(std::memcmp(input.data(),"MNMGLIV1",8)&&std::memcmp(input.data(),"MNMGLIV2",8)))throw std::runtime_error("wire magic");
        auto h=[&](unsigned i){return u32(input,i*4);};
        unsigned size=h(5),w=h(6),height=h(7),stride=h(8),bytes=h(9);
        const unsigned format=h(19);if(format>1||h(2)!=1+format||h(3)!=256||size>1048576||size<40||!w||w>stride||stride>2048||!height||height>2048||bytes!=stride*height*2||input.size()!=256+size+bytes*2+1536)throw std::runtime_error("wire extent");
        Bytes frame(input.begin()+256,input.begin()+256+size);const auto source=frame;
        const auto* before=input.data()+256+size;const auto* after=before+bytes;const auto* table=after+bytes;const auto* tableAfter=table+256;const auto* fx=tableAfter+256;const auto* fxAfter=fx+512;
        std::vector<std::uint16_t> pixels(64+bytes/2,0xa55a);std::memcpy(pixels.data()+32,before,bytes);
        global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()+32));global(0x6a2dc8,stride);global(0x6e1f88,format);
        *reinterpret_cast<std::uint16_t*>(0x6e1f7c)=format?0x7c00:0xf800;*reinterpret_cast<std::uint16_t*>(0x6e1f7e)=format?0x3e0:0x07e0;*reinterpret_cast<std::uint16_t*>(0x6e1f80)=0x001f;
        for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,h(12+i));
        std::memcpy(reinterpret_cast<void*>(0x6f5dac),table,256);global(0x6f5eac,h(17));
        std::memcpy(mnm_glyph_seed_fx,fx,512);mnm_glyph_seed_flags=h(28);mnm_glyph_function=0x581ec0;
        mnm_glyph_probe(0x581ec0,reinterpret_cast<std::uintptr_t>(frame.data()),std::int32_t(h(25)),std::int32_t(h(30)),h(31),h(32),h(33));
        if(std::memcmp(pixels.data()+32,after,bytes)||std::memcmp(reinterpret_cast<void*>(0x6f5dac),tableAfter,256)||!fpEqual(mnm_glyph_after_fx,fxAfter)||std::memcmp(mnm_glyph_arguments,input.data()+44*4,16))throw std::runtime_error(std::string("original completion mismatch: ")+argv[arg]);
        auto originalEdx=mnm_glyph_gpr[2];if(originalEdx>=reinterpret_cast<std::uintptr_t>(frame.data())&&originalEdx<=reinterpret_cast<std::uintptr_t>(frame.data()+frame.size()))originalEdx=originalEdx-reinterpret_cast<std::uintptr_t>(frame.data())+h(10);
        if(mnm_glyph_gpr[0]!=h(41)||mnm_glyph_gpr[1]!=h(40)||originalEdx!=h(39))throw std::runtime_error("original volatile GPR mismatch");
        MnmGlyphStateInput state{frame.data(),size,h(10),0x6f5dac,h(11),h(28),std::int32_t(h(12)),std::int32_t(h(13)),std::int32_t(h(14)),std::int32_t(h(15)),std::int32_t(h(25)),std::int32_t(h(30)),h(31),h(32),h(33),h(16),(half(fx)>>10)&3};
        MnmGlyphState result{};state.entry_sp=mnm_glyph_call_sp-4;
        if(mnm_glyph_backend_state_format(&state,&result,format)||(result.flags&0xcd5)!=(mnm_glyph_after_flags&0xcd5))throw std::runtime_error("original stack-specific flags");
        state.entry_sp=h(11);std::memcpy(pixels.data()+32,before,bytes);
        std::uint32_t coverage[64];std::memcpy(coverage,table,256);alignas(16) unsigned char seed[512],out[512];std::memcpy(seed,fx,512);
        MnmGlyphBackend native{state,pixels.data()+32,w,height,stride,coverage,seed,out,{},format};
        if(mnm_glyph_backend(&native)||std::memcmp(pixels.data()+32,after,bytes)||std::memcmp(coverage,tableAfter,256)||!fpEqual(out,fxAfter)||native.result.eax!=h(41)||native.result.ecx!=h(40)||native.result.edx!=h(39)||(native.result.flags&0xcd5)!=(h(42)&0xcd5)||std::memcmp(native.result.arguments,input.data()+44*4,16))throw std::runtime_error("native retained completion mismatch");
        for(unsigned i:{0u,1u,2u,4u})if(h(20+i)!=h(34+i))throw std::runtime_error("captured callee state mismatch");
        if(frame!=source)throw std::runtime_error("source changed");
        for(unsigned i=0;i<32;++i)if(pixels[i]!=0xa55a||pixels[32+bytes/2+i]!=0xa55a)throw std::runtime_error("guard changed");
        if(mnm_glyph_gpr[3]!=0xb1b2b3b4||mnm_glyph_gpr[4]!=mnm_glyph_call_sp+16||mnm_glyph_gpr[5]!=0xe1e2e3e4||mnm_glyph_gpr[6]!=0x51525354||mnm_glyph_gpr[7]!=0x71727374)throw std::runtime_error("original probe callee/stack mismatch");
        words+=bytes/2;
    }
    std::cout<<argc-2<<" retained live completions match independent original/native replay; "<<words<<" RGB565 words\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
