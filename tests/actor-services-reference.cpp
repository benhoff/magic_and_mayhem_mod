#include "../reconstruction/rendering/actor_services.hpp"
#include "../reconstruction/rendering/actor_service_controllers.hpp"
#include "../reconstruction/rendering/actor_service_world.hpp"
#include "../reconstruction/rendering/actor_service_ui.hpp"
#include "../reconstruction/rendering/actor_service_dependencies.hpp"
#include "../reconstruction/rendering/actor_service_popups.hpp"
#include "../reconstruction/rendering/actor_service_collections.hpp"
#include "../reconstruction/rendering/actor_service_effects.hpp"
#include "../reconstruction/rendering/actor_service_construction.hpp"
#include "../reconstruction/rendering/actor_service_composition.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <signal.h>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <ucontext.h>
#include "actor-services-generated.hpp"
#ifdef MNM_ORIGINAL_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#include "original-linux-seh.hpp"
extern "C" unsigned actor_services_cleanup(unsigned);
extern "C" unsigned actor_services_call(unsigned,unsigned,unsigned,unsigned,const unsigned*,unsigned);
#endif
namespace s=mnm::reconstruction::actor_services;
using U=s::U;
constexpr U Base=0x10000000,Object=Base,Other=Base+0x2000,Manager=Base+0x4000,
    Pool=Base+0x5000,Catalog=Base+0xa000,Array=Base+0xe000,
    Nodes=Base+0xf000,Buffer=Base+0x12000,Output=Base+0x14000,Code=Base+0x20000;
