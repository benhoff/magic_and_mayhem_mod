#pragma once
#include "simulation/movement.hpp"
namespace mnm::scene {
struct CreatureChoice { game::Handle handle; game::Point position; game::Action action; };
std::vector<CreatureChoice> creatureChoices(const game::State&);
// Transient application selection; the simulation owns all queued commands.
class Orders {
    std::optional<game::Handle> selected_;
public:
    std::optional<game::Handle> selected() const {return selected_;}
    void select(const game::State&,std::optional<game::Handle>);
    void synchronize(const game::State&);
    void move(game::MovementSession&,game::Point);
    void stop(game::MovementSession&);
    std::uint32_t cancelQueuedMoves(game::MovementSession&);
};
}
