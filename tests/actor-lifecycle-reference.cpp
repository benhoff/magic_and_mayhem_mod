#include "../reconstruction/rendering/actor_ownership.hpp"
#include "actor-lifecycle-providers.hpp"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <csignal>
#include <sys/resource.h>
#include <ucontext.h>
#ifdef MNM_ORIGINAL_REFERENCE
extern "C" void actor_predicate();
static void writeActorProviders();
static volatile unsigned RunningCase;
static void actorFault(int signal,siginfo_t* info,void* context){
    auto* cpu=static_cast<ucontext_t*>(context);char text[256];
    int n=std::snprintf(text,sizeof(text),"actor fixture signal=%d case=%u address=%p eip=%lx ecx=%lx edx=%lx esi=%lx\n",signal,RunningCase,info->si_addr,
        static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_EIP]),static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_ECX]),
        static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_EDX]),static_cast<unsigned long>(cpu->uc_mcontext.gregs[REG_ESI]));
    if(n>0)write(2,text,static_cast<std::size_t>(n));
    _exit(128+signal);
}
#endif
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
namespace a=mnm::reconstruction::actor_lifecycle;
using U=a::U;
constexpr U Pool=0x10000000,Map=0x10010000,Definition=0x10030000,
    Terrain=0x10031000,Columns=0x10032000,Manager=0x10040000,Callback=0x1004f000;
