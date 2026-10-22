// Original 58b3e0 remains intact; COM results are explicit scripted boundaries.
// Compare every registry/object byte and ordered call, including the shared cap.
#include "../reconstruction/rendering/canvas_release.hpp"
#include <cstring>
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
static std::vector<U> events;
static U calls[3],zeroAt[3];
static U invoke(ReleaseOperation operation,U token) {
    U result=0xdeadbeef;
    if(operation==ReleaseOperation::release) {
        const unsigned index=token/100-1;
        result=++calls[index]>=zeroAt[index]?0:1;
    }
    events.insert(events.end(),{U(operation),token,result});return result;
}
static void emit(const void* data,std::size_t n){std::cout.write(static_cast<const char*>(data),n);}
#ifdef MNM_ORIGINAL_REFERENCE
struct Interface { U* table; U token; };
static U table[39];
static U __attribute__((stdcall)) drop(Interface* self){return invoke(ReleaseOperation::release,self->token);}
static U __attribute__((stdcall)) unlock(Interface* self,U flags){
    if(flags)throw std::runtime_error("PageUnlock flags");
    return invoke(ReleaseOperation::pageUnlock,self->token);
}
#endif
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected pinned PE");
    map_image(read(argv[1]));
    const auto* entry=reinterpret_cast<const unsigned char*>(0x58b3e0);
    const Bytes original(entry,entry+0xd8);
    table[2]=reinterpret_cast<U>(&drop);table[38]=reinterpret_cast<U>(&unlock);
    Interface interfaces[3]{{table,100},{table,200},{table,300}};
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    for(U count:{1u,2u,5u,128u})for(U at=0;at<count;++at) {
        if(at!=0&&at!=count/2&&at!=count-1)continue;
        for(U first:{1u,2u,999u,1000u,1001u})for(U second:{1u,2u,999u,1000u,1001u})
        for(bool locked:{false,true})for(bool palette:{false,true})
        for(bool primary:{false,true})for(bool absent:{false,true}) {
            std::array<ReleaseCanvas,128> objects;
            for(U i=0;i<128;++i)for(U j=0;j<16;++j)objects[i].words[j]=0xa5000000u+i*256+j;
            CanvasRegistry registry;registry.count=count;registry.pageLocked=locked;
            registry.primary=primary?at+1:0xcafe0000;
            for(U i=0;i<count;++i){registry.entries[i]=i+1;objects[i].words[7]=i;}
            auto& object=objects[at];object.words[1]=100;object.words[2]=absent?0:200;
            object.words[3]=palette?300:0;
            events.clear();calls[0]=calls[1]=calls[2]=0;
            zeroAt[0]=first;zeroAt[1]=second;zeroAt[2]=2;
#ifdef MNM_ORIGINAL_REFERENCE
            object.words[1]=reinterpret_cast<U>(interfaces);
            object.words[2]=absent?0:reinterpret_cast<U>(interfaces+1);
            object.words[3]=palette?reinterpret_cast<U>(interfaces+2):0;
            global(0x6e1f1c,locked);global(0x6f8154,count);
            global(0x6f98c0,primary?reinterpret_cast<U>(&object):0xcafe0000);
            for(U i=0;i<128;++i)global(0x6f6950+i*4,i<count?reinterpret_cast<U>(&objects[i]):0);
            using Release=void(__attribute__((thiscall)) *)(void*);
            reinterpret_cast<Release>(0x58b3e0)(&object);
            auto token=[&](U pointer){
                if(!pointer||pointer==0xcafe0000)return pointer;
                const U begin=reinterpret_cast<U>(objects.data()),delta=pointer-begin;
                if(delta>=sizeof(objects)||delta%sizeof(ReleaseCanvas))throw std::runtime_error("Unknown registry pointer");
                return delta/U(sizeof(ReleaseCanvas))+1;
            };
            for(U i=0;i<128;++i)registry.entries[i]=token(*reinterpret_cast<U*>(0x6f6950+i*4));
            registry.count=*reinterpret_cast<U*>(0x6f8154);registry.primary=token(*reinterpret_cast<U*>(0x6f98c0));
            for(U i=1;i<=3;++i)
                if(object.words[i]==reinterpret_cast<U>(interfaces+i-1))object.words[i]=100*i;
            if(std::memcmp(entry,original.data(),original.size()))throw std::runtime_error("Original release bytes changed");
#else
            releaseCanvas(at+1,object,registry,invoke,[&](U t)->ReleaseCanvas&{return objects.at(t-1);});
#endif
            const U header[]{id++,count,at,first,second,U(locked),U(palette),U(primary),U(absent),
                registry.count,registry.primary,U(events.size()),calls[0],calls[1],calls[2]};
            emit(header,sizeof(header));emit(objects.data(),sizeof(objects));
            emit(registry.entries.data(),sizeof(registry.entries));emit(events.data(),events.size()*4);
        }
    }
    if(id!=3600)throw std::runtime_error("Release matrix denominator");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
