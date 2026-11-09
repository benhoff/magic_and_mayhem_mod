#include <array>
#include <cassert>
#include <cstdint>
using u32=std::uint32_t;
using u8=std::uint8_t;
#include "../runtime/native-draw/hidden_spellbox_copy.h"
int main(){
 const std::array<u8,5> call={0xe8,0xa6,0x53,1,0};
 unsigned checked=0,accepted=0;
 // Independent geometric oracle: positive40x40 footprint, same placement,
 // no intersection with either640x480 plane, plus exact caller/profile.
 for(int x: {-40,-1,0,600,601,639,640,641,800})
 for(int y: {-40,-1,0,440,441,479,480,512,600}){
  const std::array<u32,4> r={u32(x),u32(y),u32(x+40),u32(y+40)};
  const bool expected=x>=0&&y>=0&&(x>=640||y>=480);
  const bool actual=nd_hidden_spellbox_copy(15,0x5770fa,call.data(),640,480,16,640,480,16,r.data(),u32(x),u32(y));
  assert(actual==expected);++checked;accepted+=actual;
 }
 const std::array<u32,4> r={640,512,680,552};
 assert(!nd_hidden_spellbox_copy(15,0x5770fb,call.data(),640,480,16,640,480,16,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(14,0x5770fa,call.data(),640,480,16,640,480,16,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(15,0x5770fa,call.data(),800,600,16,640,480,16,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(15,0x5770fa,call.data(),640,480,16,800,600,16,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(15,0x5770fa,call.data(),640,480,32,640,480,16,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(15,0x5770fa,call.data(),640,480,16,640,480,32,r.data(),640,512));
 assert(!nd_hidden_spellbox_copy(15,0x5770fa,call.data(),640,480,16,640,480,16,r.data(),641,512));
 for(unsigned i=0;i<call.size();++i){auto changed=call;changed[i]^=1;
  assert(!nd_hidden_spellbox_copy(15,0x5770fa,changed.data(),640,480,16,640,480,16,r.data(),640,512));++checked;
 }
 assert(checked==86&&accepted>0);
}
