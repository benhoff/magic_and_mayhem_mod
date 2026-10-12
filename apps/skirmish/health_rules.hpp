#pragma once
#include "simulation/health_rules.hpp"
namespace mnm::skirmish {
constexpr std::uint64_t ordinaryHealthIdentity = 0x4e53323348454131ULL;
std::shared_ptr<const game::HealthRules> ordinaryHealthRules();
std::shared_ptr<const game::HealthRules> resolveHealthRules(std::uint64_t);
} // namespace mnm::skirmish
