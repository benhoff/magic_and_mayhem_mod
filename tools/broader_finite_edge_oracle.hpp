#pragma once
// Oracle-only application of the documented native retained-destination policy.
// The original PE instructions and every in-range sample remain unchanged.
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>
namespace broader_edge {
inline std::uint32_t word(const std::vector<unsigned char>& b,std::size_t at){
 if(at>b.size()||b.size()-at<4)throw std::runtime_error("Policy source word bounds");
 std::uint32_t result;std::memcpy(&result,b.data()+at,4);return result;
}
struct Tap {unsigned ordinal,destination,sample;std::uint16_t retained;};
inline std::vector<Tap> taps;
class Guard final {
 std::vector<std::uint16_t>& pixels_;std::vector<std::pair<unsigned,std::uint16_t>> restore_;
public:
 Guard(unsigned ordinal,unsigned entry,const std::vector<unsigned char>& record,std::vector<std::uint16_t>& pixels):pixels_(pixels){
  if(word(record,8)!=9||word(record,56)!=3)return;
  const auto width=word(record,20),height=word(record,24),size=word(record,76),period=word(record,64),backend=word(record,60);
  if(!width||!height||width>2048||height>2048||pixels.size()!=std::size_t(width)*height||size<40||record.size()!=96+size+64||word(record,80)!=64||!period||period>16||!((backend==9&&(entry==0x596490||entry==0x5968a4))||(backend==7&&entry==0x5806f0)))throw std::runtime_error("Finite-edge oracle domain");
  const auto sw=word(record,100),sh=word(record,104);
  if(!sw||!sh||sw>2048||sh>2048||40+sh*8>size||word(record,96)!=size)throw std::runtime_error("Policy sprite dimensions");
  const auto left=std::int64_t(std::int32_t(word(record,32)))-std::int32_t(word(record,108));
  const auto top=std::int64_t(std::int32_t(word(record,36)))-std::int32_t(word(record,112));
  const std::int64_t clipLeft=std::int32_t(word(record,40)),clipTop=std::int32_t(word(record,44)),clipRight=std::int32_t(word(record,48)),clipBottom=std::int32_t(word(record,52));
  if(clipLeft<0||clipTop<0||clipRight>width||clipBottom>height||clipRight<clipLeft||clipBottom<clipTop)throw std::runtime_error("Policy clip bounds");
  for(unsigned i=0;i<16;++i)if(word(record,96+size+i*4)>16)throw std::runtime_error("Policy displacement bounds");
  const auto first=word(record,140);
  for(unsigned row=0;row<sh;++row){
   const auto begin=word(record,136+row*8),end=row+1<sh?word(record,144+row*8):first;
   if(begin<40+sh*8||begin>end||end>first||first>size)throw std::runtime_error("Policy sprite row bounds");
   const auto y=top+row;unsigned x=0;bool opaque=false;
   for(unsigned at=begin;at<end;++at){const unsigned run=record.at(96+at);if(run>sw-x)throw std::runtime_error("Policy sprite run bounds");
    if(opaque&&y>=clipTop&&y<clipBottom){
     const auto delta=word(record,96+size+(unsigned(y)%period)*4);
     for(unsigned col=x;col<x+run;++col){const auto px=left+col;if(px<clipLeft||px>=clipRight)continue;
      const auto destination=std::uint64_t(y)*width+px,sample=destination+delta;
      if(sample>=pixels.size()){const auto d=unsigned(destination);restore_.emplace_back(d,pixels.at(d));taps.push_back({ordinal,d,unsigned(sample),pixels.at(d)});}
     }
    }
    x+=run;opaque=!opaque;
   }
   if(x!=sw)throw std::runtime_error("Policy incomplete sprite row");
  }
  // Author bounded backing solely for the isolated original call. The vector's
  // logical extent and every in-range pixel are unchanged. Those padding reads
  // receive no original-equivalence credit; their covered destinations restore.
  const auto logical=pixels.size();pixels.resize(logical+32,0);pixels.resize(logical);
 }
 ~Guard(){for(const auto& value:restore_)pixels_[value.first]=value.second;}
 Guard(const Guard&)=delete;Guard& operator=(const Guard&)=delete;
};
}
