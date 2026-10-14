#include "command_action_bodies.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
OwnedCommandActionBodies::OwnedCommandActionBodies(
    AutonomousWorld &w, std::vector<CreatureExecution> &e,
    std::vector<CreatureMaintenance> &m, std::vector<MovementBehaviorState> &v,
    CommandActionBodyState &state, OwnedCreatureCommands &commands,
    CommandActionBodyEnvironment environment, CommandActionBodyProviders providers)
    : world_(w), execution_(e), maintenance_(m), movement_(v), state_(state),
      commands_(commands), environment_(environment), providers_(std::move(providers)) {
  const auto n = w.creatures.size();
  if (!n || !commands.owns(w,e) || e.size()!=n || m.size()!=n || v.size()!=n ||
      state.callbacks.size()!=n || state.motion.size()!=n || state.segments.size()!=n ||
      environment.flight.dimensions.x!=w.dimensions.x ||
      environment.flight.dimensions.y!=w.dimensions.y ||
      environment.flight.dimensions.z!=w.dimensions.z ||
      !providers_.callbacks || !providers_.motion || !providers_.flight || !providers_.objects)
    throw std::invalid_argument("owned action body roots/providers differ");
}
void OwnedCommandActionBodies::tick(AutonomousCreature &a, unsigned handler) {
  if (a.ai.slot>=world_.creatures.size() || &a!=&world_.creatures[a.ai.slot])
    throw std::invalid_argument("owned action body actor is foreign");
  const auto slot=a.ai.slot;
  auto &e=execution_.at(slot);
  auto action=[this](auto &actor,unsigned n){commands_.action(actor,n);};
  auto behavior=[this](auto &actor,unsigned n){commands_.transition(actor,n);};
  auto run=[this](auto &actor){commands_.dispatch(actor);};
  auto reset=[this](auto &actor){commands_.reset(actor);};
  switch(handler) {
  case 1: case 3: case 10: case 14: case 18: case 20: {
    auto s=providers_.callbacks(slot);
    s.action=action;s.behavior=behavior;s.runBehavior=run;s.reset=reset;
    auto &state=state_.callbacks.at(slot);
    if(handler==1) tickIdleAction(a,e,world_,state,s);
    else if(handler==3) tickCorpseAction(a,s);
    else if(handler==14) tickStunAction(a,e,state,s);
    else if(handler==18) tickInteractionAction(a,environment_.callbacks,s);
    else tickMeleeAction(a,e,world_,maintenance_.at(slot),state,handler==20,
                         environment_.callbacks,s);
    break;
  }
  case 2: {
    auto s=providers_.motion(slot);
    s.action=action;s.behavior=behavior;s.runBehavior=run;s.reset=reset;
    s.bindBehavior=[this](auto &actor,unsigned n){commands_.bindOriginal(actor,n);};
    tickMotionAction(a,e,state_.motion.at(slot),movement_.at(slot),world_,s);
    break;
  }
  case 13: {
    auto s=providers_.objects(slot);s.behavior=run;s.reset=reset;
    tickObjectAction(a,state_.objects,s);break;
  }
  case 21: case 23: case 24: {
    auto s=providers_.flight(slot);s.behavior=behavior;s.reset=reset;
    if(handler==21) tickFinishingAction(a,e,s);
    else if(handler==23) tickAscendingAction(a,e,state_.motion.at(slot),movement_.at(slot),
                            state_.segments.at(slot),world_,environment_.flight,s);
    else tickDescendingAction(a,e,state_.segments.at(slot),movement_.at(slot),
                              environment_.flight,s);
    break;
  }
  default: throw std::invalid_argument("owned action body selector");
  }
}
} // namespace mnm::reconstruction::combat
