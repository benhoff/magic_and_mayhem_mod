#include "world.hpp"
#include "movement.hpp"
#include <limits>
#include <stdexcept>
#include <utility>

namespace mnm::game {
namespace {
const Entity* resolve(const State& s,Handle h) {
    if(h.slot>=s.slots.size()) return nullptr;
    const auto& slot=s.slots[h.slot];
    return slot.generation==h.generation && slot.entity ? &*slot.entity : nullptr;
}
bool known(Family f) {return static_cast<std::uint32_t>(f)<=2;}
void cancel(Entity& e) {
    if(!e.motion) return;
    auto& m=*e.motion;m.action=Action::cancelled;m.route.clear();m.next=0;m.goal.reset();m.origin={e.x,e.y,e.z};m.fine.reset();m.previous.reset();m.segmentTicks=0;
}
bool apply(State& s,const Command& c) {
    if(!resolve(s,c.subject)) return false;
    if((c.operation==Operation::target || c.operation==Operation::move) && c.target && !resolve(s,*c.target)) return false;
    auto& slot=s.slots[c.subject.slot];
    switch(c.operation) {
    case Operation::target: slot.entity->target=c.target; break;
    case Operation::cleanup: slot.entity->cleaned=true;slot.entity->target.reset();cancel(*slot.entity);break;
    case Operation::release:
        for(auto& peer:s.slots) if(peer.entity) {
            if(peer.entity->target==std::optional<Handle>(c.subject)) peer.entity->target.reset();
            if(peer.entity->motion && peer.entity->motion->goal==std::optional<Handle>(c.subject)) {
                if(active(peer.entity->motion->action)) cancel(*peer.entity);
                else peer.entity->motion->goal.reset();
            }
        }
        slot.entity.reset();
        // Generation zero retires an exhausted slot permanently rather than aliasing.
        slot.generation=slot.generation==std::numeric_limits<std::uint32_t>::max()?0:slot.generation+1;
        break;
    case Operation::move: {
        auto& e=*slot.entity;
        if(!s.navigation || !e.motion || e.cleaned) return false;
        auto destination=*c.destination;
        if(c.target) {const auto* goal=resolve(s,*c.target);destination={goal->x,goal->y,goal->z};}
        if(!contains(destination,*s.navigation)) return false;
        const auto budget=e.motion->budget;const auto sampleMotion=e.motion->sampleMotion;const auto continuousMotion=e.motion->continuousMotion;
        e.motion=CreatureMotion{};e.motion->sampleMotion=sampleMotion;e.motion->continuousMotion=continuousMotion;e.motion->action=Action::planning;e.motion->origin={e.x,e.y,e.z};
        e.motion->destination=destination;e.motion->goal=c.target;e.motion->budget=budget;break;
    }
    case Operation::motion: {
        auto& e=*slot.entity;if(!e.motion || e.cleaned) return false;
        e.x=c.update->position.x;e.y=c.update->position.y;e.z=c.update->position.z;e.motion=c.update->motion;break;
    }
    }
    return true;
}
}
World::World(std::uint32_t capacity,Limits limits):limits_(limits) {
    if(capacity>limits.slots || std::uint64_t(capacity)*Limits::slotCharge>limits.bytes) throw std::invalid_argument("world capacity exceeds limit");
    state_.slots.resize(capacity);validate(state_,limits_);
}
const Entity* World::find(Handle h) const {return resolve(state_,h);}
void World::validate(const State& s,const Limits& l) {
    if(s.slots.size()>l.slots || s.pending.size()>l.commands || s.phase20>=20 || s.phase90>=90 || s.expansionBudget>53)
        throw std::invalid_argument("invalid world bounds or scheduler fields");
    std::uint64_t size=s.map.size()+std::uint64_t(s.campaign.size())+s.systems.size();
    auto charge=[&](std::uint64_t n) {if(size>l.bytes || n>l.bytes-size) throw std::invalid_argument("world byte budget exceeded");size+=n;};
    charge(std::uint64_t(s.slots.size())*Limits::slotCharge+std::uint64_t(s.pending.size())*Limits::commandCharge);
    if(s.navigation) {
        const auto d=s.navigation->dimensions;
        if(s.map.empty() || d.x<1 || d.y<1 || d.z<1 || d.x>1024 || d.y>1024 || d.z>32)
            throw std::invalid_argument("invalid navigation binding");
    }
    for(const auto& slot:s.slots) if(slot.entity) {
        if(!slot.generation || !known(slot.entity->family)) throw std::invalid_argument("invalid entity identity/family");
        charge(slot.entity->state.size());
        if(slot.entity->target && !resolve(s,*slot.entity->target)) throw std::invalid_argument("dangling entity target");
        if(slot.entity->motion) {
            if(!s.navigation) throw std::invalid_argument("movement state requires bound map");
            validateMotion(*slot.entity,*s.navigation);charge(slot.entity->motion->route.size()*28);
            if(slot.entity->motion->fine) charge(72);
            if(slot.entity->motion->previous) charge(84);
            if(slot.entity->motion->goal && !resolve(s,*slot.entity->motion->goal)) throw std::invalid_argument("dangling movement goal");
        }
    }
    for(const auto& c:s.pending) {
        validateCommand(c);
        if(c.operation==Operation::motion) throw std::invalid_argument("internal motion update cannot be queued");
        if(c.destination && (!s.navigation || !contains(*c.destination,*s.navigation))) throw std::invalid_argument("invalid queued move destination");
    }
}
void World::restore(State next) {
    if(stepping_) throw std::logic_error("cannot restore during tick");
    validate(next,limits_);state_=std::move(next);
}
Handle World::spawn(Entity entity) {
    if(stepping_) throw std::logic_error("cannot spawn during tick callback");
    auto next=state_;
    for(std::uint32_t i=0;i<next.slots.size();++i) if(!next.slots[i].entity && next.slots[i].generation) {
        Handle h{i,next.slots[i].generation};next.slots[i].entity=std::move(entity);
        validate(next,limits_);state_=std::move(next);return h;
    }
    throw std::runtime_error("entity pool exhausted");
}
void World::enqueue(Command command) {
    if(stepping_) throw std::logic_error("emit tick commands through callback output");
    auto next=state_;next.pending.push_back(command);validate(next,limits_);state_=std::move(next);
}
TickReport World::step(TickInput input,const System& system) {
    if(stepping_) throw std::logic_error("recursive world tick");
    TickReport report;
    if(!input.admitted) return report;
    stepping_=true;
    struct Guard {bool& value;~Guard(){value=false;}} guard{stepping_};
    auto next=state_;
    std::uint64_t commandCount=next.pending.size();
    auto run=[&](const std::vector<Command>& commands) {
        for(const auto& c:commands) {
            validateCommand(c);
            if(apply(next,c)) ++report.applied;else ++report.rejected;
        }
    };
    run(next.pending);next.pending.clear();
    ++next.sequence;
    if(!input.alternateMode) ++next.tick;
    auto phase=[&](Phase p) {
        report.phases.push_back(p);
        if(system) {
            std::vector<Command> commands;system(p,next,commands);
            commandCount+=commands.size();
            if(commandCount>limits_.commands) throw std::invalid_argument("tick command budget exceeded");
            run(commands);
        }
    };
    if(!input.alternateMode) {
        phase(Phase::maintenance);
        next.expansionBudget=input.suppressSearch?0:53;
        phase(Phase::decisions);
        next.phase20=(next.phase20+1)%20;next.phase90=(next.phase90+1)%90;
        phase(Phase::secondaryCreatures);
    }
    phase(Phase::effects);phase(Phase::map);phase(Phase::queuedMap);phase(Phase::audio);
    validate(next,limits_);state_=std::move(next);report.advanced=true;return report;
}
}
