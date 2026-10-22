#include "../reconstruction/rendering/particle_motion.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static void output(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));
    const auto* code=reinterpret_cast<const unsigned char*>(0x545590);const Bytes original(code,code+0x1e3);
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native arguments");
#endif
    const std::array<int,3> positions[]{{-129,257,-1},{0,0,0},{127,95,128},{-5,8,INT32_MAX}};
    const std::array<int,3> velocities[]{{-7,9,-1},{0,0,0},{3,-2,1},{-257,129,INT32_MIN}};
    const std::array<int,3> colours[]{{192,0,0},{-1,256,-7},{255,127,65},{INT32_MAX,INT32_MIN,-17}};
    const int delays[]{0,1,-1,2},levels[]{0,1,101,-1},steps[]{1,1,7,-1};
    const U seeds[]{0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef};U id=0;
    for(unsigned type=0;type<8;++type)for(int width:{1,2,16,127})for(int depth:{0,1,64})
    for(unsigned pos=0;pos<4;++pos)for(unsigned vel=0;vel<4;++vel)for(unsigned config=0;config<4;++config)
    for(bool format:{false,true})for(unsigned ownerState=0;ownerState<3;++ownerState)for(unsigned seedIndex=0;seedIndex<(type==7?6u:1u);++seedIndex){
        [[maybe_unused]] ParticleWorld world{width,width+1,depth};ParticleObject object;
        for(unsigned i=0;i<object.bytes.size();++i)object.bytes[i]=unsigned(i*17+13);
        object.put(0,0x5c73c0);
        for(unsigned axis=0;axis<3;++axis){object.put(4+axis*4,positions[pos][axis]);object.put(16+axis*4,velocities[vel][axis]);object.put(0x1c+axis*4,colours[config][axis]);}
        object.put(0x28,levels[config]);object.put(0x2c,steps[config]);object.put(0x30,delays[config]);
        ParticleOwnerFacts owner{type==7||ownerState!=0,ownerState==2,{-12,17,7},config==2?-31:3,config==1?-9:5,ownerState==0?0u:ownerState==1?1u:0x12345678u};
        for(unsigned axis=0;axis<3;++axis){object.put(0x48+axis*4,positions[pos][axis]);object.put(0x54+axis*4,(config+axis)%2);}
        const unsigned ownerOffset=type==4?0x60:type==5||type==6?0x10:type==7?0x1c:0;
        if(ownerOffset)object.put(ownerOffset,owner.present?0x1010:0);
        if(type==5||type==6){object.put(0x14,levels[config]);for(unsigned axis=0;axis<3;++axis)object.put(0x18+axis*4,positions[pos][axis]);}
        const auto seed=seeds[seedIndex];Kind8Random random(seed);U result=0;
        const ParticleSeekConfig seeking{config==0?0:config==1?1:config==2?4:15,config==2?100:3,config==0?0:config==1?1:config==2?7:31};
#ifdef MNM_ORIGINAL_REFERENCE
        U descriptor[104]{},actor[72]{};
        descriptor[0x198/4]=U(owner.horizontalOffset);descriptor[0x19c/4]=U(owner.verticalOffset);
        actor[0xac/4]=reinterpret_cast<U>(descriptor);actor[0xe4/4]=owner.active;actor[0x114/4]=owner.keepAlive;
        for(unsigned axis=0;axis<3;++axis)actor[0x14/4+axis]=U(owner.position[axis]);
        if(ownerOffset)object.put(ownerOffset,owner.present?particleSigned(reinterpret_cast<U>(actor)):0);
        global(0x5eb480,width);global(0x5eb484,width+1);global(0x5eb488,depth);global(0x6e1f88,format);
        global(0x5e14c0,seeking.jitter);global(0x5e14b8,seeking.nearDistance);global(0x5e14bc,seeking.maximumVelocity);
        global(0x6574c8,random.first);global(0x6574cc,random.second);global(0x6f2a9c,random.lcg);std::memcpy(reinterpret_cast<void*>(0x6574d0),random.words.data(),1000);
        if(type<2){
            using Base=void(__attribute__((thiscall)) *)(void*,int,int,int);
            using Moving=void(__attribute__((thiscall)) *)(void*,int,int,int,int,int,int);
            const auto& p=positions[pos];const auto& v=velocities[vel];
            if(type==0)reinterpret_cast<Base>(0x545590)(object.bytes.data(),p[0],p[1],p[2]);
            else reinterpret_cast<Moving>(0x545650)(object.bytes.data(),p[0],p[1],p[2],v[0],v[1],v[2]);
        }else{
            const U entries[]{0,0,0x5456c0,0x546120,0x546350,0x549820,0x549c90,0x549380};
            using Update=U(__attribute__((thiscall)) *)(void*);result=reinterpret_cast<Update>(entries[type])(object.bytes.data());
            if(type==5||type==6)result&=255;
        }
        if(ownerOffset)object.put(ownerOffset,owner.present?0x1010:0);
        random.first=*reinterpret_cast<U*>(0x6574c8);random.second=*reinterpret_cast<U*>(0x6574cc);random.lcg=*reinterpret_cast<U*>(0x6f2a9c);std::memcpy(random.words.data(),reinterpret_cast<void*>(0x6574d0),1000);
#else
        switch(type){
        case 0:case 1:constructParticleMotion(object,world,positions[pos],velocities[vel],type==1);break;
        case 2:result=updateParticleMotion(object,world);break;
        case 3:result=updateFadingParticle(object,world,format);break;
        case 4:result=updateFadingParticle(object,world,format,&owner);break;
        case 5:case 6:result=updateAttachedParticle(object,world,owner);break;
        case 7:result=updateSeekingParticle(object,world,owner,seeking,random);break;
        }
#endif
        const U header[]{id++,type,U(width),U(depth),pos,vel,config,U(format),ownerState,seed,result};
        output(header,sizeof(header));output(object.bytes.data(),object.bytes.size());output(&random,sizeof(random));
    }
    if(id!=59904)throw std::runtime_error("Motion denominator "+std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
    if(std::memcmp(original.data(),code,original.size()))throw std::runtime_error("Original motion/constructor instructions changed");
#endif
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
