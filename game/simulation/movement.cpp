#include "movement.hpp"
#include "persistence/snapshot.hpp"
#include <cstdlib>
#include <stdexcept>

namespace mnm::game {
bool contains(Point p,const NavigationBinding& b) {
    return p.x>=0 && p.y>=0 && p.z>=0 && p.x<b.dimensions.x && p.y<b.dimensions.y && p.z<b.dimensions.z;
}
void Navigation::validateSegmentHistory(const Entity&,const SegmentHistory&) const {throw std::invalid_argument("segment setup driver unavailable");}
FineMotion Navigation::prepareFineMotion(const Entity&,const RoutePoint&) const {throw std::invalid_argument("sample motion driver unavailable");}
bool Navigation::advanceFineMotion(const Entity&,const RoutePoint&,FineMotion&) const {throw std::invalid_argument("sample motion driver unavailable");}
void Navigation::validateFineMotion(const Entity&,const RoutePoint&,const FineMotion&) const {throw std::invalid_argument("sample motion driver unavailable");}
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
    if((m.continuousMotion && !m.sampleMotion) || (!m.continuousMotion && (m.previous || m.segmentTicks)) ||
       m.segmentTicks>100000 || (!m.fine && m.segmentTicks) ||
       (m.continuousMotion && ((m.fine && !m.segmentTicks) || (m.next && !m.previous)))) throw std::invalid_argument("invalid segment continuation policy");
    auto cursor=[&](const FineMotion& f) {
        if(f.animation && (!m.continuousMotion || f.animation->sequence>=4096 || f.animation->pc>65536 ||
           (f.animation->displayed && *f.animation->displayed>=65536) ||
           f.animation->elapsed>f.animation->delay || f.animation->breakFlag>1 ||
           (!f.animation->active && f.animation->displayed))) throw std::invalid_argument("invalid animation cursor");
    };
    if(m.fine) cursor(*m.fine);
    if(m.previous) cursor(m.previous->motion);
    if(m.previous) {
        const auto& h=*m.previous;const auto& f=h.motion;
        if(m.next && (h.direction!=m.route.at(m.next-1).direction || h.vertical!=m.route.at(m.next-1).verticalDelta || h.category!=m.route.at(m.next-1).category))
            throw std::invalid_argument("previous segment disagrees with consumed route point");
        if(h.direction<0 || h.direction>7 || h.vertical || h.category || f.progress<192 || f.progress>=384 ||
           f.rate<0 || f.rate>1000000 || f.duration<1 || f.duration>1000000 || f.accumulator<0 || f.accumulator>2000000 ||
           f.frame>48 || f.animationFrame>=(f.animation?50U:12U) || f.initialFrame>=48 || std::llabs(std::int64_t(f.initialResidualX))>1000000 || std::llabs(std::int64_t(f.initialResidualY))>1000000 || std::llabs(std::int64_t(f.travelX))>f.progress || std::llabs(std::int64_t(f.travelY))>f.progress ||
           f.heightDelta || f.heightOrigin!=position.z*16 || f.fine.z!=position.z*16 ||
           std::llabs(std::int64_t(f.fine.x))>binding.dimensions.x*32+64 ||
           std::llabs(std::int64_t(f.fine.y))>binding.dimensions.y*32+64 ||
           std::llabs(std::int64_t(f.residualX))>1000000 || std::llabs(std::int64_t(f.residualY))>1000000)
            throw std::invalid_argument("invalid previous segment");
    }
    if(m.fine) {
        const auto& f=*m.fine;
        if(!m.continuousMotion && (f.animationFrame || f.initialFrame || f.initialResidualX || f.initialResidualY)) throw std::invalid_argument("legacy driver has continuous cursor state");
        if(!m.sampleMotion || m.action!=Action::moving || f.rate<(m.continuousMotion?0:1) || f.rate>1000000 ||
           f.duration<1 || f.duration>1000000 || f.accumulator<0 || f.accumulator>2000000 ||
           f.progress<0 || f.progress>=192 || f.frame>=(m.continuousMotion?48U:12U) ||
           (m.continuousMotion && (f.animationFrame>=(f.animation?50U:12U) || f.initialFrame>=48 || std::llabs(std::int64_t(f.initialResidualX))>1000000 || std::llabs(std::int64_t(f.initialResidualY))>1000000)) ||
           std::llabs(std::int64_t(f.travelX))>f.progress || std::llabs(std::int64_t(f.travelY))>f.progress ||
           std::llabs(std::int64_t(f.heightDelta))>16 || f.heightOrigin!=position.z*16 ||
           (!m.continuousMotion && (f.fine.x!=position.x*32+f.travelX/6 || f.fine.y!=position.y*32+f.travelY/6)) ||
           (m.continuousMotion && (std::llabs(std::int64_t(f.fine.x)-position.x*32)>64 || std::llabs(std::int64_t(f.fine.y)-position.y*32)>64)) ||
           f.fine.z!=f.heightOrigin+f.heightDelta*f.progress/192 ||
           std::llabs(std::int64_t(f.residualX))>(m.continuousMotion?1000000:64) || std::llabs(std::int64_t(f.residualY))>(m.continuousMotion?1000000:64))
            throw std::invalid_argument("invalid fine motion continuation");
    }
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
    if(!(state.animation==navigation.animationBinding())) throw std::invalid_argument("movement animation resource mismatch");
    unsigned creatures=0;
    for(const auto& slot:state.slots) if(slot.entity && slot.entity->motion) {
        const auto& e=*slot.entity;
        if(++creatures>1 || e.type!=navigation.creatureType()) throw std::invalid_argument("movement slice supports one captured creature profile");
        validateMotion(e,*state.navigation);
        if(e.motion->previous) navigation.validateSegmentHistory(e,*e.motion->previous);
        if(e.motion->sampleMotion) {
            if(e.motion->fine) navigation.validateFineMotion(e,e.motion->route.at(e.motion->next),*e.motion->fine);
            else if(e.motion->action==Action::moving) (void)navigation.prepareFineMotion(e,e.motion->route.at(e.motion->next));
        }
        auto probe=e;
        if(probe.motion->continuousMotion) {probe.motion->previous.reset();probe.motion->fine.reset();probe.motion->segmentTicks=0;}
        // Validate every saved edge, including already consumed edges, on rebound inputs.
        probe.x=e.motion->origin.x;probe.y=e.motion->origin.y;probe.z=e.motion->origin.z;
        for(const auto& waypoint:e.motion->route) {
            if(!navigation.accepts(probe,waypoint)) throw std::invalid_argument("saved route rejected by rebound map");
            if(e.motion->sampleMotion) (void)navigation.prepareFineMotion(probe,waypoint);
            probe.x=waypoint.position.x;probe.y=waypoint.position.y;probe.z=waypoint.position.z;
        }
    }
    for(const auto& c:state.pending) if(c.operation==Operation::motion) throw std::invalid_argument("internal motion update cannot be pending at checkpoint boundary");
}
MovementSession::MovementSession(World world,std::shared_ptr<const Navigation> navigation):world_(std::move(world)),navigation_(std::move(navigation)) {
    if(!navigation_) throw std::invalid_argument("navigation resource required");
    validateBinding(world_.state(),*navigation_);
}
Handle MovementSession::spawn(Entity entity,bool sampleMotion,bool continuousMotion) {
    if(entity.family==Family::creature) {
        entity.motion=CreatureMotion{};entity.motion->sampleMotion=sampleMotion;entity.motion->continuousMotion=continuousMotion;entity.motion->origin=entity.motion->destination={entity.x,entity.y,entity.z};
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
                    motion.action=Action::blocked;motion.route.clear();motion.next=0;motion.origin=position;motion.fine.reset();motion.previous.reset();motion.segmentTicks=0;
                } else {
                    bool completed=true;
                    if(motion.sampleMotion) {
                        if(!motion.fine) motion.fine=navigation_->prepareFineMotion(e,waypoint);
                        completed=navigation_->advanceFineMotion(e,waypoint,*motion.fine);
                        if(motion.continuousMotion) ++motion.segmentTicks;
                    }
                    if(completed) {
                        if(motion.continuousMotion) motion.previous=SegmentHistory{*motion.fine,waypoint.direction,waypoint.verticalDelta,waypoint.category};
                        motion.fine.reset();motion.segmentTicks=0;position=waypoint.position;++motion.next;
                        if(motion.next==motion.route.size()) {
                            if(position==motion.destination) motion.action=Action::arrived;
                            else {motion.action=Action::planning;motion.route.clear();motion.next=0;motion.origin=position;}
                        }
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
                if(motion.sampleMotion && motion.action==Action::moving) {
                    auto probe=e;
                    if(probe.motion->continuousMotion) {probe.motion->previous.reset();probe.motion->fine.reset();probe.motion->segmentTicks=0;}
                    for(const auto& point:motion.route) {
                        (void)navigation_->prepareFineMotion(probe,point);
                        probe.x=point.position.x;probe.y=point.position.y;probe.z=point.position.z;
                    }
                }
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
    if(!resolver) throw std::invalid_argument("map resolver required");
    restoreResources(path,[&](const State& state){return resolver(state.map);});
}
void MovementSession::restoreResources(const std::filesystem::path& path,const NavigationResolver& resolver) {
    auto state=readSnapshot(path,world_.limits());
    if(!resolver) throw std::invalid_argument("navigation resolver required");
    auto candidate=resolver(state);if(!candidate) throw std::invalid_argument("missing saved resources");
    validateBinding(state,*candidate);
    world_.restore(std::move(state));navigation_=std::move(candidate);
}
}