constexpr U Secondary=0x10020000;
struct Memory {
    std::vector<unsigned char> globals,owned;
    std::vector<U> events;
    U callbackMode{},callbackCalls{},maximumHealth=100,providerMode{};
    Memory():globals(0x102000),owned(0x50000) {
#ifdef MNM_ORIGINAL_REFERENCE
        if(mmap(reinterpret_cast<void*>(Pool),owned.size(),PROT_READ|PROT_WRITE|PROT_EXEC,
            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)
            throw std::runtime_error("owned low-address mapping");
#endif
    }
    unsigned char* pointer(U p,U n) {
        if(p>=0x600000 && U(p-0x600000)<=globals.size() && n<=globals.size()-(p-0x600000)) {
#ifdef MNM_ORIGINAL_REFERENCE
            return reinterpret_cast<unsigned char*>(p);
#else
            return globals.data()+p-0x600000;
#endif
        }
        if(p>=Pool && U(p-Pool)<=owned.size() && n<=owned.size()-(p-Pool)) {
#ifdef MNM_ORIGINAL_REFERENCE
            return reinterpret_cast<unsigned char*>(p);
#else
            return owned.data()+p-Pool;
#endif
        }
        throw std::runtime_error("actor memory extent");
    }
    U get32(U p){U v;std::memcpy(&v,pointer(p,4),4);return v;}
    U get16(U p){std::uint16_t v;std::memcpy(&v,pointer(p,2),2);return v;}
    U get8(U p){return *pointer(p,1);}
    void set32(U p,U v){std::memcpy(pointer(p,4),&v,4);}
    void set16(U p,U v){std::uint16_t w=std::uint16_t(v);std::memcpy(pointer(p,2),&w,2);}
    void set8(U p,U v){*pointer(p,1)=static_cast<unsigned char>(v);}
    void reset(U seed) {
#ifdef MNM_ORIGINAL_REFERENCE
        RunningCase=seed;
#endif
        events.clear();callbackCalls=0;
        std::memset(pointer(0x600000,U(globals.size())),0,globals.size());
        for(U i=0;i<0x50000;++i)*pointer(Pool+i,1)=static_cast<unsigned char>((i*73+seed*17)^(i>>5));
        #ifdef MNM_ORIGINAL_REFERENCE
        auto* code=reinterpret_cast<unsigned char*>(Callback);code[0]=0xe9;
        U jump=U(reinterpret_cast<std::uintptr_t>(actor_predicate))-Callback-5;
        std::memcpy(code+1,&jump,4);
        writeActorProviders();
#endif
        for(U i=0;i<8;++i){set32(Pool+i*a::Stride,i);set32(Pool+i*a::Stride+4,0);}
        set32(0x6def58,Pool);set32(0x6def5c,8);set32(0x6df180,8);
        set32(Manager,Pool);set32(Manager+4,8);set32(Manager+8,0);
        set32(Manager+12,Pool);set32(Manager+16,Callback);set32(Manager+20,0);
        set32(Manager+0x228,8);
    }
    U invokeFast(U id,U receiver,U second) {return invoke(id,receiver,{second});}
    U invoke(U id,U receiver,std::initializer_list<U> args) {
        events.insert(events.end(),{id,receiver,U(args.size())});
        events.insert(events.end(),args.begin(),args.end());
        if(id==Callback) {
            ++callbackCalls;
            U slot=(receiver-Pool)/a::Stride;
            if(callbackMode==2 && callbackCalls==1)set32(Manager+0x228,slot);
            if(callbackMode==3 && callbackCalls==1) {
                set32(Manager+12,Pool+6*a::Stride);set32(Manager+8,6);
            }
            return callbackMode==0?0:callbackMode==1?(slot&1):1;
        }
        if(id==0x50ab30) {
            // Owned callback control, not recovery of50ab30 itself.
            if(callbackMode&1)set32(receiver+0x5fc,20);
            if(callbackMode&2)set32(receiver+0x5f0,3);
            return 0;
        }
        if(id==0x50db60) {
            set32(receiver+0x5fc,*args.begin());return 0;
        }
        if(id==0x4943f0) {
            if(receiver<Secondary || receiver>=Secondary+4*0x22e)throw std::runtime_error("secondary provider extent");
            set32(receiver+4,0);return 0;
        }
        if(id==0x463cb0)return 0x10036000;
        if(id==0x474b40)return 7;
        if(id==0x5075f0)return maximumHealth;
        if(id==0x414890)return providerMode&1;
        if(id==0x5057b0)return 2;
        if(id==0x5055a0)return providerMode&1;
        if(id==0x466870)return providerMode&2?0x10036000:0;
        if(id==0x52d0c0) {
            U sentinel=get32(receiver+4);set32(receiver+8,0);
            set32(sentinel,sentinel);set32(sentinel+4,sentinel);return 0;
        }
        if(actorProvider(id))return 0;
        throw std::runtime_error("unknown actor provider");
    }
    void snapshot(U ordinal,U result,bool manager,bool map,bool lifecycle=false) {
        auto emit=[](const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);};
        U header[]={ordinal,result,U(events.size()),manager?1u:0u,map?1u:0u};emit(header,sizeof(header));
        emit(pointer(Pool,8*a::Stride),8*a::Stride);
        if(manager)emit(pointer(Manager,0x22c),0x22c);
        if(map)emit(pointer(Map-16,6*5*4*12+32),6*5*4*12+32);
        if(lifecycle) {
            emit(pointer(Secondary,4*0x22e),4*0x22e);
            for(const auto& range:std::array<std::array<U,2>,14>{{
                {{0x6def58,0x22c}},{{0x6e9218,0x650}},{{0x643a80,0x1100}},
                {{0x6de0e8,27*36}},{{0x6903e8,4}},{{0x690618,4}},{{0x6e007c,4}},
                {{0x6a46bc,4}},{{0x6c502f,4}},{{0x689498,0x448}},{{0x689920,4}},{{0x6dbe93,1}},
                {{0x6b13ed,30*0x2d1+8*36}},{{0x10033000,0x400}}}})
                emit(pointer(range[0],range[1]),range[1]);
        }
        emit(events.data(),events.size()*4);
    }
};
#ifdef MNM_ORIGINAL_REFERENCE
static Memory* active;
extern "C" __attribute__((force_align_arg_pointer)) U actor_provider(U id,U self,U argument) {
    try{return active->invoke(id,self,id==0x50db60?std::initializer_list<U>{argument}:std::initializer_list<U>{});}
    catch(...){_exit(83);}
}
#define BRIDGE(name,id,pop) extern "C" __attribute__((naked)) void name(){asm volatile( \
    "push %ebp\nmov %esp,%ebp\npush 8(%ebp)\npush %ecx\npush $" #id "\ncall actor_provider\nadd $12,%esp\nleave\nret $" #pop);}
