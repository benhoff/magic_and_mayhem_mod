#pragma once
#include "paired-encounter-original.hpp"
namespace {
void *regionManager=reinterpret_cast<void *>(0x6eaf08);
inline void publishRegions(const std::vector<model::NavigationRegion> &g){
  if(g.size()>512)throw std::runtime_error("original regional fixture capacity");
  auto *base=reinterpret_cast<unsigned char *>(get(regionManager));
  global(0x6eaf10,g.size());
  for(unsigned i=0;i<g.size();++i){auto *raw=base+660*i;std::memset(raw,0,660);const auto &v=g[i];put(raw,4,v.position.x);put(raw,8,v.position.y);put(raw,12,v.position.z);put(raw,524,v.parameter);put(raw,528,v.traffic);
    for(unsigned j=0;j<18;++j){auto *edge=raw+16+28*j;const auto &e=v.connections[j];put(edge,0,e.cost);put(edge,4,e.destination);std::memcpy(edge+8,e.clearance.data(),20);}
    std::memcpy(raw+532,v.danger.data(),56);std::memcpy(raw+588,v.deductedDanger.data(),56);put(raw,644,v.dangerBase,1);
  }
}
struct WireQuery { unsigned char bytes[84]{}; };
inline WireQuery wire(const Original &o,const model::RegionalQuery &q){WireQuery v;put(v.bytes,0,q.creature?0:1);put(v.bytes,4,address(o.actors.data()));std::memcpy(v.bytes+24,&q.movement,22);put(v.bytes,74,q.dangerPenalty);put(v.bytes,78,q.trafficPenalty);return v;}
}
