#include "../reconstruction/rendering/screen_lifetime.hpp"
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
using U = std::uint32_t;
constexpr U base=0x650000, owner=base+0x100, callback=base+0x600;
constexpr U surface=base+0x800, alternate=base+0x880, vector=owner+0x1f;
using Storage=std::array<unsigned char,0x900>;
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Memory {
    Storage bytes{}; bool original=false;
    unsigned char* at(U address,U n=4) {
        need(address>=base && address-base+n<=bytes.size(),"Unadmitted address");
#ifdef MNM_ORIGINAL_REFERENCE
        if(original)return reinterpret_cast<unsigned char*>(address);
#endif
        return bytes.data()+address-base;
    }
    U get(U address){U v;std::memcpy(&v,at(address),4);return v;}
    void set(U address,U v){std::memcpy(at(address),&v,4);}
    void byte(U address,U v){*at(address,1)=v;}
    Storage snapshot(){Storage result;std::memcpy(result.data(),at(base,result.size()),result.size());return result;}
};
struct Call {U kind,arg;Storage bytes;bool operator==(const Call& b)const{return kind==b.kind&&arg==b.arg&&bytes==b.bytes;}};
static Memory* memory;static U mutation,disposalFlags;static bool allocationFails,realDisposal;
static std::vector<Call> calls;
struct Services {
    U allocate(U size) {
        calls.push_back({1,size,memory->snapshot()});
        need(size==12,"Callback size");
        if(mutation==1){memory->set(owner+0x13,alternate);memory->set(owner+0x47,0x77777777);}
        return allocationFails?0:callback;
    }
    void free(U address) {
        calls.push_back({2,address,memory->snapshot()});
        if(mutation==2){memory->set(vector+4,123);memory->set(vector+8,456);memory->set(vector+16,789);}
        if(mutation==3 && address==callback){memory->set(owner+0x47,0x12345678);memory->set(surface+0x28,0);}
    }
    void dispose(U object,U flags) {
        need(object==callback && flags==1,"Callback disposal ABI");
        if(realDisposal){mnm::reconstruction::disposeOriginalScreenCallback(*memory,*this,object,flags);return;}
        calls.push_back({3,flags,memory->snapshot()});
        if(mutation==3){memory->set(owner+0x47,0x12345678);memory->set(surface+0x28,0);}
    }
    U invoke(U function,U receiver,U argument){need(function==0x4a8550&&receiver==owner,"Stored callback invocation");(void)argument;return 0;}
};
#ifdef MNM_ORIGINAL_REFERENCE
// The original PE callers do not promise the Linux i386 SSE stack alignment.
static U __attribute__((force_align_arg_pointer)) allocateAdapter(U size){return Services{}.allocate(size);}
static void __attribute__((force_align_arg_pointer)) freeAdapter(U address){Services{}.free(address);}
static void __attribute__((thiscall,force_align_arg_pointer)) disposeAdapter(void* object,U flags){Services{}.dispose(U(object),flags);}
static void adapt(U address,void* function){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;const U delta=U(function)-address-5;std::memcpy(p+1,&delta,4);}
static void original(U operation) {
    using Fn=U(__attribute__((thiscall))*)(void*);
    constexpr U entries[]={0x4a8430,0x4cab70,0x4a8480,0x4a8500,0x4a84e0};
    if(operation<5){const U result=reinterpret_cast<Fn>(entries[operation])((void*)(operation==1?vector:owner));if(operation==0)need(result==owner,"Constructor result");}
    else if(operation==5) {
        for(U pc:{0x4a8500u,0x4a8500u,0x4a84e0u,0x4a84e0u,0x4a8480u})reinterpret_cast<Fn>(pc)((void*)owner);
    } else if(operation==6) {
        for(U pc:{0x4a8490u,0x4a84a0u,0x4a84b0u,0x4a84c0u,0x4a8540u,0x475c40u})reinterpret_cast<Fn>(pc)((void*)owner);
        using Callback=U(__attribute__((thiscall))*)(void*,U);
        need(reinterpret_cast<Callback>(0x4a8550)((void*)owner,0xffffffff)==0,"Class11 callback result");
    } else {
        using Callback=U(__attribute__((thiscall))*)(void*,U);
        const U result=reinterpret_cast<Callback>(operation==7?0x4bc980:0x4bc790)((void*)callback,disposalFlags);
        need(result==(operation==7?callback:0),"Original callback return");
    }
}
#endif
static void model(U operation,Memory& m) {
    using namespace mnm::reconstruction;Services p;
    if(operation==0)constructOriginalClass11Screen(m,owner);
    if(operation==1)destroyOriginalScreenVector(m,p,vector);
    if(operation==2)destroyOriginalClass11Screen(m,p,owner);
    if(operation==3)prepareOriginalClass11Callback(m,p,owner);
    if(operation==4)releaseOriginalClass11Callback(m,p,owner);
    if(operation==5){prepareOriginalClass11Callback(m,p,owner);prepareOriginalClass11Callback(m,p,owner);releaseOriginalClass11Callback(m,p,owner);releaseOriginalClass11Callback(m,p,owner);destroyOriginalClass11Screen(m,p,owner);}
    if(operation==7)need(disposeOriginalScreenCallback(m,p,callback,disposalFlags)==callback,"Model deleting-destructor result");
    if(operation==8)need(invokeOriginalScreenCallback(m,p,callback,disposalFlags)==0,"Model invocation result");
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    need(argc==2,"Pinned executable required");map_image(read(argv[1]));
    adapt(0x597850,(void*)allocateAdapter);adapt(0x597870,(void*)freeAdapter);
    U disposal=U((void*)disposeAdapter);std::memcpy((void*)0x5c6488,&disposal,4);
    constexpr std::pair<U,U> ranges[]={{0x4a8430,0x4a847e},{0x4a8480,0x4a848e},{0x4a84e0,0x4a84fa},{0x4a8500,0x4a8540},{0x4cab50,0x4cab69},{0x4cab70,0x4cabcc},{0x4bc790,0x4bc7a0},{0x4bc980,0x4bc99e},{0x4bcc00,0x4bcc07},{0x4a8490,0x4a8491},{0x4a84a0,0x4a84a1},{0x4a84b0,0x4a84b1},{0x4a84c0,0x4a84c1},{0x4a8540,0x4a8541},{0x475c40,0x475c41},{0x4a8550,0x4a8555}};
    std::vector<Bytes> before;for(auto r:ranges)before.emplace_back((unsigned char*)r.first,(unsigned char*)r.second);
#else
    (void)argv;need(argc==1,"Native model takes no executable");
#endif
    U cases=0;std::uint64_t checksum=0;
    for(U operation=0;operation<9;++operation)for(U seed:{0u,0xa5u,0xffu})
    for(U allocated:{0u,1u})for(U count:{0u,1u,4u})for(U existing:{0u,1u})
    for(U failure:{0u,1u})for(U mut=0;mut<4;++mut)
    for(U real=0;real<((operation==4||operation==5)?2u:1u);++real)
    for(U flags:operation==7?std::vector<U>{0,1,2,3,0x100,0xffffffff}:std::vector<U>{1}){
        Memory input;input.bytes.fill(seed);
        input.set(owner+0x13,surface);input.set(owner+0x47,existing?callback:0);
        input.set(vector+4,allocated?base+0x400:0);
        input.set(vector+8,allocated?base+0x400+count*4:0);
        input.set(vector+12,allocated?base+0x480:0);
        input.set(callback,0x5c6484);input.set(callback+4,0x4a8550);input.set(callback+8,owner);
        mutation=mut;allocationFails=failure;realDisposal=real;disposalFlags=flags;
        auto expected=input;memory=&expected;calls.clear();model(operation,expected);
        const auto expectedCalls=calls;
#ifdef MNM_ORIGINAL_REFERENCE
        std::memcpy((void*)base,input.bytes.data(),input.bytes.size());
        U disposal=real?0x4bc980:U((void*)disposeAdapter);std::memcpy((void*)0x5c6488,&disposal,4);
        Memory actual;actual.original=true;memory=&actual;calls.clear();original(operation);
        if(actual.snapshot()!=expected.snapshot() || calls!=expectedCalls){std::cerr<<"operation "<<operation<<" case "<<cases<<'\n';throw std::runtime_error("Original lifetime comparison mismatch");}
#endif
        // Independent lifecycle requirements in addition to model equivalence.
        if(operation==0)need(expected.get(owner)==0x5c6454&&expected.get(owner+4)==11&&!expected.get(owner+0x47),"Class11 constructor contract");
        if(operation==2)need(expected.get(owner+0x47)==(existing?callback:0),"Destructor unexpectedly releases callback");
        if(operation==3 && !existing)need(expected.get((mut==1?alternate:surface)+0x28)==(failure?0:callback),"Allocation publication contract");
        if(operation==4 && existing)need(!expected.get(owner+0x47),"Callback not cleared");
        for(auto b:expected.snapshot())checksum=checksum*131+b;
        for(const auto& c:expectedCalls){checksum=checksum*131+c.kind;checksum=checksum*131+c.arg;for(auto b:c.bytes)checksum=checksum*131+b;}
        ++cases;
    }
#ifdef MNM_ORIGINAL_REFERENCE
    for(U i=0;i<before.size();++i)need(Bytes((unsigned char*)ranges[i].first,(unsigned char*)ranges[i].second)==before[i],"Recovered lifetime body changed");
#endif
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
