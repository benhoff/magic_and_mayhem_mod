#include "../reconstruction/rendering/particle_owner_lifecycle.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
struct OwnerArena {
    struct Block{void* address;U size;bool alive;};std::vector<Block> blocks;std::vector<U> events;
    OriginalCreatureSlot* owner{};U calls{},fail{};
    ~OwnerArena(){for(auto& b:blocks)if(b.alive)std::free(b.address);}
    U ownerAddress()const{
#ifdef MNM_ORIGINAL_REFERENCE
        return U(reinterpret_cast<std::uintptr_t>(owner));
#else
        return 0x10000;
#endif
    }
    U encode(U token)const{
        if(!token)return 0;
#ifdef MNM_ORIGINAL_REFERENCE
        return U(reinterpret_cast<std::uintptr_t>(blocks.at(token-1).address));
#else
        return token;
#endif
    }
    U decode(U value)const{
        if(!value)return 0;
#ifdef MNM_ORIGINAL_REFERENCE
        for(U i=U(blocks.size());i>0;--i)if(blocks[i-1].alive&&value==U(reinterpret_cast<std::uintptr_t>(blocks[i-1].address)))return i;
        throw std::runtime_error("Unknown owner allocation");
#else
        if(value>blocks.size())throw std::runtime_error("Unknown native allocation");
        return value;
#endif
    }
    U allocate(U size){
        ++calls;if(calls==fail){events.insert(events.end(),{1,size,0});return 0;}
        void* p=std::malloc(size);if(!p)throw std::runtime_error("Host allocation");std::memset(p,0xa5,size);
        blocks.push_back({p,size,true});events.insert(events.end(),{1,size,U(blocks.size())});return encode(U(blocks.size()));
    }
    void release(U address){
        const U token=decode(address);U first=token?read(address,0):0;
        if(token&&blocks[token-1].size==48)first=decode(first);
        events.insert(events.end(),{2,token,first});if(!token)return;
        auto& b=blocks.at(token-1);if(!b.alive)throw std::runtime_error("Double free");std::free(b.address);b.alive=false;
    }
    U read(U address,U at)const{const auto& b=blocks.at(decode(address)-1);if(!b.alive||at+4>b.size)throw std::runtime_error("Read extent");U value;std::memcpy(&value,static_cast<unsigned char*>(b.address)+at,4);return value;}
    void write(U address,U at,U value){auto& b=blocks.at(decode(address)-1);if(!b.alive||at+4>b.size)throw std::runtime_error("Write extent");std::memcpy(static_cast<unsigned char*>(b.address)+at,&value,4);}
    void byte(U address,U at,unsigned char value){auto& b=blocks.at(decode(address)-1);if(!b.alive||at>=b.size)throw std::runtime_error("Byte extent");static_cast<unsigned char*>(b.address)[at]=value;}
};
#ifdef MNM_ORIGINAL_REFERENCE
static OwnerArena* active;
extern "C" __attribute__((force_align_arg_pointer)) void* owner_allocate(U size){try{return reinterpret_cast<void*>(active->allocate(size));}catch(...){_exit(83);}}
extern "C" __attribute__((force_align_arg_pointer)) void owner_release(void* p){try{active->release(U(reinterpret_cast<std::uintptr_t>(p)));}catch(...){_exit(84);}}
template<std::size_t N>static void boundary(U at,void* destination,const unsigned char(&signature)[N]){
    auto* p=reinterpret_cast<unsigned char*>(at);if(std::memcmp(p,signature,N))throw std::runtime_error("Owner boundary signature");const U rel=U(reinterpret_cast<std::uintptr_t>(destination))-at-5;p[0]=0xe9;std::memcpy(p+1,&rel,4);
}
#endif
static void initialize(OriginalCreatureSlot& slot,OwnerArena& arena,U callback,U list){
    slot.bytes.fill(0x6d);slot.put(0x708,0);slot.put(0xcf3,0);slot.put(0x8f9,0);
    if(callback){const auto p=arena.allocate(16);arena.write(p,0,0x5c7264);arena.write(p,4,0x51e890);arena.write(p,8,0);arena.write(p,12,arena.ownerAddress());slot.put(0x708,p);slot.put(0xcf3,p);}
    if(list){const auto p=arena.allocate(12),head=arena.allocate(48);arena.write(p,4,head);arena.write(p,8,list-1);arena.write(head,0,head);arena.write(head,4,head);
        U previous=head;for(U i=0;i+1<list;++i){const auto node=arena.allocate(48);arena.write(node,0,head);arena.write(node,4,previous);arena.write(previous,0,node);arena.write(head,4,node);previous=node;}slot.put(0x8f9,p);
    }
}
static void snapshot(OriginalCreatureSlot slot,ParticleObject particle,OwnerArena& arena,U result){
    for(U at:{0x708u,0xcf3u,0x8f9u})slot.put(at,arena.decode(slot.get(at)));
    if(slot.get(0xac))slot.put(0xac,0x20000);
    particle.put(0x10,0x10000);
    emit(&result,4);emit(slot.bytes.data(),slot.bytes.size());emit(particle.bytes.data(),particle.bytes.size());
    const U h[]{U(arena.blocks.size()),U(arena.events.size())};emit(h,sizeof(h));
    // Pointer-bearing event heads were normalized while that allocation
    // generation was still alive, before its address could be reused.
    emit(arena.events.data(),arena.events.size()*4);
    for(U i=0;i<arena.blocks.size();++i){const auto& b=arena.blocks[i];const U fields[]{b.size,U(b.alive)};emit(fields,sizeof(fields));if(!b.alive)continue;
        std::vector<unsigned char> bytes(static_cast<const unsigned char*>(b.address),static_cast<const unsigned char*>(b.address)+b.size);
        auto normalize=[&](U at,U value){std::memcpy(bytes.data()+at,&value,4);};
        if(b.size==16)normalize(12,0x10000);
        if(b.size==12)normalize(4,arena.decode(arena.read(arena.encode(i+1),4)));
        if(b.size==48){normalize(0,arena.decode(arena.read(arena.encode(i+1),0)));normalize(4,arena.decode(arena.read(arena.encode(i+1),4)));}
        emit(bytes.data(),bytes.size());
    }
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc<2||argc>3)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));OriginalLinuxSeh seh;
    const unsigned char alloc[]{0x8b,0x44,0x24,4,0x6a,0},free[]{0x8b,0x44,0x24,4,0x85,0xc0};boundary(0x597850,reinterpret_cast<void*>(&owner_allocate),alloc);boundary(0x597870,reinterpret_cast<void*>(&owner_release),free);
    std::vector<std::pair<U,Bytes>> unchanged;
    for(auto range:std::vector<std::pair<U,U>>{{0x5060a0,0x50628e},{0x52cc70,0x52ccb4},{0x52cbe0,0x52cc07},{0x549820,0x5499b8},{0x549c90,0x549e0a}}){const auto* p=reinterpret_cast<unsigned char*>(range.first);unchanged.push_back({range.first,Bytes(p,p+range.second-range.first)});}
    if(argc==3){OriginalCreatureSlot slot;OwnerArena arena;arena.owner=&slot;active=&arena;initialize(slot,arena,0,0);arena.fail=U(std::stoul(argv[2]));using Reset=void(__attribute__((thiscall)) *)(void*,U);reinterpret_cast<Reset>(0x5060a0)(&slot,0);throw std::runtime_error("Expected list allocation fault");}
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    for(U callback:{0u,1u})for(U list:{0u,1u,4u})for(U index:{0u,1u,0xffffffffu})for(U position=0;position<3;++position)
        for(U offset=0;offset<3;++offset)for(U failure:{0u,1u})for(U alive:{0u,1u})for(U method:{0u,1u}){
            OriginalCreatureSlot slot;OwnerArena arena;arena.owner=&slot;initialize(slot,arena,callback,list);
            const U h[]{id++,callback,list,index,position,offset,failure,alive,method};emit(h,sizeof(h));
            U descriptor[104]{};descriptor[0x198/4]=U(int(offset)-3);descriptor[0x19c/4]=U(5-int(offset));
            ParticleObject particle;particle.put(0,method?0x5c7514:0x5c74fc);particle.put(4,4);particle.put(8,5);particle.put(12,6);particle.put(0x14,20);
            particle.put(0x18,-2);particle.put(0x1c,3);particle.put(0x20,4);
#ifdef MNM_ORIGINAL_REFERENCE
            active=&arena;particle.put(0x10,particleSigned(arena.ownerAddress()));
            const U descriptorAddress=U(reinterpret_cast<std::uintptr_t>(descriptor));global(0x5eb480,128);global(0x5eb484,96);global(0x5eb488,64);
#else
            particle.put(0x10,0x10000);const U descriptorAddress=0x20000;
#endif
            auto activate=[&](U activeFlag,U bias){slot.put(4,1);slot.put(0xe4,activeFlag);slot.put(0xac,descriptorAddress);slot.put(0x14,position*31+bias);slot.put(0x18,position*23+bias);slot.put(0x1c,position*17+bias);};
            activate(alive,0);
            for(U phase=0;phase<4;++phase){
                if(phase==1||phase==3){arena.fail=failure?arena.calls+1:0;
#ifdef MNM_ORIGINAL_REFERENCE
                    using Reset=void(__attribute__((thiscall)) *)(void*,U);reinterpret_cast<Reset>(0x5060a0)(&slot,index);seh.check();
#else
                    resetOriginalParticleOwner(slot,index,arena);
#endif
                }
                if(phase==2)activate(1,47); // Same storage, new authored activation facts.
                U result;
#ifdef MNM_ORIGINAL_REFERENCE
                using Update=U(__attribute__((thiscall)) *)(void*);result=reinterpret_cast<Update>(method?0x549c90:0x549820)(particle.bytes.data())&255;
#else
                const ParticleOwnerFacts owner{true,slot.get(0xe4)!=0,{particleSigned(slot.get(0x14)),particleSigned(slot.get(0x18)),particleSigned(slot.get(0x1c))},particleSigned(descriptor[0x198/4]),particleSigned(descriptor[0x19c/4]),0};
                result=updateAttachedParticle(particle,{128,96,64},owner);
#endif
                snapshot(slot,particle,arena,result);
            }
        }
#ifdef MNM_ORIGINAL_REFERENCE
    for(const auto& [at,bytes]:unchanged)if(std::memcmp(reinterpret_cast<void*>(at),bytes.data(),bytes.size()))throw std::runtime_error("Original owner lifecycle body changed");
#endif
    if(id!=1296)throw std::runtime_error("Owner lifecycle denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
