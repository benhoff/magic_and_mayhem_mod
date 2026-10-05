#include "tick_clock.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::game {
void TickClock::play(std::uint64_t now) {if(!playing_) {playing_=true;last_=now;remainder_=0;}}
void TickClock::pause() {playing_=false;remainder_=0;}
TickBatch TickClock::admit(std::uint64_t now) {
    if(!playing_) return {};
    if(now<last_) throw std::invalid_argument("Playback clock moved backwards");
    const auto elapsed=now-last_;
    const auto fraction=elapsed%intervalMs+remainder_;
    const auto due=elapsed/intervalMs+fraction/intervalMs;
    const auto ticks=std::min<std::uint64_t>(due,maxCatchUp);
    last_=now;remainder_=fraction%intervalMs;
    return {std::uint32_t(ticks),due-ticks};
}
}
