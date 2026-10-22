#include "../reconstruction/rendering/world_queue_sort.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
int main(int argc,char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));
    const auto* body=reinterpret_cast<const unsigned char*>(0x4fff60);
    const Bytes original(body,body+723);
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    auto test=[&](const std::vector<U>& keys,U tail){
        std::vector<PhysicalWorldRow> rows(keys.size()+tail);
        for(U i=0;i<rows.size();++i)for(U j=0;j<9;++j)
            rows[i][j]=0xa5000000u+i*257+j*0x10201u;
        for(U i=0;i<keys.size();++i)rows[i][0]=keys[i];
        const auto before=rows;
#ifdef MNM_ORIGINAL_REFERENCE
        U header[]{U(reinterpret_cast<std::uintptr_t>(rows.data())),0,U(keys.size()),U(rows.size()),0};
        using Sort=void(__attribute__((thiscall)) *)(void*);
        reinterpret_cast<Sort>(0x4fff60)(header);
#else
        sortOriginalWorldRows(rows,U(keys.size()));
#endif
        for(U i=U(keys.size());i<rows.size();++i)if(rows[i]!=before[i])throw std::runtime_error("Sort wrote inactive row");
        for(U i=1;i<keys.size();++i)if(worldOrderKey(rows[i-1][0])>worldOrderKey(rows[i][0]))throw std::runtime_error("Signed order");
        const U h[]{id++,U(keys.size()),U(rows.size())};emit(h,sizeof(h));emit(rows.data(),rows.size()*36);
    };
    for(U size=0;size<=8;++size){
        std::vector<U> permutation(size);for(U i=0;i<size;++i)permutation[i]=i;
        do {test(permutation,2);}while(std::next_permutation(permutation.begin(),permutation.end()));
    }
    for(U size:{0u,1u,2u,3u,4u,5u,8u,16u,31u,64u,127u,257u,1024u,12000u})
        for(U pattern=0;pattern<8;++pattern)for(U seed:{0u,1u,0xdeadbeefu}){
            std::vector<U> keys(size);U random=seed;
            for(U i=0;i<size;++i){random=random*1664525u+1013904223u;
                const U edge[]{0x80000000u,0xffffffffu,0,1,0x7fffffffu};
                keys[i]=pattern==0?i:pattern==1?size-i:pattern==2?seed:pattern==3?edge[i%5]:
                    pattern==4?random:pattern==5?i%3:pattern==6?(i<size/2?i:size-i):random%7;
            }test(keys,3);
        }
#ifdef MNM_ORIGINAL_REFERENCE
    if(std::memcmp(body,original.data(),original.size()))throw std::runtime_error("Original sort changed");
#else
    std::vector<PhysicalWorldRow> one(1);const auto saved=one;
    try{sortOriginalWorldRows(one,2);throw std::runtime_error("Invalid extent accepted");}
    catch(const std::invalid_argument&){}
    if(one!=saved)throw std::runtime_error("Refusal mutated rows");
#endif
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
