#include "terrain_static_lighting.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
struct Rect {int x0,y0,x1,y1;};
std::vector<Rect> split(Rect r,unsigned width,unsigned height){
 // The binary first shifts rectangles whose upper endpoints exceed a period,
 // then splits negative lower endpoints. Boundaries use > and include period.
 if(unsigned(r.x1)>width){r.x0-=int(width);r.x1-=int(width);}
 if(unsigned(r.y1)>height){r.y0-=int(height);r.y1-=int(height);}
 const int w=int(width),h=int(height);
 std::vector<Rect> result;
 if(r.y0<0){
  if(r.x0<0){
   result={{0,0,r.x1,r.y1},{r.x0+w,r.y0+h,w,h},
           {0,r.y0+h,r.x1,h},{r.x0+w,0,w,r.y1}};
  }else result={{r.x0,0,r.x1,r.y1},{r.x0,r.y0+h,r.x1,h}};
 }else if(r.x0<0)result={{0,r.y0,r.x1,r.y1},{r.x0+w,r.y0,w,r.y1}};
 else result={r};
 return result;
}
void validateView(unsigned width,unsigned height,TerrainLightView v){
 if(!width || !height || width>128 || height>128 || v.column>=width || v.row>=height || v.extent>std::min(width,height))
  throw std::invalid_argument("Static light view outside bounded contract");
}
}
bool admitsTerrainStaticLight(unsigned width,unsigned height,TerrainLightView v,unsigned x,unsigned y,unsigned index){
 validateView(width,height,v);
 if(x>=width || y>=height || index>33)throw std::invalid_argument("Static light admission outside bounded contract");
 if(!index)return false;
 const int half=int(index/2),vh=int(v.extent/2);
 const auto sources=split({int(x)-half,int(y)-half,int(x)-half+int(index),int(y)-half+int(index)},width,height);
 const auto views=split({int(v.column)-vh,int(v.row)-vh,int(v.column)-vh+int(v.extent),int(v.row)-vh+int(v.extent)},width,height);
 const auto in=[](int value,int lo,int hi){return value>=lo && value<=hi;};
 // The original view iterator is not reset for later source fragments; only
 // the first source rectangle reaches the endpoint comparisons.
 const auto& s=sources.front();
 for(const auto& t:views)
  if((in(s.x0,t.x0,t.x1)||in(s.x1,t.x0,t.x1)) &&
     (in(s.y0,t.y0,t.y1)||in(s.y1,t.y0,t.y1)))return true;
 return false;
}
void TerrainStaticLightCycle::step(TerrainLightField& field,const std::vector<TerrainLightObject>& objects,
 unsigned scanCount,TerrainLightView view,bool published,int changed,bool enabled){
 if(objects.size()>65536 || scanCount>objects.size())throw std::invalid_argument("Static light scan exceeds owned capacity");
 if(!enabled)return;
 validateView(field.width_,field.height_,view);
 for(unsigned i=0;i<scanCount;++i){const auto& o=objects[i];if(!o.active || !o.lightIndex)continue;
  const auto size=o.lightIndex/2+1;
  if(o.lightIndex<2 || o.lightIndex>33 || o.column>=field.width_ || o.row>=field.height_ || o.layer>=field.mapLayers_ || o.admissionColumn>=field.width_ || o.admissionRow>=field.height_ || field.width_<size || field.height_<size)
   throw std::invalid_argument("Static light object outside bounded contract");
 }
 const auto admitted=[&](const TerrainLightObject& o,bool recount){return o.active && o.lightIndex &&
  admitsTerrainStaticLight(field.width_,field.height_,view,recount?o.admissionColumn:o.column,recount?o.admissionRow:o.row,o.lightIndex);};
 const auto recount=[&]{state_.remaining=0;for(unsigned i=0;i<scanCount;++i)if(admitted(objects[i],true))++state_.remaining;};
 if(!state_.active)recount();
 if(!state_.active && !state_.remaining && !state_.requested && !changed)return;
 if(state_.phase==0 || changed){
  field.buffers_[1]=field.buffers_[published?0:2];state_.active=true;
  recount();state_.requested=state_.remaining!=0;state_.cursor=0;
 }
 if(state_.remaining){
  if(changed==1){state_.quota=state_.remaining;state_.cursor=0;state_.phase=1;}
  else state_.quota=(state_.remaining+1)/(2-state_.phase);
  unsigned processed=0;
  while(state_.cursor<scanCount && processed<state_.quota){const auto& o=objects[state_.cursor++];
   if(!admitted(o,false))continue;
   field.stampStatic({o.column,o.row,o.layer,o.lightIndex/2+1});++processed;--state_.remaining;
  }
 }
 state_.phase=(state_.phase+1)%2;
 if(!state_.phase){field.buffers_[0]=field.buffers_[1];state_.active=false;}
}
}
