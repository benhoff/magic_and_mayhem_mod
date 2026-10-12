#pragma once
#include "combat.hpp"
#include <array>

namespace mnm::game {
// Explicit authored-arena rules, distinct from original game balance/behavior.
struct SkirmishRules {
    static constexpr unsigned manaMaximum = 20, manaInitial = 20, regenerationTicks = 3;
    static constexpr unsigned summonCost = 8, boltCost = 4, castTicks = 2, flightTicks = 4,
                              boltDamage = 10;
    static constexpr unsigned wizardHealth = 48, creatureHealth = 24, creatureDamage = 6,
                              aiInterval = 8, livingCreatures = 6;
};
enum class UnitRole : std::uint32_t { wizard, sentinel };
enum class Spell : std::uint32_t { none, summon, bolt };
enum class Outcome : std::uint32_t { running, victory, defeat, draw };
enum class SkirmishOrderKind : std::uint32_t { summon, bolt, move, stop, attack };
struct SkirmishOrder {
    SkirmishOrderKind kind = SkirmishOrderKind::stop;
    Handle subject;
    std::optional<Handle> target;
    std::optional<Point> destination;
};
struct UnitRoleRecord {
    Handle actor;
    UnitRole role = UnitRole::sentinel;
};
struct Wizard {
    Handle actor;
    std::uint32_t mana = SkirmishRules::manaInitial, regeneration = 0;
    Spell spell = Spell::none;
    std::uint32_t remaining = 0;
    Point aim{};
    std::optional<Handle> target;
};
struct Projectile {
    std::uint32_t id = 0, owner = 0;
    Handle source, target;
    Point origin, destination;
    std::uint32_t remaining = SkirmishRules::flightTicks;
};
struct SkirmishState {
    Outcome outcome = Outcome::running;
    std::uint32_t aiWait = 0, nextProjectile = 1;
    std::array<Wizard, 2> wizards;
    std::vector<UnitRoleRecord> units;
    std::vector<Projectile> projectiles;
    std::vector<SkirmishOrder> pending;
};
Bytes encodeSkirmish(const SkirmishState &);
SkirmishState decodeSkirmish(const Bytes &);
void validateSkirmish(const State &, const CombatState &, const SkirmishState &);
bool arenaCell(Point);
enum class SkirmishEventKind { casting, summoned, launched, impact, missed, refused, finished };
enum class SkirmishRefusal { none, busy, mana, placement, capacity, identity, ownership, target };
struct SkirmishEvent {
    SkirmishEventKind kind;
    Handle actor;
    std::optional<Handle> target;
    Spell spell = Spell::none;
    SkirmishRefusal refusal = SkirmishRefusal::none;
};
struct SkirmishReport {
    CombatReport combat;
    std::vector<SkirmishEvent> events;
};
class SkirmishSession {
    CombatSession combat_;
    std::shared_ptr<const Navigation> navigation_;
    void publish(CombatSession, SkirmishState);

  public:
    explicit SkirmishSession(std::shared_ptr<const Navigation>,
                             std::shared_ptr<const HealthRules> = {});
    SkirmishSession(World, std::shared_ptr<const Navigation>,
                    std::shared_ptr<const HealthRules> = {});
    const World &world() const { return combat_.world(); }
    const CombatSession &combat() const { return combat_; }
    SkirmishState state() const { return decodeSkirmish(world().state().campaign); }
    void enqueue(SkirmishOrder);
    SkirmishReport step(TickInput = {});
    void restore(const std::filesystem::path &, const NavigationResolver &);
};
} // namespace mnm::game
