#pragma once
#include <cstdint>
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction {
// Whole 00501e00 local normal-return contract. Stream, allocation, linked-record
// codecs/list services, kernel generation and extent publication are supplied.
// Preserve partial original state on each failure; native RAII is separate.
template<class Memory> class TerrainLightMapLoad {
 using U=std::uint32_t;Memory& m;
 bool word(U p,U stream){return m.readGuest(p,4,1,stream)==1;}
public:
 explicit TerrainLightMapLoad(Memory& memory):m(memory){}
 U load(U p,U stream) {
  const U failure=~U(0);
  if(m.readGuest(p,0x4c,1,stream)!=1)return failure;
  const U layers=m.get32(p+12),height=m.get32(p+8),width=m.get32(p+4),area=m.get32(p+16),cells=m.get32(p+20);
  if(!width || !height || !layers || width>128 || height>128 || layers>32 || cells>65536)throw std::invalid_argument("Map loader dimensions outside owned storage");
  for(U i=0;i<layers;++i)m.set32(p+0x6432+i*4,i*area);
  for(U i=0;i<height;++i)m.set32(p+0x64b2+i*4,i*width);
  if(U old=m.get32(p+0x4c))m.release(old);
  U data=m.allocate(cells*12);m.set32(p+0x4c,data);if(!data)return failure;
  for(U i=0;i<cells;++i){m.set32(data+i*12,~U(0));m.set32(data+i*12+4,~U(0));m.set32(data+i*12+8,0);}
  U total=0x4c;
  for(U i=0;i<cells;++i){if(m.readGuest(data+i*12,12,1,stream)!=1)return failure;total+=12;}
  if(!word(p+0x50,stream))return failure;
  U size=m.loadSection(p+0x54,stream);if(size==failure)return failure;total+=size+4;
  size=m.loadSection(p+0x6c,stream);if(size==failure)return failure;
  for(auto range:{std::pair<U,U>{0x84,0x4b},{0x1b0,0x78},{0x390,0x78},{0x570,0x1c},{0x5e0,0x7b}})
   if(m.readGuest(p+range.first,4,range.second,stream)!=range.second)return failure;
  const U count=((layers+1)/2)*area;m.set32(p+0x7e0,count);
  U serializedCount=0;if(m.readValue(serializedCount,stream)!=1)return failure;
  total+=size+0x3a14;
  if(serializedCount!=count)return failure;
  for(U off:{0x7ccu,0x7d0u,0x7d4u,0x7d8u,0x7dcu}){
   if(U old=m.get32(p+off))m.release(old);
   const U handle=m.allocate(count);m.set32(p+off,handle);if(!handle)return failure;
  }
  if(m.readGuest(m.get32(p+0x7dc),1,count,stream)!=count)return failure;
  U records=0;total+=count;if(m.readValue(records,stream)!=1)return failure;
  m.clearLinked(p+0x7e4);
  if(std::int32_t(records)>0){
   m.initializeLoadedRecord();
   for(U i=0;i<records;++i){
    U handle=0;if(m.readValue(handle,stream)!=1)return failure;
    const U cell=handle==failure?0:m.get32(0x6c54dc)+handle*12;
    const U amount=m.loadLinkedRecord(cell,stream);if(amount==failure || amount+4==failure)return failure;
    total+=amount+4;
    const U sentinel=m.get32(p+0x7e8),previous=m.get32(sentinel+4);
    const U node=m.allocateLinkedNode(p+0x7e4,sentinel,previous);
    m.set32(sentinel+4,node);m.set32(m.get32(node+4),node);m.copyLoadedRecord(node+8);
    m.set32(p+0x7ec,m.get32(p+0x7ec)+1);
   }
  }
  for(U off:{0x7f0u,0x7f4u,0x7f8u,0x7fcu,0x800u})if(!word(p+off,stream))return failure;
  if(m.loadTail(p+0x66b2,stream)==failure)return failure;
  m.set32(0x5e41a8,1);m.rebuildKernels(p);
  U mapLayers=m.get32(0x6c549c);m.set32(0x5e1780,mapLayers);m.set32(0x5e1784,mapLayers<<4);
  m.set32(0x6e1fa0,0);m.set32(0x6e1fa4,0);
  m.publishExtents(m.get32(0x6c5494)<<5,m.get32(0x6c5498)<<5,mapLayers<<4);
  return total+20;
 }
};
}