constexpr U GetStatus=0x1001f000,Stop=0x1001f040,Position=0x1001f080,Constructor=0x1001f100;
static constexpr std::array<std::array<U,2>,76> Globals{{
    {{0x5e1900,0x40}},{{0x5fbf00,0x40}},{{0x5fcc80,4}},{{0x643a80,0xb00}},
    {{0x65b250,0x1500}},{{0x689920,4}},{{0x6a4646,4}},{{0x6def58,0x240}},
    {{0x6e1f88,4}},{{0x6e9848,12}},{{0x6f2a94,4}},{{0x6f437c,4}},
    {{0x6fa78c,16}},{{0x6e0000,16}},{{0x5c7144,0x120}},{{0x6dbe9d,1}},{{0x5d9e38,4}},{{0x5e16fc,4}},{{0x6de6d5,8}},{{0x6c482c,4}},{{0x6c4859,1}},{{0x6903e8,16}},{{0x689498,0x450}},{{0x6b1270,0x1000}},{{0x5fed7c,4}},{{0x6c5040,0x110}},{{0x65660c,4}},{{0x6c5494,0x50}},{{0x6cb8be,0x100}},{{0x6cb942,0x100}},{{0x6b1248,8}},{{0x5e5594,0x34}},{{0x6a5fd8,0xa600}},{{0x6bb743,4}},{{0x6e1ee4,4}},{{0x6e2032,1}},{{0x6e1f0c,4}},{{0x6dbf00,0x2700}},{{0x6db967,0x300}},{{0x6f37f8,8}},{{0x6e9808,4}},{{0x5e04cc,4}},{{0x641ab8,0x600}},{{0x641fbc,4}},{{0x5f2094,0x58}},{{0x5f1ec8,4}},{{0x6f6b50,0x2d88}},{{0x6593e0,16}},{{0x6903dc,4}},{{0x6c4e3c,4}},{{0x6a4642,0x40}},{{0x6c4e40,0x200}},{{0x6e1f3c,4}},{{0x6c5c88,4}},{{0x6f3a70,0x30}},{{0x6b1250,8}},{{0x6a42e8,4}},{{0x6f694c,0x204}},{{0x642034,0xc38}},{{0x6e1f1c,4}},{{0x658174,4}},{{0x6a2dc8,4}},{{0x6def50,4}},{{0x6cbb6c,4}},{{0x6a49b8,4}},{{0x656618,4}},{{0x5eb480,12}},{{0x6c4830,4}},{{0x6cbb52,8}},{{0x6f3808,60}},{{0x5eb940,1000}},{{0x6574c8,1008}},{{0x5c73f0,4}},{{0x5c7404,4}},{{0x6cbb8a,0x1c08}},{{0x6c4630,0x2a}}
}};
struct Memory;
static Memory* active;
static volatile U runningEntry,runningVariant;
static void fault(int,siginfo_t*,void*);
#ifdef MNM_ORIGINAL_REFERENCE
static bool bodiesUnchanged(){for(const auto& b:ProtectedBodies)if(std::memcmp(reinterpret_cast<const void*>(b.address),b.expected,b.length))return false;return true;}
#endif
struct MemoryFault:std::runtime_error{using std::runtime_error::runtime_error;};
struct Memory {
    std::vector<unsigned char> globals=std::vector<unsigned char>(0x142000),owned=std::vector<unsigned char>(0x24000);
    std::vector<U> events,completedServices;U variant{},calls{},temporaryTop{},heapAttempts{},heapRetries{},allocationTop{};
    U temporary(U size){U result=Base+0x18000+temporaryTop;temporaryTop+=(size+3)&~3u;if(temporaryTop>0x5000)throw std::runtime_error("fixture temporary extent");return result;}
    unsigned char* pointer(U p,U n){
        if(p>=0x5c0000&&p-0x5c0000<=globals.size()&&n<=globals.size()-(p-0x5c0000)){
#ifdef MNM_ORIGINAL_REFERENCE
            return reinterpret_cast<unsigned char*>(p);
#else
            return globals.data()+p-0x5c0000;
#endif
        }
        if(p>=Base&&p-Base<=owned.size()&&n<=owned.size()-(p-Base)){
#ifdef MNM_ORIGINAL_REFERENCE
            return reinterpret_cast<unsigned char*>(p);
#else
            return owned.data()+p-Base;
#endif
        }
        throw MemoryFault("actor service owned address extent: "+std::to_string(p));
    }
    U get32(U p){U v;std::memcpy(&v,pointer(p,4),4);return v;}
    U get16(U p){std::uint16_t v;std::memcpy(&v,pointer(p,2),2);return v;}
    U get8(U p){return *pointer(p,1);}
    void set32(U p,U v){std::memcpy(pointer(p,4),&v,4);}
    void set16(U p,U v){std::uint16_t v16=std::uint16_t(v);std::memcpy(pointer(p,2),&v16,2);}
    void set8(U p,U v){*pointer(p,1)=static_cast<unsigned char>(v);}
    void digest(){
        U h=2166136261u;auto add=[&](U p,U n){for(U i=0;i<n;++i)h=(h^get8(p+i))*16777619u;};
        add(Base,0x18000);for(auto range:Globals)add(range[0],range[1]);events.push_back(h);
    }
    unsigned char* localPointer(U p,U n){
#ifdef MNM_ORIGINAL_REFERENCE
        U anchor=U(reinterpret_cast<std::uintptr_t>(&p));
        if(p>=anchor&&p-anchor<65536&&n<=512)return reinterpret_cast<unsigned char*>(p);
#endif
        return pointer(p,n);
    }
    bool isTemporary(U p){
        if(p>=Base+0x18000&&p<Base+0x1d000)return true;
#ifdef MNM_ORIGINAL_REFERENCE
        U anchor=U(reinterpret_cast<std::uintptr_t>(&p));if(p>=anchor&&p-anchor<65536)return true;
#endif
        return false;
    }
    U external(U id,U self,U second,std::initializer_list<U> args){
        U a[12]{};std::copy(args.begin(),args.end(),a);U normalized[12]{};std::copy(args.begin(),args.end(),normalized);
        std::vector<U> payload;
        auto record=[&](U index,U count,U token){normalized[index]=token;payload.push_back(count);auto* data=localPointer(a[index],count);for(U i=0;i<count;++i)payload.push_back(data[i]);};
        auto stringRecord=[&](U index){U length=0;while(length<255&&localPointer(a[index]+length,1)[0])++length;if(length==255)throw std::runtime_error("fixture string extent");if(isTemporary(a[index]))normalized[index]=0x20000000;payload.push_back(length+1);for(U i=0;i<=length;++i)payload.push_back(localPointer(a[index]+i,1)[0]);};
        if(id==0x58ad90)record(0,108,0x20000000);
        if(id==0x58c4a0)record(1,16,0x20000000);
        if(id==0x58c8a0)record(1,16,0x20000000);
        if(id==0x58d5b0)record(2,16,0x20000000);
        if(id==0x4a5a90){normalized[0]=0x20000000;payload.push_back(localPointer(a[0],1)[0]);for(U i=4;i<20;++i)payload.push_back(localPointer(a[0]+i,1)[0]);}
        if(id==0x4a5ce0){normalized[1]=0x20000004;normalized[5]&=255;for(U i=0;i<16;++i)payload.push_back(localPointer(a[1]+i,1)[0]);}
        if(id==0x58d1a0&&isTemporary(a[0]))stringRecord(0);
        if(id==0x493f20)record(1,252,0x20000000);
        if(id==0x4f4330)record(3,22,0x20000000);
        if(id==0x4d9f30||id==0x57d310)stringRecord(0);
        if(id==0x59c1fd)normalized[0]=0x20000000;
        if(id==0x5c50c0){normalized[2]=0x20000000;normalized[3]=0x20001000;}
        if(id==0x4694e0){U x,y,z;std::memcpy(&x,localPointer(self,4),4);std::memcpy(&y,localPointer(second,4),4);std::memcpy(&z,localPointer(a[0],4),4);self=0x20000008;second=0x20000004;normalized[0]=0x20000000;payload.insert(payload.end(),{x,y,z});}
        if(id==0x4f49e0)normalized[3]=0x20000000;
        events.insert(events.end(),{id,self,second,U(args.size())});events.insert(events.end(),normalized,normalized+args.size());events.insert(events.end(),payload.begin(),payload.end());digest();++calls;
        switch(id){
        case 0x416590:break;
        case 0x1001f440:return get32(self+40);
        case 0x1001f480:set32(self+44,a[0]);break;
        case 0x58ad90:set32(self+4,Other);break;
        case 0x58c4a0:case 0x4dcab0:case 0x56b590:case 0x566e60:case 0x5685a0:case 0x5695c0:case 0x569c80:case 0x56a8d0:case 0x56b1e0:break;
        case 0x1001f380:return variant&4&&++heapRetries<3?1:0;
        case 0x1001f3c0:break;
        case 0x1001f400:set32(self+8,a[0]);break;
        case 0x4694e0:{U v=7;std::memcpy(localPointer(a[0],4),&v,4);return variant&4?0:1;}
        case 0x469170:break;
        case 0x509aa0:set32(self+0x17c+a[0]*24+20,0);break;
        case 0x506220:return variant&4;
        case 0x51e7d0:set32(self+0x128,a[0]);break;
        case 0x416d80:return variant*3+1;
        case 0x522770:return a[0]&&!(variant&8)?0:Output+0x800+a[0]*64;
        case 0x5437f0:return variant&8?0:0xffff;
        case 0x4681f0:case 0x4680f0:case 0x467fe0:set32(second+4,0);break;
        case 0x5097b0:return variant&8?0:1;
        case 0x571310:break;
        case 0x543ee0:set32(self+0x2a,a[0]);break;
        case 0x51e500:return variant*127-700;
        case 0x507fa0:set32(self+0x24+a[0]*4,get32(self+0x24+a[0]*4)+a[1]);break;
        case 0x4db080:set32(self,a[0]);break;
        case 0x543c60:set32(self+4,0);break;
        case 0x4f49e0:{U values[]={a[0]*32+16,a[1]*32+16,a[2]*16};std::memcpy(localPointer(a[3],12),values,12);break;}
        case 0x4eb710:set16(self,a[0]);break;
        case 0x540430:return variant&16?U(-1):3;
        case 0x540140:break;
        case 0x545e90:set32(self,Output+0xc00);set32(Output+0xc00,0x1001f340);set32(self+8,a[0]);set32(self+12,a[1]);set32(self+16,a[2]);return variant&4?0:self;
        case 0x5457e0:return variant&8;
        case 0x1001f340:set32(self+20,a[0]);break;
        case 0x4f4330:return variant&1;
        case 0x494ab0:set32(self+0x180,a[0]);break;
        case 0x565180:return (variant+a[1])%3!=0;
        case 0x5654e0:if(variant&2)set32(a[0]+0x27c,1);return (variant+a[1])&1;
        case 0x566d80:if(variant&4)set32(a[0],0);break;
        case 0x466900:return 0x643a80+(variant%3)*0x154;
        case 0x58b660:return variant&2?0:Buffer;
        case 0x4d53d0:if(UsePopups&&get32(self+0x1518))set32(self+0x1518,get32(self+0x1518)-1);break;
        case 0x57dc20:case 0x57dc60:case 0x57de00:case 0x4d72d0:case 0x58bc10:case 0x4d47f0:case 0x4a5a90:case 0x4a5ce0:case 0x56f000:case 0x1001f280:case 0x1001f2c0:break;
        case 0x1001f300:return variant&1;
        case 0x58d1a0:return variant&1?0:1;
        case 0x4d2e50:case 0x4d6e90:case 0x4de6c0:case 0x597b40:break;
        case 0x1001f1c0:if(variant&4)set32(self+4,U(-1));break;
        case 0x1001f200:set32(self+16,0);if(runningEntry==0x42a340&&(variant&16))set32(Manager+8,get32(Manager+8)-4);break;
        case 0x1001f240:set32(0x5f2094,Other);break;
        case 0x597d40:if(!(variant&2))set32(0x5f2094,Other);return variant&1?0:1;
        case 0x598a10:++heapAttempts;if(heapAttempts<=(UseComposition&&runningEntry==0x506290?0:variant%3))return 0;if(UseComposition){U result=Buffer+allocationTop;allocationTop+=(a[1]+15)&~15u;if(allocationTop>0x5000)throw std::runtime_error("composed heap extent");return result;}return Buffer;
        case 0x597df0:++heapRetries;if(variant&8)set32(0x5f2094,Other+64);return heapRetries<2?1:0;
        case 0x58c8a0:case 0x58d5b0:case 0x4d3c90:case 0x4d5640:case 0x4d6620:case 0x505220:case 0x5c520c:case 0x1001f140:case 0x1001f180:break;
        case 0x49cdb0:return variant&1?Other+0x1000:0;
        case 0x493f20:set32(self+40,a[0]);break;
        case 0x4d9f30:if(runningEntry==0x4da690&&(variant&2)&&get32(self+0x161)<10)set32(self+0x161,get32(self+0x161)+1);break;
        case 0x59c1fd:{
            std::string text;U first=a[2],secondText=a[3];
            if(a[1]==0x5e2f54||a[1]==0x5e2f38){text="fixture_"+std::to_string(a[2])+"_"+std::to_string(a[3])+"_"+std::to_string(a[1]);std::memcpy(localPointer(a[0],U(text.size()+1)),text.c_str(),text.size()+1);return U(text.size());}
            if(runningEntry==0x56c430)secondText=0;
            for(U at=first;at&&localPointer(at,1)[0];++at)text.push_back(char(localPointer(at,1)[0]));
            if(secondText){text.push_back(' ');for(U at=secondText;localPointer(at,1)[0];++at)text.push_back(char(localPointer(at,1)[0]));}
            if(text.size()>250)throw std::runtime_error("fixture formatted string extent");
            std::memcpy(localPointer(a[0],U(text.size()+1)),text.c_str(),text.size()+1);return U(text.size());
        }
        case 0x4644d0:if(variant&8)return 0;set32(self,Other+0xa00);return 1;
        case 0x5050d0:return variant&16?0:1;
        case 0x57d310:if(variant&32)return 0;set32(self,Other+0x1000);return 1;
        case 0x57d690:set32(self+4,get32(self+4)+1);return 1;
        case 0x5c50c0:{if(variant&4)return 0;const char* path="C:/fixture/game.ani";std::memcpy(localPointer(a[2],20),path,20);U part=a[2]+11;std::memcpy(localPointer(a[3],4),&part,4);return 19;}
        case 0x488330:set16(a[0]+6,0xffff);break;
        case 0x59cae2:return variant&1?Other:0;
        case 0x59d50b:case 0x59c4c3:break;
        case 0x468470:if(variant&8)set32(self+0x7c,1);break;
        case 0x4eb820:set16(self,a[0]);break;
        case 0x564e90:set32(a[1]+0xddf,1);break;
        case 0x464350:set32(self,1);set8(self+0x92,0);break;
        case 0x465000:set32(self+4,get32(self+24)+44);set32(self+24,get32(self+24)+44);if(variant&8)set32(self+8,0);break;
        case 0x464ec0:set32(self+4,get32(self+24)+44);set32(self+24,get32(self+24)+44);if(get32(self+40)>0)set32(self+8,0);return variant&2?2:0;
        case 0x465130:set32(self+12,0);break;
        case 0x54e200:return variant*127+3;
        case 0x510480:return variant&1;
        case 0x54dce0:case 0x50aa30:case 0x485b10:case 0x50e400:case 0x513f50:case 0x50e3d0:case 0x50d060:
        case 0x4d6c60:case 0x4d39b0:case 0x4cb9b0:case 0x4f7970:case 0x4d7a90:
        case 0x4dea00:case 0x4d7a00:case 0x4d78b0:case 0x4d3b20:case 0x4d6c80:
        case 0x4d3b40:break;
        case 0x4667c0:set32(self+16,a[0]);break;
        case 0x469000:if(variant&1)set32(self+0x17c+second*24+20,0);break;
        case 0x490260:set32(self+0x212,U(-1));break;
        case 0x4943f0:set32(self+4,0);break;
        case 0x4a5000:if(variant&1)set32(Object+0x1c5,1);break;
        case 0x514e40:set32(self+0x7c,a[0]+1);break;
        case 0x5338c0:set32(self+48,a[0]);break;
        case 0x58a680:return Output;
        case 0x5977d0:case 0x597880:return variant&1?0:Buffer;
        case 0x598f70:if(variant&2)set32(a[0]+12,0xdeadbeef);break;
        case GetStatus:set32(a[1],get32(a[0]+8));return get32(a[0]+4);
        case Stop:
            // The callback may publish a different buffer before Position.
            if(variant&8)set32(Object,Buffer+0x100);
            return get32(a[0]+12);
        case Position:return get32(a[0]+16);
        case Constructor:set32(self,get32(self)+1);break;
        default:throw std::runtime_error("unknown downstream provider "+std::to_string(id)+" self "+std::to_string(self));
        }
        return 0;
    }
    U invokeFast(U id,U self,U second){return fastcall(id,self,second,{});}
    U invoke(U id,U self,std::initializer_list<U> args){
        if(UseComposition&&id==0x520790&&args.size()==1&&*args.begin()==0x20000000)return invoke(id,self,{temporary(20)});
        if(UseComposition){if(auto result=s::Composition<Memory>(*this).tryRun(id,self,args)){if(std::find(completedServices.begin(),completedServices.end(),id)==completedServices.end())completedServices.push_back(id);return *result;}}
        if(UseProviders)for(U entry:ProviderRoots)if(entry==id)return s::Providers<Memory>(*this).run(id,self,args);
        for(U entry:ValueRoots)if(entry==id)return s::Services<Memory>(*this).run(id,self,args);
        for(U entry:ControllerRoots)if(entry==id)return s::Controllers<Memory>(*this).run(id,self,args);
        if(UseWorldServices)for(U entry:WorldRoots)if(entry==id)return s::WorldServices<Memory>(*this).run(id,self,args);
        if(UseUiServices)for(U entry:UiRoots)if(entry==id)return s::UiServices<Memory>(*this).run(id,self,args);
        if(UseDependencies)for(U entry:DependencyRoots)if(entry==id)return s::Dependencies<Memory>(*this).run(id,self,args);
        if(UsePopups)for(U entry:PopupRoots)if(entry==id)return s::PopupServices<Memory>(*this).run(id,self,args);
        if(UseCollections)for(U entry:CollectionRoots)if(entry==id)return s::CollectionServices<Memory>(*this).run(id,self,args);
        if(UseEffects)for(U entry:EffectRoots)if(entry==id)return s::EffectServices<Memory>(*this).run(id,self,args);
        if(UseConstruction)for(U entry:ConstructionRoots)if(entry==id)return s::ConstructionServices<Memory>(*this).run(id,self,args);
        if(id==0x509790)return self+0x17c+*args.begin()*24;
        if(id==0x59cb60){auto it=args.begin();U dst=*it++,src=*it++,count=*it;std::memmove(pointer(dst,count),pointer(src,count),count);return dst;}
        return external(id,self,0,args);
    }
    U fastcall(U id,U self,U second,std::initializer_list<U> args){
        if(UseComposition){if(auto result=s::Composition<Memory>(*this).tryRun(id,self,args,second)){if(std::find(completedServices.begin(),completedServices.end(),id)==completedServices.end())completedServices.push_back(id);return *result;}}
        if(UseProviders)for(U entry:ProviderRoots)if(entry==id)return s::Providers<Memory>(*this).run(id,self,args,second);
        for(U entry:ControllerRoots)if(entry==id)return s::Controllers<Memory>(*this).run(id,self,args,second);
        if(UseCollections)for(U entry:CollectionRoots)if(entry==id)return s::CollectionServices<Memory>(*this).run(id,self,args,second);
        if(UseEffects)for(U entry:EffectRoots)if(entry==id)return s::EffectServices<Memory>(*this).run(id,self,args,second);
        return external(id,self,second,args);
    }
    U com(U id,U buffer,std::initializer_list<U> args){
        if(args.size()==0)return external(id,0,0,{buffer});
        return external(id,0,0,{buffer,*args.begin()});
    }
    void ring(U list,U count,bool sentinel){
        const U head=Nodes;
        set32(list+4,head);set32(list+8,count);
        U total=count+(sentinel?1:0);
        for(U i=0;i<total;++i){U node=head+i*64;set32(node,head+((i+1)%total)*64);set32(node+4,head+((i+total-1)%total)*64);set32(node+8,2);set32(node+16,Object+i*64);set32(node+20,Output+i*4);}
    }
    void reset(U seed){
        variant=seed;calls=0;temporaryTop=0;heapAttempts=heapRetries=allocationTop=0;events.clear();completedServices.clear();
        for(auto range:Globals)std::memset(pointer(range[0],range[1]),0,range[1]);
        for(U i=0;i<0x18000;++i)set8(Base+i,(i*73+seed*17)^(i>>5));
        set32(0x5fbf28,Other);set32(0x6f2a94,Catalog);set32(0x6f437c,Manager);
        set32(0x6def58,Pool);set32(0x6def5c,4);set32(0x6df180,4);
        set32(0x5e1910,35);set32(0x5e1914,U(-50));
        set32(0x6a4646,seed&1?Other:0);set32(0x689920,seed%4);set32(0x6e1f88,seed&1);
        for(U i=0;i<8;++i){U owner=0x643a80+i*0x154;set32(owner,i%2);set32(owner+4,i%5);set32(owner+12,i%5);set32(owner+16,i%3==0?U(-1):i%2);for(U j=0;j<8;++j){set8(owner+24+j,(j+seed)&1);set8(owner+32+j,(j+seed)&2);}}
        for(U i=0;i<2;++i)set32(0x65b290+i*0x93a,400+i);
        for(U i=0;i<20;++i){U row=Catalog+i*0x2dc;set32(row,0);set32(row+24,100+i);set32(row+0x2d0,i%4==0);set32(row+0x2d4,i%3?i+20:0);}
        for(U i=0;i<4;++i){U actor=Pool+i*0xe4b;set32(actor,i);set32(actor+4,seed&(1u<<i));set32(actor+0xa8,i&1);set32(actor+0xe4,i!=2);set32(actor+0x128,i==3?10:i*10);}
        set32(Object+0xac,Catalog);set32(Object+0x174,seed%8);set32(Object+0x124,seed%3);set32(Object+0x8c1,seed&1?77:0);
        set32(Object+0x1c5,seed%4);for(U i=0;i<3;++i)set32(Object+i*0x97+0x89,i&1?Other:Object);
    }
    void snapshot(U id,U seed,U result,U outcome=0){
        auto emit=[](const void* data,std::size_t size){auto* bytes=static_cast<const unsigned char*>(data);while(size){ssize_t n=write(STDOUT_FILENO,bytes,size);if(n<0&&errno==EINTR)continue;if(n<=0)_exit(88);bytes+=n;size-=std::size_t(n);}};
#ifndef MNM_ORIGINAL_REFERENCE
        if(UseComposition){std::sort(completedServices.begin(),completedServices.end());std::fprintf(stderr,"completed_services %x %u",id,seed);for(U entry:completedServices)std::fprintf(stderr," %x",entry);std::fprintf(stderr,"\n");}
#endif
        U header[]={id,seed,result,U(events.size()),outcome};emit(header,sizeof(header));emit(pointer(Base,0x18000),0x18000);
        for(auto range:Globals)emit(pointer(range[0],range[1]),range[1]);
        emit(events.data(),events.size()*4);
    }
};
static void fault(int signal,siginfo_t*,void*){
    if(active&&(signal==SIGSEGV||signal==SIGFPE)){
#ifdef MNM_ORIGINAL_REFERENCE
        if(!bodiesUnchanged())_exit(87);
#endif
        active->snapshot(runningEntry,runningVariant,0,signal==SIGFPE?2:1);
        std::cout.flush();_exit(0);
    }
    _exit(128+signal);
}
#ifdef MNM_ORIGINAL_REFERENCE
extern "C" __attribute__((force_align_arg_pointer)) U actor_services_provider(U id,U self,U* args,U second){
    try{
        const ServiceProvider* spec=nullptr;for(const auto& candidate:ServiceProviders)if(candidate.id==id)spec=&candidate;
        if(!spec)_exit(84);
        if(!spec->receiver)self=0;
        if(!spec->second)second=0;
        unsigned argc=spec->arguments;
        if(id==0x59c1fd&&runningEntry==0x56c430)argc=3;
        if(id==0x59d50b&&runningEntry==0x485b10)argc=6;
        switch(argc){
        case 0:return active->external(id,self,second,{});
        case 1:return active->external(id,self,second,{args[0]});
        case 2:return active->external(id,self,second,{args[0],args[1]});
        case 3:return active->external(id,self,second,{args[0],args[1],args[2]});
        case 5:return active->external(id,self,second,{args[0],args[1],args[2],args[3],args[4]});
        case 4:return active->external(id,self,second,{args[0],args[1],args[2],args[3]});
        case 6:return active->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5]});
        case 7:return active->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6]});
        case 8:return active->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]});
        case 11:return active->external(id,self,second,{args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]});
        default:_exit(85);
        }
    }catch(const std::exception& e){std::fprintf(stderr,"provider %x: %s\n",id,e.what());_exit(86);}
}
static void installProviders(){
    // Preflight the complete layout before installing any code. Fixed COM/
    // callback tokens and anonymous import stubs occupy disjoint owned arenas.
    std::vector<std::array<U,2>> regions;U planned=0;
    for(const auto& spec:ServiceProviders){
        U address=spec.id>=Base?spec.id:Code+planned*32;++planned;
        if(address<Base+0x1f000||address+26>Base+0x24000)throw std::runtime_error("provider trampoline extent");
        for(auto range:regions)if(address<range[1]&&range[0]<address+26)throw std::runtime_error("overlapping provider trampolines");
        regions.push_back({address,address+26});
    }
    U index=0;
    for(const auto& spec:ServiceProviders){
        U address=spec.id>=Base?spec.id:Code+index*32;++index;auto* p=reinterpret_cast<unsigned char*>(address);
        const unsigned char prefix[]={0x55,0x89,0xe5,0x52,0x8d,0x45,0x08,0x50,0x51,0x68};std::memcpy(p,prefix,sizeof(prefix));std::memcpy(p+10,&spec.id,4);
        p[14]=0xe8;U relative=U(reinterpret_cast<std::uintptr_t>(actor_services_provider))-address-19;std::memcpy(p+15,&relative,4);
        const unsigned char tail[]={0x83,0xc4,0x10,0xc9,0xc2};std::memcpy(p+19,tail,sizeof(tail));std::uint16_t pop=std::uint16_t(spec.pop);std::memcpy(p+24,&pop,2);
        if(spec.id<Base){
            auto* target=reinterpret_cast<unsigned char*>(spec.id);for(U i=0;i<8;++i)if(target[i]!=spec.bytes[i])throw std::runtime_error("provider entry guard");
            if(spec.id>=0x5c0000)std::memcpy(target,&address,4);
            else{target[0]=0xe9;relative=address-spec.id-5;std::memcpy(target+1,&relative,4);}
        }
    }
}
#endif
static bool returnsValue(U id){for(U entry:{0x545e60u,0x545690u,0x54aa10u,0x54aa00u,0x54aa80u,0x54e200u,0x464b20u,0x464b30u,0x465000u,0x464ec0u,0x510480u,0x545e90u,0x5457e0u,0x466900u,0x565180u,0x56b790u,0x56ba50u,0x56ba60u,0x56ba70u,0x56ba80u,0x464350u,0x4a4df0u,0x469000u,0x49cdb0u,0x4667c0u,0x58a680u,0x5977d0u,0x597880u,0x598f70u,0x414890u,0x5055a0u,0x520790u,0x463cb0u,0x5057b0u,0x411e50u,0x466870u,0x474b40u,0x49bcf0u,0x49c530u,0x4de090u,0x5075f0u,0x597850u})if(id==entry)return true;return false;}
int main(int argc,char** argv)try{
    struct sigaction action{};action.sa_sigaction=fault;action.sa_flags=SA_SIGINFO;sigemptyset(&action.sa_mask);for(int n:{SIGSEGV,SIGILL,SIGFPE,SIGABRT})sigaction(n,&action,nullptr);
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("expected pinned original path");
    map_image(read(argv[1]));OriginalLinuxSeh seh;
    if(UseComposition||UseProviders)for(U i=0;i<250;++i)if(*reinterpret_cast<const U*>(0x5eb940+i*4)!=(i+1)%250)throw std::runtime_error("original R250 index table differs");
    if(mmap(reinterpret_cast<void*>(Base),0x24000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("owned map");
#else
    (void)argc;(void)argv;
#endif
    Memory memory;active=&memory;s::Services<Memory> native(memory);unsigned cases=0;
#ifdef MNM_ORIGINAL_REFERENCE
    installProviders();if(!bodiesUnchanged())throw std::runtime_error("original body altered during provider installation");
#endif
    for(U entry:ServiceRoots)for(U seed=0;seed<64;++seed){
        runningEntry=entry;runningVariant=seed;memory.reset(seed);U p=Object;[[maybe_unused]] U second=0;std::vector<U> args;
        auto w=[&](U at,U value){memory.set32(at,value);};auto r=[&](U at){return memory.get32(at);};
        bool isController=UseComposition&&entry==0x50ab30;for(U id:ControllerRoots)isController|=id==entry;
        if(isController||(UseWorldServices||(UseComposition&&(entry==0x490260||entry==0x564e90||entry==0x508b00||entry==0x509230||entry==0x5093e0||entry==0x5121c0||entry==0x512340||entry==0x5137d0||entry==0x514e40)))||(UseDependencies||(UseComposition&&(entry==0x490260||entry==0x4d4970||entry==0x4d9ac0||entry==0x4d9f30||entry==0x564e90||entry==0x508b00||entry==0x509230||entry==0x5093e0||entry==0x5121c0||entry==0x512340||entry==0x5137d0||entry==0x514e40)))){
            w(0x644520,1);w(0x6894a0,Other+0x600);w(0x6898d0,Other+0x1000);
            for(U off:{0x160u,0x230u,0x240u}){w(Other+0x1000+off,2);w(Other+0x1004+off,0);}
            for(U i=0;i<4;++i){
                U peer=Pool+i*0xe4b;w(peer+0xac,Catalog);w(peer+0x174,i%2);w(peer+0x7c,0);w(peer+0x68,(seed/16)&1);w(peer+0x6e8,0);w(peer+0x8f9,Other+0x500);w(peer+0x668,(seed/7)%3-1);w(peer+0x60c,i);w(peer+0x5e8,U(-1));w(peer+0x8f1,2);w(peer+0x8f5,seed%4);
            }
            w(Other+0x508,0);w(Catalog,seed&1?0:17);w(Catalog+0x3c,(seed/2)&1);
            for(U descriptor:{Other+0x600,Other+0x800}){w(descriptor+4,Array);w(descriptor+8,Buffer);}
            for(U i=0;i<400;++i)w(Array+i*4,(i%8)*2);
            for(U i=0;i<32;++i){w(Buffer+i*44,0);w(Buffer+i*44+4,U(-1));}
            for(U controller:{p,p+0xb0,p+0x160,p+0x7d5,p+0x825}){w(controller,0);w(controller+4,Buffer);w(controller+24,Buffer);w(controller+28,Other+0x600);}
            w(p+0xac,Catalog);w(p+0x174,1);w(p+0x5fc,seed%3==0?2:seed%3==1?15:10);w(p+0x5f0,(seed/4)%9);w(p+0x618,(seed/27)&1);w(p+0x108,(seed/3)&1);w(p+0x7cd,(seed/8)%4);w(p+0x60c,seed%8);w(p+0xf4,seed%3);w(p+0xa0,seed%4);w(p+0x664,(seed/2)&1);w(p+0x64c,seed%5);w(p+0x6a8,(seed/5)%5);w(p+0x10c,seed%4);w(p+0xe4,seed%7!=0);w(p+0x7c,0);w(p+0x56c,5);
            for(U i=0;i<5;++i)w(p+0x570+i*4,i-1);
        }
        if((UseWorldServices||(UseComposition&&(entry==0x490260||entry==0x564e90||entry==0x508b00||entry==0x509230||entry==0x5093e0||entry==0x5121c0||entry==0x512340||entry==0x5137d0||entry==0x514e40)))){
            constexpr U Map=Base+0x16000,Terrain=Base+0x3200;
            w(0x6c5494,6);w(0x6c5498,5);w(0x6c54a0,30);w(0x6c54dc,Map);w(0x65660c,Terrain);w(0x6a49c0,0);
            w(0x6cb8be,U(-30));for(U i=0;i<4;++i)w(0x6cb8c2+i*4,i*30);for(U i=0;i<5;++i)w(0x6cb942+i*4,i*6);
            for(U i=0;i<120;++i){U at=Map+i*12;memory.set16(at,i%4);memory.set16(at+2,0xffff);memory.set16(at+4,(i+seed)%3==0?0:0xffff);memory.set16(at+6,(i+seed)%5==0?0:0xffff);memory.set16(at+8,0);memory.set16(at+10,(i+seed)%7==0?0x13:0);if((i+seed)%13==0)memory.set8(at+9,64);}
            for(U i=0;i<4;++i){w(Terrain+i*0x164+0x94,i==3?16:0);w(Terrain+i*0x164+0xc8,seed&1?U(-1):(i+1)%4);}
            for(U table:{0x5e5594u,0x5e55acu})for(U i=0;i<3;++i){w(table+i*8,i==1?0:1);w(table+i*8+4,i==0?0:1);}
            w(p,0);w(p+8,seed%6);w(p+12,(seed/6)%5);w(p+16,1+(seed/30)%2);w(p+0xa8,seed%28);w(p+0xe4,seed%7!=0);w(p+0xac,Catalog);w(p+0x7c,(seed/8)&1);w(Catalog+8,1+(seed/16)%2);w(Catalog+12,1+(seed/32)%2);
            w(p+0x855,(seed/8)&1);w(p+0x7d1,0);w(p+0x8c9,(seed/4)&1);w(p+0x744,0);w(p+0xddf,0);w(0x6bb743,1234);w(0x6a6529+r(p+0xa8)*0x5c9,77);w(p+0x5ec,3);w(p+0x6e8,0);w(p+0x8f9,Other+0x500);w(p+0x5f0,seed&1?3:0);w(p+0x5fc,seed&1?10:20);
            w(0x689498,Other);w(0x68949c,4);w(0x6898dc,4);w(0x6b1248,Other+0x1000);w(0x6b124c,seed%2);
            for(U i=0;i<4;++i){U effect=Other+i*0x22e;w(effect,i);w(effect+4,1);w(effect+40,i==1?14:0);w(effect+0x212,i+7);memory.set16(effect+0x1a0,i==3?0xffff:i+1);}
            w(0x5fed7c,0);
        }
        if((UseUiServices||(UseComposition&&(entry==0x4d4970||entry==0x4d9ac0)))){
            const char localized[]="localized",owner[]="player",sprite[]="sprite.spr";
            std::memcpy(memory.pointer(Output,sizeof(localized)),localized,sizeof(localized));std::memcpy(memory.pointer(Output+256,sizeof(owner)),owner,sizeof(owner));std::memcpy(memory.pointer(Other+0xa18,sizeof(sprite)),sprite,sizeof(sprite));
            w(0x6e1ee4,seed&1);w(0x6e1f0c,(seed/16)&1);memory.set8(0x6e2032,(seed/8)&1);w(p+0x1ef,Other);w(Other+0x68,seed&2);w(p+0xcb,100);w(p+0xc7,seed%5*25);w(Other+0xac,Catalog);w(Catalog,17);w(Catalog+24,100);w(Catalog+0x198,7);w(Other+0xe4,(seed/5)%5*25);w(Other+0x124,0);w(p+0x89,seed&4?0:Other);w(p+0x8e,seed%5*25);
        }
        if((UseDependencies||(UseComposition&&(entry==0x490260||entry==0x4d4970||entry==0x4d9ac0||entry==0x4d9f30||entry==0x564e90||entry==0x508b00||entry==0x509230||entry==0x5093e0||entry==0x5121c0||entry==0x512340||entry==0x5137d0||entry==0x514e40)))){
            constexpr U Vtable=Other+0x1d00;
            w(p,Vtable);w(Vtable,0x1001f1c0);w(p+4,seed%3==2?U(-1):seed%2);w(p+8,seed+3);
            w(0x5e04cc,seed%3);w(0x641fbc,seed%3);for(U i=0;i<30;++i)w(0x641ab8+i*4,i+seed*13);
            w(p+0x1ef,seed&1?Other:0);
            w(0x5f2094,seed&4?Other:0);memory.set8(0x5f20e8,seed&8);w(0x5f1ec8,seed&16?0x1001f240:0);
        }
        if((UsePopups||(UseComposition&&(entry==0x4d4970||entry==0x4d9f30)))){
            constexpr U Vtable=Other+0x1d00,Surface=Other+0x1200;
            for(U off:{0x10u,0x14u,0x18u,0x1cu,0x24u})w(Vtable+off,0x1001f140);
            w(Vtable+0x80,0x1001f280);w(Surface,Vtable);w(p+0x18ec,Other);w(Other+0xac,Catalog);w(Catalog,17);w(Other+0xe4,(seed/4)&1);w(Other+0x174,1);w(Other+0x7c,0);w(Other+0x56c,3);w(Other+0x5e8,U(-1));w(Other+0x8f1,2);w(Other+0x8f5,(seed/8)%3);for(U i=0;i<3;++i)w(Other+0x570+i*4,i);
            for(U i=0;i<26;++i){U row=p+i*0xc6;for(U off:{0xfcu,0x36u}){w(row+off,Vtable);w(row+off+4,U(-1));w(row+off+8,5);}w(row+0x1b1,(seed+i)%100);w(row+0x1b5,i%2);w(row+0x139,seed%3);memory.set8(row+0x1c1,(seed*7+i)&255);}
            for(U i=0;i<3;++i){U widget=p+0x1596+i*0xc6;w(widget,Vtable);w(widget+4,U(-1));w(widget+8,5);w(widget+0xb5,(seed+i*17)%100);w(widget+0xb9,i%2);}
            w(p+0x151c,Vtable);w(p+0x1518,seed%5);w(p+0x17e8,(seed/5)%4);w(p+0x1921,seed%6);w(p+0x1910,seed&1);w(p+0x1908,seed%5==4?U(-1):seed%4);w(p+0x190c,(seed/5)%4==3?U(-1):(seed/5)%3);w(p+0x183c,0);w(p+0x17ec,0);w(p+0xcb,100);w(p+0xc7,seed%5*25);w(p+0x1ef,Other);w(Other+0x68,seed&2);
            w(p+0x11,13);w(p+0x15,17);w(p+0x19,123);w(p+0x1d,127);w(0x6a4646,seed&4?0:Surface);w(0x6a4656,40);w(0x6a465a,320);w(0x6a4662,3);w(0x6a4666,5);w(0x6593e0,Other+0x1400);w(Other+0x140c,seed%5);w(0x6593e8,Other+0x1500);w(0x6593ec,Other+0x1600);w(0x6903dc,0x1001f2c0);w(0x6c4e3c,0x1001f300);
            for(U i=0;i<4;++i){w(Other+0x1500+i*4,i*64);w(Other+0x1600+i*64+28,(seed+i)&1?U(-1):0);}
            w(0x6de6d9,seed&1);w(0x6de6d5,(seed/2)&1);memory.set8(0x6e2032,(seed/4)&1);w(0x6e1ee4,seed&1);
        }
        if((UseCollections||(UseComposition&&(entry==0x564e90||entry==0x514e40||entry==0x490260)))){
            p=Other;w(p,1);w(p+4,1);w(p+8,2);w(p+12,2);w(p+16,1);w(p+0xac,Catalog);w(p+0xa8,17);w(Catalog,17);w(Catalog+24,100);w(Catalog+40,30);w(Catalog+20,40);w(Catalog+8,1);w(Catalog+12,1);w(p+0x174,1);w(p+0x7c,0);w(p+0xe4,80);w(p+0x124,0);w(p+0x128,0);w(p+0x60c,seed%8);
            constexpr U Definition=Catalog+0x800;w(Definition+4,Array);w(Definition+8,Buffer);
            for(U off:{0xb0u,0x12cu,0x7d5u}){w(p+off,0);w(p+off+4,Buffer);w(p+off+24,Buffer);w(p+off+28,Definition);}
            w(0x6894a0,Definition);w(0x6898d0,Other+0x1000);w(Other+0x1240,2);w(Other+0x1244,0);
            w(p+0x6b4,5);w(p+0x6b8,10);w(p+0x6c4,80);w(p+0x6c8,100);w(p+0x6cc,6);w(p+0x6d0,Output+0x400);
            w(Base+0x10008,0x1001f200);memory.set16(Base+0x1000e,0xcad0);w(0x5f2094,Other);w(0x5f1ec8,0);
        }
        if((UseEffects||(UseComposition&&entry==0x490260))){
            constexpr U First=Base+0x9000;w(0x689498,First-125*0x22e);w(0x68949c,126);w(First,125);w(First+4,0);w(First+0x20e,0);w(0x6898dc,1);w(0x6f3a70,Nodes+0x400);w(0x6f3a78,0);w(0x6f3a90,0);w(0x6c5c88,9);w(0x6b1250,Base+0x6000);w(0x6b1254,123);w(0x6a42e8,0);w(0x68991c,seed&2);w(0x5e1904,100);w(0x5e1908,30);
        }
        if(UseConstruction){
            p=Other;w(p+4,seed%5==4?U(-1):seed%4);w(p+8,9);w(p+0x85,seed&1?Base+0x11000:0);w(p+0x93,seed&2?Output:0);w(Output,Output+0x100);w(Output+0x104,0x1001f400);w(0x6e1f1c,seed&1);w(0x6f8154,2);w(0x6f98c0,p+0x21);w(0x6f6b4c,7);w(0x6f694c,9);
            constexpr U Table=Output+0x200;w(Table+8,0x1001f380);w(Table+0x98,0x1001f3c0);
            for(U i=0;i<3;++i)w(Output+0x300+i*32,Table);
            for(U i=0;i<2;++i){U c=p+(i?0x53:0x21);w(c+4,Output+0x300);w(c+8,seed&8?0:Output+0x320);w(c+12,seed&16?Output+0x340:0);w(c+28,i);w(0x6f6950+i*4,c);}for(U i=2;i<128;++i)w(0x6f6950+i*4,0);
            w(0x642c68,4);w(0x5e04cc,seed%6==5?U(-1):seed%5);for(U i=0;i<4;++i){U widget=Nodes+i*64;w(0x642034+i*4,widget);w(widget+4,i);w(widget+8,1);}w(Base+0x10008,0x1001f200);memory.set16(Base+0x1000e,0xcad0);
        }
        if(UseProviders){
#include "actor-provider-seeds.inc"
        }
        switch(entry){
        case 0x4a4df0:case 0x464350:case 0x463270:break;
        case 0x4a4f70:p=seed%5==4?U(-1):seed%4;break;
        case 0x58b3e0:p+=0x21;break;
        case 0x59bf87:args={Nodes+(seed%7)*17,17,seed%7,Constructor};break;
        case 0x59c4ab:p=Manager;w(p-32,seed&1);w(p-28,seed%7);w(p+8,Nodes+(seed%7)*17);w(p+12,17);w(p+24,Constructor);break;
        case 0x469000:{
            second=seed%3;U desc=p+0x17c+second*24;for(U i=0;i<5;++i)w(desc+i*4,30+i);w(desc+20,seed%4);U at=Base+0x16000+(2+2*6+30)*12;memory.set16(at+6,seed&8?0:0xffff);w(0x6b1248,Base+0x6800);w(0x6b124c,1);w(Base+0x682a,seed&16?50:1);w(Base+0x686a,U(-1));args={seed&32?31u:30u,seed&2?U(-1):seed%4};break;
        }
        case 0x468470:{
            second=Base+0x6800;U types[]={0,1,2,3,4,6,5,19,50,53,54,55,56,97,122,123,61};U type=types[seed%17];w(second+4,seed%13!=0);w(second+0x2a,type);for(U i=0;i<4;++i)w(second+0x6a+i*4,3+i);w(0x6b1248,Base+0x6900);w(0x6b124c,1);w(Base+0x6904,1);w(Base+0x692a,5);for(U i=0;i<4;++i)w(Base+0x696a+i*4,3+i);w(Catalog,seed&16?17:0);w(p+0x7c,0);w(p+0xe4,60);w(Base+0x6000+type*32+20,2);memory.set8(0x6a6500+17*0x5c9,10);for(U i=0;i<8;++i)w(p+0x24+i*4,i%2);break;
        }
        case 0x490260:{
            U types[]={65,69,70,91,92,1},starts[]={0x167,0x153,0x159,0x14d,0x160,0};U k=seed%6;w(p+0x4c,types[k]);w(p+20,64);w(p+24,96);w(p+28,48);w(p+40,1);w(p+0x19c,0);w(p+0x5c,0);w(p+0x194,Base+0x16000+(2+2*6+2*30)*12);for(U z=0;z<3;++z)memory.set16(Base+0x16000+(2+2*6+z*30)*12,seed&1?starts[k]+z:1);w(0x6e1f3c,seed&2);w(0x6898e4,Nodes);w(0x6898e8,2);w(Nodes,Nodes+64);w(Nodes+4,Nodes+128);w(Nodes+64,Nodes+128);w(Nodes+68,Nodes);w(Nodes+72,seed&32?0:p);w(Nodes+128,Nodes);w(Nodes+132,Nodes+64);w(Nodes+136,seed&32?p:0);w(Base+8,0x1001f200);memory.set16(Base+14,0xcad0);break;
        }
        case 0x42a340:{
            p=Manager;U begin=Base+0x11000,size=seed%7,capacity=seed&8?size:10;w(p+4,begin);w(p+8,begin+size*4);w(p+12,begin+capacity*4);for(U i=0;i<16;++i)w(begin+i*4,i+100);w(Output,777);args={begin+((seed/7)%(size+1))*4,(seed/14)%4,seed&32?begin:Output};break;
        }
        case 0x49cdb0:{
            p=Manager;constexpr U First=Base+0x9000,SecondaryBase=First-125*0x22e;
            w(p,SecondaryBase);w(p+4,123+seed%8);w(0x689498,SecondaryBase);w(0x68949c,131);w(0x6898dc,131);
            for(U i=0;i<7;++i){U row=First+i*0x22e;w(row,125+i);w(row+4,seed&8?1:(seed+i)%3);w(row+0x20e,(seed+i)%5==0);U types[]={85,14,26,40,58,39,1};w(row+40,types[(seed+i)%7]);w(row+8,i%6);w(row+12,2);w(row+16,1);w(row+0x19c,0);w(row+0x5c,0);}break;
        }
        case 0x514e40:
            w(p+0x124,seed%3);w(p+0x128,seed%3==0?30:60);w(0x6a650d+17*0x5c9,25);w(0x6a6511+17*0x5c9,50);w(0x6def78+17*4,seed&8);if(seed&16)w(Catalog,24);if(seed&32)w(p+0xe4,0);args={seed%7};break;
        case 0x564e90:{
            U actor=p;p=Manager;constexpr U Records=Base+0x7800;w(p,Records);w(p+8,seed%5);w(p+32,17);U kinds[]={0,1,2,3,6,7,9,8};U event=kinds[(seed/5)%8];
            for(U i=0;i<4;++i){U row=Records+i*0x47c;w(row,(seed+i)&1);w(row+4,(seed+i)%3);w(row+8,(seed+i)%3);w(row+0x138,seed%3);memory.set8(row+0x144,seed&8?'a':0);memory.set8(row+0x145,0);w(row+0x27c,(seed+i)&1);w(row+0x470,(seed+i)&2);w(row+0x24,0);}memory.set16(0x5fc874,0);args={event,actor,(seed/3)%3,2,3,4};break;
        }
        case 0x4d3b40:case 0x4d3c90:case 0x4d5640:case 0x4d78b0:case 0x4d7a00:case 0x4d7a90:break;
        case 0x4d6c80:args={seed,0};if(seed&16)w(p+0x1908,12);break;
        case 0x4d6620:
            w(Output,seed%101);w(Output+4,seed&1?3:0);w(Output+16,seed%7);w(0x6b1250,Catalog);w(0x6b1254,100);for(U i=0;i<100;++i)w(Catalog+i*32+12,5);w(p+0x1518,seed&16?26:seed%5);if(seed&32){w(p+0x183c,1);w(p+0x17ec,1);}if(seed&8){w(p+0x1b5,1);w(p+0x1b1,seed%101);}args={Output,seed};break;
        case 0x4d9f30:
            w(p+0x16d,seed&4?0:Other+0x1200);w(p+0x179,160);w(p+0x17d,30);w(p+0x181,320);std::memcpy(memory.pointer(Output,8),"message",8);args={Output};break;
        case 0x465130:w(p+24,Buffer);w(p+4,Output);w(Buffer,seed&1?6:4);w(Buffer+44,0);break;
        case 0x4667c0:
            if(seed&32)p=0x643a80;
            for(U i=0;i<8;++i){w(p+i*0x154,seed&(1u<<i));w(p+i*0x154+4,(seed+i)&1);}args={seed%10==9?U(-1):seed%10};break;
        case 0x485b10:
            memory.set8(p,seed&1?'f':0);w(p+0x100,17);w(p+0x104,seed);args={Output,seed*3};break;
        case 0x4a5000:case 0x4cb9b0:case 0x4d39b0:case 0x4d6c60:break;
        case 0x4d3b20:args={seed*7};break;
        case 0x4dea00:w(p+8,seed%3);args={(seed/3)%3};break;
        case 0x50e3d0:args={seed%10,(seed/10)&1};break;
        case 0x513f50:break;
        case 0x5338c0:w(p,Output);w(p+4,seed%35);args={0xabcd0000+seed*1963};break;
        case 0x54dce0:
            for(U i=0;i<6;++i)w(p+0x84+i*4,i%3);
            w(p+0xfc,seed%6);w(p+0x80,999);args={(seed/6)%4};break;
        case 0x58a680:
            w(p,seed&1?Other:0);w(p+4,Array);w(p+8,Buffer);w(Other+12,seed%7);for(U i=0;i<8;++i)w(Array+i*4,i*7);args={(seed/7)%9};break;
        case 0x58d5b0:{
            U counts[]={0,1,4,127,128,130},occupied=counts[seed%6];for(U i=0;i<131;++i)w(0x6f6b50+i*44,i<occupied);
            for(U i=0;i<4;++i)w(Output+i*4,seed+i*7);
            p=Other+0x29;args={0,Other,seed&1?Output:0,U(-1)-seed,seed,1};break;
        }
        case 0x5977d0:args={seed%8==0?0:seed%8==1?U(-1):seed*17,seed%4,seed&32?Other+128:0};p=0;break;
        case 0x597880:args={seed%8==0?0:seed%8==1?U(-1):seed*17};p=0;break;
        case 0x598f70:w(Base+8,0x1001f200);memory.set16(Base+14,seed&1?0xcad0+(seed&7):0x1234);args={Other+32};p=0;break;
        case 0x414890:args={seed%9-3,(seed*3)%11-3,seed%5-1};w(Catalog+8,1+(seed/2)%2);w(Catalog+12,1+(seed/4)%2);w(Catalog+0x44,seed%3);w(p+0x104,seed%2);w(p+0xa8,seed%28);memory.set8(p+0x723,(seed/8)&1);w(p+0x5ec,seed%16);break;
        case 0x4637c0:break;
        case 0x4d3a30:args={(seed/5)%5*25};if(seed==63)w(p+0xcb,0);break;
        case 0x49d640:args={Other};w(Other,7);w(Other+20,11);w(Other+24,13);w(Other+28,17);break;
        case 0x4d4970:{
            w(p+0x18ec,seed%7==0?0:Other);U counts[]={0,1,3,25,26};w(Other+0x56c,counts[seed%5]);w(Other+0x8f5,(seed/5)&1);w(Other+0x8f1,1);w(Other+0x178,(seed/10)%4);w(p+0x17e8,(seed/4)%4);w(Array+16,0x1001f140);
            for(U i=0;i<25;++i){U ids[]={0,26,27,28,49,50,52,54,58,86,12};w(Other+0x570+i*4,ids[(i+seed)%11]);}
            for(U i=0;i<3;++i){w(p+0x1596+i*0xc6,Array);w(p+0x15d3+i*0xc6,seed&1?2:0);w(p+0x164f+i*0xc6,0);w(Other+0x17c+i*24+20,(seed+i)&1);}break;
        }
        case 0x4d9ac0:
            w(p+0x594,seed%5);w(p+0x59c,(seed/5)%5);w(Array+20,0x1001f180);for(U i=0;i<4;++i){w(p+8+i*0x8a,i+3);w(p+12+i*0x8a,Array);w(p+0x56c+i*4,(seed+i)%4);}args={3+(seed/7)%5};break;
        case 0x4da690:w(p+0x161,seed%12);w(p+0x165,(seed/12)%3-1);for(U i=0;i<10;++i)w(p+0x139+i*4,i+3);args={seed+50};break;
        case 0x4da720:args={Output+256,seed};break;
        case 0x5055a0:{
            U type=seed%5==0?0:seed%5==1?24:seed%5==2?25:seed%5==3?26:17,descriptor=0x6dbf00+type*0x114,resource=0x6de0e8+type*36;
            for(U off:{0u,4u,8u})w(descriptor+off,seed&1?Other+0xa00+off*64:0);
            for(U off:{0u,4u,8u,12u})w(resource+off,seed&2?Other+0xc00:0);
            for(U i=0;i<4;++i){w(resource+16+i*4,seed&4?Nodes+i*64:0);w(Nodes+i*64,0);}w(resource+32,seed&2?Other+0xe00:0);w(0x6e9808,seed&16);args={type,Output};break;
        }
        case 0x56c430:
            w(0x6f37f8,Manager);w(0x6f37fc,seed%5);for(U i=0;i<4;++i){U record=Manager+i*64,actor=Pool+i*0xe4b;w(record,1);std::memcpy(memory.pointer(record+4,7),"player",7);w(record+36,(seed/5)%3);w(record+40,i);w(record+60,(seed/15)&1);w(actor+8,i+3);w(actor+12,i+4);w(actor+16,i+5);w(actor+0xa8,seed&1?27:17);}args={seed%6==5?U(-1):seed%5};break;
        case 0x508b00:w(p+0x906,seed%5);w(p+0x6a8,(seed/5)%5);args={seed%5};break;
        case 0x509230:case 0x5093e0:args={r(p+8),r(p+12),r(p+16)};break;
        case 0x4943f0:
            p=Other+(seed%4)*0x22e;w(p+8,seed%6);w(p+12,(seed/6)%5);w(p+16,1);w(p+0x19c,0);w(p+0x5c,0);if(seed&1)w(p+40,43);break;
        case 0x508250:memory.set8(p+0x722,(seed/4)&1);args={(seed*3)%13-3,(seed*7)%11-3};break;
        case 0x51a8f0:case 0x51aa30:break;
        case 0x520790:
            w(p+0x5f0,seed%16);w(p+0x5ec,(seed/16)%16);
            for(U offset:{0x96bu,0xbc3u,0x6f8u}){w(p+offset,seed%19-7);w(p+offset+4,(seed*3)%17-6);w(p+offset+8,seed%9-2);}args={Output};break;
        case 0x5244f0:
            w(p+0x92e,seed%3);w(p+0x8ed,seed%11-3);w(p+0xf8,seed&1);w(p+0x94,(seed/2)&1);w(p+0x90,(seed/4)&1);w(p+0x8c,(seed/8)&1);w(p+0x104,(seed/16)&1);w(p+0x84,seed*11);w(p+0x88,seed*7);w(p+0x7cd,seed%3);w(0x6bfd88,9);w(0x6c27c7,13);
            for(U i=0;i<28;++i){w(0x6a5fd8+i*0x5c9,(seed+i)%3);w(Catalog+0x1a0+i*4,(seed+i)%47-5);w(Catalog+0x280+i*4,(seed*3+i)%59-7);}for(U offset:{0x5a9u,0x5adu,0x5c5u,0x589u})w(Catalog+offset,seed%13-3);break;
        case 0x463cb0:w(0x5fbf1c,seed&4?Other:0);p=0;break;
        case 0x464cb0:for(U i=0;i<32;++i)w(Buffer+i*44,seed&1?4:0);args={seed%8,(seed/2)&1};break;
        case 0x472730:w(p+0x100b3,seed&1?Other:0);w(0x6de6d9,seed%5-1);w(0x6de6d5,seed&1);w(0x6903e8,(seed/5)%4);memory.set8(0x6c4859,(seed/20)&1);w(0x6c482c,Other);break;
        case 0x494a50:w(p+40,seed&1?52:1);w(p+36,Catalog);w(Catalog+0x438,Other+0x800);w(Other+0x800+0x2d0,2);w(p+68,seed%5);args={seed%8};break;
        case 0x5057b0:
            p=seed%5==0?0:seed%5==1?24:seed%5==2?25:seed%5==3?26:17;second=seed%8;w(0x644520,seed%8);w(0x689920,(seed/8)&1?5:0);memory.set8(0x6dbe9d,(seed/16)&1);
            for(U i=0;i<72;++i)w(0x5c7144+i*4,Output+i*16);
            for(U i=0;i<2;++i)w(0x65b34c+i*0x93a,i);
            w(0x643a90+second*0x154,seed%2);break;
        case 0x509bd0:args={seed&1?U(-1):seed%4};break;
        case 0x50db60:args={seed%27};break;
        case 0x520e70:memory.set8(p+0x7cc,(seed/16)&1);w(p+0xa8,seed%7==0?27:17);break;
        case 0x407b90:w(p+0x89,seed%6);break;
        case 0x411e50:w(p,seed%32);break;
        case 0x464030:args={seed&2?Object:Other};break;
        case 0x464ad0:for(U i=0;i<3;++i)w(p+i*4,seed&(1u<<i)?Other+i*64:0);break;
        case 0x464ca0:args={Other};break;
        case 0x464e20:{
            w(p+28,Other);w(Other+4,Array);w(Other+8,Buffer);for(U i=0;i<4;++i)w(Array+i*4,i*5);
            U old=seed%4;w(p,old);w(p+4,Buffer+old*220+44*(seed%3));w(p+24,Buffer+old*220-44*(seed%2));args={(seed/4)%4};break;
        }
        case 0x465490:w(p+4,seed&1);w(p+8,seed&2);args={seed&4?U(-1):7};break;
        case 0x466870:p=0x643a80;args={seed%9==8?U(-1):seed%8};break;
        case 0x474b10:{for(U i=0;i<40;++i)memory.set8(Output+i,i<seed%40?'A'+i%26:0);args={Output};break;}
        case 0x474b40:p=seed;break;
        case 0x49bcf0:memory.ring(p,4,false);args={seed%5==4?Other:Object+(seed%4)*64};break;
        case 0x49e170:
            w(p+0x44c,Nodes);for(U i=0;i<4;++i){w(Nodes+i*64,Nodes+((i+1)%4)*64);w(Nodes+i*64+8,Other+i*0x300);w(Other+i*0x300+0x212,i%2);}args={seed%3};break;
        case 0x49c530:case 0x4de090:
            w(Manager+24,(seed/16)&1);w(Manager+28,(seed/32)&1);memory.ring(Manager+0x25c,4,false);
            for(U i=0;i<3;++i){U voice=Object+i*64,buffer=Buffer+i*0x100;w(voice,seed%7==i?0:buffer);w(voice+32,i<2?voice+64:0);w(buffer,Array);w(buffer+4,(seed+i)%5==0?U(-1):0);w(buffer+8,(seed+i)&1);w(buffer+12,(seed+i)%7==0?0x80000001:0);w(buffer+16,(seed+i)%11==0?0x80000002:0);}
            w(Array+0x24,GetStatus);w(Array+0x48,Stop);w(Array+0x34,Position);args=entry==0x49c530?std::vector<U>{(seed/2)&1}:std::vector<U>{(seed/2)&1,(seed/4)&1};break;
        case 0x4e1260:args={Other,seed&1};break;
        case 0x5075f0:w(Catalog,seed%6==0?24:seed%6==1?25:seed%6==2?26:seed%6==3?0:17);if(seed%7==0)w(p+0x174,U(-1));break;
        case 0x50eb60:w(p+0x668,seed%4==3?0x7fffffff:seed%4-1);args={(seed/4)&1};break;
        case 0x5121c0:case 0x512340:
            w(p+0x6e8,seed&1?Other:0);w(p+0x8f9,Manager);memory.ring(Manager,seed%4,true);
            for(U i=1;i<=seed%4;++i){w(Nodes+i*64+8,i%3);w(Nodes+i*64+40,seed&2?Other+i*0x300:0);}break;
        case 0x5137d0:for(U i=0;i<42;++i){w(p+0x17c+i*24,(i+seed)%7==0?U(-1):(i+seed)%5==0?19:90+i);w(p+0x17c+i*24+20,(i+seed)%3);}break;
        case 0x52ae60:w(p,Pool);w(p+0x228,seed%5);args={(seed/5)%2,(seed/10)%2};break;
        case 0x52d0c0:memory.ring(p,seed%5,true);break;
        case 0x54dad0:case 0x54dba0:case 0x54dcb0:case 0x54ddf0:
            w(p+0x80,entry==0x54dba0?1+seed%5:seed%31);w(p+0xfc,entry==0x54dba0?1+(seed/5)%5:seed%31);w(p+0x104,Catalog+(seed%4)*0x2dc);
            for(U i=0;i<30;++i){w(p+8+i*4,i%6);w(p+0x84+i*4,20+i%6);}args={seed%7};if(entry==0x54ddf0)args.clear();break;
        case 0x571d20:memory.ring(p,1+seed%5,false);args={Nodes+((seed/5)%(1+seed%5))*64};if(seed&1)w(args[0]+20,0);break;
        case 0x57d0d0:for(U i=0;i<4;++i)w(Nodes+i*64,i+1<seed%5?Nodes+(i+1)*64:0);args={seed%5?Nodes:0};p=0;break;
        case 0x58f750:w(p+28,seed&2?Other:0);args={3+seed%5,4+(seed/5)%6};break;
        case 0x58f7f0:case 0x58f830:w(p+4,3+seed%10);w(p+16,seed%16-3);w(p+20,1+seed%13);args={1+(seed*7)%23};break;
        case 0x58f890:w(p,2+seed%7);w(p+4,3+(seed/7)%7);w(p+24,seed%11);w(p+28,Buffer);break;
        case 0x597850:p=0;args={seed*17};break;
        case 0x597870:case 0x5979b0:p=0;args={seed&1?Other:0};break;
        case 0x59c441:p=0;args={Other,17,seed%7==6?U(-1):seed%6,Constructor,0};break;
        default:break;
        }
        if(UseComposition){
#include "actor-composition-seeds.inc"
        }
        std::cout.flush();const pid_t child=fork();if(child<0)throw std::runtime_error("case fork");
        if(child){int status=0;if(waitpid(child,&status,0)!=child||!WIFEXITED(status)||WEXITSTATUS(status))throw std::runtime_error("case process failed");++cases;continue;}
        try {
        U result;
#ifdef MNM_ORIGINAL_REFERENCE
        U pop=0;for(const auto& spec:ServiceAbis)if(spec.id==entry)pop=spec.pop;
        result=entry==0x59c4ab?actor_services_cleanup(p):actor_services_call(entry,p,second,U(args.size()),args.data(),pop);seh.check();if(!bodiesUnchanged())throw std::runtime_error("original body altered during execution");
#else
        bool controller=false;for(U id:ControllerRoots)controller|=id==entry;
        auto execute=[&](std::initializer_list<U> values){return UseComposition?memory.fastcall(entry,p,second,values):UseProviders?s::Providers<Memory>(memory).run(entry,p,values,second):UseConstruction?s::ConstructionServices<Memory>(memory).run(entry,p,values):UseEffects?s::EffectServices<Memory>(memory).run(entry,p,values,second):controller?s::Controllers<Memory>(memory).run(entry,p,values,second):UsePopups?s::PopupServices<Memory>(memory).run(entry,p,values):UseCollections?s::CollectionServices<Memory>(memory).run(entry,p,values,second):UseWorldServices?s::WorldServices<Memory>(memory).run(entry,p,values):UseUiServices?s::UiServices<Memory>(memory).run(entry,p,values):UseDependencies?s::Dependencies<Memory>(memory).run(entry,p,values):native.run(entry,p,values);};
        if(args.size()==0)result=execute({});
        else if(args.size()==1)result=execute({args[0]});
        else if(args.size()==2)result=execute({args[0],args[1]});
        else if(args.size()==3)result=execute({args[0],args[1],args[2]});
        else if(args.size()==6)result=execute({args[0],args[1],args[2],args[3],args[4],args[5]});
        else if(args.size()==4)result=execute({args[0],args[1],args[2],args[3]});
        else if(args.size()==5)result=execute({args[0],args[1],args[2],args[3],args[4]});
        else throw std::runtime_error("root argument extent");
#endif
        if(UseComposition&&entry==0x464cb0){
            for(U tick=0;tick<seed%5;++tick){
#ifdef MNM_ORIGINAL_REFERENCE
                actor_services_call(0x464ec0,p,0,0,nullptr,0);seh.check();
#else
                memory.invoke(0x464ec0,p,{});
#endif
            }
        }
        if(UseComposition&&entry==0x506290){
#ifdef MNM_ORIGINAL_REFERENCE
            actor_services_call(0x506d30,p,0,0,nullptr,0);U cleanupArgument=(seed/4)&1;
            actor_services_call(0x51f5e0,p,0,1,&cleanupArgument,4);seh.check();
            if(!bodiesUnchanged())throw std::runtime_error("lifecycle body altered during retirement");
#else
            memory.invoke(0x506d30,p,{});memory.invoke(0x51f5e0,p,{(seed/4)&1});
#endif
        }
        (void)r;memory.snapshot(entry,seed,(entry==0x411e50||entry==0x4667c0)?(result&0xff):(returnsValue(entry)?result:0));
        }catch(const MemoryFault& error){if(UseComposition)std::fprintf(stderr,"owned_fault %x %u %s\n",entry,seed,error.what());memory.snapshot(entry,seed,0,1);}
        catch(const std::domain_error&){memory.snapshot(entry,seed,0,2);}
        std::cout.flush();_exit(0);
    }
    U snapshotBytes=0x18000;for(auto range:Globals)snapshotBytes+=range[1];
    std::cerr<<"snapshot_bytes "<<snapshotBytes<<'\n';
    std::cerr<<"cases "<<cases<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<std::hex<<runningEntry<<" variant "<<std::dec<<runningVariant<<": "<<e.what()<<'\n';return 1;}
