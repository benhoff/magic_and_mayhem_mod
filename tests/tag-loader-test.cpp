#include "tag.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("TAG assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v) {
    for(std::size_t i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(v>>(8*i));
}
void error(const std::vector<std::uint8_t>& b,TagErrorCode expected,const TagLimits& limits={}) {
    const auto result=decodeTag(b,limits);require(std::holds_alternative<TagError>(result));
    require(std::get<TagError>(result).code==expected);
}
}
int main() try {
    std::vector<std::uint8_t> good(36,0);
    good[0]='A';good[2]=0xff; // NUL terminator followed by preserved nonzero bytes.
    put(good,8,0xffffffffU);
    for(std::size_t i=12;i<20;++i) good[i]=0xe9; // Full unterminated name.
    put(good,20,0x80000000U);
    auto bytes=good;auto result=decodeTag(bytes);require(std::holds_alternative<TagAsset>(result));
    auto asset=std::get<TagAsset>(result);bytes.clear();
    require(asset.sourceBytes==36 && asset.entries.size()==3);
    require(tagEntryName(asset.entries[0])=="A" && asset.entries[0].nameBytes[2]==0xff);
    require(asset.entries[0].occurrence==0xffffffffU);
    require(tagEntryName(asset.entries[1])==std::string(8,static_cast<char>(0xe9)));
    require(asset.entries[1].occurrence==0x80000000U);
    require(tagEntryName(asset.entries[2]).empty());
    // Headerless files cannot detect truncation at a complete record boundary.
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        if(n%12) error(bytes,TagErrorCode::malformedData);
        else require(std::get<TagAsset>(decodeTag(bytes)).entries.size()==n/12);
    }
    for(std::size_t n=1;n<12;++n) {
        bytes=good;bytes.resize(good.size()+n);error(bytes,TagErrorCode::malformedData);
    }
    bytes=good;bytes.resize(48);require(std::get<TagAsset>(decodeTag(bytes)).entries.size()==4);
    TagLimits limits;limits.inputBytes=35;error(good,TagErrorCode::limitExceeded,limits);
    limits={};limits.records=2;error(good,TagErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=3*sizeof(TagEntry)-1;error(good,TagErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=3*sizeof(TagEntry);require(std::holds_alternative<TagAsset>(decodeTag(good,limits)));
    limits={};limits.records=0;limits.decodedBytes=0;require(std::get<TagAsset>(decodeTag({},limits)).entries.empty());
    std::cout<<"TAG raw names, unsigned occurrences, ownership, partial records and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
