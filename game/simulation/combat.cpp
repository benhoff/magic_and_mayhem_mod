#include "combat.hpp"
#include "occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <tuple>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t,8> magic{{'M','N','M','C','O','M','0','1'}};
void require(bool v,const char* reason) {if(!v) throw std::invalid_argument(reason);}
void statsValid(const CombatStats& s) {
    require(s.maximumHealth>=1 && s.maximumHealth<=1000000 && s.damage>=1 && s.damage<=1000000 &&
            s.windupTicks>=1 && s.windupTicks<=100000 && s.recoveryTicks>=1 && s.recoveryTicks<=100000,"invalid native combat stats");
}
const Entity* entity(const State& s,Handle h) {
    if(h.slot>=s.slots.size()) return nullptr;
    const auto& slot=s.slots[h.slot];
    return slot.generation==h.generation && slot.entity? &*slot.entity:nullptr;
}
Fighter* fighter(CombatState& c,Handle h) {
    for(auto& f:c.fighters) if(f.actor==h) return &f;
    return nullptr;
}
std::int64_t distance(const Entity& a,const Entity& b) {
    return std::max(std::llabs(std::int64_t(a.x)-b.x),std::llabs(std::int64_t(a.y)-b.y));
}
bool adjacent(const Entity& a,const Entity& b) {return a.z==b.z && distance(a,b)==1;}
Point position(const Entity& e) {return {e.x,e.y,e.z};}
bool eligible(const State& s,const Fighter& a,const Fighter& b,std::uint32_t range) {
    const auto* own=entity(s,a.actor);const auto* other=entity(s,b.actor);
    return own && other && !(a.actor==b.actor) && b.health && !other->cleaned &&
        own->owner!=other->owner && own->z==other->z && distance(*own,*other)<=range;
}
void clearAttack(Fighter& f) {f.target.reset();f.remaining=0;f.action=f.health?CombatAction::idle:CombatAction::dead;}
void stopMotion(Entity& e) {
    auto& m=*e.motion;
    m.pose=e.cleaned?std::nullopt:displayedPose(m);m.action=Action::cancelled;
    m.route.clear();m.next=0;m.goal.reset();m.origin=position(e);
    m.fine.reset();m.previous.reset();m.segmentTicks=0;
}
std::optional<Point> approach(const State& s,const Navigation& nav,Handle h,const Entity& target) {
    const auto& own=*entity(s,h);
    MovementReservations occupied(s,nav.binding().dimensions,nav.creatureType(),nav.creatureFootprint());
    std::vector<Point> candidates;
    for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x) {
        if(!x && !y) continue;
        Point p{target.x+x,target.y+y,target.z};
        if(!contains(p,nav.binding())) continue;
        // Reservations include unfinished fine edges, not only logical positions.
        bool free=true;
        const auto footprint=nav.creatureFootprint();
        for(int z=0;z<footprint.z;++z) {
            Point cell{p.x,p.y,p.z+z};
            if(!contains(cell,nav.binding())) {free=false;break;}
            const auto owner=occupied.owner(cell);
            if(owner && !(*owner==h)) {free=false;break;}
        }
        if(free) candidates.push_back(p);
    }
    const auto key=[&](Point p) {return std::tuple{std::max(std::llabs(std::int64_t(own.x)-p.x),std::llabs(std::int64_t(own.y)-p.y)),p.y,p.x};};
    std::sort(candidates.begin(),candidates.end(),[&](Point a,Point b){return key(a)<key(b);});
    if(candidates.empty()) return {};
    return candidates.front(); // Unreachable endpoints remain explicitly blocked.
}
}
Bytes encodeCombat(const CombatState& c) {
    require(c.acquisitionRange>=1 && c.acquisitionRange<=64 && c.fighters.size()<=32,"invalid combat bounds");
    Bytes b(magic.begin(),magic.end());
    const auto word=[&](std::uint32_t n){for(unsigned i=0;i<4;++i)b.push_back(std::uint8_t(n>>(8*i)));};
    word(c.acquisitionRange);word(static_cast<std::uint32_t>(c.fighters.size()));
    for(const auto& f:c.fighters) {
        statsValid(f.stats);
        word(f.actor.slot);word(f.actor.generation);word(f.stats.maximumHealth);word(f.stats.damage);
        word(f.stats.windupTicks);word(f.stats.recoveryTicks);word(f.health);word(static_cast<std::uint32_t>(f.action));word(f.remaining);
        word(f.target?1:0);word(f.target?f.target->slot:0);word(f.target?f.target->generation:0);
    }
    return b;
}
CombatState decodeCombat(const Bytes& b) {
    require(b.size()>=16 && std::equal(magic.begin(),magic.end(),b.begin()),"unknown native combat subsystem format");
    std::size_t p=8;
    const auto word=[&](){require(p<=b.size() && b.size()-p>=4,"truncated native combat state");
        std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(b[p++])<<(8*i);return v;};
    CombatState c;c.acquisitionRange=word();const auto count=word();
    require(count<=32 && b.size()==16+std::size_t(count)*48,"invalid native combat record count/size");
    for(std::uint32_t i=0;i<count;++i) {
        Fighter f;f.actor.slot=word();f.actor.generation=word();f.stats.maximumHealth=word();f.stats.damage=word();
        f.stats.windupTicks=word();f.stats.recoveryTicks=word();f.health=word();f.action=static_cast<CombatAction>(word());f.remaining=word();
        const auto present=word();Handle target;target.slot=word();target.generation=word();
        require(present<=1 && (present || (!target.slot && !target.generation)),"invalid native combat target encoding");
        if(present) f.target=target;
        c.fighters.push_back(f);
    }
    return c;
}
void validateCombat(const State& state,const CombatState& c) {
    (void)encodeCombat(c);
    std::optional<std::uint32_t> previous;
    for(const auto& f:c.fighters) {
        const auto* e=entity(state,f.actor);
        require(e && e->family==Family::creature && e->motion,"combat actor identity unavailable");
        require(!previous || f.actor.slot>*previous,"combat actors must be unique and ordered by slot");previous=f.actor.slot;
        require(f.health<=f.stats.maximumHealth && static_cast<std::uint32_t>(f.action)<=4,"invalid combat health/action");
        require((f.health==0)==(f.action==CombatAction::dead) && (f.health==0)==e->cleaned,"combat death/cleanup mismatch");
        const bool timed=f.action==CombatAction::windingUp || f.action==CombatAction::recovering;
        require(timed?(f.remaining>=1 && f.remaining<=(f.action==CombatAction::windingUp?f.stats.windupTicks:f.stats.recoveryTicks)):f.remaining==0,"invalid combat phase clock");
        require((f.action==CombatAction::idle || f.action==CombatAction::dead)?!f.target:bool(f.target),"combat phase requires a living target");
        if(f.target) {
            const auto it=std::find_if(c.fighters.begin(),c.fighters.end(),[&](const Fighter& b){return b.actor==*f.target;});
            require(it!=c.fighters.end() && eligible(state,f,*it,c.acquisitionRange),"invalid combat target identity/hostility/range");
        }
    }
    for(std::uint32_t i=0;i<state.slots.size();++i) if(state.slots[i].entity) {
        const auto it=std::find_if(c.fighters.begin(),c.fighters.end(),[&](const Fighter& f){return f.actor==Handle{i,state.slots[i].generation};});
        require(it!=c.fighters.end(),"combat world contains an unregistered entity");
    }
}
CombatSession::CombatSession(World world,std::shared_ptr<const Navigation> nav):movement_(std::move(world),nav),navigation_(std::move(nav)) {
    require(navigation_->maxMovingCreatures()>1 && navigation_->creatureFootprint().x==1 && navigation_->creatureFootprint().y==1,"combat requires bounded same-profile multi-creature navigation");
    auto state=movement_.world().state();
    if(state.systems.empty()) state.systems=encodeCombat({});
    const auto c=decodeCombat(state.systems);validateCombat(state,c);
    World initialized(0,movement_.world().limits());initialized.restore(std::move(state));
    movement_=MovementSession(std::move(initialized),navigation_);
}
void CombatSession::publish(MovementSession candidate,CombatState c) {
    auto state=candidate.world().state();state.systems=encodeCombat(c);validateCombat(state,c);
    World world(0,candidate.world().limits());world.restore(std::move(state));
    MovementSession complete(std::move(world),navigation_);movement_=std::move(complete);
}
Handle CombatSession::spawn(Entity e,CombatStats stats,bool fineMotion) {
    statsValid(stats);require(e.family==Family::creature && !e.cleaned,"combat spawn requires a living creature");
    auto candidate=movement_;auto c=combat();
    const auto h=candidate.spawn(std::move(e),fineMotion,fineMotion,fineMotion);
    c.fighters.push_back({h,stats,stats.maximumHealth,CombatAction::idle,0,{}});
    std::sort(c.fighters.begin(),c.fighters.end(),[](const Fighter& a,const Fighter& b){return a.actor.slot<b.actor.slot;});
    publish(std::move(candidate),std::move(c));return h;
}
CombatReport CombatSession::step(TickInput input) {
    CombatReport report;
    if(!input.admitted) return report;
    auto c=combat();auto candidate=movement_;
    // Movement-only advancement refuses opaque systems. Combat owns this blob
    // and republishes it only after all movement/combat checks succeed.
    auto plain=candidate.world().state();plain.systems.clear();World world(0,candidate.world().limits());world.restore(std::move(plain));
    candidate=MovementSession(std::move(world),navigation_);
    if(input.alternateMode) {report.movement=candidate.step(input);publish(std::move(candidate),std::move(c));return report;}
    const auto emit=[&](CombatEventKind k,Handle a,Handle b,std::uint32_t amount=0){report.events.push_back({k,a,b,amount});};
    for(auto& f:c.fighters) {
        if(!f.health) continue;
        const auto& own=*candidate.world().find(f.actor);
        auto* target=f.target?fighter(c,*f.target):nullptr;
        if(!target || !eligible(candidate.world().state(),f,*target,c.acquisitionRange)) {
            clearAttack(f);
            Fighter* best=nullptr;
            for(auto& b:c.fighters) if(eligible(candidate.world().state(),f,b,c.acquisitionRange) &&
                (!best || distance(own,*candidate.world().find(b.actor))<distance(own,*candidate.world().find(best->actor)))) best=&b;
            if(best) {f.target=best->actor;f.action=CombatAction::approaching;emit(CombatEventKind::acquired,f.actor,best->actor);target=best;}
        }
        if(!target) {if(active(own.motion->action))candidate.stop(f.actor);continue;}
        const auto& other=*candidate.world().find(target->actor);
        if(adjacent(own,other)) {
            if(active(own.motion->action)) candidate.stop(f.actor);
        } else {
            if(f.action==CombatAction::windingUp) emit(CombatEventKind::missed,f.actor,target->actor);
            f.action=CombatAction::approaching;f.remaining=0;
            const auto retained=own.motion->destination;
            const auto endpointDistance=std::max(std::llabs(std::int64_t(retained.x)-other.x),std::llabs(std::int64_t(retained.y)-other.y));
            // Keep an admitted approach while its endpoint still borders the
            // target. Rechoosing against transient reservations every tick can
            // repeatedly reset fine movement before its first edge completes.
            if(active(own.motion->action) && retained.z==other.z && endpointDistance==1) continue;
            const auto dest=approach(candidate.world().state(),*navigation_,f.actor,other);
            if(dest && (own.motion->destination!=*dest || own.motion->action==Action::idle || own.motion->action==Action::cancelled)) {
                candidate.move(f.actor,*dest);emit(CombatEventKind::approach,f.actor,target->actor);
            } else if(!dest && active(own.motion->action)) candidate.stop(f.actor);
        }
    }
    report.movement=candidate.step(input);
    auto after=candidate.world().state();
    for(auto& f:c.fighters) {
        if(!f.health || !f.target) continue;
        auto* target=fighter(c,*f.target);auto& own=*after.slots[f.actor.slot].entity;
        if(!target || !eligible(after,f,*target,c.acquisitionRange)) {clearAttack(f);stopMotion(own);continue;}
        auto& other=*after.slots[target->actor.slot].entity;
        if(!adjacent(own,other)) {
            if(f.action==CombatAction::windingUp) emit(CombatEventKind::missed,f.actor,target->actor);
            f.action=CombatAction::approaching;f.remaining=0;continue;
        }
        // Grid-adjacent combat stops any remaining route at the committed cell.
        stopMotion(own);
        if(f.action==CombatAction::windingUp) {
            if(--f.remaining) continue;
            const auto amount=std::min(f.stats.damage,target->health);target->health-=amount;
            emit(CombatEventKind::hit,f.actor,target->actor,amount);
            if(!target->health) {
                other.cleaned=true;other.target.reset();stopMotion(other);clearAttack(*target);
                emit(CombatEventKind::died,target->actor,f.actor);
                for(auto& peer:c.fighters) if(peer.target==std::optional<Handle>(target->actor)) {
                    clearAttack(peer);stopMotion(*after.slots[peer.actor.slot].entity);
                }
            } else {f.action=CombatAction::recovering;f.remaining=f.stats.recoveryTicks;}
        } else if(f.action==CombatAction::recovering) {
            if(--f.remaining) continue;
            f.action=CombatAction::windingUp;f.remaining=f.stats.windupTicks;emit(CombatEventKind::windup,f.actor,target->actor);
        } else {f.action=CombatAction::windingUp;f.remaining=f.stats.windupTicks;emit(CombatEventKind::windup,f.actor,target->actor);}
    }
    // Targets that left acquisition range during movement cannot survive in a
    // published checkpoint, even if the actor was processed before its target.
    for(auto& f:c.fighters) if(f.target) {
        auto* target=fighter(c,*f.target);
        if(!target || !eligible(after,f,*target,c.acquisitionRange)) {clearAttack(f);stopMotion(*after.slots[f.actor.slot].entity);}
    }
    World complete(0,candidate.world().limits());complete.restore(std::move(after));
    publish(MovementSession(std::move(complete),navigation_),std::move(c));return report;
}
void CombatSession::restore(const std::filesystem::path& path,const NavigationResolver& resolver) {
    require(bool(resolver),"combat restore requires a resource resolver");
    auto state=readSnapshot(path,world().limits());
    require(!state.systems.empty(),"checkpoint contains no combat subsystem");
    auto nav=resolver(state);require(bool(nav),"combat restore resource unavailable");
    World world(0,this->world().limits());world.restore(std::move(state));
    CombatSession candidate(std::move(world),std::move(nav));*this=std::move(candidate);
}
}
