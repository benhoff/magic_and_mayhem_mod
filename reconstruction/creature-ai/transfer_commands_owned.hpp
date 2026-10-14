#pragma once
#include "transfer_delivery_owned.hpp"
namespace mnm::reconstruction::combat {
enum class CommandBehaviorBinding {
  supplied, idle, move, target, coordinate, death, state6, state7, state8,
  immediate, standingRanged, autonomous, none, displacement, state14, state15, state16
};
using CommandBehaviorBindings = std::vector<CommandBehaviorBinding>;
// Canonical actor/execution storage and callback identity are borrowed for one
// staged operation. Lower bodies preserve backing/topology; callers publish only
// after success. A behavior-word edit does not change the installed binding.
class OwnedCreatureCommands {
  AutonomousWorld &world_;
  std::vector<CreatureExecution> &execution_;
  CommandBehaviorBindings &bindings_;
  std::function<ExecutionServices(unsigned)> executionServices_;
  std::function<BehaviorExecutionServices(unsigned)> idleServices_;
  std::function<FollowServices(unsigned)> followServices_;
  std::function<ExecutionServices(unsigned)> queueServices_;
  std::function<void(AutonomousCreature &, unsigned)> body_;
  std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)> ownedBody_;
  std::function<void(AutonomousCreature &)> supplied_;
  std::function<void(unsigned)> release_;
  std::function<void(AutonomousCreature &, bool)> peer_;
  std::vector<unsigned char> queueActive_;
  bool originalBodies_ = false;
  std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)> actionTransition_;
  std::function<void(AutonomousCreature &, OwnedCreatureCommands &)> actionTick_;
  ExecutionServices services(unsigned);
  void bind(AutonomousCreature &, unsigned);

public:
  OwnedCreatureCommands(
      AutonomousWorld &, std::vector<CreatureExecution> &,
      CommandBehaviorBindings &,
      std::function<ExecutionServices(unsigned)>,
      std::function<BehaviorExecutionServices(unsigned)>,
      std::function<FollowServices(unsigned)>,
      std::function<ExecutionServices(unsigned)>,
      std::function<void(AutonomousCreature &, unsigned)> body,
      std::function<void(AutonomousCreature &)> supplied,
      std::function<void(unsigned)> release,
      std::function<void(AutonomousCreature &, bool)> peer);
  void reset(AutonomousCreature &);
  // Install once before execution; the callback receives this operation's
  // context, preserving its queue recursion/topology admission.
  void installBodies(std::function<void(AutonomousCreature &, unsigned,
                                       OwnedCreatureCommands &)>, bool original = false);
  void transition(AutonomousCreature &, unsigned);
  void bindOriginal(AutonomousCreature &, unsigned);
  void installActions(
      std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)>,
      std::function<void(AutonomousCreature &, OwnedCreatureCommands &)>);
  bool ownsActions() const;
  void action(AutonomousCreature &, unsigned);
  void tickAction(AutonomousCreature &);
  bool owns(const AutonomousWorld &, const std::vector<CreatureExecution> &) const;
  void idle(AutonomousCreature &);
  void dispatch(AutonomousCreature &);
  bool queue(AutonomousCreature &);
  void deliver(AutonomousCreature &, const CreatureOrder &);
  void move(AutonomousCreature &, Cell);
  bool target(AutonomousCreature &, std::int32_t);
  void coordinate(AutonomousCreature &, Cell);
  void append(AutonomousCreature &, CreatureOrder);
  void clear(AutonomousCreature &);
  void follow(AutonomousCreature &, std::int32_t);
};
void transferCreatureOwnerCommandsOwned(
    unsigned actor, AutonomousWorld &, std::vector<CreatureExecution> &,
    CommandBehaviorBindings &, CreatureTransferState &, CreatureTransferWorld &,
    const OwnerVisionSharing &, CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &,
    const std::function<ExecutionServices(unsigned)> &,
    const std::function<BehaviorExecutionServices(unsigned)> &,
    const std::function<FollowServices(unsigned)> &,
    const std::function<ExecutionServices(unsigned)> &,
    const std::function<void(AutonomousCreature &, unsigned)> &body, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
