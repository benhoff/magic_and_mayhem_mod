#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include <ostream>

namespace wave_fixture {
inline void word(std::vector<std::uint8_t>& b,unsigned at,unsigned v){for(unsigned k=0;k<4;++k)b.at(at+k)=std::uint8_t(v>>(k*8));}
inline std::vector<std::uint8_t> frame(unsigned split=0){
  const unsigned deltaBytes=split?4:2,pixelBase=112+9*deltaBytes;
  std::vector<std::uint8_t> b(pixelBase+9*7);
  word(b,0,b.size());word(b,4,7);word(b,8,9);
  for(unsigned y=0;y<9;++y){word(b,40+y*8,112+y*deltaBytes);word(b,44+y*8,pixelBase+y*7);b[112+y*deltaBytes+1]=split?2:7;if(split)b[112+y*deltaBytes+3]=5;}
  return b;
}
inline std::vector<std::uint8_t> sprite(unsigned split=0){
  const auto f=frame(split);std::vector<std::uint8_t> b(796+f.size());
  word(b,0,0x00525053);word(b,4,b.size());word(b,8,4);word(b,12,1);word(b,16,1);
  std::copy(f.begin(),f.end(),b.begin()+796);return b;
}
template<class F> void cases(F f){
  for(unsigned a=1;a<=8;++a)for(unsigned implicit:{0u,1u})for(unsigned phase:{0u,3u,15u,31u})
    for(int x:{0,10,37})for(int y:{-2,4})for(unsigned split:{0u,1u})f(a,implicit,phase,x,y,split);
}
template<class V> std::uint32_t hash(const V& pixels){
  std::uint32_t h=2166136261u;
  for(auto p:pixels)for(unsigned k=0;k<2;++k){h^=(p>>(k*8))&255;h*=16777619u;}
  return h;
}
template<class V> void pattern(V& pixels){for(unsigned i=0;i<pixels.size();++i)pixels[i]=std::uint16_t(i*17+31);}
template<class V> void write(std::ostream& out,const V& pixels){for(auto p:pixels){out.put(char(p&255));out.put(char((p>>8)&255));}}
}
