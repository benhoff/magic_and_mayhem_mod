#pragma once
#include "world.hpp"
#include <memory>
namespace mnm::game {
enum class CreatureOrderKind { move, attack, coordinateAttack, follow, clear };
struct CreatureIntent {
  Handle actor;
  CreatureOrderKind kind = CreatureOrderKind::move;
  Point position;
  std::optional<Handle> target;
  bool queued = false, repeat = false;
};
struct CreatureRuleEvent {
  Handle actor;
  std::string kind;
  std::optional<Handle> target;
  std::int32_t value = 0;
};
// Rules own a versioned, portable State.systems continuation. The game service
// owns admission, entity generations and atomic publication, independently of
// Qt, reconstructed builds, asset paths and legacy runtime adapters.
class CreatureRuleRuntime {
public:
  virtual ~CreatureRuleRuntime() = default;
  virtual void initialize(State &, std::uint32_t seed) const = 0;
  virtual void validate(const State &) const = 0;
  virtual std::vector<CreatureRuleEvent> step(State &, TickInput) const = 0;
  virtual void command(State &, const CreatureIntent &) const = 0;
};
class CreatureRuleSession {
  World world_;
  std::shared_ptr<const CreatureRuleRuntime> rules_;

public:
  CreatureRuleSession(std::shared_ptr<const CreatureRuleRuntime>, Bytes profile,
                      std::uint32_t seed = 1, std::uint32_t capacity = 64);
  CreatureRuleSession(World, std::shared_ptr<const CreatureRuleRuntime>);
  const World &world() const { return world_; }
  std::vector<CreatureRuleEvent> step(TickInput = {});
  void command(const CreatureIntent &);
};
} // namespace mnm::game
