#pragma once
#include "world.hpp"
#include <filesystem>
#include <memory>

namespace mnm::game {
bool contains(Point,const NavigationBinding&);
bool active(Action);
void validateMotion(const Entity&,const NavigationBinding&);
void validateCommand(const Command&);
enum class PlanStatus {reachable, unreachable, budgetExhausted};
struct RoutePlan {
    PlanStatus status=PlanStatus::unreachable;
    std::vector<RoutePoint> route;
    std::uint32_t expansions=0; // Actual planner work, used by native shared-budget policy.
};
// Native simulation consumes owned coordinates/results, never reconstruction
// objects, raw game pointers or build-specific addresses.
class Navigation {
public:
    virtual ~Navigation()=default;
    virtual NavigationBinding binding() const=0;
    virtual std::optional<AnimationBinding> animationBinding() const {return {}; }
    virtual std::uint32_t creatureType() const=0;
    virtual RoutePlan plan(const Entity&,Point,std::uint32_t budget) const=0;
    virtual bool accepts(const Entity&,const RoutePoint&) const=0;
    virtual bool supportsStationaryOccupants() const {return false;}
    virtual std::uint32_t maxMovingCreatures() const {return 1;}
    virtual Point creatureFootprint() const {return {1,1,1};}
    virtual void validateOccupants(const State&) const {}
    virtual RoutePlan planInWorld(const State&,Handle,Point,std::uint32_t budget) const;
    virtual bool acceptsInWorld(const State&,Handle,const RoutePoint&) const;
    virtual Point finePosition(const Entity&) const;
    // Optional driver; services without recovered sample evidence refuse it.
    virtual void validateSegmentHistory(const Entity&,const SegmentHistory&) const;
    virtual FineMotion prepareFineMotion(const Entity&,const RoutePoint&) const;
    virtual bool advanceFineMotion(const Entity&,const RoutePoint&,FineMotion&) const;
    virtual void validateFineMotion(const Entity&,const RoutePoint&,const FineMotion&) const;
};
using NavigationResolver=std::function<std::shared_ptr<const Navigation>(const State&)>;
using MapResolver=std::function<std::shared_ptr<const Navigation>(const std::string&)>;
class MovementSession {
    World world_;
    std::shared_ptr<const Navigation> navigation_;
    static void validateBinding(const State&,const Navigation&);
public:
    MovementSession(World,std::shared_ptr<const Navigation>);
    const World& world() const {return world_;}
    Handle spawn(Entity,bool sampleMotion=false,bool continuousMotion=false,bool terrainMotion=false);
    Handle spawnBlocker(Entity); // Stationary same-profile creature; owns no motion driver.
    void enqueue(Command);
    void move(Handle,Point,std::optional<Handle> goal={});
    void stop(Handle h) {enqueue({Operation::stop,h,{}});}
    std::uint32_t cancelQueuedMoves(Handle h) {return world_.cancelQueuedMoves(h);}
    TickReport step(TickInput={});
    Point finePosition(const Entity& e) const {return e.motion && e.motion->fine?e.motion->fine->fine:navigation_->finePosition(e);}
    // Load map + validate routes before committing either resource or world.
    void restore(const std::filesystem::path&,const MapResolver&);
    void restoreResources(const std::filesystem::path&,const NavigationResolver&);
};
}
