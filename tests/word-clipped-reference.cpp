// Map and execute original bytes unchanged. The independent state model and
// clipped CPU raster and recovered workspace use encoded input and initial state only.
#include "../reconstruction/rendering/word_backend_state.h"
#include "../renderer/sprites/word_clipped.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>
using Bytes=std::vector<std::uint8_t>;
extern "C" {
alignas(16) std::uint8_t mnm_probe_seed_fx[512],mnm_probe_before_fx[512],mnm_probe_after_fx[512],mnm_probe_host_fx[512];
std::uint32_t mnm_probe_function,mnm_probe_saved_esp,mnm_probe_call_esp,mnm_probe_seed_flags,mnm_probe_after_flags;
std::uint32_t mnm_probe_gpr[8],mnm_probe_arguments[3],mnm_probe_guards[4];
void mnm_word_probe(std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t);
void _word_enter_scalar(void);
void _word_enter_forward(void);
std::uint32_t _word_original, mnm_clipped_handled;
// Host-only stand-in for a future clipped adapter. The actual production entry
// assembly is linked unchanged; Win32 admission/logging/LastError/reentry and
// live integration are excluded. Empty frames forward to preserve original FP.
int _word_route(std::uint32_t* registers){
    const auto backend=*reinterpret_cast<std::uint32_t*>(registers[3]+4);
    _word_original=0x400000+backend;
    auto* arguments=reinterpret_cast<std::uint32_t*>(registers[3]+12);
    auto* frame=reinterpret_cast<std::uint8_t*>(arguments[0]);
    MnmWordCanvas canvas{*reinterpret_cast<std::uint8_t**>(0x658174),0,
        *reinterpret_cast<std::uint32_t*>(0x6a49b8),*reinterpret_cast<std::uint32_t*>(0x656618),
        *reinterpret_cast<std::uint32_t*>(0x6a2dc8)};
    canvas.bytes=canvas.stride_words*canvas.height*2;
    MnmWordClip clip{*reinterpret_cast<std::int32_t*>(0x6e0008),
        *reinterpret_cast<std::int32_t*>(0x6cbb6c),std::int32_t(canvas.width),std::int32_t(canvas.height)};
    std::uint32_t size;std::memcpy(&size,frame,4);MnmWordClippedDraw draw{};
    if(mnm_word_sprite_clip_admit(frame,size,&canvas,&clip,arguments[1],arguments[2],&draw)||
       !draw.width||!draw.height)return 0;
    MnmWordBackendInput in{frame,size,arguments[0],std::uint32_t(reinterpret_cast<std::uintptr_t>(canvas.pixels)),
        canvas.width,canvas.height,canvas.stride_words,backend,clip.left,clip.top,
        std::int32_t(arguments[1]),std::int32_t(arguments[2])};
    MnmWordBackendState state{};
    if(mnm_word_backend_state(&in,reinterpret_cast<std::uint32_t*>(0x5f1e50),&state))return 0;
    if(mnm_word_sprite_clip_draw(frame,size,&canvas,&clip,arguments[1],arguments[2],&draw))return 0;
    std::memcpy(reinterpret_cast<void*>(0x5f1e50),state.words,64);
    arguments[1]=std::uint32_t(state.argument_x);arguments[2]=std::uint32_t(state.argument_y);
    mnm_clipped_handled=1;return 1;
}
}
static Bytes read(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("open input");return Bytes(std::istreambuf_iterator<char>(f),{});}
static std::uint32_t word(const Bytes& b,std::size_t p){if(p>b.size()||b.size()-p<4)throw std::runtime_error("extent");std::uint32_t n;std::memcpy(&n,b.data()+p,4);return n;}
static std::uint16_t half(const std::uint8_t* p){std::uint16_t n;std::memcpy(&n,p,2);return n;}
static void global(std::uintptr_t a,std::uint32_t n){std::memcpy(reinterpret_cast<void*>(a),&n,4);}
static void map(const Bytes& b){
    auto pe=word(b,60),opt=pe+24,length=word(b,opt+56);
    if(word(b,opt+28)!=0x400000)throw std::runtime_error("image base");
    auto* m=static_cast<std::uint8_t*>(mmap(reinterpret_cast<void*>(0x400000),length,7,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    if(m==MAP_FAILED)throw std::runtime_error("map image");
    auto count=unsigned(b.at(pe+6)|b.at(pe+7)<<8),table=opt+unsigned(b.at(pe+20)|b.at(pe+21)<<8);
    for(unsigned i=0;i<count;++i){auto at=table+i*40,n=word(b,at+16),raw=word(b,at+20),rva=word(b,at+12);
        if(raw>b.size()||n>b.size()-raw||rva>length||n>length-rva)throw std::runtime_error("section extent");
        std::memcpy(m+rva,b.data()+raw,n);}
    const std::uint8_t signature[]={0x55,0x8b,0xec,0x56,0x57,0x53};
    for(auto address:{0x596cb8,0x597086})if(std::memcmp(reinterpret_cast<void*>(address),signature,6))throw std::runtime_error("backend signature");
}
static void seed(std::uint32_t variant){
    alignas(16) std::uint8_t host[512];
    const double a=1.25,b=-2.0,c=3.141592653589793;
    __asm__ volatile("fxsave %0\n\tfninit\n\tfldl %1\n\tfldl %2\n\tfldl %3\n\tfxsave %4\n\tfxrstor %0"
                     :"=m"(host):"m"(a),"m"(b),"m"(c),"m"(mnm_probe_seed_fx):"memory");
    const std::uint16_t control=std::uint16_t(0x037f|((variant&3)<<10));
    std::memcpy(mnm_probe_seed_fx,&control,2);
    auto status=half(mnm_probe_seed_fx+2);
    if(variant&1)status|=0x0221; // masked sticky exceptions plus incoming C1
    std::memcpy(mnm_probe_seed_fx+2,&status,2);
    const std::uint32_t mxcsr=0x1f80|((variant&3)<<13);
    std::memcpy(mnm_probe_seed_fx+24,&mxcsr,4);
    for(unsigned i=0;i<128;++i)mnm_probe_seed_fx[160+i]=std::uint8_t(i*37+variant);
    const std::uint32_t patterns[]={0x202,0xed7,0x602,0xa97};
    mnm_probe_seed_flags=patterns[variant%4];
}
int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("PE manifest output");
    map(read(argv[1]));std::ifstream manifest(argv[2]);std::ofstream output(argv[3],std::ios::binary);
    if(!manifest||!output)throw std::runtime_error("manifest/output open");
    output.write("MNMWCT01",8);const std::uint32_t version=1,recordBytes=256;
    output.write(reinterpret_cast<const char*>(&version),4);output.write(reinterpret_cast<const char*>(&recordBytes),4);
    std::string path;unsigned count=0;
    while(std::getline(manifest,path)){
        auto b=read(path.c_str());
        if(b.size()<208||std::memcmp(b.data(),"MNMWRC01",8)||word(b,8)!=1||word(b,12)!=b.size())throw std::runtime_error("sample header");
        auto size=word(b,32),bytes=word(b,36),width=word(b,40),height=word(b,44),stride=word(b,48),backend=word(b,60);
        if(size<40||size>4*1024*1024||!bytes||bytes>16*1024*1024||b.size()!=208+size+bytes*2||
           !width||!height||width>2048||height>2048||stride<width||stride>4096||bytes!=stride*height*2||
           (backend!=0x197086&&backend!=0x196cb8))throw std::runtime_error("sample bounds");
        Bytes frame(b.begin()+208,b.begin()+208+size),guarded(bytes+68,0xa5);
        auto* pixels=guarded.data()+32+(word(b,28)&3);
        std::memcpy(pixels,b.data()+208+size,bytes);
        const auto frameAddress=std::uint32_t(reinterpret_cast<std::uintptr_t>(frame.data()));
        const auto canvasAddress=std::uint32_t(reinterpret_cast<std::uintptr_t>(pixels));
        std::uint32_t before[16];std::memcpy(before,b.data()+80,64);
        MnmWordBackendInput input{frame.data(),size,frameAddress,canvasAddress,width,height,stride,backend,
                                  std::int32_t(word(b,64)),std::int32_t(word(b,68)),
                                  std::int32_t(word(b,52)),std::int32_t(word(b,56))};
        MnmWordBackendState predicted{};
        if(mnm_word_backend_state(&input,before,&predicted))throw std::runtime_error("model input refusal");
        std::memcpy(reinterpret_cast<void*>(0x5f1e50),before,64);
        global(0x658174,canvasAddress);global(0x6a2dc8,stride);global(0x6a49b8,width);global(0x656618,height);
        global(0x6e0008,word(b,64));global(0x6cbb6c,word(b,68));
        seed(word(b,76));mnm_probe_function=0x400000+backend;
        mnm_word_probe(mnm_probe_function,frameAddress,word(b,52),word(b,56));
        std::uint32_t row[64]={};row[1]=backend;row[2]=frameAddress;row[3]=canvasAddress;
        row[4]=mnm_probe_seed_flags;row[5]=mnm_probe_after_flags;row[6]=UINT32_MAX;
        std::memcpy(row+8,reinterpret_cast<void*>(0x5f1e50),64);std::memcpy(row+24,predicted.words,64);
        for(unsigned i=0;i<16;++i)if(row[8+i]!=row[24+i]){row[6]=i;break;}
        if(row[6]==UINT32_MAX)row[0]|=1;
        row[48]=std::uint32_t(predicted.argument_x);row[49]=std::uint32_t(predicted.argument_y);
        row[50]=mnm_probe_arguments[1];row[51]=mnm_probe_arguments[2];
        if(mnm_probe_arguments[0]==frameAddress&&row[48]==row[50]&&row[49]==row[51])row[0]|=2;
        const std::uint32_t expected[]={0,0xc1c2c3c4,0xd1d2d3d4,0xb1b2b3b4,mnm_probe_call_esp,0xe1e2e3e4,0x51525354,0x71727374};
        std::memcpy(row+40,mnm_probe_gpr,32);
        if(!std::memcmp(expected,mnm_probe_gpr,32))row[0]|=4;
        const std::uint32_t guards[]={0xaa55aa55,0x55aa55aa,0x2468ace0,0x13579bdf};
        if(mnm_probe_gpr[4]==mnm_probe_call_esp&&!std::memcmp(guards,mnm_probe_guards,16))row[0]|=8;
        if((mnm_probe_after_flags&0x8c5)==0x44&&((mnm_probe_after_flags^mnm_probe_seed_flags)&0x400)==0)row[0]|=16;
        auto* pre=mnm_probe_before_fx;auto* post=mnm_probe_after_fx;
        const bool empty=!word(frame,4)||!word(frame,8);
        if(half(pre)==half(post)&&pre[4]==post[4]&&half(post+2)==(half(pre+2)&(empty?0xffff:0xfdff)))row[0]|=32;
        bool values=true;for(unsigned i=0;i<3;++i)values=values&&!std::memcmp(pre+32+i*16,post+32+i*16,10);
        if(values)row[0]|=64;
        if(!std::memcmp(pre+160,post+160,128))row[0]|=128;
        if(!std::memcmp(pre+24,post+24,4))row[0]|=256;
        bool canvasGuard=true;for(std::size_t i=0;i<guarded.size();++i)
            if((i<std::size_t(pixels-guarded.data())||i>=std::size_t(pixels-guarded.data())+bytes)&&guarded[i]!=0xa5)canvasGuard=false;
        if(canvasGuard&&!std::memcmp(pixels,b.data()+208+size+bytes,bytes)&&!std::memcmp(frame.data(),b.data()+208,size))row[0]|=512;
        MnmWordCanvas canvas{pixels,bytes,width,height,stride};MnmWordClippedDraw draw{};
        MnmWordClip clip{input.clip_left,input.clip_top,std::int32_t(width),std::int32_t(height)};
        row[7]=mnm_word_sprite_clip_admit(frame.data(),size,&canvas,&clip,input.anchor_x,input.anchor_y,&draw)==MNM_WORD_OK;
        row[52]=0;row[53]=empty;
        row[54]=half(pre+2);row[55]=half(post+2);row[56]=half(pre);row[57]=half(post);
        std::memcpy(row+58,pre+24,4);std::memcpy(row+59,post+24,4);
        row[61]=mnm_probe_call_esp;row[62]=mnm_probe_gpr[4];row[63]=empty;
        alignas(16) std::uint8_t originalFx[512];std::memcpy(originalFx,post,512);
        std::memcpy(pixels,b.data()+208+size,bytes);
        std::memcpy(reinterpret_cast<void*>(0x5f1e50),before,64);
        seed(word(b,76));mnm_clipped_handled=0;
        mnm_probe_function=std::uint32_t(reinterpret_cast<std::uintptr_t>(backend==0x197086?&_word_enter_scalar:&_word_enter_forward));
        mnm_word_probe(mnm_probe_function,frameAddress,word(b,52),word(b,56));
        std::uint32_t nativeMask=0;
        if(!std::memcmp(row+8,reinterpret_cast<void*>(0x5f1e50),64))nativeMask|=1;
        if(mnm_probe_arguments[0]==frameAddress&&mnm_probe_arguments[1]==row[48]&&mnm_probe_arguments[2]==row[49])nativeMask|=2;
        bool gpr=true;for(unsigned i=0;i<8;++i)if(mnm_probe_gpr[i]!=(i==4?mnm_probe_call_esp:expected[i]))gpr=false;
        if(gpr)nativeMask|=4;
        if(mnm_probe_gpr[4]==mnm_probe_call_esp&&!std::memcmp(guards,mnm_probe_guards,16))nativeMask|=8;
        if(((mnm_probe_after_flags^row[5])&0xcc5)==0)nativeMask|=16;
        post=mnm_probe_after_fx;
        if(half(post)==half(originalFx)&&half(post+2)==half(originalFx+2)&&post[4]==originalFx[4])nativeMask|=32;
        values=true;for(unsigned i=0;i<3;++i)values=values&&!std::memcmp(post+32+i*16,originalFx+32+i*16,10);
        if(values)nativeMask|=64;
        if(!std::memcmp(post+160,originalFx+160,128))nativeMask|=128;
        if(!std::memcmp(post+24,originalFx+24,4))nativeMask|=256;
        canvasGuard=true;for(std::size_t i=0;i<guarded.size();++i)
            if((i<std::size_t(pixels-guarded.data())||i>=std::size_t(pixels-guarded.data())+bytes)&&guarded[i]!=0xa5)canvasGuard=false;
        if(canvasGuard&&!std::memcmp(pixels,b.data()+208+size+bytes,bytes)&&!std::memcmp(frame.data(),b.data()+208,size))nativeMask|=512;
        row[60]=nativeMask;row[52]=mnm_clipped_handled;row[53]=!mnm_clipped_handled;
        output.write(reinterpret_cast<const char*>(row),sizeof(row));++count;
    }
    if(!output)throw std::runtime_error("output write");
    std::cout<<count<<" original/model cases\n";return 0;
}catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}
