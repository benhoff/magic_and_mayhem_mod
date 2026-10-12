#pragma once
#include "health_rules.hpp"
#include "world.hpp"
#include <memory>
namespace mnm::game {
enum class CreaturePhase : std::uint32_t {
  idle,
  pursuing,
  attacking,
  ready,
  stunned,
  dead,
  blocked,
  knockback
};
struct CreatureActor {
  Handle handle;
  Point home;
  std::uint32_t health = 0, facing = 0, cooldown = 0, recent = 0, age = 0;
  std::int32_t target = -1, stunTimer = 0;
  CreaturePhase phase = CreaturePhase::idle;
  Bytes cursor;
};
struct CreatureEncounter {
  std::uint64_t rules = 0, healthRules = 0;
  Bytes random;
  std::uint32_t quietTicks = 0;
  Point dimensions{16, 16, 3};
  std::vector<std::uint8_t> walls;
  std::vector<CreatureActor> actors;
};
struct DamageRoll {
  std::uint32_t amount = 0;
  bool stun = false;
};
struct StunRoll {
  std::int32_t duration = 0;
  std::optional<Point> knockback;
};
// Application adapters supply recovered rules. The service owns all mutable
// world, RNG, animation, target, movement and interruption state.
class CreatureRules {
public:
  virtual ~CreatureRules() = default;
  virtual std::uint64_t identity() const = 0;
  virtual std::uint32_t maximumHealth() const = 0;
  virtual int width() const = 0;
  virtual Bytes random(std::uint32_t) const = 0;
  virtual void validate(const CreatureActor &, const Bytes &) const = 0;
  virtual std::int32_t select(unsigned, const State &,
                              const CreatureEncounter &) const = 0;
  virtual bool reach(Point, Point, Point,
                     const std::function<bool(Point, Point)> &) const = 0;
  virtual unsigned facing(Point, Point, Point) const = 0;
  virtual Bytes start(bool stun, unsigned facing) const = 0;
  virtual std::int32_t animation(Bytes &, bool stun, unsigned facing) const = 0;
  virtual DamageRoll damage(Bytes &) const = 0;
  virtual StunRoll stun(Bytes &, const Entity &, const CreatureActor &,
                        const Entity &, Point,
                        const std::function<bool(Point)> &) const = 0;
  virtual std::int32_t stunTick(std::int32_t timer,
                                std::int32_t event) const = 0;
  virtual std::uint32_t cooldown(Bytes &) const = 0;
};
Bytes encodeCreatureEncounter(const CreatureEncounter &);
CreatureEncounter decodeCreatureEncounter(const Bytes &);
struct CreatureEvent {
  unsigned actor = 0;
  std::int32_t target = -1;
  std::string kind;
  std::uint32_t amount = 0;
};
class CreatureEncounterSession {
  World world_;
  std::shared_ptr<const CreatureRules> rules_;
  std::shared_ptr<const HealthRules> health_;
  void validate(const State &, const CreatureEncounter &) const;

public:
  CreatureEncounterSession(std::shared_ptr<const CreatureRules>,
                           std::shared_ptr<const HealthRules>, Bytes profile,
                           std::uint32_t seed = 1);
  CreatureEncounterSession(World, std::shared_ptr<const CreatureRules>,
                           std::shared_ptr<const HealthRules>);
  const World &world() const { return world_; }
  CreatureEncounter state() const {
    return decodeCreatureEncounter(world_.state().systems);
  }
  std::uint32_t maximumHealth() const { return rules_->maximumHealth(); }
  bool finished() const;
  std::vector<CreatureEvent> step(TickInput = {});
};
} // namespace mnm::game
