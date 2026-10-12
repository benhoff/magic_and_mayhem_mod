#pragma once
#include <cstdint>
#include <memory>

namespace mnm::game {
// A stateless health backend. Its identity is part of the combat checkpoint;
// game services know no executable addresses or build-specific reconstruction.
class HealthRules {
  public:
    virtual ~HealthRules() = default;
    virtual std::uint64_t identity() const = 0;
    virtual std::uint32_t change(std::uint32_t current, std::uint32_t maximum, std::int32_t delta,
                                 bool defended) const = 0;
};
std::shared_ptr<const HealthRules> nativeFixtureHealthRules();
} // namespace mnm::game
