#pragma once
#include "health_rules.hpp"
#include "world.hpp"
#include <array>
#include <memory>

namespace mnm::game {
struct MeleeSample {
  std::int32_t animationEvent = 0;
  std::uint32_t damage = 0, cooldown = 0, draws = 0;
  bool finished = false;
};
// Immutable adapter; every mutable controller/RNG byte is owned by the session.
// Services depend on this contract, never on Qt or original packed objects.
class MeleeDriver {
public:
  virtual ~MeleeDriver() = default;
  virtual std::uint64_t identity() const = 0;
  virtual std::uint32_t maximumHealth() const = 0;
  virtual Bytes random(std::uint32_t seed) const = 0;
  virtual Bytes start() const = 0;
  virtual void validate(const Bytes &cursor, const Bytes &random,
                        bool attacking) const = 0;
  virtual MeleeSample advance(Bytes &cursor, Bytes &random) const = 0;
};
enum class ExchangePhase : std::uint32_t { attacking, recovering, dead };
struct ExchangeActor {
  Handle actor;
  std::uint32_t health = 0, cooldown = 0;
  ExchangePhase phase = ExchangePhase::attacking;
  Bytes cursor;
};
struct ExchangeState {
  std::uint64_t driver = 0, healthRules = 0;
  Bytes random;
  std::array<ExchangeActor, 2> actors;
};
Bytes encodeExchange(const ExchangeState &);
ExchangeState decodeExchange(const Bytes &);
struct ExchangeEvent {
  Handle actor, target;
  std::int32_t animation = 0;
  std::uint32_t requested = 0, applied = 0, draws = 0;
  bool died = false;
};
class MeleeExchangeSession {
  World world_;
  std::shared_ptr<const MeleeDriver> driver_;
  std::shared_ptr<const HealthRules> healthRules_;
  void validate(const State &, const ExchangeState &) const;

public:
  MeleeExchangeSession(std::shared_ptr<const MeleeDriver>,
                       std::shared_ptr<const HealthRules>, Bytes profile,
                       std::uint32_t seed = 1);
  MeleeExchangeSession(World, std::shared_ptr<const MeleeDriver>,
                       std::shared_ptr<const HealthRules>);
  const World &world() const { return world_; }
  ExchangeState state() const { return decodeExchange(world_.state().systems); }
  std::uint32_t maximumHealth() const { return driver_->maximumHealth(); }
  bool finished() const;
  std::vector<ExchangeEvent> step(TickInput = {});
};
} // namespace mnm::game
