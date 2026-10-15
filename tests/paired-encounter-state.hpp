#pragma once
#include "../apps/native-encounter/encounter.hpp"
#include <map>
namespace mnm::encounter::paired {
struct Field {
  unsigned value, size;
  const char *name;
};
// Build-specific field projections previously used by independent component
// fixtures. Used only to initialize the original once and compare outputs.
inline std::map<unsigned, Field> fields(const cs::Continuation &c, unsigned i) {
  const auto &a = c.world.creatures.at(i);
  const auto &e = c.execution.at(i);
  const auto &m = c.maintenance.at(i);
  const auto &v = c.movement.at(i);
  const auto &motion = c.actions.motion.at(i);
  const auto &route = c.actions.segments.at(i);
  const auto &init = c.native.initialization.at(i);
  std::map<unsigned, Field> out;
  out[0x0] = {unsigned(a.ai.slot), 4, "a.ai.slot"};
  out[0x4] = {unsigned(unsigned(a.ai.active)), 4, "unsigned(a.ai.active)"};
  out[0x8] = {unsigned(a.ai.position.x), 4, "a.ai.position.x"};
  out[0xc] = {unsigned(a.ai.position.y), 4, "a.ai.position.y"};
  out[0x10] = {unsigned(a.ai.position.z), 4, "a.ai.position.z"};
  out[0x14] = {unsigned(a.finePosition.x), 4, "a.finePosition.x"};
  out[0x18] = {unsigned(a.finePosition.y), 4, "a.finePosition.y"};
  out[0x1c] = {unsigned(a.finePosition.z), 4, "a.finePosition.z"};
  out[0x68] = {unsigned(e.marker68), 4, "e.marker68"};
  out[0x7c] = {unsigned(a.block7c), 4, "a.block7c"};
  out[0x84] = {unsigned(e.power84), 4, "e.power84"};
  out[0x88] = {unsigned(a.ai.bravery), 4, "a.ai.bravery"};
  out[0x8c] = {unsigned(a.specialVision), 4, "a.specialVision"};
  out[0x90] = {unsigned(e.mitigation90), 4, "e.mitigation90"};
  out[0x94] = {unsigned(m.timer94), 4, "m.timer94"};
  out[0x98] = {unsigned(m.random98), 4, "m.random98"};
  out[0x9c] = {unsigned(e.absorptionTimer), 4, "e.absorptionTimer"};
  out[0xa0] = {unsigned(a.mode), 4, "a.mode"};
  out[0xa4] = {unsigned(v.landingKind), 4, "v.landingKind"};
  out[0xa8] = {unsigned(a.ai.type), 4, "a.ai.type"};
  out[0xe0] = {unsigned(e.animationOffset), 4, "e.animationOffset"};
  out[0xe4] = {unsigned(a.ai.health), 4, "a.ai.health"};
  out[0xf0] = {unsigned(a.ai.corpseA), 4, "a.ai.corpseA"};
  out[0xf4] = {unsigned(a.ai.corpseB), 4, "a.ai.corpseB"};
  out[0xf8] = {unsigned(a.ai.physical), 4, "a.ai.physical"};
  out[0xfc] = {unsigned(a.ai.gateFC), 4, "a.ai.gateFC"};
  out[0x100] = {unsigned(m.upkeepDeadline), 4, "m.upkeepDeadline"};
  out[0x104] = {unsigned(a.flag104), 4, "a.flag104"};
  out[0x108] = {unsigned(a.ai.flying), 4, "a.ai.flying"};
  out[0x10c] = {unsigned(e.mode10c), 4, "e.mode10c"};
  out[0x110] = {unsigned(m.flight110), 4, "m.flight110"};
  out[0x114] = {unsigned(m.status114), 4, "m.status114"};
  out[0x118] = {unsigned(m.status118), 4, "m.status118"};
  out[0x124] = {unsigned(e.bonusMode124), 4, "e.bonusMode124"};
  out[0x164] = {unsigned(a.ai.readiness), 4, "a.ai.readiness"};
  out[0x16c] = {unsigned(e.previousMove), 4, "e.previousMove"};
  out[0x170] = {unsigned(e.previousMoveVariant), 4, "e.previousMoveVariant"};
  out[0x174] = {unsigned(a.ai.owner), 4, "a.ai.owner"};
  out[0x178] = {unsigned(a.inventoryCount), 4, "a.inventoryCount"};
  out[0x570] = {unsigned(a.selectedSpell), 4, "a.selectedSpell"};
  out[0x5ec] = {unsigned(a.ai.behavior), 4, "a.ai.behavior"};
  out[0x5f0] = {unsigned(a.ai.pending), 4, "a.ai.pending"};
  out[0x5fc] = {unsigned(a.ai.action), 4, "a.ai.action"};
  out[0x608] = {unsigned(motion.direction), 4, "motion.direction"};
  out[0x60c] = {unsigned(e.animationFacing), 4, "e.animationFacing"};
  out[0x610] = {unsigned(init.vertical), 4, "segment.vertical"};
  out[0x614] = {init.previousDirection, 4, "segment.previousDirection"};
  out[0x618] = {unsigned(unsigned(a.ai.motion)), 4, "unsigned(a.ai.motion)"};
  out[0x61c] = {unsigned(a.castSpell), 4, "a.castSpell"};
  out[0x638] = {unsigned(a.castSource), 4, "a.castSource"};
  out[0x63c] = {unsigned(a.castTarget), 4, "a.castTarget"};
  out[0x64c] = {unsigned(a.ai.target), 4, "a.ai.target"};
  out[0x650] = {unsigned(a.chosenDestination.x), 4, "a.chosenDestination.x"};
  out[0x654] = {unsigned(a.chosenDestination.y), 4, "a.chosenDestination.y"};
  out[0x658] = {unsigned(a.chosenDestination.z), 4, "a.chosenDestination.z"};
  out[0x660] = {unsigned(m.timer660), 4, "m.timer660"};
  out[0x664] = {unsigned(a.ai.engaged), 4, "a.ai.engaged"};
  out[0x668] = {unsigned(a.ai.incoming), 4, "a.ai.incoming"};
  out[0x66c] = {unsigned(a.cooldown), 4, "a.cooldown"};
  out[0x680] = {unsigned(a.ai.history), 4, "a.ai.history"};
  out[0x684] = {unsigned(a.substate), 4, "a.substate"};
  out[0x688] = {unsigned(a.routePhase), 4, "a.routePhase"};
  out[0x68c] = {unsigned(a.range), 4, "a.range"};
  out[0x690] = {unsigned(a.ai.corpseClaim), 4, "a.ai.corpseClaim"};
  out[0x694] = {unsigned(a.routeMode), 4, "a.routeMode"};
  out[0x698] = {unsigned(e.commandMode), 4, "e.commandMode"};
  out[0x6a0] = {unsigned(e.marker6a0), 4, "e.marker6a0"};
  out[0x6a4] = {unsigned(a.targetMovingAttacker), 4, "a.targetMovingAttacker"};
  out[0x6a8] = {unsigned(a.alternateTarget), 4, "a.alternateTarget"};
  out[0x6ac] = {unsigned(e.routeRenewal), 4, "e.routeRenewal"};
  out[0x6b0] = {unsigned(m.recent6b0), 4, "m.recent6b0"};
  out[0x704] = {unsigned(a.ai.flag704), 4, "a.ai.flag704"};
  out[0x71c] = {unsigned(std::uint8_t(a.block71c)), 1,
                "std::uint8_t(a.block71c)"};
  out[0x71d] = {unsigned(m.block71d), 1, "m.block71d"};
  out[0x71e] = {unsigned(init.inlineSample), 4, "segment.inlineSample"};
  out[0x722] = {unsigned(e.block722), 1, "e.block722"};
  out[0x723] = {unsigned(std::uint8_t(a.block723)), 1,
                "std::uint8_t(a.block723)"};
  out[0x744] = {unsigned(m.status744), 4, "m.status744"};
  out[0x748] = {unsigned(m.status748), 4, "m.status748"};
  out[0x74c] = {unsigned(m.status74c), 4, "m.status74c"};
  out[0x750] = {unsigned(m.marker750), 4, "m.marker750"};
  out[0x73c] = {init.specialRate73c, 4, "segment.specialRate"};
  out[0x778] = {unsigned(m.status778), 4, "m.status778"};
  out[0x77c] = {unsigned(m.status77c), 4, "m.status77c"};
  out[0x780] = {unsigned(m.status780), 4, "m.status780"};
  out[0x784] = {unsigned(a.block784), 4, "a.block784"};
  out[0x78c] = {unsigned(m.status78c), 4, "m.status78c"};
  out[0x790] = {unsigned(m.status790), 4, "m.status790"};
  out[0x798] = {unsigned(a.form), 4, "a.form"};
  out[0x7cc] = {unsigned(m.animation7cc), 1, "m.animation7cc"};
  out[0x7cd] = {unsigned(a.animationKind), 4, "a.animationKind"};
  out[0x809] = {unsigned(m.flight809), 4, "m.flight809"};
  out[0x80d] = {unsigned(a.ai.recent), 4, "a.ai.recent"};
  out[0x811] = {unsigned(a.ai.age), 4, "a.ai.age"};
  out[0x815] = {unsigned(a.ai.flag815), 4, "a.ai.flag815"};
  out[0x819] = {unsigned(m.status819), 4, "m.status819"};
  out[0x81d] = {unsigned(m.status81d), 4, "m.status81d"};
  out[0x821] = {unsigned(m.status821), 4, "m.status821"};
  out[0x855] = {unsigned(e.block855), 4, "e.block855"};
  out[0x889] = {unsigned(m.animation889), 4, "m.animation889"};
  out[0x8bd] = {unsigned(m.animation8bd), 4, "m.animation8bd"};
  out[0x8c5] = {unsigned(m.status8c5), 4, "m.status8c5"};
  out[0x8c9] = {unsigned(a.protection8c9), 4, "a.protection8c9"};
  out[0x8cd] = {unsigned(unsigned(m.status8cd)), 4, "unsigned(m.status8cd)"};
  out[0x8d5] = {unsigned(m.status8d5), 4, "m.status8d5"};
  out[0x8d9] = {unsigned(m.status8d9), 4, "m.status8d9"};
  out[0x8dd] = {unsigned(m.status8dd), 4, "m.status8dd"};
  out[0x8e1] = {unsigned(m.powerDeadline), 4, "m.powerDeadline"};
  out[0x8ed] = {unsigned(e.valueScale8ed), 4, "e.valueScale8ed"};
  out[0x902] = {unsigned(e.queuedCount), 4, "e.queuedCount"};
  out[0x906] = {unsigned(unsigned(e.follow)), 4, "unsigned(e.follow)"};
  out[0x92e] = {unsigned(e.dirtyValue), 4, "e.dirtyValue"};
  out[0x932] = {unsigned(a.ai.dirty932), 4, "a.ai.dirty932"};
  out[0x962] = {unsigned(e.block962), 1, "e.block962"};
  out[0x963] = {unsigned(e.block963), 4, "e.block963"};
  out[0x96b] = {unsigned(a.destination.x), 4, "a.destination.x"};
  out[0x96f] = {unsigned(a.destination.y), 4, "a.destination.y"};
  out[0x973] = {unsigned(a.destination.z), 4, "a.destination.z"};
  out[0x977] = {unsigned(route.cursor), 4, "route.cursor"};
  out[0x97b] = {unsigned(route.count), 4, "route.count"};
  out[0x98f] = {unsigned(unsigned(motion.vertical)), 4,
                "unsigned(motion.vertical)"};
  out[0xb3f] = {unsigned(motion.progress), 4, "motion.progress"};
  out[0xb43] = {unsigned(motion.step), 4, "motion.step"};
  out[0xb47] = {unsigned(motion.travelX), 4, "motion.travelX"};
  out[0xb4b] = {unsigned(motion.travelY), 4, "motion.travelY"};
  out[0xb4f] = {unsigned(motion.deltaX), 4, "motion.deltaX"};
  out[0xb53] = {unsigned(motion.deltaY), 4, "motion.deltaY"};
  out[0xb57] = {unsigned(motion.heightDelta), 4, "motion.heightDelta"};
  out[0xb5b] = {unsigned(motion.heightStep), 4, "motion.heightStep"};
  out[0xb5f] = {unsigned(motion.heightOrigin), 4, "motion.heightOrigin"};
  out[0xb63] = {unsigned(e.moveVariant), 4, "e.moveVariant"};
  out[0xb67] = {unsigned(motion.duration), 4, "motion.duration"};
  out[0xb6b] = {unsigned(motion.accumulator), 4, "motion.accumulator"};
  out[0xb6f] = {unsigned(e.routeChanged), 4, "e.routeChanged"};
  out[0xb73] = {unsigned(e.markerB73), 4, "e.markerB73"};
  for (unsigned j = 0; j < 3; ++j)
    out[0xb77 + 4 * j] = {e.actionRouteCounters[j], 4, "motion.counter"};
  out[0xb83] = {unsigned(motion.initialResidualX), 4,
                "motion.initialResidualX"};
  out[0xb87] = {unsigned(motion.initialResidualY), 4,
                "motion.initialResidualY"};
  out[0xb8b] = {unsigned(a.ai.marker), 4, "a.ai.marker"};
  out[0xbc3] = {unsigned(a.ai.home.x), 4, "a.ai.home.x"};
  out[0xbc7] = {unsigned(a.ai.home.y), 4, "a.ai.home.y"};
  out[0xbcb] = {unsigned(a.ai.home.z), 4, "a.ai.home.z"};
  out[0xcff] = {unsigned(unsigned(m.lifespan)), 4, "unsigned(m.lifespan)"};
  out[0xddf] = {unsigned(unsigned(m.suspension)), 4, "unsigned(m.suspension)"};
  out[0xde3] = {unsigned(a.blockDE3), 4, "a.blockDE3"};
  out[0xde7] = {unsigned(a.region), 4, "a.region"};
  for (unsigned j = 0; j < 8; ++j) {
    out[0x24 + 4 * j] = {unsigned(a.ai.visible[j]), 4, "visibility"};
    out[0xe0b + 4 * j] = {m.visibilityAge[j], 4, "visibilityAge"};
  }
  return out;
}
} // namespace mnm::encounter::paired
