#include "animation.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void word(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v){for(unsigned k=0;k<4;++k)b.at(at+k)=std::uint8_t(v>>(k*8));}
static std::vector<std::uint8_t> fixture(){
    std::vector<std::uint8_t> b(44+12+3*44,0);word(b,0,0x00494e41);word(b,4,b.size());word(b,8,3);word(b,12,5);word(b,16,0xaabbccdd);word(b,20,3);
    b[24]='x';b[43]=255;word(b,44,0);word(b,48,2);word(b,52,3);
    word(b,56,0);word(b,60,17);word(b,64,0xfffffffe);word(b,68,0x80000000);word(b,72,0x12345678);
    word(b,100,6);word(b,104,0xffffffff);word(b,144,6);word(b,148,0xffffffff);return b;
}
int main()try{
    auto b=fixture();auto parsed=decodeAnimation(b);require(std::holds_alternative<Animation>(parsed),"Fixture decode failed");
    auto a=std::get<Animation>(std::move(parsed));b.assign(b.size(),0);
    require(a.records.size()==3 && a.starts==std::vector<std::uint32_t>{0,2,3} && a.records[1].argument==-1,"Owned records/offsets differ");
    require(a.spriteName[0]=='x' && a.spriteName[19]==255 && a.opaqueHeader==0xaabbccdd && a.records[0].metadata[1]==0x80000000,"Raw metadata lost");
    auto rejected=[](std::vector<std::uint8_t> input,AnimationErrorCode code,AnimationLimits limits={}){auto result=decodeAnimation(input,limits);require(std::holds_alternative<AnimationError>(result),"Malformed ANI accepted");require(std::get<AnimationError>(result).code==code,"ANI error category differs");};
    rejected({},AnimationErrorCode::malformedData);
    b=fixture();word(b,0,0);rejected(b,AnimationErrorCode::invalidFormat);
    b=fixture();word(b,12,4);rejected(b,AnimationErrorCode::unsupportedVersion);
    b=fixture();word(b,4,200);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,8,4);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,20,1);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,44,1);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,48,0);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,52,4);rejected(b,AnimationErrorCode::malformedData);
    b=fixture();word(b,100,0);rejected(b,AnimationErrorCode::malformedData);
    AnimationLimits limits;limits.inputBytes=10;rejected(fixture(),AnimationErrorCode::limitExceeded,limits);
    limits={};limits.records=2;rejected(fixture(),AnimationErrorCode::limitExceeded,limits);
    limits={};limits.sequences=1;rejected(fixture(),AnimationErrorCode::limitExceeded,limits);
    std::cout<<"ANI ownership, raw metadata, extents, terminal records and budgets pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
