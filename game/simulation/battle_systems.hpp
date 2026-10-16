#pragma once
#include "world.hpp"
namespace mnm::game {
// Native battle continuation. Combat owns health; an optional app runtime owns
// only its effect payload/identities. Unextended v1-v3 combat bytes stay exact.
struct BattleEffectsState {
  std::uint64_t identity = 0;
  Bytes data;
  std::vector<Handle> entities;
};
struct BattleRulesState {
  std::uint64_t identity = 0;
  Bytes data;
};
struct BattleSystems {
  Bytes combat;
  BattleEffectsState effects;
  BattleRulesState rules = {};
};
bool isBattleSystems(const Bytes &);
BattleSystems decodeBattleSystems(const Bytes &);
Bytes encodeBattleSystems(const BattleSystems &);
} // namespace mnm::game
