#include "../reconstruction/rendering/world_queue_owner.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
struct Provider {
    struct Block {void* address{};U size{};bool alive{};};
    std::array<Block,2> blocks{};std::vector<U> events;U calls{},failure{};unsigned char fill{};
    ~Provider(){for(auto& b:blocks)if(b.alive)std::free(b.address);}
    U allocate(U size){
        const U id=++calls;if(id>2||size!=(id==1?432000u:14577u))throw std::runtime_error("Allocation request");
        events.insert(events.end(),{1,size,id==failure?0:id});
        if(id==failure)return 0;
        void* p=std::malloc(size);if(!p)throw std::runtime_error("Host allocation");
        std::memset(p,fill,size);blocks[id-1]={p,size,true};return id;
    }
    void clear(U token,U size){auto& b=blocks.at(token-1);if(!b.alive||b.size!=size)throw std::runtime_error("Clear extent");std::memset(b.address,0,size);}
    U token(std::uintptr_t p)const{if(!p)return 0;for(U i=0;i<2;++i)if(p==reinterpret_cast<std::uintptr_t>(blocks[i].address))return i+1;throw std::runtime_error("Unknown allocation identity");}
    void release(U id){events.insert(events.end(),{2,id,0});if(!id)return;auto& b=blocks.at(id-1);if(!b.alive)throw std::runtime_error("Duplicate release");std::free(b.address);b.alive=false;}
    void snapshot(){for(auto& b:blocks){const U h[]{b.size,U(b.alive)};emit(h,sizeof(h));if(b.alive)emit(b.address,b.size);}}
};
#ifdef MNM_ORIGINAL_REFERENCE
static Provider* active;
extern "C" __attribute__((force_align_arg_pointer)) void* world_allocate(U size){try{const U id=active->allocate(size);return id?active->blocks[id-1].address:nullptr;}catch(...){_exit(81);}}
extern "C" __attribute__((force_align_arg_pointer)) void world_free(void* p){try{active->release(active->token(reinterpret_cast<std::uintptr_t>(p)));}catch(...){_exit(82);}}
template<std::size_t N>static void boundary(U at,void* destination,const unsigned char(&signature)[N]){
    auto* p=reinterpret_cast<unsigned char*>(at);if(std::memcmp(p,signature,N))throw std::runtime_error("Boundary signature");
    const U rel=U(reinterpret_cast<std::uintptr_t>(destination))-at-5;p[0]=0xe9;std::memcpy(p+1,&rel,4);
}
#endif
static void snapshot(OriginalWorldOwner owner,const WorldMaskGeometry& geometry,Provider& provider,bool normalize){
    if(normalize)for(unsigned at:{0x229u,0x22du,0x239u})owner.put(at,provider.token(owner.get(at)));
    emit(owner.bytes.data(),owner.bytes.size());emit(&geometry,sizeof(geometry));provider.snapshot();
    const U n=U(provider.events.size());emit(&n,4);emit(provider.events.data(),n*4);
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc<2||argc>3)throw std::runtime_error("Expected executable and optional fault mode");
    map_image(read(argv[1]));OriginalLinuxSeh seh;
    const unsigned char alloc[]{0x8b,0x44,0x24,4,0x6a,0},free[]{0x8b,0x44,0x24,4,0x85,0xc0};
    boundary(0x597850,reinterpret_cast<void*>(&world_allocate),alloc);boundary(0x597870,reinterpret_cast<void*>(&world_free),free);
    std::vector<std::pair<U,Bytes>> unchanged;
    for(auto range:std::vector<std::pair<U,U>>{{0x4bfda0,0x4bfeac},{0x4bdd80,0x4bddc6},{0x4a4df0,0x4a4e12},{0x4a4f70,0x4a4ffe},{0x464c50,0x464c78},{0x501350,0x5013b1}}){
        const auto* p=reinterpret_cast<unsigned char*>(range.first);unchanged.push_back({range.first,Bytes(p,p+range.second-range.first)});
    }
    if(argc==3){Provider provider;provider.failure=2;active=&provider;OriginalWorldOwner owner;owner.bytes.fill(0xa5);using Ctor=void*(__attribute__((thiscall)) *)(void*);reinterpret_cast<Ctor>(0x4bfda0)(&owner);throw std::runtime_error("Expected null-mask fault");}
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    for(U seed:{0u,0xa5u,0xffu})for(U fail:{0u,1u})for(U dropMask:{0u,1u})
        for(U index:{0xffffffffu,0u,1u,2u,3u})for(U selected:{0xffffffffu,0u,1u,2u,3u,4u}){
            Provider provider;provider.fill=static_cast<unsigned char>(seed);provider.failure=fail;
            OriginalWorldOwner owner;owner.bytes.fill(static_cast<unsigned char>(seed));WorldMaskGeometry geometry;
            const U h[]{id++,seed,fail,dropMask,index,selected};emit(h,sizeof(h));
#ifdef MNM_ORIGINAL_REFERENCE
            active=&provider;using Ctor=void*(__attribute__((thiscall)) *)(void*);
            if(reinterpret_cast<Ctor>(0x4bfda0)(&owner)!=&owner)throw std::runtime_error("Constructor receiver");
            geometry={*reinterpret_cast<U*>(0x6e8d58),*reinterpret_cast<U*>(0x6e8d5c),*reinterpret_cast<U*>(0x6e817c),*reinterpret_cast<U*>(0x6e8960),*reinterpret_cast<U*>(0x6e8178)};
            snapshot(owner,geometry,provider,true);
#else
            constructOriginalWorldOwner(owner,geometry,provider);snapshot(owner,geometry,provider,false);
#endif
            if(dropMask){provider.release(2);owner.put(0x239,0);}
            OriginalOwnerRegistry registry;registry.count=4;registry.selected=selected;
            for(U i=0;i<4;++i){registry.objects.push_back({0x1000+i,i,0x2000+i});registry.entries.push_back(i);}
            owner.put(4,index);
#ifdef MNM_ORIGINAL_REFERENCE
            U pointers[4];for(U i=0;i<4;++i)pointers[i]=U(reinterpret_cast<std::uintptr_t>(registry.objects[i].data()));
            if(index!=0xffffffffu)pointers[index]=U(reinterpret_cast<std::uintptr_t>(owner.bytes.data()));
            for(U i=0;i<4;++i)global(0x642034+i*4,pointers[i]);
            global(0x642c68,4);global(0x5e04cc,selected);
            using Destroy=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Destroy>(0x4bdd80)(&owner);
            registry.count=*reinterpret_cast<U*>(0x642c68);registry.selected=*reinterpret_cast<U*>(0x5e04cc);
            for(U i=0;i<4;++i){const U value=*reinterpret_cast<U*>(0x642034+i*4);bool found=false;for(U j=0;j<4;++j)if(value==pointers[j]){registry.entries[i]=j;found=true;break;}if(!found)throw std::runtime_error("Registry pointer");}
            if(index!=0xffffffffu)std::memcpy(registry.objects[index].data(),owner.bytes.data(),12);
            snapshot(owner,geometry,provider,true);seh.check();
#else
            destroyOriginalWorldOwner(owner,provider,[&](U at){
                if(at!=0xffffffffu){std::memcpy(registry.objects[at].data(),owner.bytes.data(),12);registry.remove(at);owner.put(8,registry.objects[at][2]);}
            });
            if(index!=0xffffffffu)std::memcpy(registry.objects[index].data(),owner.bytes.data(),12);
            snapshot(owner,geometry,provider,false);
#endif
            const U rh[]{registry.count,registry.selected};emit(rh,sizeof(rh));emit(registry.entries.data(),registry.entries.size()*4);emit(registry.objects.data(),registry.objects.size()*12);
        }
#ifdef MNM_ORIGINAL_REFERENCE
    for(const auto& [at,bytes]:unchanged)if(std::memcmp(reinterpret_cast<void*>(at),bytes.data(),bytes.size()))throw std::runtime_error("Original ownership body changed");
#endif
    if(id!=360)throw std::runtime_error("Owner denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
