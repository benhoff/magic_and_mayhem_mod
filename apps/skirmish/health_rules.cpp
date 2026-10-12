#include "health_rules.hpp"
#include "health.hpp"
#include <stdexcept>
namespace mnm::skirmish {
namespace {
class OrdinaryRules final : public game::HealthRules {
  public:
    std::uint64_t identity() const override { return ordinaryHealthIdentity; }
    std::uint32_t change(std::uint32_t current, std::uint32_t maximum, std::int32_t delta,
                         bool defended) const override {
        if (!maximum || maximum > 1000000 || current > maximum || delta < -1000000 ||
            delta > 1000000 || (defended && delta >= 0))
            throw std::invalid_argument("outside native ordinary-health adapter domain");
        namespace h = mnm::reconstruction::combat;
        h::State state;
        state.health = current;
        h::Inputs p;
        p.baseMaximum = maximum;
        p.notify = 0;
        return h::apply(state, p, defended ? h::Entry::defendedDamage : h::Entry::signedHealth,
                        defended ? -delta : delta)
            .state.health;
    }
};
} // namespace
std::shared_ptr<const game::HealthRules> ordinaryHealthRules() {
    static const auto rules = std::make_shared<OrdinaryRules>();
    return rules;
}
std::shared_ptr<const game::HealthRules> resolveHealthRules(std::uint64_t id) {
    if (!id)
        return game::nativeFixtureHealthRules();
    if (id == ordinaryHealthIdentity)
        return ordinaryHealthRules();
    throw std::invalid_argument("unsupported checkpoint health rules");
}
} // namespace mnm::skirmish
