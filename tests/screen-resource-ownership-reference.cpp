#include "../reconstruction/rendering/screen_resource_ownership.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U=std::uint32_t;
constexpr U Base=0x720000, Owner=Base+0x100, FakeTable=Base+0xf000;
constexpr U Resource=Base+0xe000, Alternate=Base+0xe100, Size=0x10000;
constexpr U Tables[]={0x5c5ebc,0x5c5ef4,0x5c6120,0x5c63e4,0x5c641c,0x5c6454,
    0x5c648c,0x5c64c4,0x5c64fc,0x5c6534,0x5c659c,0x5c65d4,0x5c660c,
    0x5c6644,0x5c667c,0x5c66b4,0x5c66ec,0x5c67f8,0x5c689c,0x5c6940,0x5c6978};
constexpr U Entries[]={0x473c10,0x474fe0,0x486260,0x4a7030,0x4a7d60,0x4a84e0,
    0x4a8d00,0x4a9e20,0x4aae70,0x4ac510,0x4af070,0x4b0060,0x4b0f50,
    0x4b2010,0x4b2ae0,0x4b40c0,0x4b70d0,0x4b8730,0x4b95b0,0x4bb580,0x4bc1c0};
constexpr U Ends[]={0x473d01,0x4750f1,0x486313,0x4a7107,0x4a7e37,0x4a84fa,
    0x4a8e31,0x4a9f65,0x4aafb5,0x4ac693,0x4af1b6,0x4b0161,0x4b10a3,
    0x4b20fe,0x4b2c0d,0x4b42ba,0x4b7236,0x4b87c8,0x4b96b9,0x4bb685,0x4bc29f};
