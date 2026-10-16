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
struct BattleSystems {
  Bytes combat;
  BattleEffectsState effects;
};
bool isBattleSystems(const Bytes &);
BattleSystems decodeBattleSystems(const Bytes &);
Bytes encodeBattleSystems(const BattleSystems &);
} // namespace mnm::game
