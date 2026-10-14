#include "character_cost.hpp"
#include <cmath>
#include <limits>

namespace mnm::recovered::campaign {
std::optional<CharacterCosts> characterCosts(std::int32_t value, const CharacterCostCoefficients& c) {
    if (value < 0 || value > 1000000 || c.increment <= 0 || c.increment > 1000000 ||
        !std::isfinite(c.initial) || !std::isfinite(c.linear) || !std::isfinite(c.exponential) ||
        c.initial < 0 || c.linear < 0 || c.exponential < 1) return std::nullopt;
    const auto index = value / c.increment;
    auto cost = [&c](std::int32_t n) -> std::optional<std::int32_t> {
        const double total = double(c.initial) + std::pow(double(c.linear) * n, double(c.exponential));
        if (!std::isfinite(total) || total < 0 || total > std::numeric_limits<std::int32_t>::max())
            return std::nullopt;
        return static_cast<std::int32_t>(total); // Original positive-domain truncation.
    };
    const auto decrease = cost(index), increase = cost(index + 1);
    if (!decrease || !increase) return std::nullopt;
    return CharacterCosts{*decrease, *increase};
}
}
