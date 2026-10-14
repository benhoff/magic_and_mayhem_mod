#include "../reconstruction/animation/controller.hpp"
#include <iostream>
#include <stdexcept>
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
#define main unused_forward_ani_main
#include "animation-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
namespace {
std::uint64_t digest=1469598103934665603ull;
std::array<unsigned,10> words(const AnimationControllerState &s){
  return {unsigned(s.forward.pc),unsigned(s.forward.displayedRecord.value_or(0xffffffffu)),unsigned(s.forward.active),s.forward.delay,s.forward.elapsed,s.forward.repeats,s.forward.breakFlag,unsigned(s.reverse),s.reverseCursor,s.reverseLength};
}
void mix(std::uint32_t n){for(unsigned i=0;i<4;++i)digest=(digest^((n>>(8*i))&255))*1099511628211ull;}
std::vector<mnm::assets::AnimationRecord> sequence(unsigned n) {
  std::vector<mnm::assets::AnimationRecord> r;
  auto add=[&](unsigned op,int arg){r.push_back({op,arg,{}});};
  if(n%2)add(1,int(n%4));
  if(n%3==0)add(5,-7);
  for(unsigned i=0;i<1+n%12;++i){add(0,int(i));if(i%3==1)add(5,5);}
  if(n%4==0)add(4,-int(r.size()));
  else if(n%4==1)add(5,2);
  add(6,0);return r;
}
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
std::vector<std::pair<unsigned,Bytes>> guards;
void compare(const std::uint32_t *raw,const AnimationControllerState &s,
             const mnm::assets::AnimationRecord *origin) {
  auto index=[&](unsigned p){return (std::uintptr_t(p)-std::uintptr_t(origin))/44;};
  if(bool(raw[2])!=s.forward.active || raw[3]!=s.forward.delay || raw[4]!=s.forward.elapsed ||
     raw[5]!=s.forward.repeats || index(raw[6])!=s.forward.pc ||
     (raw[1]?std::int64_t(index(raw[1])):-1)!=(s.forward.displayedRecord?std::int64_t(*s.forward.displayedRecord):-1) ||
     raw[8]!=s.forward.breakFlag || bool(raw[9])!=s.reverse || raw[10]!=s.reverseCursor || raw[11]!=s.reverseLength)
    throw std::runtime_error("original/full ANI controller state mismatch");
}
#endif
} // namespace
int main(int argc,char **argv)try {
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
  if(argc!=2)throw std::runtime_error("expected pinned PE");
  mapImage(read(argv[1]));
  for(auto span:{std::pair<unsigned,unsigned>{0x464cb0,0x464d64},{0x464d70,0x464e17},{0x464e20,0x464ebd},{0x464ec0,0x464fda},{0x465000,0x4650a8}})
    guards.push_back({span.first,Bytes(reinterpret_cast<unsigned char*>(span.first),reinterpret_cast<unsigned char*>(span.second))});
  using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
  using Step=int(__attribute__((thiscall)) *)(void*);
  using Switch=void(__attribute__((thiscall)) *)(void*,unsigned);
#else
  (void)argc;(void)argv;
#endif
  unsigned states=0,reverseStates=0;
  for(unsigned n=0;n<256;++n)for(unsigned mode=0;mode<2;++mode) {
    auto first=sequence(n),second=first;for(auto &r:second)if(!r.opcode)r.argument+=100;
    NoCdAnimationController native(first);native.start(mode!=0);
    unsigned selected=0;
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
    auto records=first;records.insert(records.end(),second.begin(),second.end());
    std::uint32_t offsets[]={0,unsigned(first.size()),unsigned(records.size())};
    std::uint32_t asset[]={0,reinterpret_cast<std::uintptr_t>(offsets),reinterpret_cast<std::uintptr_t>(records.data())};
    std::uint32_t raw[12]={};raw[7]=reinterpret_cast<std::uintptr_t>(asset);
    reinterpret_cast<Start>(0x464cb0)(raw,0,mode);
#endif
    for(unsigned tick=0;tick<96;++tick) {
      const auto s=native.state();
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
      compare(raw,s,records.data()+selected*first.size());
#endif
      for(auto v:words(s))mix(v);
      ++states;reverseStates+=s.reverse;
      // Every step restores owned state before proceeding; no hidden player
      // object or host pointer can be necessary to resume its timing.
      NoCdAnimationController resumed(selected?second:first);resumed.restore(s);
      if(tick==31){native.rewind();resumed.rewind();
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
        reinterpret_cast<Step>(0x464d70)(raw);
#endif
      }
      if(tick==13 && s.forward.active && s.forward.displayedRecord) {
        selected=1;native.switchSequence(second);resumed.switchSequence(second);
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
        reinterpret_cast<Switch>(0x464e20)(raw,1);
#endif
      }
      if(tick==47){native.requestBreak();resumed.requestBreak();
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
        raw[8]=1;
#endif
      }
      const auto event=native.tick();
      if(event!=resumed.tick() || words(native.state())!=words(resumed.state()))throw std::runtime_error("restored ANI event differs");
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
      if(event!=reinterpret_cast<Step>(0x464ec0)(raw))throw std::runtime_error("original reverse ANI event differs");
#endif
      mix(unsigned(event));
    }
  }
#ifdef MNM_ANIMATION_CONTROLLER_REFERENCE
  for(const auto &g:guards)if(std::memcmp(reinterpret_cast<void*>(g.first),g.second.data(),g.second.size()))throw std::runtime_error("original ANI body changed");
#endif
  std::cout<<"{\"success\":true,\"cases\":512,\"functions\":5,\"states\":"<<states<<",\"reverse_states\":"<<reverseStates<<",\"outcome_digest\":"<<digest<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
