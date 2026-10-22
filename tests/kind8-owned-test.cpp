#include "../reconstruction/rendering/kind8_owned.hpp"
#include <fstream>
#include <iostream>
using u32=std::uint32_t;using u16=std::uint16_t;using u8=std::uint8_t;
#include "kind8-owned-record.h"
using namespace mnm::reconstruction;
int main(int argc,char** argv)try{
    if(argc!=2)throw std::runtime_error("Expected owned output directory");
    const std::string out=argv[1];std::ofstream rng(out+"/native-rng.bin",std::ios::binary),f(out+"/native-owned.bin",std::ios::binary);
    const u32 seeds[]{0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef};
    for(auto seed:seeds){Kind8Random random(seed);for(u32 i=0;i<=1024;++i){
        const u32 header[]{seed,i,i?random.next():0,random.lcg,random.first,random.second};
        rng.write(reinterpret_cast<const char*>(header),sizeof(header));rng.write(reinterpret_cast<const char*>(random.words.data()),1000);
    }}
    u32 id=0;
    const int dimensions[]{-1,1,2,3,16,17},heights[]{-3,0,15,31,127,255,256,512,600};
    const std::array<int,2> positions[]{{-129,257},{0,0},{127,95}};
    const std::array<int,3> colours[]{{192,0,0},{24,0,0},{255,127,65},{-1,256,-7}};
    for(unsigned kind=0;kind<6;++kind)for(bool format:{false,true})for(int dimension:dimensions)
    for(bool enabled:{false,true})for(int z:heights)for(auto xy:positions)for(auto seed:seeds)
    for(bool deleting:{false,true})for(unsigned variant=0;variant<((kind==3||kind==4)?4u:1u);++variant){
        Kind8Random random(seed);auto object=constructOwnedKind8(Kind8Constructor(kind),{xy[0],xy[1],z,-7,9,-1},format,dimension,enabled,random);
        if(variant)setOwnedKind8Colour(object,colours[variant],format);
        Kind8OwnedRecord record{};
        const u32 header[]{id++,kind,u32(format),u32(dimension),u32(enabled),u32(z),u32(xy[0]),u32(xy[1]),seed,u32(deleting),u32(object.plane.size()),1,1,0,u32(kind==5),variant};
        std::copy(std::begin(header),std::end(header),record.header);
        std::copy(object.bytes.begin(),object.bytes.end(),record.constructed);
        std::copy(object.plane.begin(),object.plane.end(),record.plane);
        record.random[0]=random.first;record.random[1]=random.second;std::copy(random.words.begin(),random.words.end(),record.random+2);
        auto frees=retireOwnedKind8(object,deleting);
        if(!deleting){std::copy(object.bytes.begin(),object.bytes.end(),record.nondeleted);frees.insert(frees.end(),{2,1});object.alive=false;}
        record.header[13]=frees.size()/2;std::copy(frees.begin(),frees.end(),record.frees);
        f.write(reinterpret_cast<const char*>(&record),sizeof(record));
    }
    if(id!=93312||!rng||!f||sizeof(Kind8OwnedRecord)!=2896)throw std::runtime_error("Incomplete owned kind8 output");
    bool refused=false;Kind8Random random(0);
    auto dead=constructOwnedKind8(Kind8Constructor::flag,{0,0,256,0,0,0},false,2,false,random);
    retireOwnedKind8(dead,true);try{retireOwnedKind8(dead,true);}catch(const std::invalid_argument&){refused=true;}
    if(!refused)throw std::runtime_error("Duplicate retirement accepted");
    std::cout<<"Known kind8 owned construction/retirement and RNG outputs generated\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
