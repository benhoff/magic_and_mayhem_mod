#include "movement.hpp"
#include "persistence/snapshot.hpp"
#include <cstdlib>
#include <stdexcept>

namespace mnm::game {
bool contains(Point p,const NavigationBinding& b) {
    return p.x>=0 && p.y>=0 && p.z>=0 && p.x<b.dimensions.x && p.y<b.dimensions.y && p.z<b.dimensions.z;
}
bool active(Action a) {return a==Action::planning || a==Action::moving;}
void validateCommand(const Command& c) {
    if(static_cast<std::uint32_t>(c.operation)>4 ||
       (c.operation!=Operation::target && c.operation!=Operation::move && c.target) ||
       ((c.operation==Operation::move)!=bool(c.destination)) ||
       ((c.operation==Operation::motion)!=bool(c.update)))
        throw std::invalid_argument("invalid command encoding");
}
void validateMotion(const Entity& e,const NavigationBinding& binding) {
    if(!e.motion) return;
    const auto& m=*e.motion;
    const Point position{e.x,e.y,e.z};
    if(e.family!=Family::creature || !contains(position,binding) || !contains(m.origin,binding) || !contains(m.destination,binding) ||
       static_cast<std::uint32_t>(m.action)>6 || m.budget<2 || m.budget>4096 || m.route.size()>16 || m.next>m.route.size() ||
       (e.cleaned && active(m.action))) throw std::invalid_argument("invalid creature movement state");
    if((m.action==Action::moving && (m.route.empty() || m.next==m.route.size())) ||
       (m.action!=Action::moving && m.action!=Action::arrived && (!m.route.empty() || m.next)) ||
       (m.action==Action::arrived && (position!=m.destination || m.next!=m.route.size())))
        throw std::invalid_argument("invalid movement progress");
    Point previous=m.origin;
    for(const auto& waypoint:m.route) {
        const auto p=waypoint.position;
        const auto near=[](std::int32_t a,std::int32_t b,std::int32_t dimension) {
            auto delta=std::llabs(std::int64_t(a)-b);return delta<=1 || delta==dimension-1;
        };
        if(!contains(p,binding) || p==previous || !near(p.x,previous.x,binding.dimensions.x) ||
           !near(p.y,previous.y,binding.dimensions.y) || std::llabs(std::int64_t(p.z)-previous.z)>1 ||
           waypoint.direction<0 || waypoint.direction>7 || waypoint.verticalDelta<-1 || waypoint.verticalDelta>1 ||
           waypoint.category<0 || waypoint.category>5) throw std::invalid_argument("invalid movement waypoint");
        previous=p;
    }
    if(position!=(m.next?m.route[m.next-1].position:m.origin)) throw std::invalid_argument("position disagrees with route progress");
}
void MovementSession::validateBinding(const State& state,const Navigation& navigation) {
    if(!state.navigation || !(*state.navigation==navigation.binding()) || state.map.empty())
        throw std::invalid_argument("movement map identity mismatch");
    unsigned creatures=0;
    for(const auto& slot:state.slots) if(slot.entity && slot.entity->motion) {
        const auto& e=*slot.entity;
        if(++creatures>1 || e.type!=navigation.creatureType()) throw std::invalid_argument("movement slice supports one captured creature profile");
        validateMotion(e,*state.navigation);
        auto probe=e;
        // Validate every saved edge, including already consumed edges, on rebound inputs.
        probe.x=e.motion->origin.x;probe.y=e.motion->origin.y;probe.z=e.motion->origin.z;
        for(const auto& waypoint:e.motion->route) {
            if(!navigation.accepts(probe,waypoint)) throw std::invalid_argument("saved route rejected by rebound map");
            probe.x=waypoint.position.x;probe.y=waypoint.position.y;probe.z=waypoint.position.z;
        }
    }
    for(const auto& c:state.pending) if(c.operation==Operation::motion) throw std::invalid_argument("internal motion update cannot be pending at checkpoint boundary");
}
MovementSession::MovementSession(World world,std::shared_ptr<const Navigation> navigation):world_(std::move(world)),navigation_(std::move(navigation)) {
    if(!navigation_) throw std::invalid_argument("navigation resource required");
    validateBinding(world_.state(),*navigation_);
}
Handle MovementSession::spawn(Entity entity) {
    if(entity.family==Family::creature) {
        entity.motion=CreatureMotion{};entity.motion->origin=entity.motion->destination={entity.x,entity.y,entity.z};
        if(entity.type!=navigation_->creatureType()) throw std::invalid_argument("creature profile mismatch");
        for(const auto& slot:world_.state().slots) if(slot.entity && slot.entity->motion) throw std::invalid_argument("only one moving creature supported");
    } else if(entity.motion) throw std::invalid_argument("motion requires creature family");
    validateMotion(entity,navigation_->binding());return world_.spawn(std::move(entity));
}
void MovementSession::enqueue(Command command) {
    if(command.operation==Operation::motion) throw std::invalid_argument("motion updates belong to tick system");
    if(command.destination && !contains(*command.destination,navigation_->binding())) throw std::invalid_argument("move destination outside map");
    world_.enqueue(std::move(command));
}
void MovementSession::move(Handle subject,Point destination,std::optional<Handle> goal) {
    Command c{Operation::move,subject,goal};c.destination=destination;enqueue(std::move(c));
}
TickReport MovementSession::step(TickInput input) {
    return world_.step(input,[this,input](Phase phase,const State& state,std::vector<Command>& commands) {
        if(phase!=Phase::maintenance && phase!=Phase::decisions) return;
        if(!state.navigation || !(*state.navigation==navigation_->binding())) throw std::invalid_argument("movement resource changed");
        for(std::uint32_t i=0;i<state.slots.size();++i) if(state.slots[i].entity && state.slots[i].entity->motion) {
            const auto& e=*state.slots[i].entity;auto motion=*e.motion;Point position{e.x,e.y,e.z};bool changed=false;
            if(phase==Phase::maintenance && motion.action==Action::moving) {
                const auto waypoint=motion.route.at(motion.next);
                if(!navigation_->accepts(e,waypoint)) {
                    motion.action=Action::blocked;motion.route.clear();motion.next=0;motion.origin=position;
                } else {
                    position=waypoint.position;++motion.next;
                    if(motion.next==motion.route.size()) {
                        if(position==motion.destination) motion.action=Action::arrived;
                        else {motion.action=Action::planning;motion.route.clear();motion.next=0;motion.origin=position;}
                    }
                }
                changed=true;
            }
            if(phase==Phase::decisions && motion.action==Action::planning && !input.suppressSearch) {
                auto plan=navigation_->plan(e,motion.destination,motion.budget);
                motion.origin=position;motion.next=0;motion.route=std::move(plan.route);
                if(plan.status==PlanStatus::budgetExhausted) motion.action=Action::searchLimited;
                else if(plan.status==PlanStatus::unreachable) motion.action=Action::blocked;
                else motion.action=position==motion.destination?Action::arrived:Action::moving;
                if(motion.action==Action::searchLimited || motion.action==Action::blocked) motion.route.clear();
                changed=true;
            }
            if(changed) {
                Command c{Operation::motion,{i,state.slots[i].generation},{}};
                c.update=MotionUpdate{position,std::move(motion)};commands.push_back(std::move(c));
            }
        }
    });
}
void MovementSession::restore(const std::filesystem::path& path,const MapResolver& resolver) {
    auto state=readSnapshot(path,world_.limits());
    if(!resolver) throw std::invalid_argument("map resolver required");
    auto candidate=resolver(state.map);if(!candidate) throw std::invalid_argument("missing saved map");
    validateBinding(state,*candidate);
    world_.restore(std::move(state));navigation_=std::move(candidate);
}
}
