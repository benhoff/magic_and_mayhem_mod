#pragma once
#include <cstdint>

namespace mnm::reconstruction {
// Build-specific shared screen tick. Storage and callbacks are borrowed; this
// recovers local dispatch order, not the internals/lifetime of called services.
template<class State,class Services>
void tickOriginalScreen(State& s,Services& p){
 const auto next=s.request(0x33);
 if(next&&!s.fading()){
  if(std::int32_t(s.depth())>=15){p.warning(0x5ebff4);return;}
  if(s.current())p.slot(s.current(),8);
  s.depth(s.depth()+1);s.stack(s.depth(),next);s.snapshotToken();s.current(next);
  while(p.peek(0x100,0x108)){}
  while(p.peek(0x201,0x201)){}
  p.call(0x4a4370);p.slot(s.current(),0);return;
 }
 const auto selection=s.request(0x37);
 if(selection&&!s.fading()){p.select(selection);return;}
 if(s.request(0x43)&&!s.fading()){
  if(std::int32_t(s.depth())<=0){p.warning(0x5ec034);return;}
  p.slot(s.current(),4);s.depth(s.depth()-1);s.snapshotToken();s.current(s.stack(s.depth()));
  p.call(0x4a4370);if(s.current())p.slot(s.current(),12);return;
 }
 if(!s.active())return;
 if(!s.fading()&&s.screenClass()==22)p.special(16);
 if(s.fading()&&s.counter()){
  p.fade();const auto counter=s.counter();s.counter(counter+1);
  if(std::int32_t(counter)>=5)s.fading(false);
 }else{p.slot(s.self(),40);s.counter(s.counter()+1);}
 if(!s.suppressed()){
  s.gate(0);if(!s.fading())p.call(0x4a45b0);
  p.present();p.call(0x4a4380);s.gate(1);
 }
 p.slot(s.self(),44);p.call(0x4a5070);
}
} // namespace mnm::reconstruction
