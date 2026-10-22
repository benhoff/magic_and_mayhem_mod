#pragma once
#include "kind8_random.hpp"
#include <map>
#include <stdexcept>

namespace mnm::reconstruction {
struct OriginalRandomSnapshot {
  std::uint32_t first{}, second{};
  std::array<std::uint32_t, 250> words{};
};
// 54e170 uses one global LCG workspace6f2a9c for ALL R250 receivers. R250
// transitions use only their receiver's1008bytes. A foreign seed changes the
// shared LCG workspace without reseeding the particle stream.
class OriginalRandomStreams {
  std::map<std::uint32_t, Kind8Random> streams;
  std::uint32_t globalLcg{};

public:
  static constexpr std::uint32_t particle = 0x6574c8;
  void seed(std::uint32_t token, std::uint32_t value) {
    auto initialized = Kind8Random(value);
    globalLcg = initialized.lcg;
    streams.insert_or_assign(token, std::move(initialized));
  }
  std::uint32_t next(std::uint32_t token) { return streams.at(token).next(); }
  OriginalRandomSnapshot snapshot(std::uint32_t token) const {
    const auto &stream = streams.at(token);
    return {stream.first, stream.second, stream.words};
  }
  std::uint32_t sharedLcg() const { return globalLcg; }
  template <class Clock>
  void seedParticleRoute(std::uint32_t callSite, Clock &&clock) {
    switch (callSite) {
    case 0x4bec0a:
      seed(particle, 0x499602d2);
      return;
    case 0x4e4484:
    case 0x55780e:
      seed(particle, std::uint32_t(clock()));
      return;
    default:
      throw std::invalid_argument("Unrecovered original particle reseed route");
    }
  }
};
} // namespace mnm::reconstruction
