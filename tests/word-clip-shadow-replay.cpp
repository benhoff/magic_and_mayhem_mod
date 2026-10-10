// Independent execution of pinned, unmodified original word sprite backends.
// Native mode uses identical inputs, never the captured expected output.
#include "../renderer/sprites/word_clipped.h"
#include "../reconstruction/rendering/word_backend_state.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>
using Bytes=std::vector<std::uint8_t>;
static Bytes read(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("open input");return Bytes(std::istreambuf_iterator<char>(f),{});}
static std::uint32_t word(const Bytes& b,std::size_t p){if(p>b.size()||b.size()-p<4)throw std::runtime_error("extent");std::uint32_t n;std::memcpy(&n,b.data()+p,4);return n;}
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
    if(*reinterpret_cast<std::uint8_t*>(0x597086)!=0x55||*reinterpret_cast<std::uint8_t*>(0x596cb8)!=0x55)throw std::runtime_error("backend entry");
}
int main(int argc,char** argv)try{
    if(argc!=5)throw std::runtime_error("PE sample output original|native");
    auto b=read(argv[2]);
    if(b.size()<208||std::memcmp(b.data(),"MNMWRC01",8)||word(b,8)!=1||word(b,12)!=b.size())throw std::runtime_error("sample header");
    auto size=word(b,32),bytes=word(b,36),width=word(b,40),height=word(b,44),stride=word(b,48),backend=word(b,60);
    if(word(b,16)!=3||word(b,72)||word(b,76)||size<48||size>4*1024*1024||!bytes||bytes>16*1024*1024||b.size()!=208+size+bytes*2||
       !width||!height||width>2048||height>2048||stride<width||stride>4096||bytes!=stride*height*2||
       (backend!=0x197086&&backend!=0x196cb8))throw std::runtime_error("sample bounds");
    auto* frame=static_cast<std::uint8_t*>(mmap(nullptr,size,3,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if(frame==MAP_FAILED)throw std::runtime_error("frame allocation");
    std::memcpy(frame,b.data()+208,size);
    Bytes guarded(bytes+68,0xa5);auto* pixels=guarded.data()+32+(word(b,28)&3);
    std::memcpy(pixels,b.data()+208+size,bytes);
    std::uint32_t result=0,scratch[16]={};
    if(std::string(argv[4])=="original"){
        map(read(argv[1]));
        std::memcpy(reinterpret_cast<void*>(0x5f1e50),b.data()+80,64);
        global(0x658174,reinterpret_cast<std::uintptr_t>(pixels));global(0x6a2dc8,stride);
        global(0x6a49b8,width);global(0x656618,height);global(0x6e0008,word(b,64));global(0x6cbb6c,word(b,68));
        using Draw=std::uint32_t(*)(void*,std::int32_t,std::int32_t);
        result=reinterpret_cast<Draw>(0x400000+backend)(frame,word(b,52),word(b,56));
        std::memcpy(scratch,reinterpret_cast<void*>(0x5f1e50),64);
        // Normalize only the two workspace fields overwritten with frame pointers.
        scratch[7]-=reinterpret_cast<std::uintptr_t>(frame);scratch[10]-=reinterpret_cast<std::uintptr_t>(frame);
    }else if(std::string(argv[4])=="native"){
        MnmWordCanvas c{pixels,bytes,width,height,stride};MnmWordClippedDraw d{};
        MnmWordClip clip{std::int32_t(word(b,64)),std::int32_t(word(b,68)),std::int32_t(width),std::int32_t(height)};
        std::uint32_t before[16];std::memcpy(before,b.data()+80,64);
        MnmWordBackendInput input{frame,size,std::uint32_t(reinterpret_cast<std::uintptr_t>(frame)),
            std::uint32_t(reinterpret_cast<std::uintptr_t>(pixels)),width,height,stride,backend,
            clip.left,clip.top,std::int32_t(word(b,52)),std::int32_t(word(b,56))};
        MnmWordBackendState state{};
        result=mnm_word_backend_state(&input,before,&state);
        if(!result)result=mnm_word_sprite_clip_draw(frame,size,&c,&clip,word(b,52),word(b,56),&d);
        if(!result){std::memcpy(scratch,state.words,64);scratch[7]-=reinterpret_cast<std::uintptr_t>(frame);scratch[10]-=reinterpret_cast<std::uintptr_t>(frame);}
    }else throw std::runtime_error("mode");
    for(std::size_t i=0;i<guarded.size();++i)if((i<std::size_t(pixels-guarded.data())||i>=std::size_t(pixels-guarded.data())+bytes)&&guarded[i]!=0xa5)throw std::runtime_error("destination guard");
    if(std::memcmp(frame,b.data()+208,size))throw std::runtime_error("source changed");
    std::ofstream out(argv[3],std::ios::binary);out.write(reinterpret_cast<char*>(&result),4);out.write(reinterpret_cast<char*>(scratch),64);out.write(reinterpret_cast<char*>(pixels),bytes);
    if(!out)throw std::runtime_error("output write");
    return 0;
}catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}
