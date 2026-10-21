#include "../reconstruction/rendering/canvas_initializer.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static std::vector<U> events;static std::array<U,8> returnedFormat;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
struct StartupProvider {
    void window(U hwnd){events.insert(events.end(),{1,hwnd});}
    void mode(U hwnd,U width,U height,U depth){events.insert(events.end(),{2,hwnd,width,height,depth});}
    void create(U receiver,const std::array<U,27>& desc,U file,U line){events.insert(events.end(),{3,receiver,file,line});events.insert(events.end(),desc.begin(),desc.end());}
    void fill(U receiver,U colour){events.insert(events.end(),{4,receiver,colour});}
    std::array<U,8> format(U receiver){events.insert(events.end(),{5,receiver,32,0,0,0,0,0,0,0});return returnedFormat;}
};
#ifdef MNM_ORIGINAL_REFERENCE
static StartupProvider provider;
static void __attribute__((fastcall,force_align_arg_pointer)) windowCall(U hwnd,U){provider.window(hwnd);}
static void __attribute__((fastcall,force_align_arg_pointer)) modeCall(U hwnd,U width,U height,U depth){provider.mode(hwnd,width,height,depth);}
static void __attribute__((thiscall,force_align_arg_pointer)) createCall(void* receiver,const U* desc,U file,U line){
    std::array<U,27> values;std::memcpy(values.data(),desc,108);provider.create(U(reinterpret_cast<std::uintptr_t>(receiver)),values,file,line);
}
static void __attribute__((thiscall,force_align_arg_pointer)) fillCall(void* receiver,U colour){provider.fill(U(reinterpret_cast<std::uintptr_t>(receiver)),colour);}
struct Interface {void** table;};
static U __attribute__((stdcall,force_align_arg_pointer)) formatCall(Interface*,U* values){
    if(values[0]!=32)throw std::runtime_error("Format size");
    for(unsigned i=1;i<8;++i)if(values[i])throw std::runtime_error("Format request not cleared");
    const auto result=provider.format(0x6a4fe0);std::memcpy(values,result.data(),32);return 0;
}
template<std::size_t N>static void boundary(U at,void* destination,const unsigned char(&signature)[N]){
    auto* p=reinterpret_cast<unsigned char*>(at);if(std::memcmp(p,signature,N))throw std::runtime_error("Initializer boundary signature");
    const U rel=U(reinterpret_cast<std::uintptr_t>(destination))-at-5;p[0]=0xe9;std::memcpy(p+1,&rel,4);
}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));
    const unsigned char mode[]{0xa1,0xcc,0x1e,0x6e,0},create[]{0xa1,0x54,0x81,0x6f,0},fill[]{0x83,0xec,0x6c};
    boundary(0x58a9b0,reinterpret_cast<void*>(&modeCall),mode);boundary(0x58aa90,reinterpret_cast<void*>(&windowCall),mode);
    boundary(0x58ad90,reinterpret_cast<void*>(&createCall),create);boundary(0x58bc10,reinterpret_cast<void*>(&fillCall),fill);
    void* table[22]{};table[21]=reinterpret_cast<void*>(&formatCall);Interface surface{table};
    global(0x6a4fe0+8,U(reinterpret_cast<std::uintptr_t>(&surface)));
    const auto* init=reinterpret_cast<const unsigned char*>(0x4e40d0);const Bytes initBytes(init,init+403);
    const auto* query=reinterpret_cast<const unsigned char*>(0x58d540);const Bytes queryBytes(query,query+100);
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");StartupProvider provider;
#endif
    U id=0;
    for(U window:{0u,1u,2u})for(U video:{0u,1u,0xffffffffu})for(U padding:{0u,1u,2u})
        for(U px:{0u,3u,96u,0xffffffffu})for(U py:{0u,2u,0xffffffffu})for(U depth:{0u,8u,16u,24u,32u,0xffffffffu})
        for(U alternate:{0u,1u,0xffffffffu})for(U source:{0u,0x12340000u})for(U format=0;format<4;++format){
            OriginalCanvasStartup state;state.window=window;state.video=video;state.depth=depth;
            for(U i=0;i<36;++i)state.globals[i]=static_cast<unsigned char>(i*17+id);
            state.put(0,alternate);state.put(4,source);state.put(8,px);state.put(12,py);state.put(16,padding);
            returnedFormat={32,0x40,format==2?0x10001u:0u,format==3?0xffff8010u:16u,format==0?0xf800u:format==1?0x7c00u:0xff0000u,0x7e0,31,0};events.clear();
            U result;
#ifdef MNM_ORIGINAL_REFERENCE
            global(0x6a2920,window);global(0x6de6d5,video);global(0x5e13fc,depth);std::memcpy(reinterpret_cast<void*>(0x6e1f68),state.globals.data(),36);
            using Initialize=U(__attribute__((thiscall)) *)(void*);result=reinterpret_cast<Initialize>(0x4e40d0)(reinterpret_cast<void*>(0x1234));
            std::memcpy(state.globals.data(),reinterpret_cast<void*>(0x6e1f68),36);
#else
            result=initializeOriginalCanvas(state,0x1234,provider);
#endif
            if(state.get(0)!=alternate||state.get(4)!=source)throw std::runtime_error("Ordinary initializer touched alternate state");
            const U h[]{id++,window,video,padding,px,py,depth,alternate,source,format,result,U(events.size())};emit(h,sizeof(h));emit(state.globals.data(),36);emit(events.data(),events.size()*4);
        }
#ifdef MNM_ORIGINAL_REFERENCE
    if(std::memcmp(init,initBytes.data(),initBytes.size())||std::memcmp(query,queryBytes.data(),queryBytes.size()))throw std::runtime_error("Original initializer or query changed");
#endif
    if(id!=46656)throw std::runtime_error("Initializer denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
