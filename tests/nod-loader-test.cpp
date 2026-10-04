#include "nod.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool value) {if(!value) throw std::runtime_error("NOD assertion failed");}
void put(std::vector<std::uint8_t>& bytes,std::size_t at,std::uint32_t value) {
    for(unsigned i=0;i<4;++i) bytes.at(at+i)=static_cast<std::uint8_t>(value>>(i*8));
}
std::vector<std::uint8_t> fixture(std::uint32_t count=1) {
    std::vector<std::uint8_t> bytes(24+count*494);
    put(bytes,0,0x00444f4e);put(bytes,4,bytes.size());put(bytes,8,1);put(bytes,12,count);
    for(std::size_t i=16;i<bytes.size();++i) bytes[i]=static_cast<std::uint8_t>(i);
    return bytes;
}
void rejects(const std::vector<std::uint8_t>& bytes,NodErrorCode code,const NodLimits& limits={}) {
    auto result=decodeNod(bytes,limits);require(std::holds_alternative<NodError>(result));
    require(std::get<NodError>(result).code==code);
}
}
int main() try {
    auto bytes=fixture();put(bytes,16,1);put(bytes,20,0xffffffffU);put(bytes,24,0x80000000U);
    put(bytes,28,0x7fffffffU);put(bytes,32,0xfffffffeU);put(bytes,36,88);put(bytes,40,0xffffffffU);
    put(bytes,16+478,0x87654321);put(bytes,bytes.size()-8,1000);put(bytes,bytes.size()-4,123);
    auto result=decodeNod(bytes);auto asset=std::get<NodAsset>(result);bytes.clear();
    require(asset.sourceBytes==518 && asset.nodes.size()==1 && asset.trailerWords[1]==123);
    const auto& node=asset.nodes[0];require(node.word4==0xffffffffU && node.position[0]==(-2147483647-1));
    require(node.position[1]==2147483647 && node.position[2]==-2);
    require(node.connections[0].value==88 && node.connections[0].target==-1);
    require(node.connections[0].metadata[0]==44 && node.connections[0].metadata[16]==60);
    require(node.tailWords[2]==0x87654321);
    require(std::get<NodAsset>(decodeNod(fixture(2))).nodes.size()==2);
    require(std::get<NodAsset>(decodeNod(fixture(0))).nodes.empty());
    bytes=fixture();put(bytes,0,0);rejects(bytes,NodErrorCode::invalidFormat);
    bytes=fixture();put(bytes,8,2);rejects(bytes,NodErrorCode::unsupportedVersion);
    bytes=fixture();put(bytes,4,0);rejects(bytes,NodErrorCode::malformedData);
    bytes=fixture();put(bytes,12,0xffffffffU);rejects(bytes,NodErrorCode::limitExceeded);
    bytes=fixture();put(bytes,12,2);rejects(bytes,NodErrorCode::malformedData);
    const auto good=fixture();
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        if(n>=16) put(bytes,4,n);
        rejects(bytes,NodErrorCode::malformedData);
    }
    bytes=good;bytes.push_back(0);put(bytes,4,bytes.size());rejects(bytes,NodErrorCode::malformedData);
    NodLimits limits;limits.nodes=0;rejects(good,NodErrorCode::limitExceeded,limits);
    limits={};limits.inputBytes=good.size()-1;rejects(good,NodErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=sizeof(NodNode)-1;rejects(good,NodErrorCode::limitExceeded,limits);
    limits.decodedBytes=sizeof(NodNode);require(std::holds_alternative<NodAsset>(decodeNod(good,limits)));
    std::cout<<"NOD packed slots, signed fields, raw metadata, trailer, ownership and limits passed\n";
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