using BytesOwned=std::array<unsigned char,Size>;
static U currentTable,currentProfile,currentMutation;
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Memory {
    BytesOwned bytes{}; bool original=false;
    unsigned char* at(U address,U n=4) {
        if(!(address>=Base && address-Base<=Size-n)) { std::cerr<<"address "<<std::hex<<address<<" table "<<currentTable<<std::dec<<" profile "<<currentProfile<<" mutation "<<currentMutation<<" original "<<original<<"\n"; throw std::runtime_error("Unadmitted storage"); }
#ifdef MNM_ORIGINAL_REFERENCE
        if(original)return reinterpret_cast<unsigned char*>(address);
#endif
        return bytes.data()+address-Base;
    }
    U get(U a){U v;std::memcpy(&v,at(a),4);return v;}
    void set(U a,U v){std::memcpy(at(a),&v,4);}
    void byte(U a,U v){*at(a,1)=v;}
    BytesOwned snapshot(){BytesOwned b;std::memcpy(b.data(),at(Base,Size),Size);return b;}
};
struct Call {
    U kind,a,b,c,d; BytesOwned bytes;
    bool operator==(const Call& x)const{return kind==x.kind&&a==x.a&&b==x.b&&c==x.c&&d==x.d&&bytes==x.bytes;}
};
static Memory* memory;
static std::vector<Call> calls;
static U mutation,events,allocations;static bool retargeted;
static U arrayAt(U offset,bool alternate=false){return Base+0x1000+(offset/4)*0x100+(alternate?0x5000:0);}
static void event(U kind,U a,U b=0,U c=0,U d=0) {
    calls.push_back({kind,a,b,c,d,memory->snapshot()});
    if(mutation==1 && kind==1 && a==0x58b3e0 && b!=Alternate) memory->set(Owner+0x13,Alternate);
    if(mutation==2 && kind==2 && !retargeted) {
        retargeted=true;
        // Retarget all pointer arrays to separately admitted copies before the
        // caller clears its old element and before the next iteration loads.
        for(U offset=0x3b;offset<=0xb7;offset+=4)
            if(memory->get(Owner+offset)==arrayAt(offset))memory->set(Owner+offset,arrayAt(offset,true));
    }
    if(mutation==3 && kind==1 && a==0x475c10)memory->set(Owner+0x57,1);
    if(mutation==4) {memory->set(Owner+8,7);memory->set(Owner+0x2f,123);}
    if(mutation==5 && kind==1 && a==0x58b3e0)memory->set(Owner+0x13,0);
    ++events;
}
struct Services {
    U allocate(U size){event(7,size);return Base+0xc000+(allocations++)*0x100;}
    void destruct(U function,U receiver){event(1,function,receiver);}
    void free(U receiver){event(2,receiver);}
    void dispose(U receiver,U flags){need(flags==1,"Dispose flag");event(3,receiver,flags);}
    void destroyArray(U pointer,U stride,U count,U function){event(4,pointer,stride,count,function);}
    void freeArray(U receiver){event(5,receiver);}
    void bitmapPlane(U receiver,U value){event(6,receiver,value);}
};
#ifdef MNM_ORIGINAL_REFERENCE
template<U Function> static void __attribute__((thiscall,force_align_arg_pointer)) destructorAdapter(void* receiver){Services{}.destruct(Function,U(receiver));}
static U __attribute__((force_align_arg_pointer)) allocateAdapter(U size){return Services{}.allocate(size);}
static void __attribute__((force_align_arg_pointer)) freeAdapter(U address){Services{}.free(address);}
static void __attribute__((force_align_arg_pointer)) arrayFreeAdapter(U address){Services{}.freeArray(address);}
static void __attribute__((thiscall,force_align_arg_pointer)) disposeAdapter(void* address,U flags){Services{}.dispose(U(address),flags);}
static void __attribute__((stdcall,force_align_arg_pointer)) arrayAdapter(U address,U stride,U count,U destructor){Services{}.destroyArray(address,stride,count,destructor);}
static void __attribute__((thiscall,force_align_arg_pointer)) planeAdapter(void* address,U value){Services{}.bitmapPlane(U(address),value);}
static U originalMenuCall(U entry){using Fn=U(__attribute__((thiscall))*)(void*);return reinterpret_cast<Fn>(entry)((void*)Owner);}
static void adapt(U address,void* fn){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;const U delta=U(fn)-address-5;std::memcpy(p+1,&delta,4);}
#endif
static Memory input(U table,U seed,U profile) {
    Memory m;m.bytes.fill(seed);m.set(Owner,table);m.set(Owner+8,profile==0?0:1);
    for(U offset=0x3b;offset<=0xb7;offset+=4) {
        const U a=arrayAt(offset), alt=arrayAt(offset,true);
        m.set(a-4,profile%4);m.set(alt-4,profile%4);
        for(U i=0;i<48;++i) {
            const U value=(profile==1 || ((profile+i+offset)&3)==0)?0:Resource;
            m.set(a+4*i,i?value:Resource);m.set(alt+4*i,i?value:Resource);
        }
        const bool present=profile<3 || ((offset/4+profile)%5)!=0;
        m.set(Owner+offset,present?a:0);
    }
    // Non-null pointer arrays are required even when every element is null.
    for(U offset=0x4b;offset<=0x5f;offset+=4) m.set(Owner+offset,arrayAt(offset));
    if(table==0x5c67f8||table==0x5c689c)for(U offset=0xa0;offset<=0xb4;offset+=4)m.set(Owner+offset,arrayAt(offset));
    if(table==0x5c6644)m.set(Owner+0x57,profile==0?0:(profile==1?0xffffffff:profile%6));
    if(table==0x5c66b4) {
        for(U offset=0x47;offset<=0x67;offset+=4)m.set(Owner+offset,profile==1?0:Resource);
        m.set(Owner+0x11b,profile==1?0:Resource);
    }
    m.set(Owner+0x13,profile==1?0:Resource);
    m.set(Owner+0x23,profile==1?0:Base+0x800);
    m.set(Owner+0x27,profile==1?0:Base+0x800+profile%5*4);
    m.set(Owner+0x2b,Base+0x880);m.set(Owner+0x2f,89);
    m.set(Resource,FakeTable);m.set(Alternate,FakeTable);
#ifdef MNM_ORIGINAL_REFERENCE
    // One table for direct callback fields and one for array entries.
    m.set(FakeTable+4,U((void*)disposeAdapter));m.set(Resource+4,U((void*)disposeAdapter));
#else
    m.set(FakeTable+4,0x12345678);m.set(Resource+4,0x12345678);
#endif
    return m;
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    need(argc==2,"Pinned PE required");map_image(read(argv[1]));
    need(mmap(reinterpret_cast<void*>(Base),Size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"Owned map");
    std::array<unsigned char,4096> thread{};user_desc segment{};segment.entry_number=U(-1);
    segment.base_addr=U(thread.data());segment.limit=0xfffff;
    segment.seg_32bit=segment.limit_in_pages=segment.useable=1;
    need(!syscall(SYS_set_thread_area,&segment),"Owned SEH segment");
    unsigned short selector=(segment.entry_number<<3)|3;asm volatile("movw %0, %%fs"::"rm"(selector));
    adapt(0x597850,(void*)allocateAdapter);
    adapt(0x597870,(void*)freeAdapter);adapt(0x5979b0,(void*)arrayFreeAdapter);
    adapt(0x59bf07,(void*)arrayAdapter);adapt(0x57d0d0,(void*)planeAdapter);
#define ADAPT(pc) adapt(pc,(void*)destructorAdapter<pc>)
    ADAPT(0x58b3e0);ADAPT(0x475c10);ADAPT(0x4d27d0);ADAPT(0x4d2a30);
    ADAPT(0x4ccd80);ADAPT(0x4d13b0);ADAPT(0x4cefa0);ADAPT(0x4cb560);
    ADAPT(0x4cdbc0);ADAPT(0x4cea90);ADAPT(0x58a0a0);ADAPT(0x4ce6d0);
    ADAPT(0x4d1ff0);ADAPT(0x559880);ADAPT(0x572400);
#undef ADAPT
    std::vector<Bytes> bodies;
    for(U i=0;i<std::size(Entries);++i)bodies.emplace_back((unsigned char*)Entries[i],(unsigned char*)Ends[i]);
    const std::pair<U,U> otherRanges[]={{0x4a6a60,0x4a6afd},{0x4a6b00,0x4a6b67},{0x4a7970,0x4a7a0a},{0x4a7a10,0x4a7a77},{0x4b1d80,0x4b1e2b},{0x4b1e30,0x4b1eae},{0x4cab50,0x4cab69},{0x4cab70,0x4cabcc},{0x4748a0,0x47493a},{0x473b00,0x473c10},{0x474d70,0x474e62}};
    std::vector<Bytes> otherBodies;for(auto range:otherRanges)otherBodies.emplace_back((unsigned char*)range.first,(unsigned char*)range.second);
    const Bytes vectorBefore((unsigned char*)0x4cacb0,(unsigned char*)0x4cacf1);
#else
    (void)argv;need(argc==1,"Native arguments");
#endif
    std::uint64_t checksum=0;U cases=0,menuCases=0,reportCases=0,retargetedCases=0;
    for(U index=0;index<std::size(Tables);++index)for(U seed:{0u,0xa5u,0xffu})
    for(U profile=0;profile<9;++profile)for(U mut=0;mut<6;++mut)for(U repeat=0;repeat<(mut?1u:2u);++repeat) {
        // Dynamic-count mutation is only admitted for the Mini pointer count.
        if(mut==3 && Tables[index]!=0x5c6644)continue;
        currentTable=Tables[index];currentProfile=profile;currentMutation=mut;
        auto start=input(Tables[index],seed,profile);auto expected=start;
        mutation=mut;memory=&expected;calls.clear();events=0;retargeted=false;
        Services services;
        for(U n=0;n<=repeat;++n)need(mnm::reconstruction::releaseOriginalScreenResources(expected,services,Tables[index],Owner),"Known table rejected");
        const auto expectedCalls=calls;if(retargeted)++retargetedCases;
#ifdef MNM_ORIGINAL_REFERENCE
        std::memcpy((void*)Base,start.bytes.data(),Size);Memory actual;actual.original=true;
        memory=&actual;calls.clear();events=0;retargeted=false;
        using Fn=void(__attribute__((thiscall))*)(void*);
        for(U n=0;n<=repeat;++n)reinterpret_cast<Fn>(Entries[index])((void*)Owner);
        if(actual.snapshot()!=expected.snapshot() || calls!=expectedCalls) {
            std::cerr<<"table "<<std::hex<<Tables[index]<<std::dec<<" profile "<<profile<<" seed "<<seed<<" mutation "<<mut<<" calls "<<calls.size()<<"/"<<expectedCalls.size()<<'\n';
            throw std::runtime_error("Resource release mismatch");
        }
#endif
        // Hash only normalized semantic memory; native function addresses are
        // fixture bindings, excluded from the cross-configuration checksum.
        expected.set(FakeTable+4,0);expected.set(Resource+4,0);
        for(auto b:expected.snapshot())checksum=checksum*131+b;
        for(const auto& c:expectedCalls){for(U v:{c.kind,c.a,c.b,c.c,c.d})checksum=checksum*131+v;}
        ++cases;
    }
    constexpr U menuTables[]={0x5c63e4,0x5c641c,0x5c6644};
    [[maybe_unused]] constexpr U constructors[]={0x4a6a60,0x4a7970,0x4b1d80};
    [[maybe_unused]] constexpr U destructors[]={0x4a6b00,0x4a7a10,0x4b1e30};
    for(U index=0;index<3;++index)for(U seed:{0u,0xa5u,0xffu})
    for(U profile=0;profile<9;++profile)for(U mut:{0u,1u,2u,4u,5u})for(U operation=0;operation<3;++operation) {
        currentTable=menuTables[index];currentProfile=profile;currentMutation=mut;
        auto start=input(currentTable,seed,profile),expected=start;Services services;
        mutation=mut;memory=&expected;calls.clear();events=allocations=0;retargeted=false;
        auto modelConstruct=[&]{need(mnm::reconstruction::constructOriginalCoreMenu(expected,services,currentTable,Owner),"Menu construction rejected");};
        auto modelDestroy=[&]{need(mnm::reconstruction::destroyOriginalCoreMenu(expected,services,currentTable,Owner),"Menu destruction rejected");};
        if(operation==0)modelConstruct();
        else if(operation==1)modelDestroy();
        else for(U n=0;n<2;++n){modelConstruct();modelDestroy();}
        const auto expectedCalls=calls;if(retargeted)++retargetedCases;
#ifdef MNM_ORIGINAL_REFERENCE
        std::memcpy((void*)Base,start.bytes.data(),Size);Memory actual;actual.original=true;
        memory=&actual;calls.clear();events=allocations=0;retargeted=false;
        auto construct=[&]{need(originalMenuCall(constructors[index])==Owner,"Constructor return");};
        auto destroy=[&]{originalMenuCall(destructors[index]);};
        if(operation==0)construct();else if(operation==1)destroy();else for(U n=0;n<2;++n){construct();destroy();}
        need(*reinterpret_cast<U*>(thread.data())==0,"Original SEH chain not restored");
        if(actual.snapshot()!=expected.snapshot() || calls!=expectedCalls){
            std::cerr<<"menu "<<std::hex<<currentTable<<std::dec<<" operation "<<operation<<" profile "<<profile<<" mutation "<<mut<<" calls "<<calls.size()<<"/"<<expectedCalls.size()<<'\n';
            throw std::runtime_error("Enclosing menu lifetime mismatch");
        }
#endif
        expected.set(FakeTable+4,0);expected.set(Resource+4,0);
        for(auto b:expected.snapshot())checksum=checksum*131+b;
        for(const auto& c:expectedCalls)for(U v:{c.kind,c.a,c.b,c.c,c.d})checksum=checksum*131+v;
        ++menuCases;
    }
    for(U seed:{0u,0xa5u,0xffu})for(U prof=0;prof<9;++prof)for(U mut:{0u,1u,2u,4u,5u})for(U operation=0;operation<3;++operation) {
        const U table=operation==2?0x5c5ef4:0x5c5ebc;
        currentTable=table;currentProfile=prof;currentMutation=mut;
        auto start=input(table,seed,prof);start.set(Owner+0x2a5,prof==1?0:Base+0xd000);
        for(U i=0;i<9;++i)start.set(Base+0xd000+4*i,(prof+i)%3?Resource:0);
        auto expected=start;Services services;memory=&expected;mutation=mut;calls.clear();events=allocations=0;retargeted=false;
        if(operation==0)mnm::reconstruction::exitOriginalCampaignReport(expected,services,Owner);
        else need(mnm::reconstruction::destroyOriginalResultScreen(expected,services,table,Owner),"Result destructor rejected");
        const auto expectedCalls=calls;if(retargeted)++retargetedCases;
#ifdef MNM_ORIGINAL_REFERENCE
        std::memcpy((void*)Base,start.bytes.data(),Size);Memory actual;actual.original=true;memory=&actual;calls.clear();events=allocations=0;retargeted=false;
        originalMenuCall(operation==0?0x4748a0:operation==1?0x473b00:0x474d70);
        need(*reinterpret_cast<U*>(thread.data())==0,"Report SEH chain not restored");
        if(actual.snapshot()!=expected.snapshot()||calls!=expectedCalls){std::cerr<<"report operation "<<operation<<" profile "<<prof<<" mutation "<<mut<<'\n';throw std::runtime_error("Result lifetime mismatch");}
#endif
        expected.set(FakeTable+4,0);expected.set(Resource+4,0);
        for(auto b:expected.snapshot())checksum=checksum*131+b;
        for(const auto& c:expectedCalls)for(U v:{c.kind,c.a,c.b,c.c,c.d})checksum=checksum*131+v;
        ++reportCases;
    }
    need(retargetedCases>0,"Array retarget schedules not exercised");
    Memory unknown;Services p;need(!mnm::reconstruction::releaseOriginalScreenResources(unknown,p,0x5c6564,Owner),"Purecall admitted as concrete");
#ifdef MNM_ORIGINAL_REFERENCE
    for(U i=0;i<std::size(Entries);++i)need(Bytes((unsigned char*)Entries[i],(unsigned char*)Ends[i])==bodies[i],"Original entry changed");
    need(Bytes((unsigned char*)0x4cacb0,(unsigned char*)0x4cacf1)==vectorBefore,"Original vector body changed");
    for(U i=0;i<std::size(otherRanges);++i)need(Bytes((unsigned char*)otherRanges[i].first,(unsigned char*)otherRanges[i].second)==otherBodies[i],"Original menu/report lifetime body changed");
#endif
    std::cout<<"{\"success\":true,\"tables\":21,\"cases\":"<<cases<<",\"menu_cases\":"<<menuCases<<",\"report_cases\":"<<reportCases<<",\"retargeted_cases\":"<<retargetedCases<<",\"checksum\":"<<checksum<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
