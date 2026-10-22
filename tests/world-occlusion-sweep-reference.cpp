#include "../reconstruction/rendering/world_occlusion_sweep.hpp"
#include <cstring>
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
static void put(std::vector<unsigned char>& bytes,unsigned at,U value){std::memcpy(bytes.data()+at,&value,4);}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));std::vector<std::pair<U,Bytes>> unchanged;
    for(auto range:std::vector<std::pair<U,U>>{{0x5015f0,0x5017da},{0x5013c0,0x5015e3},{0x5017e0,0x5017ff}}){
        const auto* p=reinterpret_cast<unsigned char*>(range.first);unchanged.push_back({range.first,Bytes(p,p+range.second-range.first)});
    }
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    for(U count:{0u,1u,2u,5u})for(int kind:{0,1,2,8,9,33})for(U mapping=0;mapping<4;++mapping)
        for(U clock:{0u,3u})for(U pattern:{0u,1u})for(U association=0;association<3;++association)
        for(U match=0;match<4;++match)for(U video:{0u,1u})for(U coordinates=0;coordinates<3;++coordinates){
            OcclusionCatalog catalog;catalog.lastFrame=2;
            for(U frame=0;frame<5;++frame){
                std::vector<unsigned char> sprite(256,0xa5);put(sprite,0,256);put(sprite,12,0);put(sprite,16,0);put(sprite,32,40);put(sprite,36,128);
                sprite[41]=4;sprite[42]=sprite[43]=0;
                for(U row=0;row<4;++row){put(sprite,44+row*4,0x0000ffffu^(frame*0x010101u));put(sprite,128+row*4,0x0000ffffu^(frame*0x010101u));}
                catalog.frames.push_back(std::move(sprite));
            }
            const U selections[4][3]{{0,1,2},{0,0,0},{2,3,4},{1,2,0}};
            std::vector<unsigned char> resources(2048,0x6d),mask(14577+16,0x6d);
            for(U i=0;i<14577;++i)mask[8+i]=pattern?0xff:0;
            for(U actor=0;actor<3;++actor)for(U tick=0;tick<4;++tick)for(U slot=0;slot<3;++slot){
                const U selected=selections[(mapping+actor+tick)%4][slot];catalog.actorFrames[actor][tick][slot]=selected;
                put(resources,(slot==0?0x84u:slot==1?0xd0u:0xe0u)+actor*356+tick*4,selected);
            }
            std::vector<OcclusionActor> actors(3);
            for(U a=0;a<3;++a)for(U w=0;w<6;++w)actors[a][w]=std::uint16_t(w==0?a:pattern?0xffff:0);
            std::vector<PhysicalWorldRow> rows(6);
            for(U i=0;i<6;++i){
                for(U j=0;j<9;++j)rows[i][j]=0xa5000000u+i*257+j;
                const U actor=i%3;const auto& selection=catalog.actorFrames[actor][clock];
                rows[i][1]=match==3?5:catalog.resolve(selection[match]);
                rows[i][2]=U(coordinates==0?8:coordinates==1?-33:(video?831:671));
                rows[i][3]=U(coordinates==0?8:coordinates==1?-33:(video?631:511));
                rows[i][5]=association==0?0:association==1?(i%2?actor+1:0):actor+1;rows[i][6]=U(kind);
            }
            const auto sourceBefore=catalog.frames;const auto resourcesBefore=resources;
#ifdef MNM_ORIGINAL_REFERENCE
            const auto pointer=[](const void* p){return U(reinterpret_cast<std::uintptr_t>(p));};
            std::vector<unsigned char> source(5*256);U offsets[5];
            for(U i=0;i<5;++i){offsets[i]=i*256;std::memcpy(source.data()+i*256,catalog.frames[i].data(),256);}
            U asset[]{0,0,0,2},catalogHeader[]{pointer(asset),0,pointer(offsets),pointer(source.data())};
            global(0x68995d,pointer(catalogHeader));global(0x689961,clock);global(0x65660c,pointer(resources.data()));
            global(0x6e817c,43);global(0x6de6d5,video);
            for(auto& row:rows){row[1]=pointer(source.data()+(row[1]-1)*256);if(row[5])row[5]=pointer(actors[row[5]-1].data());}
            U header[]{pointer(rows.data()),0,count,6,pointer(mask.data()+8)};
            using Sweep=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Sweep>(0x5015f0)(header);
            for(auto& row:rows){
                const U delta=row[1]-pointer(source.data());if(delta>=source.size()||delta%256)throw std::runtime_error("Sprite identity");row[1]=delta/256+1;
                if(row[5]){const U deltaActor=row[5]-pointer(actors.data());if(deltaActor>=actors.size()*12||deltaActor%12)throw std::runtime_error("Actor identity");row[5]=deltaActor/12+1;}
            }
            for(U i=0;i<5;++i)if(std::memcmp(source.data()+i*256,catalog.frames[i].data(),256))throw std::runtime_error("Borrowed sprite mutated");
#else
            sweepOriginalWorldOcclusion(rows,count,actors,catalog,clock,mask,8,video!=0);
#endif
            if(resources!=resourcesBefore||catalog.frames!=sourceBefore)throw std::runtime_error("Catalog mutation");
            const U h[]{id++,count,U(kind),mapping,clock,pattern,association,match,video,coordinates};emit(h,sizeof(h));emit(rows.data(),rows.size()*36);emit(actors.data(),actors.size()*12);emit(mask.data(),mask.size());
        }
#ifdef MNM_ORIGINAL_REFERENCE
    for(const auto& [at,bytes]:unchanged)if(std::memcmp(reinterpret_cast<void*>(at),bytes.data(),bytes.size()))throw std::runtime_error("Original sweep body changed");
#endif
    if(id!=27648)throw std::runtime_error("Sweep denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
