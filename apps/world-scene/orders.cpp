#include "orders.hpp"
#include <stdexcept>
namespace mnm::scene {
namespace {
bool eligible(const game::Entity& e) {return !e.cleaned && e.family==game::Family::creature && bool(e.motion);}
bool valid(const game::State& state,game::Handle h) {
    return h.slot<state.slots.size() && state.slots[h.slot].generation==h.generation &&
        state.slots[h.slot].entity && eligible(*state.slots[h.slot].entity);
}
}
std::vector<CreatureChoice> creatureChoices(const game::State& state) {
    std::vector<CreatureChoice> choices;
    for(std::uint32_t i=0;i<state.slots.size();++i) {
        const auto& slot=state.slots[i];
        if(slot.entity && eligible(*slot.entity)) choices.push_back({{i,slot.generation},{slot.entity->x,slot.entity->y,slot.entity->z},slot.entity->motion->action});
    }
    return choices;
}
void Orders::select(const game::State& state,std::optional<game::Handle> h) {
    if(h && !valid(state,*h)) throw std::invalid_argument("Creature is no longer available");
    selected_=h;
}
void Orders::synchronize(const game::State& state) {
    if(selected_ && !valid(state,*selected_)) selected_.reset();
}
void Orders::stop(game::MovementSession& session) {
    synchronize(session.world().state());
    if(!selected_) throw std::invalid_argument("Select a creature first");
    session.stop(*selected_);
}
std::uint32_t Orders::cancelQueuedMoves(game::MovementSession& session) {
    synchronize(session.world().state());
    if(!selected_) throw std::invalid_argument("Select a creature first");
    return session.cancelQueuedMoves(*selected_);
}
void Orders::move(game::MovementSession& session,game::Point target) {
    synchronize(session.world().state());
    if(!selected_) throw std::invalid_argument("Select a creature first");
    session.move(*selected_,target);
}
}