BRIDGE(actor_predicate,0x1004f000,0)
BRIDGE(actor_reset,0x50ab30,0)
BRIDGE(actor_action,0x50db60,4)
extern "C" __attribute__((force_align_arg_pointer)) U actor_provider_full(U id,U self,U* arguments,U secondRegister) {
    try {
        auto* spec=actorProvider(id);if(!spec)_exit(84);
        if(!spec->receiver)self=0;
        if(spec->secondRegister)return active->invokeFast(id,self,secondRegister);
        if(id==0x520790)return active->invoke(id,self,{0x20000000});
        switch(spec->arguments) {
        case 0:return active->invoke(id,self,{});
        case 1:return active->invoke(id,self,{arguments[0]});
        case 2:return active->invoke(id,self,{arguments[0],arguments[1]});
        case 3:return active->invoke(id,self,{arguments[0],arguments[1],arguments[2]});
        case 4:return active->invoke(id,self,{arguments[0],arguments[1],arguments[2],arguments[3]});
        default:_exit(85);
        }
    } catch(...) {_exit(86);}
}
static U providerStub(unsigned index){return Callback+0x100+index*32;}
static void writeActorProviders(){
    for(unsigned i=0;i<sizeof(ActorProviders)/sizeof(ActorProviders[0]);++i) {
        auto* p=reinterpret_cast<unsigned char*>(providerStub(i));
        const auto& spec=ActorProviders[i];
        const unsigned char code[]={0x55,0x89,0xe5,0x52,0x8d,0x45,0x08,0x50,0x51,0x68};
        std::memcpy(p,code,sizeof(code));std::memcpy(p+10,&spec.address,4);p[14]=0xe8;
        U relative=U(reinterpret_cast<std::uintptr_t>(actor_provider_full))-providerStub(i)-19;
        std::memcpy(p+15,&relative,4);
        const unsigned char tail[]={0x83,0xc4,0x10,0xc9,0xc2};std::memcpy(p+19,tail,sizeof(tail));
        std::uint16_t pop=std::uint16_t(spec.pop);std::memcpy(p+24,&pop,2);
    }
}
static void bind(U at,void* function,const char* bytes) {
    unsigned char signature[5];for(unsigned i=0;i<5;++i){unsigned value;char hex[3]={bytes[i*2],bytes[i*2+1],0};value=unsigned(std::strtoul(hex,nullptr,16));signature[i]=static_cast<unsigned char>(value);}
    auto* p=reinterpret_cast<unsigned char*>(at);
    if(std::memcmp(p,signature,5))throw std::runtime_error("actor provider signature");
    p[0]=0xe9;U relative=U(reinterpret_cast<std::uintptr_t>(function))-at-5;std::memcpy(p+1,&relative,4);
}
#endif
static U poolCases(Memory& m) {
    U ordinal=0;
    for(U mask=0;mask<256;++mask)for(U cursor:{0u,1u,4u,7u,8u})for(U mode=0;mode<4;++mode)for(U kind=0;kind<3;++kind){
        m.reset(mask);m.callbackMode=mode;
        for(U i=0;i<8;++i)m.set32(Pool+i*a::Stride+4,(mask>>i)&1);
        m.set32(Manager+8,cursor);m.set32(Manager+12,Pool+cursor*a::Stride);
        m.set32(Manager+20,mode&1?0x30:0);
        U result;
#ifdef MNM_ORIGINAL_REFERENCE
        using Method=U(__attribute__((thiscall))*)(void*);
        result=reinterpret_cast<Method>(kind==0?0x52a640:kind==1?0x52a670:0x52a710)(reinterpret_cast<void*>(Manager));
#else
        result=kind==0?a::firstFree(m,Manager):a::nextActive(m,Manager,kind==2);
#endif
        m.snapshot(ordinal++,result,true,false);
    }
    return ordinal;
}
static U referenceCases(Memory& m) {
    U ordinal=0;
    for(U mask=0;mask<8;++mask)for(U health:{0u,1u})for(U action:{1u,10u,20u})
    for(U task:{1u,3u,9u,10u})for(U type:{5u,17u})for(U engaged:{0u,1u})
    for(U counter:{0u,1u,0x7fffffffu,0x80000000u,0xffffffffu})for(U mode=0;mode<4;++mode){
        m.reset(ordinal);m.callbackMode=mode;U actor=Pool+a::Stride,removed=3;
        m.set32(actor+0x906,mask&1?removed:2);m.set32(actor+0x64c,mask&2?removed:2);
        m.set32(actor+0x6a8,mask&4?removed:2);m.set32(actor+0xe4,health);
        m.set32(actor+0x5fc,action);m.set32(actor+0x5ec,task);m.set32(actor+0x5f0,3);
        m.set32(actor+0xa8,type);m.set32(actor+0x664,engaged);m.set32(Pool+removed*a::Stride+0x668,counter);
#ifdef MNM_ORIGINAL_REFERENCE
        using Method=void(__attribute__((thiscall))*)(void*,U);
        reinterpret_cast<Method>(0x508b00)(reinterpret_cast<void*>(actor),removed);
#else
        a::invalidate(m,actor,removed);
#endif
        m.snapshot(ordinal++,0,false,false);
    }
    return ordinal;
}
static U mapCases(Memory& m) {
    U ordinal=0;
    for(U kind=0;kind<3;++kind)for(U x:{0u,4u,5u})for(U y:{0u,3u,4u})for(U z:{0u,1u})
    for(U wide:{1u,2u})for(U height:{1u,2u,3u})for(U state=0;state<8;++state)
    for(U column=0;column<4;++column){
        m.reset(ordinal);m.set32(Pool,3);m.set32(Pool+0xac,Definition);
        m.set32(Definition+8,wide);m.set32(Definition+12,height);
        m.set32(0x6c5494,6);m.set32(0x6c5498,5);m.set32(0x6c54a0,30);m.set32(0x6c54dc,Map);
        m.set32(0x65660c,Terrain);m.set32(Terrain+0x164+0x94,state&4?16:0);
        m.set32(0x6a49c0,column?Columns:0);
        for(U i=0;i<5;++i)m.set32(0x6cb942+i*4,i*6);
        for(U i=0;i<4;++i)m.set32(0x6cb8c2+i*4,i*30);
        for(U i=0;i<30;++i){m.set16(Columns+i*6,column==1?0:1);m.set8(Columns+i*6+2,column==3?0xff:z);m.set8(Columns+i*6+3,z+1);}
        for(U i=0;i<120;++i) {
            U cell=Map+i*12;m.set16(cell,state&4?1:0);m.set16(cell+2,0xffff);
            m.set16(cell+4,state&1?3:7);m.set16(cell+6,0xffff);
            m.set16(cell+10,(state&2?0x83:0)|(i&1?0x2000:0));
        }
#ifdef MNM_ORIGINAL_REFERENCE
        using Method=void(__attribute__((thiscall))*)(void*,U,U,U);
        if(kind!=1)reinterpret_cast<Method>(0x509230)(reinterpret_cast<void*>(Pool),x,y,z);
        if(kind!=0)reinterpret_cast<Method>(0x5093e0)(reinterpret_cast<void*>(Pool),x,y,z);
#else
        if(kind!=1)a::attach(m,Pool,x,y,z);
        if(kind!=0)a::detach(m,Pool,x,y,z);
#endif
        m.snapshot(ordinal++,0,false,true);
    }
    return ordinal;
}
static U releaseCases(Memory& m) {
    U ordinal=0;
    for(U mask=0;mask<16;++mask)for(U root=0;root<4;++root)for(U type:{0u,24u,25u,26u,17u})
    for(U mode=0;mode<4;++mode)for(U kind=0;kind<3;++kind) {
        m.reset(ordinal);m.callbackMode=mode&1;U actor=Pool+root*a::Stride;
        m.set32(0x6c5494,6);m.set32(0x6c5498,5);m.set32(0x6c54a0,30);m.set32(0x6c54dc,Map);
        m.set32(0x65660c,Terrain);m.set32(0x6a49c0,0);
        for(U i=0;i<5;++i)m.set32(0x6cb942+i*4,i*6);
        for(U i=0;i<4;++i)m.set32(0x6cb8c2+i*4,i*30);
        m.set32(0x689498,Secondary);m.set32(0x6898dc,4);m.set32(0x689920,mode%3);
        m.set8(0x6dbe93,mode&1);m.set32(0x644520,mode&1);m.set32(0x6e984c,mode);
        m.set32(0x6e1f0c,1);m.set32(0x6df180,4);
        for(U i=0;i<8;++i){m.set32(0x643a80+i*0x154,i<mode?1:0);m.set32(0x643a80+i*0x154+12,i&1?0xffffffffu:i);}
        m.set32(Definition,type);m.set32(Definition+8,mode&1?2:1);m.set32(Definition+12,mode&2?2:1);
        for(U i=0;i<4;++i) {
            U peer=Pool+i*a::Stride;m.set32(peer+4,(mask>>i)&1);m.set32(peer+0xa8,type);
            m.set32(peer+0xac,Definition);m.set32(peer+0x7c,i==root?(mode>>1):1);
            m.set32(peer+8,i);m.set32(peer+12,i);m.set32(peer+16,0);
            m.set32(peer+0x618,mode&1);m.set32(peer+0x977,0);
            m.set32(peer+0x97f,(i+1)%6);m.set32(peer+0x983,(i+1)%5);m.set32(peer+0x987,1);
            m.set32(peer+0xe4,100);m.set32(peer+0x68,mode&1);m.set32(peer+0x70,1);
            m.set32(peer+0x64c,root);m.set32(peer+0x6a8,root);m.set32(peer+0x906,root);
            m.set32(peer+0x664,1);m.set32(peer+0x668,mode);m.set32(peer+0x5fc,i&1?10:20);
            m.set32(peer+0x5ec,3);m.set32(peer+0x5f0,3);m.set32(peer+0x174,i&1);
            m.set32(peer+0x124,mode&1?2:0);m.set32(peer+0x967,mode&2?0x10037000:0);
            m.set32(peer+0x798,mode&1);m.set32(peer+0x6e8,mode&1?Secondary+i*0x22e:0);
            m.set32(peer+0x678,mode&1?0x10036000:0);m.set32(peer+0x67c,0);
            m.set32(peer+0x7cd,mode);m.set32(peer+0x7d1,mode&1?Secondary+i*0x22e:0);
            m.set32(peer+0x805,mode&1?101+i:0);m.set32(peer+0x794,mode&1?201+i:0);
            m.set32(peer+0x120,mode&1?301+i:0);
            U effect=Secondary+i*0x22e;m.set32(effect+4,1);m.set32(effect+0x48,root);m.set32(effect+0x4c,i&1?50:49);
        }
        for(U i=0;i<120;++i){U cell=Map+i*12;m.set16(cell,0);m.set16(cell+2,0xffff);m.set16(cell+4,i%4);m.set16(cell+6,0xffff);m.set16(cell+10,0x83);}
        for(U i=0;i<9;++i)m.set32(0x6de0e8+type*36+i*4,mode&1?0x10038000+i*0x40:0);
#ifdef MNM_ORIGINAL_REFERENCE
        using Cleanup=void(__attribute__((thiscall))*)(void*,U);
        using Release=void(__attribute__((thiscall))*)(void*);
        if(kind==0)reinterpret_cast<Cleanup>(0x51f5e0)(reinterpret_cast<void*>(actor),mode&1);
        else {
            reinterpret_cast<Release>(0x506d30)(reinterpret_cast<void*>(actor));
            if(kind==2)reinterpret_cast<Release>(0x506d30)(reinterpret_cast<void*>(actor));
        }
#else
        if(kind==0)a::cleanup(m,actor,mode&1);
        else {a::release(m,actor);if(kind==2)a::release(m,actor);}
#endif
        m.snapshot(ordinal++,0,false,true,true);
    }
    return ordinal;
}
static U activationCases(Memory& m) {
    U ordinal=0;
    for(U type:{0u,13u,17u,24u,25u,26u})for(U preloaded:{0u,1u,0x100u})
    for(U maximum:{0u,1u,100u})for(U mode=0;mode<4;++mode)for(U nodes:{0u,1u,3u})
    for(U owner:{0u,1u})for(U kind=0;kind<2;++kind) {
        m.reset(ordinal);m.maximumHealth=maximum;m.providerMode=mode;m.callbackMode=0;
        U actor=Pool+owner*a::Stride,definition=0x6a5f80+type*0x5c9;
        m.set32(actor+0xac,definition);m.set32(actor+0xa8,type);m.set32(actor+0x174,owner);
        m.set32(actor+8,kind==0&&mode&1?0xffffffffu:owner);m.set32(actor+12,owner);m.set32(actor+16,0);
        m.set32(actor+0xa0,mode&1?2:0);m.set32(actor+0x8f9,0x10033000);
        m.set32(0x10033004,0x10033100);m.set32(0x10033008,nodes);
        m.set32(0x10033100,nodes?0x10033200:0x10033100);
        m.set32(0x10033104,nodes?0x10033200+(nodes-1)*48:0x10033100);
        for(U i=0;i<nodes;++i) {
            U node=0x10033200+i*48;m.set32(node,i+1<nodes?node+48:0x10033100);
            m.set32(node+4,i?node-48:0x10033100);m.set32(node+8,i&1?1:2);
            m.set32(node+40,Secondary+i*0x22e);
        }
        m.set32(definition,type);m.set32(definition+8,mode&1?2:1);m.set32(definition+12,mode&2?2:1);
        m.set32(definition+24,37);m.set32(definition+20,0x1234);m.set32(definition+40,0x5678);
        m.set32(definition+0x58,77);m.set32(definition+0x3c,88);
        m.set32(definition+0x74,mode==0?0:mode==1?17:mode==2?24:0xffffffffu);
        m.set32(definition+0x7c,mode&1?0x80000000u:0xffffffffu);
        m.set32(definition+0x8c,mode&2?0x7fffffffu:2);
        m.set32(0x6c5494,6);m.set32(0x6c5498,5);m.set32(0x6c54a0,30);m.set32(0x6c54dc,Map);
        m.set32(0x65660c,Terrain);m.set32(0x6a49c0,0);m.set32(0x6c4830,0xabcdef01);
        m.set32(0x6e9850,mode==3?0xffffffffu:mode);m.set32(0x6df180,mode&2?4:0);
        m.set32(0x10036008,3);m.set32(0x1003600c,4);m.set32(0x656614,0x12345678);
        for(U i=0;i<5;++i)m.set32(0x6cb942+i*4,i*6);
        for(U i=0;i<4;++i)m.set32(0x6cb8c2+i*4,i*30);
        for(U i=0;i<120;++i){U cell=Map+i*12;m.set16(cell,0);m.set16(cell+2,0xffff);m.set16(cell+4,0xffff);m.set16(cell+6,0xffff);m.set16(cell+10,0);}
#ifdef MNM_ORIGINAL_REFERENCE
        using Baseline=void(__attribute__((thiscall))*)(void*);
        using Activate=void(__attribute__((thiscall))*)(void*,U,U,U);
        if(kind==0)reinterpret_cast<Baseline>(0x5065e0)(reinterpret_cast<void*>(actor));
        else reinterpret_cast<Activate>(0x506290)(reinterpret_cast<void*>(actor),type,owner,preloaded);
#else
        if(kind==0)a::baseline(m,actor);else a::activate(m,actor,type,owner,preloaded);
#endif
        m.snapshot(ordinal++,0,false,true,true);
    }
    return ordinal;
}
int main(int argc,char** argv)try {
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("original requires image and suite");
    struct rlimit limit{0,0};setrlimit(RLIMIT_CORE,&limit);
    struct sigaction action{};action.sa_sigaction=actorFault;action.sa_flags=SA_SIGINFO;sigemptyset(&action.sa_mask);sigaction(SIGSEGV,&action,nullptr);
    map_image(read(argv[1]));const std::string suite=argv[2];
#else
    if(argc!=2)throw std::runtime_error("native requires suite");
    const std::string suite=argv[1];
#endif
#ifdef MNM_ORIGINAL_REFERENCE
    const std::array<std::array<U,2>,14> ranges={{{{0x508b00,0x508c95}},{{0x509230,0x5093da}},{{0x5093e0,0x50962a}},{{0x52a640,0x52a661}},{{0x52a670,0x52a6b8}},{{0x52a710,0x52a773}},{{0x534520,0x5345e3}},{{0x506d30,0x5070a8}},{{0x51f5e0,0x51fa5c}},{{0x411e50,0x411e6e}},{{0x506290,0x5065d7}},{{0x5065e0,0x506d25}},{{0x40e8a0,0x40e8c8}},{{0x40e290,0x40e2b8}}}};
    std::vector<std::vector<unsigned char>> before;
    for(const auto& range:ranges)before.emplace_back(reinterpret_cast<unsigned char*>(range[0]),reinterpret_cast<unsigned char*>(range[1]));
#endif
    Memory memory;
#ifdef MNM_ORIGINAL_REFERENCE
    active=&memory;
    auto* callback=reinterpret_cast<unsigned char*>(Callback);callback[0]=0xe9;U relative=U(reinterpret_cast<std::uintptr_t>(actor_predicate))-Callback-5;std::memcpy(callback+1,&relative,4);
    writeActorProviders();
    for(unsigned i=0;i<sizeof(ActorProviders)/sizeof(ActorProviders[0]);++i) {
        if(ActorProviders[i].address==0x5c5208 || ActorProviders[i].address==0x5c520c)
            global(ActorProviders[i].address,providerStub(i));
        else bind(ActorProviders[i].address,reinterpret_cast<void*>(providerStub(i)),ActorProviders[i].bytes);
    }
#endif
    U cases=suite=="pool"?poolCases(memory):suite=="references"?referenceCases(memory):suite=="map"?mapCases(memory):suite=="release"?releaseCases(memory):suite=="activation"?activationCases(memory):0;
#ifdef MNM_ORIGINAL_REFERENCE
    for(std::size_t i=0;i<ranges.size();++i)if(std::memcmp(before[i].data(),reinterpret_cast<void*>(ranges[i][0]),before[i].size()))throw std::runtime_error("selected original body mutated");
#endif
    if(!cases)throw std::runtime_error("unknown suite");
    std::cerr<<"cases "<<cases<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
