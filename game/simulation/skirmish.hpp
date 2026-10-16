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
enum class Spell : std::uint32_t { none, summon, bolt, cure, sphere };
enum class Outcome : std::uint32_t { running, victory, defeat, draw };
enum class SkirmishOrderKind : std::uint32_t { summon, bolt, move, stop, attack, cure, sphere };
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
// Bounded authored mission policy; independent of original campaign/rules.
// Standing cells are strictly lexicographically ordered (x,y,z), perimeter excluded.
struct SkirmishMission {
    std::string map;
    Point dimensions;
    std::array<Point, 2> wizards;
    std::vector<Point> aiSummons, standing;
};
const SkirmishMission &arenaMission();
void validateMission(const SkirmishMission &);
bool missionCell(const SkirmishMission &, Point);
bool sameMission(const SkirmishMission &, const SkirmishMission &);
struct SkirmishState {
    std::optional<SkirmishMission> mission; // absent means the byte-compatible v1 arena
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
// Application runtime owns only its versioned effect continuation. Health remains
// in CombatState and mana in Wizard; calls act on the transaction candidate.
class SkirmishEffects {
  public:
    virtual ~SkirmishEffects() = default;
    virtual std::uint64_t identity() const = 0;
    virtual Bytes initial() const = 0;
    virtual void validate(const CombatSession &, const SkirmishState &) const = 0;
    virtual SkirmishRefusal cast(CombatSession &, SkirmishState &, Handle source, Handle target) const = 0;
    virtual SkirmishRefusal castSpell(CombatSession &combat, SkirmishState &meta,
                                    Spell spell, Handle source, Handle target) const {
        return spell == Spell::cure ? cast(combat, meta, source, target)
                                   : SkirmishRefusal::target;
    }
    virtual bool supports(Spell spell) const { return spell == Spell::cure; }
    virtual CombatReport step(CombatSession &, SkirmishState &, TickInput) const = 0;
    virtual void cancel(CombatSession &, SkirmishState &) const = 0;
    virtual void removed(CombatSession &, SkirmishState &, Handle) const {}
};
class SkirmishSession {
    SkirmishMission mission_;
    CombatSession combat_;
    std::shared_ptr<const Navigation> navigation_;
    std::shared_ptr<const SkirmishEffects> effects_;
    void publish(CombatSession, SkirmishState);

  public:
    explicit SkirmishSession(std::shared_ptr<const Navigation>,
                             std::shared_ptr<const HealthRules> = {},
                             std::shared_ptr<const SkirmishEffects> = {});
    SkirmishSession(SkirmishMission, std::shared_ptr<const Navigation>,
                    std::shared_ptr<const HealthRules> = {},
                    std::shared_ptr<const SkirmishEffects> = {});
    const SkirmishMission &mission() const { return mission_; }
    SkirmishSession(World, std::shared_ptr<const Navigation>,
                    std::shared_ptr<const HealthRules> = {},
                             std::shared_ptr<const SkirmishEffects> = {});
    const std::shared_ptr<const SkirmishEffects> &effectsRuntime() const { return effects_; }
    const World &world() const { return combat_.world(); }
    const CombatSession &combat() const { return combat_; }
    SkirmishState state() const { return decodeSkirmish(world().state().campaign); }
    void enqueue(SkirmishOrder);
    void removeCreature(Handle);
    SkirmishReport step(TickInput = {});
    void restore(const std::filesystem::path &, const NavigationResolver &);
};
} // namespace mnm::game
