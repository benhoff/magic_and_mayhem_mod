#pragma once
#include <stdint.h>
namespace mnm::reconstruction {
// Borrowed NoCD cursor-background state. No allocation, publication or lifetime
// policy is inferred. Queue providers must copy the temporary rectangle.
struct CursorBackgroundState {
 uint32_t enabled,screenClass,destinationInterface,rectangle[4],valid,mutex,offsetX,offsetY,deferred;
};
inline void invalidateOriginalCursorBackground(CursorBackgroundState& s){s.valid=0;}
template<class Services>
void restoreOriginalCursorBackground(CursorBackgroundState& s,Services& p){
 if(!s.enabled || s.screenClass==22 || !s.destinationInterface)return;
 for(auto coordinate:s.rectangle)if(coordinate&0x80000000u)return;
 p.wait(s.mutex,0xffffffffu); // The original ignores every wait result.
 if(s.valid){
  const uint32_t rectangle[4]={0,0,s.rectangle[2]-s.rectangle[0],s.rectangle[3]-s.rectangle[1]};
  if(rectangle[2] && rectangle[3]){
   const auto y=s.offsetY&0x80000000u?0:s.offsetY;
   const auto x=s.offsetX&0x80000000u?0:s.offsetX;
   if(s.deferred)p.queue(0x641fc8u,2u,0x6a49e0u,rectangle,x,y,1u);
   else p.copy(0x641fc8u,0x6a49e0u,rectangle,x,y);
  }
 }
 p.release(s.mutex); // Reread after callbacks; do not clear the valid flag.
}
}
