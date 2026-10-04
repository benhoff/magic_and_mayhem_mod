// Private PE mapping; selected original controller routines are never patched.
#include "no_cd.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sys/mman.h>
using Bytes=std::vector<unsigned char>;
static Bytes read(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Cannot open input");return Bytes(std::istreambuf_iterator<char>(f),{});}
static std::uint32_t u32(const Bytes& b,std::size_t at){if(at>b.size() || b.size()-at<4)throw std::runtime_error("Read extent");std::uint32_t v;std::memcpy(&v,b.data()+at,4);return v;}
static void mapImage(const Bytes& b){
    const auto pe=u32(b,60),opt=pe+24,length=u32(b,opt+56);
    if(u32(b,opt+28)!=0x400000 || length>32*1024*1024)throw std::runtime_error("Unexpected PE image");
    auto* p=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    if(p==MAP_FAILED)throw std::runtime_error("Private PE mapping failed");
    const unsigned count=std::uint16_t(b.at(pe+6))|(std::uint16_t(b.at(pe+7))<<8);
    const auto size=std::uint16_t(b.at(pe+20))|(std::uint16_t(b.at(pe+21))<<8);
    for(unsigned i=0;i<count;++i){const auto at=opt+size+i*40,rva=u32(b,at+12),n=u32(b,at+16),raw=u32(b,at+20);
        if(std::uint64_t(raw)+n>b.size() || std::uint64_t(rva)+n>length)throw std::runtime_error("PE extent");
        std::memcpy(p+rva,b.data()+raw,n);}
    if(p[0x64cb0]!=0x8b || p[0x64ec0]!=0x51 || p[0x65000]!=0x51)throw std::runtime_error("Controller entry bytes differ");
}
int main(int argc,char** argv)try{
    if(argc!=6 && argc!=8)throw std::runtime_error("Expected PE ANI sequence ticks break-tick [switch-sequence switch-tick]");
    mapImage(read(argv[1]));auto b=read(argv[2]);
    if(u32(b,0)!=0x00494e41 || u32(b,12)!=5 || u32(b,4)!=b.size())throw std::runtime_error("ANI version/header");
    const auto offsets=u32(b,20),count=u32(b,8);const auto base=44+std::uint64_t(offsets)*4;
    if(offsets<2 || offsets>4097 || count>65536 || base+std::uint64_t(count)*44!=b.size())throw std::runtime_error("ANI limits/extent");
    const auto sequence=std::stoul(argv[3]),ticks=std::stoul(argv[4]);const auto breakTick=std::stoi(argv[5]);
    if(sequence>=offsets-1 || ticks>512)throw std::runtime_error("Sequence/tick limits");
    const auto first=u32(b,44+sequence*4),end=u32(b,48+sequence*4);
    if(first>=end || end>count)throw std::runtime_error("Sequence extent");
    std::vector<mnm::assets::AnimationRecord> records(end-first);
    static_assert(sizeof(mnm::assets::AnimationRecord)==44);
    std::memcpy(records.data(),b.data()+base+first*44,records.size()*44);
    if(records.back().opcode!=6)throw std::runtime_error("Sequence stop missing");
    mnm::reconstruction::NoCdAnimationPlayer native(records);
    std::uint32_t asset[3]={reinterpret_cast<std::uintptr_t>(b.data()),reinterpret_cast<std::uintptr_t>(b.data()+44),reinterpret_cast<std::uintptr_t>(b.data()+base)};
    std::uint32_t state[12]={};state[7]=reinterpret_cast<std::uintptr_t>(asset);
    using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
    using Tick=std::int32_t(__attribute__((thiscall)) *)(void*);
    reinterpret_cast<Start>(0x464cb0)(state,sequence,0);native.start();
    auto beginning=reinterpret_cast<std::uintptr_t>(b.data()+base+first*44);
    std::cout<<'[';
    for(unsigned tick=0;tick<=ticks;++tick){
        std::int32_t event=0,nevent=0;
        if(argc==8 && tick==std::stoul(argv[7])){
            const auto target=std::stoul(argv[6]);if(target>=offsets-1)throw std::runtime_error("Switch sequence extent");
            const auto a=u32(b,44+target*4),z=u32(b,48+target*4);
            if(a>=z || z>count)throw std::runtime_error("Switch record extent");
            std::vector<mnm::assets::AnimationRecord> replacement(z-a);
            std::memcpy(replacement.data(),b.data()+base+a*44,replacement.size()*44);
            native.switchSequence(std::move(replacement));
            using Switch=void(__attribute__((thiscall)) *)(void*,unsigned);
            if(*reinterpret_cast<unsigned char*>(0x464e20)!=0x53)throw std::runtime_error("Switch entry byte differs");
            reinterpret_cast<Switch>(0x464e20)(state,target);
            beginning=reinterpret_cast<std::uintptr_t>(b.data()+base+a*44);
        }
        if(tick){if(int(tick)==breakTick){state[8]=1;native.requestBreak();}
            event=reinterpret_cast<Tick>(0x464ec0)(state);nevent=native.tick();}
        const auto pc=(state[6]-beginning)/44;
        const auto display=state[1]?std::int64_t((state[1]-beginning)/44):-1;
        const auto sprite=state[1]?*reinterpret_cast<std::int32_t*>(state[1]+4):-1;
        const auto& n=native.state();
        if(event!=nevent || pc!=n.pc || display!=(n.displayedRecord?std::int64_t(*n.displayedRecord):-1) ||
           sprite!=(native.sprite()?std::int64_t(*native.sprite()):-1) || bool(state[2])!=n.active ||
           state[3]!=n.delay || state[4]!=n.elapsed || state[5]!=n.repeats || state[8]!=n.breakFlag)
            throw std::runtime_error("Native/original controller mismatch at tick "+std::to_string(tick));
        if(tick)std::cout<<',';
        std::cout<<'['<<tick<<','<<event<<','<<sprite<<','<<pc<<','<<display<<','<<state[2]<<','<<state[3]<<','<<state[4]<<','<<state[5]<<','<<state[8]<<']';
    }
    std::cout<<"]\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
