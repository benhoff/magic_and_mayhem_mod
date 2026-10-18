#include "native_owner_visibility.hpp"
#include <stdexcept>
namespace mnm::command_session {
namespace {
model::VisibilityActor projection(const Continuation &c, unsigned i) {
  const auto &a = c.world.creatures.at(i);
  model::VisibilityActor out;
  out.slot = int(a.ai.slot);
  out.owner = a.ai.owner;
  out.type = int(a.ai.type);
  out.active = a.ai.active;
  out.health = int(a.ai.health);
  out.position = a.ai.position;
  out.finePosition = a.finePosition;
  out.hidden = a.specialVision;
  out.engaged = a.ai.engaged;
  out.incoming = a.ai.incoming;
  out.readiness = a.ai.readiness;
  out.target = a.ai.target;
  out.direction = c.actions.motion.at(i).direction;
  out.definition = c.native.ownerVisibility->definitions.at(i);
  out.observer = c.native.ownerVisibility->observers.at(i);
  out.visible = a.ai.visible;
  return out;
}
model::SightSampler sampler(const Continuation &c,
                            const NativeCreatureProfile &profile) {
  return [&c, &profile](model::Cell p, unsigned x, unsigned y, unsigned z) {
    const auto d = c.world.dimensions;
    if (p.x < 0 || p.x >= d.x || p.y < 0 || p.y >= d.y || p.z < 0 || p.z >= d.z)
      throw std::invalid_argument("native sight sample outside admitted map");
    const auto &cell =
        c.native.cells.at((std::size_t(p.z) * d.y + p.y) * d.x + p.x);
    const auto &terrain = profile.terrain.at(cell.kind);
    const auto *bits = reinterpret_cast<const unsigned char *>(&terrain);
    return model::SightSample{cell.kind != 0 || cell.occupant != 65535,
                              (bits[z * 8 + y] & (128u >> x)) != 0, false};
  };
}
} // namespace
void prepareNativeOwnerVisibility(
    Continuation &c, std::vector<model::SightDefinition> definitions,
    model::VisionRelations sharing) {
  if (!c.native.ownerKnowledge || !c.native.regionalDanger ||
      c.native.ownerVisibility || c.native.tick ||
      !c.native.ownerKnowledge->reset ||
      !c.native.regionalDanger->state.resetDanger ||
      !c.native.regionalDanger->state.resetDeduction ||
      definitions.size() != c.world.creatures.size())
    throw std::invalid_argument(
        "native visibility requires fresh knowledge and complete definitions");
  auto staged = c;
  staged.native.ownerVisibility.emplace();
  auto &v = *staged.native.ownerVisibility;
  v.definitions = std::move(definitions);
  v.sharing = sharing;
  v.observers.resize(staged.world.creatures.size());
  for (unsigned i = 0; i < v.observers.size(); ++i) {
    v.observers[i].fill(-1);
    auto &a = staged.world.creatures[i];
    for (unsigned o = 0; o < 8; ++o)
      a.ai.visible[o] = (a.ai.owner == int(o) ||
                         (staged.world.relation.at(unsigned(a.ai.owner))[o] &&
                          sharing.at(unsigned(a.ai.owner))[o]))
                            ? 30
                            : 0;
  }
  validate(staged);
  c = std::move(staged);
}
bool nativePointSight(const Continuation &c,
                      const NativeCreatureProfile &profile, unsigned observer,
                      model::Cell target) {
  return model::pointSight(projection(c, observer), target, c.world.dimensions,
                           0, sampler(c, profile));
}
void updateNativeOwnerVisibility(Continuation &c,
                                 const NativeCreatureProfile &profile) {
  if (!c.native.ownerVisibility)
    return;
  std::vector<model::VisibilityActor> actors;
  for (unsigned i = 0; i < c.world.creatures.size(); ++i)
    actors.push_back(projection(c, i));
  auto &v = *c.native.ownerVisibility;
  model::updateOwnerVisibility(v.scan, actors, c.world.relation, v.sharing,
                               [&](const auto &a, const auto &b) {
                                 return model::creatureSight(
                                     a, b, c.world.dimensions, 0,
                                     sampler(c, profile));
                               });
  for (unsigned i = 0; i < actors.size(); ++i) {
    c.world.creatures[i].ai.visible = actors[i].visible;
    v.observers[i] = actors[i].observer;
  }
}
} // namespace mnm::command_session
