#pragma once
#include "../../reconstruction/combat/autonomy.hpp"
#include <functional>
namespace mnm::reconstruction::combat {
struct SightDefinition {
  int range = 0, aimXY = 0, aimZ = 0;
  std::array<Cell, 8> offsets{};
};
struct VisibilityActor {
  int slot = 0, owner = 0, type = 0, health = 0;
  bool active = false;
  unsigned hidden = 0, engaged = 0, incoming = 0, readiness = 0;
  int target = -1;
  Cell position, finePosition;
  unsigned direction = 0;
  SightDefinition definition;
  std::array<unsigned, 8> visible{};
  std::array<int, 8> observer{{-1, -1, -1, -1, -1, -1, -1, -1}};
};
struct VisibilityState {
  unsigned capacity = 125, phase = 0;
};
using VisionRelations = std::array<std::array<std::uint8_t, 8>, 8>;
struct SightSample {
  bool occupied = false, blocked = false, attenuating = false;
};
using SightSampler =
    std::function<SightSample(Cell, unsigned, unsigned, unsigned)>;
// 4e14a0 with real trajectory initialization/stepping. Samples use 8x8x4
// terrain bits; attenuation applies only on entering a different map cell.
bool sightRay(Cell from, Cell to, int range, bool special, Extent,
              unsigned attenuation, const SightSampler &);
bool withinSightRange(const VisibilityActor &, Cell, Extent);
bool creatureSight(const VisibilityActor &, const VisibilityActor &, Extent,
                   unsigned attenuation, const SightSampler &);
bool pointSight(const VisibilityActor &, Cell, Extent, unsigned attenuation,
                const SightSampler &);
// 5084f0 ordinary facing projection, including fine-center subtraction and
// original integer-degree sectors. Coincident points retain their direction.
std::optional<unsigned> fineCoordinateFacing(Cell from, int aimXY, Cell target,
                                             Extent);
// 52b540 simulation projection: local player -1, no observation points.
// UI discovery/notifications and local commander's special reveal are excluded.
void updateOwnerVisibility(
    VisibilityState &, std::vector<VisibilityActor> &, const VisionRelations &,
    const VisionRelations &,
    const std::function<bool(const VisibilityActor &, const VisibilityActor &)>
        &);
} // namespace mnm::reconstruction::combat
