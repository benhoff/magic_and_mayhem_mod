#include "../reconstruction/rendering/drawing_session_release.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
static OriginalDrawingSession session;static std::array<ReleaseCanvas,3> canvases;
static std::vector<U> events;static std::array<U,256> calls;static U mutation,limit,restoreResult;static bool fired;
#ifdef MNM_ORIGINAL_REFERENCE
struct Interface{void** table;U token;};static std::array<Interface,256> interfaces;
static U ptr(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static U token(U address){if(!address||address==0xcafe0000u)return address;for(U i=1;i<interfaces.size();++i)if(address==ptr(&interfaces[i]))return i;throw std::runtime_error("Unknown COM identity");}
#endif
static void effect(U operation,U identity){
    if(fired)return;
#ifdef MNM_ORIGINAL_REFERENCE
    session.draw1=token(*reinterpret_cast<U*>(0x6f98c4));session.draw2=token(*reinterpret_cast<U*>(0x6f98c8));
    session.draw4=token(*reinterpret_cast<U*>(0x6f98cc));session.clipper=token(*reinterpret_cast<U*>(0x6f98d0));
    session.fullscreen=*reinterpret_cast<U*>(0x6f68d8);session.canvases.pageLocked=*reinterpret_cast<U*>(0x6e1f1c)!=0;
#endif
    if(mutation==1&&operation==1&&identity==104){session.fullscreen=1;fired=true;}
    if(mutation==2&&operation==1&&identity>=200){session.fullscreen=!session.fullscreen;fired=true;}
    if(mutation==3&&operation==1&&identity==101){session.draw2=105;fired=true;}
    if(mutation==4&&operation==2){session.draw1=106;fired=true;}
    if(mutation==5&&operation==1&&identity>=200){session.canvases.pageLocked=!session.canvases.pageLocked;fired=true;}
    if(mutation==6&&operation==1&&identity==104){session.clipper=107;fired=true;}
    if(mutation==7&&operation==1&&identity==101){session.fullscreen=1;fired=true;}
#ifdef MNM_ORIGINAL_REFERENCE
    global(0x6f68d8,session.fullscreen);global(0x6e1f1c,session.canvases.pageLocked);
    global(0x6f98c4,session.draw1?ptr(&interfaces[session.draw1]):0);
    global(0x6f98c8,session.draw2?ptr(&interfaces[session.draw2]):0);
    global(0x6f98cc,session.draw4?ptr(&interfaces[session.draw4]):0);
    global(0x6f98d0,session.clipper?ptr(&interfaces[session.clipper]):0);
#endif
}
static U invoke(U operation,U identity){
    if(!identity||identity>=calls.size())throw std::runtime_error("COM token");
    U result=0;if(operation==1)result=++calls[identity]>=limit?0:1;else if(operation==2)result=restoreResult;
    events.insert(events.end(),{operation,identity,result});effect(operation,identity);return result;
}
struct Provider {
    U release(U token){return invoke(1,token);}U restore(U token){return invoke(2,token);}
    U surface(ReleaseOperation operation,U token){return invoke(operation==ReleaseOperation::release?1u:3u,token);}
};
#ifdef MNM_ORIGINAL_REFERENCE
static U __attribute__((stdcall,force_align_arg_pointer)) release(Interface* self){return invoke(1,self->token);}
static U __attribute__((stdcall,force_align_arg_pointer)) restore(Interface* self){return invoke(2,self->token);}
static U __attribute__((stdcall,force_align_arg_pointer)) unlock(Interface* self,U flags){if(flags)throw std::runtime_error("PageUnlock flags");return invoke(3,self->token);}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc<2||argc>3)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));
    void* table[39]{};table[2]=reinterpret_cast<void*>(&release);table[19]=reinterpret_cast<void*>(&restore);table[38]=reinterpret_cast<void*>(&unlock);
    for(U i=1;i<interfaces.size();++i)interfaces[i]={table,i};
    const auto* shutdown=reinterpret_cast<const unsigned char*>(0x58acf0);const Bytes shutdownBytes(shutdown,shutdown+154);
    const auto* retire=reinterpret_cast<const unsigned char*>(0x58b3e0);const Bytes retireBytes(retire,retire+216);
    if(argc==3){
        ReleaseCanvas owner{};global(0x6f98c8,1);global(0x6f6950,ptr(&owner));global(0x6f8154,1);
        using Shutdown=void(*)();reinterpret_cast<Shutdown>(0x58acf0)();throw std::runtime_error("Expected nonprogressing shutdown");
    }
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");Provider provider;
#endif
    U id=0;
    for(U present:{0u,1u})for(U mode:{0u,1u,2u})for(U clipper:{0u,1u})for(U count:{0u,1u,3u})
        for(U page:{0u,1u})for(U palette:{0u,1u})for(U m=0;m<8;++m)for(U hr:{0u,0x88760001u})for(U credits:{1u,2u}){
            session={};session.draw1=101;session.draw2=present?102:0;session.draw4=103;session.clipper=clipper?104:0;session.fullscreen=mode;
            session.canvases.count=count;session.canvases.primary=count?1:0xcafe0000u;session.canvases.pageLocked=page;
            for(U i=0;i<3;++i){for(U j=0;j<16;++j)canvases[i].words[j]=0xa5000000u+i*256+j;
                if(i<count){session.canvases.entries[i]=i+1;canvases[i].words[1]=200+i*3;canvases[i].words[2]=201+i*3;canvases[i].words[3]=palette?202+i*3:0;canvases[i].words[7]=i;}
            }
            calls.fill(0);events.clear();mutation=m;limit=credits;restoreResult=hr;fired=false;
#ifdef MNM_ORIGINAL_REFERENCE
            global(0x6f98c4,ptr(&interfaces[101]));global(0x6f98c8,present?ptr(&interfaces[102]):0);global(0x6f98cc,ptr(&interfaces[103]));global(0x6f98d0,clipper?ptr(&interfaces[104]):0);
            global(0x6f68d8,mode);global(0x6e1f1c,page);global(0x6f8154,count);global(0x6f98c0,count?ptr(&canvases[0]):0xcafe0000u);
            for(U i=0;i<128;++i)global(0x6f6950+i*4,i<count?ptr(&canvases[i]):0);
            for(U i=0;i<count;++i)for(U j:{1u,2u,3u})if(canvases[i].words[j])canvases[i].words[j]=ptr(&interfaces[canvases[i].words[j]]);
            using Shutdown=void(*)();reinterpret_cast<Shutdown>(0x58acf0)();
            session.draw1=token(*reinterpret_cast<U*>(0x6f98c4));session.draw2=token(*reinterpret_cast<U*>(0x6f98c8));session.draw4=token(*reinterpret_cast<U*>(0x6f98cc));session.clipper=token(*reinterpret_cast<U*>(0x6f98d0));session.fullscreen=*reinterpret_cast<U*>(0x6f68d8);
            session.canvases.count=*reinterpret_cast<U*>(0x6f8154);session.canvases.pageLocked=*reinterpret_cast<U*>(0x6e1f1c)!=0;
            const auto ownerToken=[](U pointer){if(!pointer||pointer==0xcafe0000u)return pointer;const U delta=pointer-ptr(canvases.data());if(delta>=sizeof(canvases)||delta%64)throw std::runtime_error("Registry owner");return delta/64+1;};
            session.canvases.primary=ownerToken(*reinterpret_cast<U*>(0x6f98c0));for(U i=0;i<128;++i)session.canvases.entries[i]=ownerToken(*reinterpret_cast<U*>(0x6f6950+i*4));
            for(U i=0;i<count;++i)for(U j:{1u,2u,3u})if(canvases[i].words[j])canvases[i].words[j]=token(canvases[i].words[j]);
#else
            releaseOriginalDrawingSession(session,provider,[&](U token)->ReleaseCanvas&{return canvases.at(token-1);});
#endif
            const U h[]{id++,present,mode,clipper,count,page,palette,m,hr,credits,session.draw1,session.draw2,session.draw4,session.clipper,session.fullscreen,session.canvases.count,session.canvases.primary,U(session.canvases.pageLocked),U(events.size())};
            emit(h,sizeof(h));emit(canvases.data(),sizeof(canvases));emit(session.canvases.entries.data(),512);emit(calls.data(),sizeof(calls));emit(events.data(),events.size()*4);
        }
#ifdef MNM_ORIGINAL_REFERENCE
    if(std::memcmp(shutdown,shutdownBytes.data(),shutdownBytes.size())||std::memcmp(retire,retireBytes.data(),retireBytes.size()))throw std::runtime_error("Original session bodies changed");
#else
    session={};session.draw2=102;session.canvases.entries[0]=1;session.canvases.count=1;canvases[0]={};events.clear();
    try{releaseOriginalDrawingSession(session,provider,[&](U)->ReleaseCanvas&{return canvases[0];});throw std::runtime_error("Native nonprogress accepted");}
    catch(const std::invalid_argument&){}
    if(!events.empty()||session.canvases.entries[0]!=1||session.draw2!=102)throw std::runtime_error("Native refusal mutated ownership");
#endif
    if(id!=4608)throw std::runtime_error("Session denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
