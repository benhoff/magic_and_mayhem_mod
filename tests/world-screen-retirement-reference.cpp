#include "../reconstruction/rendering/world_screen_retirement.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U=std::uint32_t;
constexpr U Base=0x720000,Owner=Base+0x100,Size=0x12000;
constexpr U Globals[]={0x5d9e38,0x6de6d9,0x689498,0x68949c,0x6def58,0x6def60,
    0x6def64,0x6b1248,0x6b124c,0x6e1f94,0x6e1f98,0x6e266c,
    0x6de558,0x6de55c,0x6de560,0x6de564,0x6de568,0x6de56c,0x6de570,0x6de574,0x6de578,
    0x6a28f8,0x6a28fc,0x6a2900,0x6a2904,0x6a2908,0x6a290c,0x6a2910,0x6a2914,0x6a2918,
    0x6903e8,0x6e1f9c,0x656624};
using Snapshot=std::array<unsigned char,Size+sizeof(Globals)>;
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Memory {
    Snapshot bytes{};bool original=false;
    unsigned char* at(U address,U size=4){
        U offset=0;
        if(address>=Base && address-Base<=Size-size)offset=address-Base;
        else {
            bool found=false;
            for(U i=0;i<std::size(Globals);++i)if(address>=Globals[i] && address+size<=Globals[i]+4){offset=Size+4*i+address-Globals[i];found=true;break;}
            need(found,"Unadmitted World storage");
        }
#ifdef MNM_ORIGINAL_REFERENCE
        if(original)return reinterpret_cast<unsigned char*>(address);
#endif
        return bytes.data()+offset;
    }
    U get(U address){U value;std::memcpy(&value,at(address),4);return value;}
    U getByte(U address){return *at(address,1);}
    void set(U address,U value){std::memcpy(at(address),&value,4);}
    void byte(U address,U value){*at(address,1)=value;}
    Snapshot snapshot(){Snapshot s;std::memcpy(s.data(),at(Base,Size),Size);for(U i=0;i<std::size(Globals);++i)std::memcpy(s.data()+Size+4*i,at(Globals[i]),4);return s;}
};
struct Call {U kind,a,b,c;Snapshot bytes;bool operator==(const Call& x)const{return kind==x.kind&&a==x.a&&b==x.b&&c==x.c&&bytes==x.bytes;}};
static Memory* memory;static std::vector<Call> calls;static U iterator,mutation,profile;
static void event(U kind,U a,U b=0,U c=0){
    calls.push_back({kind,a,b,c,memory->snapshot()});
    if(mutation==1 && kind==1 && a==0x4943f0)memory->set(0x68949c,1);
    if(mutation==2 && kind==1 && a==0x52a670)memory->set(0x6def58,Base+0x1700);
    if(mutation==3 && kind==2){memory->set(Owner+8,9);memory->set(0x6903e8,91);}
}
struct Services {
    U call(U function,U receiver=0,U arg=0){
        event(1,function,receiver,arg);
        if(function==0x4a5000)return 0xa1b2c3d4;
        if(function==0x52a670)return iterator<profile%4?Base+0x1800+0x40*(iterator++):0;
        if(function==0x5c52e8)return 0xcafefeed;
        return 0;
    }
    void free(U pointer){event(2,pointer);}
    void freeArray(U pointer){event(3,pointer);}
    void bitmapPlane(U object,U pointer){event(4,object,pointer);}
};
#ifdef MNM_ORIGINAL_REFERENCE
template<U Function>static U __attribute__((thiscall,force_align_arg_pointer)) member(void* receiver){return Services{}.call(Function,U(receiver));}
template<U Function>static U __attribute__((thiscall,force_align_arg_pointer)) argument(void* receiver,U arg){return Services{}.call(Function,U(receiver),arg);}
template<U Function>static U __attribute__((force_align_arg_pointer)) plain(){return Services{}.call(Function);}
static U __attribute__((stdcall,force_align_arg_pointer)) tick(){return Services{}.call(0x5c52e8);}
static void __attribute__((force_align_arg_pointer)) freed(U pointer){Services{}.free(pointer);}
static void __attribute__((force_align_arg_pointer)) freedArray(U pointer){Services{}.freeArray(pointer);}
static void __attribute__((thiscall,force_align_arg_pointer)) plane(void* object,U pointer){Services{}.bitmapPlane(U(object),pointer);}
static void adapt(U address,void* fn){auto* at=reinterpret_cast<unsigned char*>(address);at[0]=0xe9;const U delta=U(fn)-address-5;std::memcpy(at+1,&delta,4);}
#endif
static Memory input(U seed,U count,U boundary){
    Memory m;m.bytes.fill(seed);for(U a:Globals)m.set(a,0x12345678);
    m.set(0x5d9e38,profile&1?0xffffffff:123);m.set(Owner+0x100df,profile&2?Base+0x1000:0);
    m.byte(Owner+0x100ac,profile&4?1:0);m.byte(Owner+0x100a4,profile&8?1:0);
    m.set(0x689498,Base+0x1000);m.set(0x68949c,profile%4);
    m.set(0x6b1248,Base+0x2000);m.set(0x6b124c,profile%3);
    for(U i=0;i<4;++i){m.set(Base+0x1004+i*0x22e,(profile+i)%2);m.set(Base+0x2004+i*0x7e,(profile+i)%2);m.set(Base+0x1804+i*0x40,(profile+i)%2);}
    m.set(0x6e266c,profile&1?0:Base+0x2100);
    for(U at:{0x6de558u,0x6a28f8u})for(U i=0;i<9;++i)m.set(at+4*i,((profile+i)%3)?Base+0x2200+i*16:0);
    m.set(Owner+0xfc52,count);m.set(Owner+0xfc2a,Base+0x3000);m.set(Owner+0xfc2e,Base+0x4000);
    m.set(Owner+0xfc32,Base+(boundary?0x3ffc:0x3000));m.set(Owner+0xfc36,Base+0x600);
    m.set(Base+0x600,Base+0x3000);m.set(Base+0x604,Base+0x5000);
    m.set(Owner+0xfc4a,Base+0x600);m.byte(0x6e1f9c,profile&1);
    return m;
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    need(argc==2,"Pinned PE required");map_image(read(argv[1]));
    need(mmap((void*)Base,Size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"Owned World mapping");
#define MEMBER(pc) adapt(pc,(void*)member<pc>)
    MEMBER(0x49e860);MEMBER(0x4ffad0);MEMBER(0x52baf0);MEMBER(0x4943f0);
    MEMBER(0x4a5000);MEMBER(0x52a670);MEMBER(0x506d30);MEMBER(0x543c60);
    MEMBER(0x535970);MEMBER(0x4ff9a0);MEMBER(0x464ad0);MEMBER(0x5458b0);
    MEMBER(0x4ecbe0);MEMBER(0x573960);MEMBER(0x46a930);
#undef MEMBER
    adapt(0x4711d0,(void*)argument<0x4711d0>);adapt(0x56d0a0,(void*)argument<0x56d0a0>);
    adapt(0x58bc10,(void*)argument<0x58bc10>);
    adapt(0x549230,(void*)plain<0x549230>);adapt(0x5496b0,(void*)plain<0x5496b0>);
    adapt(0x549b20,(void*)plain<0x549b20>);adapt(0x4879f0,(void*)plain<0x4879f0>);
    adapt(0x597870,(void*)freed);adapt(0x5979b0,(void*)freedArray);adapt(0x57d0d0,(void*)plane);
    U clock=U((void*)tick);std::memcpy((void*)0x5c52e8,&clock,4);
    const Bytes before((unsigned char*)0x46ab30,(unsigned char*)0x46ae8c);
#else
    (void)argv;need(argc==1,"Native arguments");
#endif
    U cases=0;std::uint64_t checksum=0;
    for(U seed:{0u,0xa5u,0xffu})for(profile=0;profile<16;++profile)
    for(U count:{0u,1u,2u,6u})for(U boundary:{0u,1u})for(mutation=0;mutation<4;++mutation){
        const auto start=input(seed,count,boundary);auto expected=start;memory=&expected;
        calls.clear();iterator=0;Services services;
        mnm::reconstruction::retireOriginalWorldScreen(expected,services,Owner);
        const auto expectedCalls=calls;
#ifdef MNM_ORIGINAL_REFERENCE
        std::memcpy((void*)Base,start.bytes.data(),Size);
        for(U i=0;i<std::size(Globals);++i)std::memcpy((void*)Globals[i],start.bytes.data()+Size+4*i,4);
        Memory actual;actual.original=true;memory=&actual;calls.clear();iterator=0;
        using Fn=void(__attribute__((thiscall))*)(void*);reinterpret_cast<Fn>(0x46ab30)((void*)Owner);
        if(actual.snapshot()!=expected.snapshot()||calls!=expectedCalls){
            std::cerr<<"profile "<<profile<<" count "<<count<<" boundary "<<boundary<<" mutation "<<mutation<<" calls "<<calls.size()<<"/"<<expectedCalls.size()<<'\n';
            throw std::runtime_error("World retirement mismatch");
        }
#endif
        need(!expected.get(Owner+8)&&!expected.get(Owner+0xfc52),"World owner/queue not retired");
        for(auto value:expected.snapshot())checksum=checksum*131+value;
        for(const auto& c:expectedCalls)for(U value:{c.kind,c.a,c.b,c.c})checksum=checksum*131+value;
        ++cases;
    }
#ifdef MNM_ORIGINAL_REFERENCE
    need(Bytes((unsigned char*)0x46ab30,(unsigned char*)0x46ae8c)==before,"Original World exit changed");
#endif
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
