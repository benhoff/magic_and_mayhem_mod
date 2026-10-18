#include "controller.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
NoCdAnimationController::NoCdAnimationController(std::vector<assets::AnimationRecord> records)
    : records_(std::move(records)), forward_(records_) {}
void NoCdAnimationController::scanReverse() {
  if(!forward_.state().active || !reverse_)return;
  reverse_=false;cursor_=0;
  AnimationState previous;
  for(unsigned budget=0;budget<65536;++budget) {
    previous=forward_.state();
    const auto &record=records_.at(previous.pc);
    const auto event=forward_.tick();++cursor_;
    if(!forward_.state().active || event==2 || (record.opcode==4 && record.argument<0)) {
      length_=cursor_;--cursor_;
      auto state=forward_.state();state.pc=previous.pc;
      state.displayedRecord=previous.displayedRecord;state.active=true;
      forward_.restore(state);reverse_=true;return;
    }
  }
  throw std::invalid_argument("bounded reverse animation scan");
}
void NoCdAnimationController::start(bool reverse) {
  reverse_=reverse;forward_.start();scanReverse();
}
void NoCdAnimationController::rewind() {forward_.start();scanReverse();}
void NoCdAnimationController::stop() {forward_.stop();reverse_=false;cursor_=length_=0;}
std::int32_t NoCdAnimationController::tick() {
  if(!forward_.state().active)return 0;
  if(!reverse_)return forward_.tick();
  cursor_=cursor_ ? cursor_-1 : length_;
  reverse_=false;forward_.start();
  std::int32_t event=0;
  for(unsigned i=0;i<cursor_;++i)event=forward_.tick();
  reverse_=true;return event;
}
void NoCdAnimationController::switchSequence(std::vector<assets::AnimationRecord> records) {
  forward_.switchSequence(records);records_=std::move(records);
}
AnimationControllerState NoCdAnimationController::state() const {
  return {forward_.state(),reverse_,cursor_,length_};
}
void NoCdAnimationController::restore(const AnimationControllerState &state) {
  if(state.reverseCursor>65536 || state.reverseLength>65536 ||
     state.reverseCursor>state.reverseLength)
    throw std::invalid_argument("reverse animation continuation span");
  forward_.restore(state.forward);reverse_=state.reverse;
  cursor_=state.reverseCursor;length_=state.reverseLength;
}
} // namespace mnm::reconstruction
