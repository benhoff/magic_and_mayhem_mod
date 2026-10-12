#pragma once
#include "movement.hpp"
#include "health_rules.hpp"

namespace mnm::game {
// Bounded native melee policy. These values are not recovered game balance.
struct CombatStats {
    std::uint32_t maximumHealth = 20, damage = 6, windupTicks = 2, recoveryTicks = 3;
};
enum class CombatAction : std::uint32_t { idle, approaching, windingUp, recovering, dead };
enum class CombatControl : std::uint32_t { automatic, hold, move, attack };
enum class CombatOrderKind : std::uint32_t { move, stop, attack };
struct CombatOrder {
    CombatOrderKind kind = CombatOrderKind::stop;
    Handle subject;
    std::optional<Handle> target;
    std::optional<Point> destination;
};
struct Fighter {
    Handle actor;
    CombatStats stats;
    std::uint32_t health = 20;
    CombatAction action = CombatAction::idle;
    std::uint32_t remaining = 0;
    std::optional<Handle> target;
    CombatControl control = CombatControl::automatic;
    std::optional<Point> destination;
};
struct CombatState {
    std::uint32_t acquisitionRange = 12;
    std::vector<Fighter> fighters;
    std::vector<CombatOrder> pending;
    std::uint64_t healthRulesIdentity = 0;
};
// Explicit little-endian subsystems v1/v2/v3 stored in State::systems. Native world
// checkpoints already own/checksum this blob, including in-flight attacks.
Bytes encodeCombat(const CombatState &);
CombatState decodeCombat(const Bytes &);
void validateCombat(const State &, const CombatState &);
enum class CombatEventKind { acquired, approach, windup, hit, missed, died, healed };
struct CombatEvent {
    CombatEventKind kind;
    Handle actor, target;
    std::uint32_t amount = 0;
};
struct CombatReport {
    TickReport movement;
    std::vector<CombatEvent> events;
    std::uint32_t rejectedOrders = 0;
};

class CombatSession {
    MovementSession movement_;
    std::shared_ptr<const Navigation> navigation_;
    std::shared_ptr<const HealthRules> healthRules_;
    void publish(MovementSession, CombatState);

  public:
    CombatSession(World, std::shared_ptr<const Navigation>,
                  std::shared_ptr<const HealthRules> = {});
    const std::shared_ptr<const HealthRules> &healthRules() const { return healthRules_; }
    const World &world() const { return movement_.world(); }
    const MovementSession &movement() const { return movement_; }
    CombatState combat() const { return decodeCombat(world().state().systems); }
    Handle spawn(Entity, CombatStats = {}, bool fineMotion = false,
                 CombatControl = CombatControl::automatic);
    void enqueue(CombatOrder);
    CombatReport damage(Handle target, std::uint32_t amount, Handle source);
    CombatReport heal(Handle target, std::uint32_t amount, Handle source);
    void holdAll();
    CombatReport step(TickInput = {});
    void restore(const std::filesystem::path &, const NavigationResolver &);
};
} // namespace mnm::game
