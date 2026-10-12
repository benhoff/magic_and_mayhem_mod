#pragma once
#include <cstdint>
#include <optional>

namespace mnm::recovered::campaign {
struct CharacterCostCoefficients {
    std::int32_t increment = 0;
    float initial = 0, linear = 0, exponential = 0;
};
struct CharacterCosts { std::int32_t decrease = 0, increase = 0; };
// Selected positive, finite No-CD pricing domain. Invalid/unrepresentable input
// refuses as a native safety policy; original error/overflow behavior is separate.
std::optional<CharacterCosts> characterCosts(std::int32_t value, const CharacterCostCoefficients&);
}
