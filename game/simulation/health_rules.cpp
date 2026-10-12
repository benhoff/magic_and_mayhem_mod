#include "health_rules.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::game {
namespace {
class FixtureRules final : public HealthRules {
  public:
    std::uint64_t identity() const override { return 0; }
    std::uint32_t change(std::uint32_t current, std::uint32_t maximum, std::int32_t delta,
                         bool defended) const override {
        if (defended && delta >= 0)
            throw std::invalid_argument("defended health change must be negative");
        if (!current)
            return 0;
        if (delta < 0) {
            const auto damage = 0U - static_cast<std::uint32_t>(delta);
            return current - std::min(current, damage);
        }
        return static_cast<std::uint32_t>(std::min(
            std::uint64_t(current) + static_cast<unsigned>(delta), std::uint64_t(maximum)));
    }
};
} // namespace
std::shared_ptr<const HealthRules> nativeFixtureHealthRules() {
    static const auto rules = std::make_shared<FixtureRules>();
    return rules;
}
} // namespace mnm::game
