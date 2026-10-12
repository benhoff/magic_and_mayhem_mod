#pragma once
#include "movement.hpp"

namespace mnm::game {
// Bounded native melee policy. These values are not recovered game balance.
struct CombatStats {
    std::uint32_t maximumHealth=20, damage=6, windupTicks=2, recoveryTicks=3;
};
enum class CombatAction : std::uint32_t {idle, approaching, windingUp, recovering, dead};
struct Fighter {
    Handle actor;
    CombatStats stats;
    std::uint32_t health=20;
    CombatAction action=CombatAction::idle;
    std::uint32_t remaining=0;
    std::optional<Handle> target;
};
struct CombatState {
    std::uint32_t acquisitionRange=12;
    std::vector<Fighter> fighters;
};
// Explicit little-endian subsystem v1 stored in State::systems. Native world
// checkpoints already own/checksum this blob, including in-flight attacks.
Bytes encodeCombat(const CombatState&);
CombatState decodeCombat(const Bytes&);
void validateCombat(const State&,const CombatState&);
enum class CombatEventKind {acquired, approach, windup, hit, missed, died};
struct CombatEvent {
    CombatEventKind kind;
    Handle actor, target;
    std::uint32_t amount=0;
};
struct CombatReport {TickReport movement; std::vector<CombatEvent> events;};

class CombatSession {
    MovementSession movement_;
    std::shared_ptr<const Navigation> navigation_;
    void publish(MovementSession,CombatState);
public:
    CombatSession(World,std::shared_ptr<const Navigation>);
    const World& world() const {return movement_.world();}
    const MovementSession& movement() const {return movement_;}
    CombatState combat() const {return decodeCombat(world().state().systems);}
    Handle spawn(Entity,CombatStats={},bool fineMotion=false);
    CombatReport step(TickInput={});
    void restore(const std::filesystem::path&,const NavigationResolver&);
};
}
