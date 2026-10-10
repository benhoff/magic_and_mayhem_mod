#include "world_rolling_packet.hpp"
#include <cstring>
#include <map>
#include <tuple>
namespace mnm::legacy {
namespace {
void put(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){for(unsigned k=0;k<4;++k)b.at(at+k)=std::uint8_t(n>>(k*8));}
}
CanvasProducerStream decodeRollingQueue(const std::vector<std::uint8_t>& bytes){
  if(bytes.size()>MNM_ROLL_INPUT)throw std::invalid_argument("Rolling queue exceeds source budget");
  auto stream=decodeCanvasProducers(bytes,false);
  if(stream.version!=3||stream.operations.empty()||stream.operations.back().fields[2]!=12)throw std::invalid_argument("Rolling packet must close one owned World queue");
  unsigned entries=0,returns=0,draws=0,checks=0,canvas=0,width=0,height=0;
  for(const auto& c:stream.operations){const auto& r=c.fields;
    if(r[2]==11){if(++entries!=1||r[14]!=1||!r[3]||!r[5]||!r[6]||r[5]>2048||r[6]>2048)throw std::invalid_argument("Rolling queue entry invalid");canvas=r[3];width=r[5];height=r[6];}
    else if(r[2]==12){if(entries!=1||++returns!=1||r[14]!=1||r[3]!=canvas||!checks||!draws)throw std::invalid_argument("Rolling queue return invalid");}
    else if(entries&&!returns){
      if(r[2]!=4&&r[2]!=9&&r[2]!=10&&r[2]!=25)throw std::invalid_argument("Unsupported producer inside rolling World");
      if(r[2]==9){if(checks||++draws>12320||r[3]!=canvas||r[5]!=width||r[6]!=height)throw std::invalid_argument("Rolling raster extent/order invalid");}
      if(r[2]==10&&r[3]==canvas){if(++checks!=1||r[5]!=width||r[6]!=height)throw std::invalid_argument("Rolling World checkpoint invalid");}
    }
  }
  if(entries!=1||returns!=1||checks!=1)throw std::invalid_argument("Incomplete rolling World packet");
  stream.queues=1;return stream;
}
std::vector<std::uint8_t> encodeRollingQueue(const std::vector<CanvasProducer>& operations){
  std::vector<std::uint8_t> bytes(64);std::memcpy(bytes.data(),"MNMPRO03",8);
  put(bytes,8,3);put(bytes,12,64);put(bytes,16,0x40209ca7);put(bytes,20,131072);put(bytes,24,32);
  unsigned sequence=0;
  using Source=std::tuple<unsigned,unsigned,unsigned,unsigned,std::vector<std::uint8_t>>;
  std::map<Source,unsigned> sources;std::size_t sourceBytes=0;
  auto append=[&](std::array<std::uint32_t,24> r,const std::vector<std::uint8_t>& payload){
    if(payload.size()>MNM_ROLL_INPUT-96||bytes.size()>MNM_ROLL_INPUT-96-payload.size())throw std::invalid_argument("Rolling queue source budget exceeded");
    r[1]=++sequence;r[18]=payload.size();r[0]=96+r[18];
    const auto at=bytes.size();bytes.resize(at+96);for(unsigned k=0;k<24;++k)put(bytes,at+k*4,r[k]);
    bytes.insert(bytes.end(),payload.begin(),payload.end());return sequence;
  };
  for(const auto& c:operations){
    if(c.fields[2]==25)continue;
    const auto& payload=c.bytes();
    auto r=c.fields;unsigned source=0;
    if(r[2]==8||r[2]==9){
      Source key{r[2],r[17],r[19],r[20],payload};auto found=sources.find(key);
      if(found!=sources.end())source=found->second;
      else if(sources.size()<2048&&payload.size()<=16*1024*1024-sourceBytes){
        std::array<std::uint32_t,24> definition{};definition[2]=25;definition[14]=r[2];definition[17]=r[17];definition[19]=r[19];definition[20]=r[20];
        source=append(definition,payload);sources.emplace(std::move(key),source);sourceBytes+=payload.size();
      }
      r[4]=source;
    }
    if(r[2]==11||r[2]==12)r[14]=1;
    append(r,source?std::vector<std::uint8_t>{}:payload);
  }
  decodeRollingQueue(bytes);return bytes;
}
}
