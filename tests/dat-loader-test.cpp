#include "dat.hpp"
#include <cstdlib>
#include <iostream>
using namespace mnm::assets;
void check(bool ok) {if(!ok) {std::cerr<<"DAT check failed\n"; std::exit(1);}}
void word(std::vector<std::uint8_t>& b,std::uint32_t v) {for(int i=0;i<4;++i) b.push_back(static_cast<std::uint8_t>(v>>(i*8)));}
template<class T> void error(const T& result,DatErrorCode code) {auto* e=std::get_if<DatError>(&result);check(e && e->code==code);}
int main() {
    std::vector<std::uint8_t> b; for(auto v:{7U,999U,1U,0x7fc01234U,1U}) word(b,v);
    for(unsigned i=0;i<12;++i) word(b,0x80000000U+i);
    word(b,0xffffffff); word(b,0x80000000);
    auto result=decodeBrainDat(b);check(std::holds_alternative<BrainDat>(result));
    auto a=std::get<BrainDat>(std::move(result)); b[0]=0;
    check(a.models[0].key==7 && a.models[0].dimension==999 && a.models[0].layers[0].scalarBits==0x7fc01234 && a.models[0].layers[0].matrix[0]==0x80000000);
    for(std::size_t n=1;n<b.size();++n) error(decodeBrainDat({b.begin(),b.begin()+static_cast<std::ptrdiff_t>(n)}),DatErrorCode::malformedData);
    DatLimits limits;limits.nodes=0;error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    limits={}; limits.layers=0;error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    limits={}; limits.records=0;error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    limits={}; limits.decodedBytes=sizeof(BrainModel);error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    limits={}; limits.inputBytes=b.size()-1;error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    b.clear();for(auto v:{7U,1U,0xffffffffU,0x80000000U,0x7fc01234U}) word(b,v);
    auto s=std::get<ExperienceDat>(decodeExperienceDat(b));b[0]=0;
    check(s.samples[0].key==7 && s.samples[0].values[0]==0xffffffff && s.samples[0].parameters[1]==0x7fc01234);
    for(std::size_t n=1;n<b.size();++n) error(decodeExperienceDat({b.begin(),b.begin()+static_cast<std::ptrdiff_t>(n)}),DatErrorCode::malformedData);
    limits={};limits.values=0;error(decodeExperienceDat(b,limits),DatErrorCode::limitExceeded);
    limits={};limits.decodedBytes=sizeof(ExperienceSample);error(decodeExperienceDat(b,limits),DatErrorCode::limitExceeded);
    b[4]=b[5]=b[6]=b[7]=255;error(decodeExperienceDat(b),DatErrorCode::limitExceeded);
    check(std::get<BrainDat>(decodeBrainDat({})).models.empty());check(std::get<ExperienceDat>(decodeExperienceDat({})).samples.empty());
    b.clear();for(auto v:{9U,0U,1U,2U}) word(b,v);auto copy=b;b.insert(b.end(),copy.begin(),copy.end());
    check(std::get<ExperienceDat>(decodeExperienceDat(b)).samples.size()==2);
    b.clear();for(auto v:{9U,0U,1U,2U,0U,3U}) word(b,v);
    check(std::get<BrainDat>(decodeBrainDat(b)).models[0].layers[0].nodes.empty());
    b.clear();for(auto v:{9U,1U,1U,2U,0xffffffffU,0U}) word(b,v);
    limits={};limits.nodes=0xffffffffU;error(decodeBrainDat(b,limits),DatErrorCode::limitExceeded);
    std::cout<<"DAT ownership, truncation, bit preservation and limits passed\n";
}
