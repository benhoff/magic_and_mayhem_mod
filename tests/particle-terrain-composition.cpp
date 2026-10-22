#include "../reconstruction/rendering/particle_terrain.hpp"
#include <fstream>
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
    if(argc!=3)throw std::runtime_error("Expected original executable and undefined-state journal");
    map_image(read(argv[1]));std::ofstream undefined(argv[2],std::ios::binary);if(!undefined)throw std::runtime_error("Undefined-state journal open");
    std::vector<std::pair<U,Bytes>> original;
    for(auto range:std::vector<std::pair<U,U>>{{0x548860,0x548aac},{0x5456c0,0x5457a0},{0x547890,0x547940},{0x4e10e0,0x4e1105}}){const auto* p=reinterpret_cast<const unsigned char*>(range.first);original.push_back({range.first,Bytes(p,p+range.second-range.first)});}
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    static_assert(sizeof(ParticleTerrainCell)==12);
    const std::array<int,3> positions[]{{0,0,1},{31,31,15},{127,95,63},{1,2,64},{7,8,0}};
    const std::array<int,3> velocities[]{{0,0,0},{1,2,-1},{-2,-1,1}};
    U id=0;
    for(U block:{0u,1u,2u})for(bool flag:{false,true})for(U occupancy=0;occupancy<3;++occupancy)
    for(int shade:{-127,-8,0,127})for(bool hit:{false,true})for(U pos=0;pos<5;++pos)for(U vel=0;vel<3;++vel)
    for(int remaining:{0,1,100})for(bool format:{false,true})for(U mutation=0;mutation<6;++mutation){
        ParticleTerrainFacts terrain;ParticleObject object;ParticleTerrainValidity validity;
        for(U i=0;i<object.bytes.size();++i)object.bytes[i]=unsigned(i*17+13);
        object.put(0,0x5c74b0);for(U axis=0;axis<3;++axis){object.put(4+axis*4,positions[pos][axis]);object.put(16+axis*4,velocities[vel][axis]);object.put(0x4a+axis*4,axis==0?255:axis==1?127:65);}
        object.put(0x1c,0);object.put(0x20,2);object.put(0x24,2);object.put(0x2e,0);object.put(0x42,0x3030);object.put(0x46,0);object.put(0x56,remaining);
        auto moving=object;updateParticleMotion(moving,{128,96,64});
        const U cx=U(moving.get(4))>>5,cy=U(moving.get(8))>>5,cz=U(moving.get(12))>>4;
        const U index=std::min(47u,cz*12+cy*4+cx);object.put(0x3e,index+1);
        object.put(0x32,hit?cx:UINT32_MAX);object.put(0x36,hit?cy:UINT32_MAX);object.put(0x3a,hit?cz:UINT32_MAX);
        for(U gen=0;gen<2;++gen){for(auto& cell:terrain.cells[gen]){cell.block=std::uint16_t(block);cell.bytes[9]=flag?0x40:0;}
            for(U i=0;i<48;++i)terrain.shades[gen][i]=std::int8_t(gen?127-shade-127:shade);}
        for(U d=0;d<3;++d)for(U i=0;i<356;++i)terrain.descriptors[d][i]=occupancy==0?0:occupancy==1?0xff:unsigned(i*73+d*29);
#ifdef MNM_ORIGINAL_REFERENCE
        const auto ptr=[](const void* p){return U(reinterpret_cast<std::uintptr_t>(p));};
        object.put(0x3e,particleSigned(ptr(&terrain.cells[0][index])));
        global(0x5eb480,128);global(0x5eb484,96);global(0x5eb488,64);global(0x6e1f88,format);global(0x5e1428,100);
        for(U n=0;n<4;++n)global(0x6cb8c2+n*4,n*12);
        for(U n=0;n<3;++n)global(0x6cb942+n*4,n*4);
        global(0x65660c,ptr(terrain.descriptors.data()));
#endif
        for(U stage=0;stage<3;++stage){
            if(stage==1)switch(mutation){
                case 0:break;
                case 1:for(auto& cell:terrain.cells[0]){cell.block=std::uint16_t((cell.block+1)%3);cell.bytes[9]^=0x40;}break;
                case 2:terrain.generation=1;break;
                case 3:for(auto& plane:terrain.shades)for(auto& value:plane)value=value==-127?127:-127;break;
                case 4:for(auto& descriptor:terrain.descriptors)for(auto& value:descriptor)value^=0xff;break;
                case 5:for(U at:{0x32u,0x36u,0x3au})object.put(at,particleSigned(UINT32_MAX));break;
            }
            U alive;
#ifdef MNM_ORIGINAL_REFERENCE
            global(0x6c54dc,ptr(terrain.cells[terrain.generation].data()));global(0x6c5c5c,ptr(terrain.shades[terrain.generation].data()));
            const auto before=object;
            using Update=U(__attribute__((thiscall)) *)(void*);alive=reinterpret_cast<Update>(0x548860)(object.bytes.data())&255;
            bool changed=false;for(U at:{0x32u,0x36u,0x3au})changed|=before.get(at)!=object.get(at);
            if(changed){
                const auto* cell=reinterpret_cast<const ParticleTerrainCell*>(U(object.get(0x3e)));
                validity.descriptorDefined=cell->block!=0;
            }
            auto recordObject=object;
            const U cached=U(object.get(0x3e)),cellBase=ptr(terrain.cells.data());
            if(cached<cellBase||cached>=cellBase+96*12||(cached-cellBase)%12)throw std::runtime_error("Cached cell escaped retained generations");
            recordObject.put(0x3e,1+(cached-cellBase)/12);
            if(!validity.descriptorDefined){const U raw[]{id,stage,U(object.get(0x42))};undefined.write(reinterpret_cast<const char*>(raw),sizeof(raw));recordObject.put(0x42,0);}
            else if(U(object.get(0x42))!=0x3030){const U descriptor=U(object.get(0x42)),base=ptr(terrain.descriptors.data());
                if(descriptor<base||descriptor>=base+3*356||(descriptor-base)%356)throw std::runtime_error("Defined descriptor escaped owned table");
                recordObject.put(0x42,0x2000+(descriptor-base)/356);}
#else
            alive=updateTerrainParticle(object,{128,96,64},terrain,100,format,validity);
            auto recordObject=object;if(!validity.descriptorDefined)recordObject.put(0x42,0);
#endif
            const U header[]{id,stage,block,U(flag),occupancy,U(shade),U(hit),pos,vel,U(remaining),U(format),mutation,alive,U(validity.descriptorDefined)};
            output(header,sizeof(header));output(recordObject.bytes.data(),recordObject.bytes.size());
        }++id;
    }
    if(id!=77760)throw std::runtime_error("Terrain denominator "+std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
    for(const auto& range:original)if(std::memcmp(reinterpret_cast<const void*>(range.first),range.second.data(),range.second.size()))throw std::runtime_error("Original terrain/motion/collision/colour body changed");
    if(!undefined)throw std::runtime_error("Undefined-state journal write");
#endif
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
