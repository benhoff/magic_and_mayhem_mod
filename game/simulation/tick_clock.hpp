#pragma once
#include <cstdint>
namespace mnm::game {
struct TickBatch {std::uint32_t ticks=0;std::uint64_t dropped=0;};
// Native wall-clock admission only. No Qt, simulation ownership or checkpoint fields.
class TickClock {
    bool playing_=false;
    std::uint64_t last_=0,remainder_=0;
public:
    static constexpr std::uint64_t intervalMs=100;
    static constexpr std::uint32_t maxCatchUp=4;
    bool playing() const {return playing_;}
    void play(std::uint64_t now);
    void pause();
    TickBatch admit(std::uint64_t now);
};
}
