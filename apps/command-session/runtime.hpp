#pragma once
#include "../../game/simulation/creature_rules_session.hpp"
#include "../../reconstruction/creature-ai/transfer_commands_owned.hpp"
#include "../../reconstruction/creature-ai/command_behaviors_owned.hpp"
namespace mnm::command_session {
namespace model = reconstruction::combat;
struct Continuation {
  model::AutonomousWorld world;
  std::vector<model::CreatureExecution> execution;
  model::CommandBehaviorBindings bindings;
  std::vector<std::uint32_t> generations;
  model::CreatureScheduler scheduler;
  model::SchedulerEnvironment environment;
  std::uint32_t seed = 1;
  game::Bytes backend;
  // v2 continuation; empty for profiles retaining explicitly supplied bodies.
  std::vector<model::CreatureMaintenance> maintenance;
  std::vector<model::MovementBehaviorState> movement;
};
// Providers operate only on this operation's staged continuation. They must not
// publish external effects; publish returned events only after session success.
struct Hooks {
  std::function<model::ExecutionServices(unsigned)> execution, queue;
  std::function<model::BehaviorExecutionServices(unsigned)> idle;
  std::function<model::FollowServices(unsigned)> follow;
  std::function<void(model::AutonomousCreature &, unsigned)> body;
  std::function<void(model::AutonomousCreature &, unsigned,
                     model::OwnedCreatureCommands &)> commandBodies;
  std::function<void(model::AutonomousCreature &)> supplied;
  std::function<void(unsigned)> release;
  std::function<void(model::AutonomousCreature &, bool)> peer;
  std::function<std::vector<game::CreatureRuleEvent>(
      Continuation &, model::OwnedCreatureCommands &, game::TickInput)> advance;
};
using HookFactory = std::function<Hooks(Continuation &)>;
game::Bytes encode(const Continuation &);
Continuation decode(const game::Bytes &);
void validate(const Continuation &);
class Runtime final : public game::CreatureRuleRuntime {
  Continuation initial_;
  game::Bytes profile_;
  HookFactory hooks_;
  Continuation read(const game::State &) const;
  void publish(game::State &, const Continuation &) const;
public:
  Runtime(Continuation initial, game::Bytes profile, HookFactory);
  void initialize(game::State &, std::uint32_t seed) const override;
  void validate(const game::State &) const override;
  std::vector<game::CreatureRuleEvent> step(game::State &, game::TickInput) const override;
  void command(game::State &, const game::CreatureIntent &) const override;
};
} // namespace mnm::command_session
