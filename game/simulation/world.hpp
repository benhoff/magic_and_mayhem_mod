#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace mnm::game {
using Bytes = std::vector<std::uint8_t>;
struct Handle {
    std::uint32_t slot=0, generation=0;
    bool operator==(Handle other) const {return slot==other.slot && generation==other.generation;}
};
enum class Family : std::uint32_t {creature, missileEffect, mapLinked};
struct Point {
    std::int32_t x=0,y=0,z=0;
    bool operator==(Point other) const {return x==other.x && y==other.y && z==other.z;}
    bool operator!=(Point other) const {return !(*this==other);}
};
struct NavigationBinding {
    Point dimensions;
    std::uint64_t fingerprint=0;
    bool operator==(const NavigationBinding& other) const {return dimensions==other.dimensions && fingerprint==other.fingerprint;}
};
enum class Action : std::uint32_t {idle, planning, moving, arrived, blocked, searchLimited, cancelled};
struct RoutePoint {
    Point position;
    std::int32_t direction=0,verticalDelta=0,category=0;
    std::uint32_t scalar=0;
};
// Continuation state for the bounded animation-sample driver. Coordinates are
// 32 fine units per XY cell and 16 per Z layer; no legacy object layout.
struct FineMotion {
    std::int32_t rate=0,duration=0,heightOrigin=0,heightDelta=0;
    std::int32_t accumulator=0,progress=0,travelX=0,travelY=0;
    Point fine;
    std::int32_t residualX=0,residualY=0;
    std::uint32_t frame=0;
};
struct CreatureMotion {
    Action action=Action::idle;
    Point origin,destination;
    std::optional<Handle> goal;
    std::uint32_t budget=300,next=0;
    std::vector<RoutePoint> route;
    bool sampleMotion=false;
    std::optional<FineMotion> fine={};
};
struct Entity {
    Family family=Family::creature;
    std::uint32_t type=0, owner=0;
    std::int32_t x=0,y=0,z=0;
    bool cleaned=false;
    std::optional<Handle> target;
    Bytes state; // Owned subsystem data; no host pointers or guessed legacy layout.
    std::optional<CreatureMotion> motion={};
};
struct Slot {std::uint32_t generation=1; std::optional<Entity> entity;};
enum class Operation : std::uint32_t {target, cleanup, release, move, motion};
struct MotionUpdate {Point position; CreatureMotion motion;};
struct Command {
    Operation operation=Operation::cleanup;
    Handle subject;
    std::optional<Handle> target;
    std::optional<Point> destination={};
    std::optional<MotionUpdate> update={};
};
struct State {
    std::uint32_t sequence=0, tick=0, phase20=0, phase90=0, expansionBudget=0;
    std::string map;
    Bytes campaign, systems; // Versioned by their future consumers, preserved exactly.
    std::vector<Slot> slots;
    std::vector<Command> pending;
    std::optional<NavigationBinding> navigation={};
};
struct Limits {
    static constexpr std::uint64_t slotCharge=256,commandCharge=256;
    std::uint32_t slots=65536, commands=65536;
    std::uint64_t bytes=64*1024*1024;
};
enum class Phase {maintenance, decisions, secondaryCreatures, effects, map, queuedMap, audio};
struct TickInput {
    bool admitted=true, alternateMode=false, suppressSearch=false;
};
struct TickReport {bool advanced=false; std::uint32_t applied=0,rejected=0; std::vector<Phase> phases;};
// A system reads staged state and emits explicit operations. External side effects
// cannot be rolled back; keep this callback deterministic and state in State.
using System = std::function<void(Phase,const State&,std::vector<Command>&)>;
class World {
    State state_;
    Limits limits_;
    bool stepping_=false;
public:
    explicit World(std::uint32_t capacity=128, Limits limits={});
    const State& state() const {return state_;}
    const Limits& limits() const {return limits_;}
    const Entity* find(Handle) const;
    Handle spawn(Entity);
    void enqueue(Command);
    // Invalid queued identities are rejected and counted at the tick boundary.
    TickReport step(TickInput={},const System& = {});
    void restore(State); // Validate everything before committing; failures preserve state.
    static void validate(const State&,const Limits&);
};
}
