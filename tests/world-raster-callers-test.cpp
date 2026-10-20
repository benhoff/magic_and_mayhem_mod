#include <array>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <vector>
using u32=std::uint32_t;
using u8=std::uint8_t;
#include "../runtime/scene/world_raster_callers.h"
static void require(bool value){if(!value)throw std::runtime_error("Native raster caller control failed");}
int main(int argc,char** argv)try{
    require(argc==2);std::ifstream file(argv[1],std::ios::binary);require(bool(file));
    const std::vector<u8> bytes{std::istreambuf_iterator<char>(file),{}};
    auto word=[&](std::size_t at){require(at<=bytes.size()&&bytes.size()-at>=4);return u32(bytes[at])|u32(bytes[at+1])<<8|u32(bytes[at+2])<<16|u32(bytes[at+3])<<24;};
    const auto pe=word(60),optional=pe+24;require(word(pe)==0x4550&&word(optional+28)==0x400000);
    require(pe+24<=bytes.size());const unsigned count=bytes.at(pe+6)|unsigned(bytes.at(pe+7))<<8;
    const unsigned optionalSize=bytes.at(pe+20)|unsigned(bytes.at(pe+21))<<8;
    auto at=[&](u32 address,std::size_t length){
        require(address>=0x400000);const u32 rva=address-0x400000;
        for(unsigned i=0;i<count;++i){const auto row=optional+optionalSize+i*40;
            const auto begin=word(row+12),size=word(row+16),offset=word(row+20);
            if(rva<begin||rva-begin>size||length>size-(rva-begin))continue;
            const auto source=std::uint64_t(offset)+rva-begin;require(source<=bytes.size()&&length<=bytes.size()-source);
            return bytes.data()+source;
        }
        throw std::runtime_error("Pinned caller is not file-backed");
    };
    const auto* method=at(0x5c73fc,4);require((u32(method[0])|u32(method[1])<<8|u32(method[2])<<16|u32(method[3])<<24)==0x54aaa0);
    const std::array<u8,21> wrapper{0x8b,0x44,0x24,0x08,0x8b,0x54,0x24,0x04,0x50,0x52,0x83,0xc1,0x34,0xe8,0x5e,0xb1,0xff,0xff,0xc2,0x08,0x00};
    require(std::equal(wrapper.begin(),wrapper.end(),at(0x54aaa0,wrapper.size())));
    unsigned pairs=0,refusals=0;
    for(const auto& row:raster_callers){const auto* original=at(row.address,10);
        require(std::equal(row.bytes,row.bytes+10,original));
        for(unsigned tag=0;tag<64;++tag){const bool expected=bool(row.tags&(1ull<<tag));
            require(bool(mnm_world_raster_caller(row.address,tag,original))==expected);
            if(!expected){++refusals;continue;}++pairs;
            require(!mnm_world_raster_caller(0,tag,original));++refusals;
            for(unsigned byte=0;byte<10;++byte){std::array<u8,10> changed;std::copy(original,original+10,changed.begin());changed[byte]^=1;require(!mnm_world_raster_caller(row.address,tag,changed.data()));++refusals;}
        }
        require(!mnm_world_raster_caller(row.address,64,original));require(!mnm_world_raster_caller(row.address,UINT32_MAX,original));require(!mnm_world_raster_caller(row.address,26,nullptr));refusals+=3;
    }
    require(mnm_world_raster_caller(0x54aab2,26,at(0x54aab2,10)));
    require(!mnm_world_raster_caller(0x54aab2,52,at(0x54aab2,10)));
    std::cout<<"{\"success\":true,\"callers\":"<<std::size(raster_callers)<<",\"admitted_pairs\":"<<pairs<<",\"refusal_controls\":"<<refusals<<",\"inherited_kind8_admitted\":true}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
