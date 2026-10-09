#include "world_raster_batch.hpp"
#include "../../protocols/include/mnm/world_raster_batch_v3.h"
#include <algorithm>
#include <cstring>
#include <limits>
namespace mnm::legacy {
namespace {
unsigned word(const std::vector<std::uint8_t>& b,std::size_t at){
  if(at>b.size()||b.size()-at<4)throw std::invalid_argument("Truncated World batch word");
  return b[at]|unsigned(b[at+1])<<8|unsigned(b[at+2])<<16|unsigned(b[at+3])<<24;
}
void put(std::vector<std::uint8_t>& b,unsigned w){for(unsigned k=0;k<4;++k)b.push_back(std::uint8_t(w>>(k*8)));}
bool admitted(unsigned entry,unsigned backend){
  constexpr std::pair<unsigned,unsigned> entries[]={{0x595677,0},{0x5947b2,1},{0x59521a,1},{0x595b47,2},{0x59603e,2},{0x57de00,3},{0x57ec90,4},{0x57f0f0,5},{0x57f5f0,6},{0x5806f0,7},{0x596490,9},{0x5968a4,9},{0x57e540,10}};
  return std::any_of(std::begin(entries),std::end(entries),[&](auto p){return p.first==entry&&p.second==backend;});
}
}
std::uint32_t worldBatchHash(const std::vector<std::uint8_t>& b){std::uint32_t h=2166136261u;for(auto c:b)h=(h^c)*16777619u;return h;}
std::uint32_t worldRasterSourceHash(const CanvasProducer& c){
  std::vector<std::uint8_t> b;b.reserve(96+c.payload.size());for(auto w:c.fields)put(b,w);b.insert(b.end(),c.payload.begin(),c.payload.end());return worldBatchHash(b);
}
unsigned worldRasterAX(const CanvasProducer& c){
  const auto& r=c.fields;const auto& p=c.payload;
  if(r[2]!=9||p.size()<40||r[19]<40||r[18]!=p.size()||r[0]!=96+p.size()||r[23])throw std::invalid_argument("Unclosed batch raster");
  const auto width=word(p,4),height=word(p,8);
  const auto left=std::int64_t(std::int32_t(r[8]))-std::int32_t(word(p,12));
  return (r[15]==0||r[15]==9)&&width&&height&&(left<std::int32_t(r[10])||left+width>=std::int32_t(r[12]));
}
WorldRasterBatch validateWorldRasterBatch(const std::vector<std::uint8_t>& request,
  unsigned ordinal,unsigned queue,unsigned sequence,unsigned canvas,unsigned width,
  unsigned height,unsigned priorRasters,const std::vector<const CanvasProducer*>& rasters){
  if(request.size()<64||std::memcmp(request.data(),MNM_WORLD_BATCH_REQUEST,8))throw std::invalid_argument("World batch magic/extent");
  WorldRasterBatch b;for(unsigned i=0;i<16;++i)b.header[i]=word(request,i*4);const auto& h=b.header;
  if(h[2]!=3||h[3]!=64||h[4]!=ordinal||h[5]!=queue||h[6]!=sequence||h[7]!=canvas||h[8]!=width||h[9]!=height||h[10]!=width||width!=800||height!=600||h[13]!=1||!h[14]||h[14]>MNM_WORLD_BATCH_MAX||h[14]!=rasters.size()||h[15]!=priorRasters+h[14]||h[11]!=h[14]*16||request.size()!=64+h[11])
    throw std::invalid_argument("World batch identity/count/guard");
  if(worldBatchHash({request.begin()+64,request.end()})!=h[12])throw std::invalid_argument("World batch descriptor checksum");
  unsigned previous=0;
  for(unsigned i=0;i<h[14];++i){
    std::array<unsigned,4> d;for(unsigned k=0;k<4;++k)d[k]=word(request,64+i*16+k*4);
    const auto& c=*rasters[i];const auto& r=c.fields;
    if(d[0]<=previous||d[0]!=r[1]||d[0]>sequence||r[3]!=canvas||r[5]!=width||r[6]!=height||!admitted(d[1],r[15])||d[2]>1||d[2]!=r[21]||d[2]!=worldRasterAX(c)||d[3]!=worldRasterSourceHash(c))
      throw std::invalid_argument("World batch raster source/entry/AX/sequence");
    previous=d[0];b.descriptors.push_back(d);
  }
  return b;
}
std::vector<std::uint8_t> worldRasterBatchReply(const WorldRasterBatch& b,const std::vector<std::uint16_t>& pixels){
  if(pixels.size()!=std::size_t(b.header[8])*b.header[9])throw std::invalid_argument("World batch completion extent");
  std::vector<std::uint8_t> payload;payload.reserve(pixels.size()*2);for(auto p:pixels){payload.push_back(std::uint8_t(p));payload.push_back(std::uint8_t(p>>8));}
  auto h=b.header;std::memcpy(h.data(),MNM_WORLD_BATCH_REPLY,8);h[11]=unsigned(payload.size());h[12]=worldBatchHash(payload);h[13]=1;
  std::vector<std::uint8_t> out;out.reserve(64+payload.size());for(auto w:h)put(out,w);out.insert(out.end(),payload.begin(),payload.end());return out;
}
}
