#include "creature_rules_session.hpp"
#include <stdexcept>
namespace mnm::game {
namespace {
void require(bool condition, const char *reason) {
  if (!condition)
    throw std::invalid_argument(reason);
}
void validateTarget(const World &world, const CreatureIntent &intent) {
  require(world.find(intent.actor) != nullptr,
          "creature command actor is stale");
  if (intent.target)
    require(world.find(*intent.target) != nullptr,
            "creature command target is stale");
  require(unsigned(intent.kind) <= unsigned(CreatureOrderKind::clear),
          "unknown creature command");
  if (intent.kind == CreatureOrderKind::attack ||
      intent.kind == CreatureOrderKind::follow)
    require(intent.target.has_value(), "creature command requires a target");
}
} // namespace
CreatureRuleSession::CreatureRuleSession(
    std::shared_ptr<const CreatureRuleRuntime> rules, Bytes profile,
    std::uint32_t seed, std::uint32_t capacity)
    : world_(capacity), rules_(std::move(rules)) {
  require(bool(rules_), "creature rules are missing");
  auto state = world_.state();
  state.campaign = std::move(profile);
  rules_->initialize(state, seed);
  rules_->validate(state);
  world_.restore(std::move(state));
}
CreatureRuleSession::CreatureRuleSession(
    World world, std::shared_ptr<const CreatureRuleRuntime> rules)
    : world_(std::move(world)), rules_(std::move(rules)) {
  require(bool(rules_), "creature rules are missing");
  rules_->validate(world_.state());
}
std::vector<CreatureRuleEvent> CreatureRuleSession::step(TickInput input) {
  if (!input.admitted)
    return {};
  auto candidate = world_;
  candidate.step(input);
  auto state = candidate.state();
  auto events = rules_->step(state, input);
  rules_->validate(state);
  candidate.restore(std::move(state));
  world_ = std::move(candidate);
  return events;
}
void CreatureRuleSession::command(const CreatureIntent &intent) {
  validateTarget(world_, intent);
  auto state = world_.state();
  rules_->command(state, intent);
  rules_->validate(state);
  world_.restore(std::move(state));
}
} // namespace mnm::game
