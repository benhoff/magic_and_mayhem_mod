#pragma once
#include "no_cd.hpp"
namespace mnm::reconstruction {
struct AnimationControllerState {
  AnimationState forward;
  bool reverse = false;
  std::uint32_t reverseCursor = 0, reverseLength = 0;
};
// Complete bounded reverse scan/rewind/replay layer of464cb0/464d70/464ec0.
// The retained forward interpreter owns dispatch. Calls are simulation steps,
// not milliseconds. Safety exceptions discard the caller's staged controller.
class NoCdAnimationController {
  std::vector<assets::AnimationRecord> records_;
  NoCdAnimationPlayer forward_;
  bool reverse_ = false;
  std::uint32_t cursor_ = 0, length_ = 0;
  void scanReverse();
public:
  explicit NoCdAnimationController(std::vector<assets::AnimationRecord>);
  void start(bool reverse = false);
  void rewind();
  void stop();
  std::int32_t tick();
  void requestBreak(){forward_.requestBreak();}
  void switchSequence(std::vector<assets::AnimationRecord>);
  AnimationControllerState state() const;
  void restore(const AnimationControllerState &);
  std::optional<std::uint32_t> sprite() const{return forward_.sprite();}
};
} // namespace mnm::reconstruction
