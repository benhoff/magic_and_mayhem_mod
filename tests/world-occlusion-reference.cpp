#include "../reconstruction/rendering/world_occlusion.hpp"
#include <array>
#include <cstring>
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;using U=std::uint32_t;
static void emit(const void* p,std::size_t n){std::cout.write(static_cast<const char*>(p),n);}
static void put(std::vector<unsigned char>& b,unsigned at,U value){std::memcpy(b.data()+at,&value,4);}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::runtime_error("Expected original executable");
    map_image(read(argv[1]));
    const auto* body=reinterpret_cast<const unsigned char*>(0x5013c0);const Bytes unchanged(body,body+547);
#else
    (void)argv;if(argc!=1)throw std::runtime_error("Unexpected native input");
#endif
    U id=0;
    auto test=[&](U video,int kind,U present,U source,U height,int x,int y,U pattern){
        std::vector<unsigned char> sprite(256,0xa5),mask(14577+16,0x6d);
        put(sprite,0,256);put(sprite,12,pattern?7u:0u);put(sprite,16,pattern?0xfffffffdu:0u);
        put(sprite,32,40);put(sprite,36,source==0?0:source==1?256:128);
        sprite[41]=static_cast<unsigned char>(height);sprite[42]=static_cast<unsigned char>(pattern?3:0);sprite[43]=static_cast<unsigned char>(pattern?2:0);
        for(U i=0;i<17;++i){put(sprite,44+i*4,pattern==0?0x0000ffffu:0xaa550ff0u^(i*0x01010101u));put(sprite,128+i*4,pattern==0?0x00ffffffu:0xaa00ff55u^(i*0x1010101u));}
        for(U i=0;i<14577;++i)mask[8+i]=static_cast<unsigned char>(pattern==0?0:pattern==1?0xff:pattern==2?0x55:i*17u);
        const auto prior=mask;const auto sourceBefore=sprite;bool result;
#ifdef MNM_ORIGINAL_REFERENCE
        U header[]{0,0,0,0,present?U(reinterpret_cast<std::uintptr_t>(mask.data()+8)):0};
        global(0x6e817c,43);global(0x6de6d5,video);
        using Check=U(__attribute__((thiscall)) *)(void*,void*,int,int,int);
        result=(reinterpret_cast<Check>(0x5013c0)(header,sprite.data(),x,y,kind)&255)!=0;
#else
        result=originalWorldOcclusion(present?&mask:nullptr,8,43,sprite,x,y,kind,video!=0);
#endif
        if(sprite!=sourceBefore)throw std::runtime_error("Source mutated");
        for(U i=0;i<8;++i)if(mask[i]!=prior[i]||mask[mask.size()-1-i]!=prior[prior.size()-1-i])throw std::runtime_error("Occlusion guard");
        const U h[]{id++,video,U(kind),present,source,height,U(x),U(y),pattern,U(result)};emit(h,sizeof(h));emit(mask.data(),mask.size());
    };
    for(U video:{0u,1u})for(int kind:{-1,0,1,2,8,9,33,34})for(U present:{0u,1u})for(U source=0;source<3;++source)
        for(U height:{0u,1u,4u,17u})for(int x:{-33,-32,-1,0,671,672,831,832})for(int y:{-33,-32,-1,0,511,512,631,632})for(U pattern:{0u,1u})
            test(video,kind,present,source,height,x,y,pattern);
    for(int x=0;x<8;++x)for(int y=0;y<8;++y)for(U pattern=0;pattern<4;++pattern)for(int kind:{0,33})
        test(0,kind,1,2,17,x,y,pattern);
#ifdef MNM_ORIGINAL_REFERENCE
    if(std::memcmp(body,unchanged.data(),unchanged.size()))throw std::runtime_error("Original occlusion body changed");
#else
    std::vector<unsigned char> shortMask(32,0xa5),source(256,0);
    put(source,0,256);put(source,32,40);put(source,36,128);source[41]=17;
    const auto saved=shortMask;
    try{originalWorldOcclusion(&shortMask,0,43,source,0,0,0,false);throw std::runtime_error("Native short mask accepted");}
    catch(const std::invalid_argument&){}
    if(shortMask!=saved)throw std::runtime_error("Native refusal partially stamped");
#endif
    if(id!=49664)throw std::runtime_error("Occlusion denominator");
    std::cerr<<"cases="<<id<<'\n';return std::cout?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
