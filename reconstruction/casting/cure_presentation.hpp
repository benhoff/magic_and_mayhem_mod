#pragma once
#include "../rendering/effect_animation_binding.hpp"
#include "../rendering/effect_placement.hpp"
#include "admission.hpp"

namespace mnm::reconstruction::casting {
struct CureContribution {
  std::optional<std::uint32_t> next, value;
};
struct CureCreator {
  std::uint32_t owner = 0;
  std::array<std::optional<std::uint32_t>, 2> contributions{};
};
struct CureAnimationDefinition {
  EffectAnimationEntry entry;
  std::vector<assets::AnimationRecord> sequence;
};
struct CureCell {
  std::uint16_t terrain = 1, head = NoEffect, otherA = NoEffect,
                otherB = NoEffect;
  std::uint32_t flags = 0;
};
struct CurePresentationRecord {
  EffectPlacementRecord placement;
  std::uint32_t reservation = 0, property = 0;
  bool animationReset = true;
  std::optional<EffectAnimationBinding> animation;
};
struct CurePresentationState {
  std::uint32_t width = 0, height = 0, layers = 0, scan = 0;
  std::vector<CureCell> cells;
  std::vector<CurePresentationRecord> records;
};
CurePresentationState makeCurePresentation(std::uint32_t width,
                                           std::uint32_t height,
                                           std::uint32_t layers,
                                           std::uint32_t capacity);
// Safe original domain; caller may stage larger compositions. No host pointers.
// Only direct type11, spell41. Logging/slot allocation/associated effect audio
// and other type setups are separate boundaries. Throws before publication.
void createCurePresentation(CurePresentationState &, std::uint32_t slot,
                            const CastEffectDescriptor &,
                            const std::vector<CureCreator> &,
                            const std::vector<CureContribution> &);
// Whole type77 selected setup resolution, with owned forward controller.
void bindCurePresentation(CurePresentationState &, std::uint32_t slot,
                          const CureAnimationDefinition &);
std::int32_t tickCurePresentation(CurePresentationState &, std::uint32_t slot);
// Whole ordinary11/77 release, with associated effect audio absent. The spatial
// recycler query is supplied only when488330 reaches its terrain0/empty gate.
void releaseCurePresentation(
    CurePresentationState &, std::uint32_t slot,
    const std::function<bool(std::uint32_t)> &cellReleaseQuery);
// Native movement adapter policy, not original4896b0 equivalence. Relinks the
// effect to the supplied valid owned grid position without resetting animation.
void moveCurePresentation(CurePresentationState &, std::uint32_t slot,
                          std::array<std::uint32_t, 3> position);
void validateCurePresentation(const CurePresentationState &);
} // namespace mnm::reconstruction::casting
