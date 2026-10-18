#include "transfer_commands_owned.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
OwnedCreatureCommands::OwnedCreatureCommands(
    AutonomousWorld &world, std::vector<CreatureExecution> &execution,
    CommandBehaviorBindings &bindings,
    std::function<ExecutionServices(unsigned)> executionServices,
    std::function<BehaviorExecutionServices(unsigned)> idleServices,
    std::function<FollowServices(unsigned)> followServices,
    std::function<ExecutionServices(unsigned)> queueServices,
    std::function<void(AutonomousCreature &, unsigned)> body,
    std::function<void(AutonomousCreature &)> supplied,
    std::function<void(unsigned)> release,
    std::function<void(AutonomousCreature &, bool)> peer)
    : world_(world), execution_(execution), bindings_(bindings),
      executionServices_(std::move(executionServices)),
      idleServices_(std::move(idleServices)),
      followServices_(std::move(followServices)),
      queueServices_(std::move(queueServices)), body_(std::move(body)),
      supplied_(std::move(supplied)), release_(std::move(release)),
      peer_(std::move(peer)), queueActive_(world.creatures.size(), 0) {}
ExecutionServices OwnedCreatureCommands::services(unsigned slot) {
  auto result = executionServices_(slot);
  result.releaseEffect = release_;
  result.peerEngagement = [&](auto &a, unsigned flag) { peer_(a, flag != 0); };
  result.behavior = [&](auto &a, unsigned kind) { bind(a, kind); };
  if (ownsActions()) result.action = [this](auto &a, unsigned kind) { action(a, kind); };
  result.performOrder = [this, slot](const CreatureOrder &o) {
    deliver(world_.creatures.at(slot), o);
  };
  return result;
}
void OwnedCreatureCommands::bind(AutonomousCreature &a, unsigned kind) {
  if (kind < 1 || kind > (originalBodies_ ? 16u : 4u))
    throw std::invalid_argument("command behavior selector");
  bindings_.at(a.ai.slot) = static_cast<CommandBehaviorBinding>(kind);
  if (kind == 1)
    idle(a);
  else if (ownedBody_)
    ownedBody_(a, kind, *this);
  else
    body_(a, kind);
}
void OwnedCreatureCommands::dispatch(AutonomousCreature &a) {
  switch (bindings_.at(a.ai.slot)) {
  case CommandBehaviorBinding::supplied:
    supplied_(a);
    break;
  case CommandBehaviorBinding::idle:
    idle(a);
    break;
  case CommandBehaviorBinding::move:
  case CommandBehaviorBinding::target:
  case CommandBehaviorBinding::coordinate:
  case CommandBehaviorBinding::death:
  case CommandBehaviorBinding::state6:
  case CommandBehaviorBinding::state7:
  case CommandBehaviorBinding::state8:
  case CommandBehaviorBinding::immediate:
  case CommandBehaviorBinding::standingRanged:
  case CommandBehaviorBinding::autonomous:
  case CommandBehaviorBinding::displacement:
  case CommandBehaviorBinding::state14:
  case CommandBehaviorBinding::state15:
  case CommandBehaviorBinding::state16:
    if (unsigned(bindings_.at(a.ai.slot)) > 4 && !originalBodies_)
      throw std::invalid_argument("original behavior provider is missing");
    if (ownedBody_)
      ownedBody_(a, unsigned(bindings_.at(a.ai.slot)), *this);
    else
      body_(a, unsigned(bindings_.at(a.ai.slot)));
    break;
  case CommandBehaviorBinding::none:
    if (!originalBodies_) throw std::invalid_argument("original null behavior is not enabled");
    break;
  default:
    throw std::invalid_argument("command behavior binding");
  }
}
void OwnedCreatureCommands::installBodies(
    std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)> body,
    bool original) {
  if (!body || ownedBody_)
    throw std::invalid_argument("command bodies must be installed once");
  for (auto active : queueActive_)
    if (active) throw std::invalid_argument("cannot install bodies during queue execution");
  ownedBody_ = std::move(body);
  originalBodies_ = original;
}
void OwnedCreatureCommands::transition(AutonomousCreature &a, unsigned kind) {
  if (kind < 1 || kind > (originalBodies_ ? 16u : 4u))
    throw std::invalid_argument("owned command transition selector");
  auto s = services(a.ai.slot);
  if (kind == 12) {
    // The original installs a null descriptor but does not invoke it. A
    // deferred or health-rejected request must retain its prior descriptor.
    auto &e = execution_.at(a.ai.slot);
    const bool admitted = a.ai.health && !a.ai.motion;
    changeBehavior(a, e, kind, s);
    if (admitted) bindings_.at(a.ai.slot) = CommandBehaviorBinding::none;
  } else changeBehavior(a, execution_.at(a.ai.slot), kind, s);
}
void OwnedCreatureCommands::bindOriginal(AutonomousCreature &a, unsigned kind) {
  if (!originalBodies_ || kind < 1 || kind > 16 ||
      a.ai.slot >= world_.creatures.size() || &a != &world_.creatures[a.ai.slot])
    throw std::invalid_argument("original command binding context differs");
  bindings_.at(a.ai.slot) = kind == 12 ? CommandBehaviorBinding::none :
                                      static_cast<CommandBehaviorBinding>(kind);
}
bool OwnedCreatureCommands::owns(const AutonomousWorld &world,
                                 const std::vector<CreatureExecution> &execution) const {
  return &world == &world_ && &execution == &execution_;
}
void OwnedCreatureCommands::installActions(
    std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)> transition,
    std::function<void(AutonomousCreature &, OwnedCreatureCommands &)> tick) {
  if (!transition || !tick || actionTransition_ || actionTick_)
    throw std::invalid_argument("command actions must be installed once");
  for (auto active : queueActive_)
    if (active) throw std::invalid_argument("cannot install actions during queue execution");
  actionTransition_ = std::move(transition);
  actionTick_ = std::move(tick);
}
bool OwnedCreatureCommands::ownsActions() const { return bool(actionTransition_); }
void OwnedCreatureCommands::action(AutonomousCreature &a, unsigned kind) {
  if (a.ai.slot >= world_.creatures.size() || &a != &world_.creatures[a.ai.slot])
    throw std::invalid_argument("owned command action actor is foreign");
  if (actionTransition_) actionTransition_(a, kind, *this);
  else executionServices_(a.ai.slot).action(a, kind);
}
void OwnedCreatureCommands::tickAction(AutonomousCreature &a) {
  if (a.ai.slot >= world_.creatures.size() || &a != &world_.creatures[a.ai.slot] || !actionTick_)
    throw std::invalid_argument("owned command action tick context differs");
  actionTick_(a, *this);
}
void OwnedCreatureCommands::reset(AutonomousCreature &a) {
  resetBehavior(a, execution_.at(a.ai.slot), world_, services(a.ai.slot));
}
void OwnedCreatureCommands::idle(AutonomousCreature &a) {
  auto s = idleServices_(a.ai.slot);
  if (ownsActions()) s.action = [this](auto &actor, unsigned kind) { action(actor, kind); };
  s.follow = [&](auto &actor) {
    auto follow = followServices_(actor.ai.slot);
    follow.move = [&](auto &mover, Cell p) { move(mover, p); };
    return followReference(actor, execution_.at(actor.ai.slot), world_, follow);
  };
  s.queue = [&](auto &actor) { return queue(actor); };
  idleBehavior(a, s);
}
bool OwnedCreatureCommands::queue(AutonomousCreature &a) {
  auto &active = queueActive_.at(a.ai.slot);
  if (active)
    throw std::invalid_argument("reentrant command queue");
  struct Guard {
    unsigned char &active;
    explicit Guard(unsigned char &value) : active(value) { active = 1; }
    ~Guard() { active = 0; }
  } guard(active);
  auto s = queueServices_(a.ai.slot);
  s.releaseEffect = release_;
  s.performOrder = [&](const CreatureOrder &o) {
    auto &e = execution_.at(a.ai.slot);
    std::vector<unsigned> serials;
    for (const auto &node : e.orders)
      serials.push_back(node.serial);
    const auto cursor = e.orderCursor;
    const auto count = e.queuedCount;
    deliver(a, o);
    std::vector<unsigned> after;
    for (const auto &node : e.orders)
      after.push_back(node.serial);
    if (after != serials || e.orderCursor != cursor || e.queuedCount != count)
      throw std::invalid_argument("command changed queue topology");
  };
  auto &e = execution_.at(a.ai.slot);
  const bool result = advanceOrders(a, e, s);
  if (!e.orders.empty() && !e.orderCursor && !e.queuedFlag)
    throw std::invalid_argument("unsafe noncycling queue continuation");
  return result;
}
void OwnedCreatureCommands::deliver(AutonomousCreature &a,
                                    const CreatureOrder &o) {
  deliverOrder(a, o, world_, [&](Cell p) { move(a, p); },
               [&](auto &b) { target(a, std::int32_t(b.ai.slot)); },
               [&](Cell p) { coordinate(a, p); });
}
void OwnedCreatureCommands::move(AutonomousCreature &a, Cell p) {
  orderMove(a, execution_.at(a.ai.slot), world_, p, services(a.ai.slot));
}
bool OwnedCreatureCommands::target(AutonomousCreature &a, std::int32_t target) {
  return orderTarget(a, execution_.at(a.ai.slot), world_, target,
                     services(a.ai.slot));
}
void OwnedCreatureCommands::coordinate(AutonomousCreature &a, Cell p) {
  orderCoordinateAttack(a, execution_.at(a.ai.slot), world_, p,
                        services(a.ai.slot));
}
void OwnedCreatureCommands::append(AutonomousCreature &a, CreatureOrder o) {
  appendOrder(a, execution_.at(a.ai.slot), world_, std::move(o),
              services(a.ai.slot));
}
void OwnedCreatureCommands::clear(AutonomousCreature &a) {
  clearOrders(execution_.at(a.ai.slot), services(a.ai.slot));
}
void OwnedCreatureCommands::follow(AutonomousCreature &a, std::int32_t target) {
  setFollow(a, execution_.at(a.ai.slot), world_, target, services(a.ai.slot));
}
void transferCreatureOwnerCommandsOwned(
    unsigned actor, AutonomousWorld &world,
    std::vector<CreatureExecution> &execution, CommandBehaviorBindings &bindings,
    CreatureTransferState &state, CreatureTransferWorld &selection,
    const OwnerVisionSharing &sharing, CreatureOwnerVisibility &visibility,
    SourceGraph &graph, const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits, const OwnerCountServices &countUI,
    OwnerTargetRepairState &repair,
    const OwnerTargetRepairServices &repairServices,
    const std::function<ExecutionServices(unsigned)> &perActor,
    const std::function<BehaviorExecutionServices(unsigned)> &perIdle,
    const std::function<FollowServices(unsigned)> &perFollow,
    const std::function<ExecutionServices(unsigned)> &perQueue,
    const std::function<void(AutonomousCreature &, unsigned)> &body, int owner,
    const CreatureTransferServices &lower) {
  OwnedCreatureCommands commands(world, execution, bindings, perActor, perIdle,
                                perFollow, perQueue, body,
                                repairServices.behavior, lower.releaseEffect,
                                repairServices.peerEngagement);
  auto consequences = lower;
  consequences.reset = [&](auto &a) { commands.reset(a); };
  auto targetRepair = repairServices;
  targetRepair.behavior = [&](auto &a) { commands.dispatch(a); };
  transferCreatureOwnerTargetsOwned(
      actor, world, execution.at(actor), state, selection, sharing, visibility,
      graph, owners, limits, countUI, repair, targetRepair, owner, consequences);
}
} // namespace mnm::reconstruction::combat
