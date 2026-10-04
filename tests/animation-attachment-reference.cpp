// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "attachment.hpp"
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
static void pointer(Bytes& bytes,std::size_t at,void* p){const auto value=reinterpret_cast<std::uintptr_t>(p);std::memcpy(bytes.data()+at,&value,4);}
static void require(bool ok){if(!ok)throw std::runtime_error("Original/native placement mismatch");}
int main(int argc,char** argv)try{
    if(argc!=7)throw std::runtime_error("Expected PE ANI base asset-index facing ticks");
    mapImage(read(argv[1]));auto b=read(argv[2]);
    const auto baseSequence=std::stoul(argv[3]),index=std::stoul(argv[4]),facing=std::stoul(argv[5]),ticks=std::stoul(argv[6]);
    if(baseSequence>4096 || index>12 || facing>7 || ticks>128)throw std::runtime_error("Fixture limits");
    const auto selection=mnm::reconstruction::modeOneSelection(baseSequence,index,facing);
    const auto offsets=u32(b,20),count=u32(b,8);const auto base=44+std::uint64_t(offsets)*4;
    if(u32(b,0)!=0x00494e41 || u32(b,12)!=5 || u32(b,4)!=b.size() || offsets<2 || offsets>4097 || count>65536 || base+count*44!=b.size() || selection.sequence>=offsets-1)throw std::runtime_error("ANI extent");
    const auto first=u32(b,44+4*selection.sequence),end=u32(b,48+4*selection.sequence);
    if(first>=end || end>count)throw std::runtime_error("Selected sequence extent");
    std::vector<mnm::assets::AnimationRecord> records(end-first);std::memcpy(records.data(),b.data()+base+first*44,records.size()*44);
    mnm::reconstruction::NoCdAnimationPlayer native(records);
    Bytes creature(0xe4b),config(37*16),assets(13*0x114);
    pointer(assets,index*0x114,b.data());pointer(assets,index*0x114+4,b.data()+44);pointer(assets,index*0x114+8,b.data()+base);
    auto word=[](Bytes& bytes,unsigned at,std::uint32_t value){std::memcpy(bytes.data()+at,&value,4);};
    word(config,36*16,baseSequence);word(config,36*16+4,index);word(creature,0x60c,facing);
    *reinterpret_cast<std::uintptr_t*>(0x6898d0)=reinterpret_cast<std::uintptr_t>(config.data());
    *reinterpret_cast<std::uintptr_t*>(0x6894a0)=reinterpret_cast<std::uintptr_t>(assets.data());
    if(*reinterpret_cast<unsigned char*>(0x51ff00)!=0x55 || *reinterpret_cast<unsigned char*>(0x51fef0)!=0x8b)throw std::runtime_error("Entry bytes differ");
    using Change=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
    using Tick=std::int32_t(__attribute__((thiscall)) *)(void*);
    const auto change=reinterpret_cast<Change>(0x51ff00);
    require(change(creature.data(),1,0)==1);native.start();require(u32(creature,0x7f1)==reinterpret_cast<std::uintptr_t>(assets.data()+selection.assetIndex*0x114));
    const auto beginning=reinterpret_cast<std::uintptr_t>(b.data()+base+first*44);
    std::cout<<"{\"asset_index\":"<<selection.assetIndex<<",\"sequence\":"<<selection.sequence<<",\"rows\":[";
    for(unsigned tick=0;tick<=ticks;++tick){
        auto event=0;
        if(tick){event=reinterpret_cast<Tick>(0x464ec0)(creature.data()+0x7d5);require(event==native.tick());}
        const auto display=u32(creature,0x7d9);const auto sprite=display?*reinterpret_cast<std::int32_t*>(display+4):-1;
        const auto& n=native.state();require(u32(creature,0x7cd)==1 && u32(creature,0x7d5)==selection.sequence);
        require(bool(u32(creature,0x7dd))==n.active && u32(creature,0x7e1)==n.delay && u32(creature,0x7e5)==n.elapsed && u32(creature,0x7e9)==n.repeats && u32(creature,0x7f5)==n.breakFlag);
        require((u32(creature,0x7ed)-beginning)/44==n.pc);
        require(sprite==(native.sprite()?std::int32_t(*native.sprite()):-1));
        if(tick)std::cout<<',';
        std::cout<<'['<<tick<<','<<event<<','<<sprite<<','<<(display?std::int64_t((display-beginning)/44):-1)<<','<<n.active<<']';
        auto before=creature;require(change(creature.data(),1,0)==0 && before==creature);
    }
    require(change(creature.data(),0,0)==1);native.stop();
    for(unsigned at:{4u,8u,12u,16u,20u,24u,32u})require(u32(creature,0x7d5+at)==0);
    require(!native.sprite() && !native.state().active && !native.state().displayedRecord);
    require(change(creature.data(),1,0)==1);native.start();require(native.sprite().value()==*reinterpret_cast<std::uint32_t*>(u32(creature,0x7d9)+4));
    std::cout<<"],\"same_mode_noop\":true,\"remove_and_reenter\":true,\"all_match\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
