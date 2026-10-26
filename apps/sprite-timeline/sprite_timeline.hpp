#pragma once
#include "../sprite-session/sprite_session.hpp"

namespace mnm::sprites {
struct TimelineEntry {
    Name name{};
    std::uint32_t occurrence=0;
    std::uint64_t ticks=1;
    std::int32_t event=0;
};
// An explicit native clip, not an interpretation of original ANI opcodes.
// All entries resolve before publication. Names, durations and bindings are owned.
class TimelineClip final {
    std::shared_ptr<const SpriteTable> table_;
    std::vector<TimelineEntry> entries_;
    std::vector<std::size_t> frames_;
public:
    TimelineClip(std::shared_ptr<const SpriteTable>,std::vector<TimelineEntry>);
    const auto& table() const {return table_;}
    const auto& entries() const {return entries_;}
    std::size_t frame(std::size_t entry) const {return frames_.at(entry);}
};
enum class TimelineMode {loop,once,pingPong};
struct TimelineCheckpoint {
    std::shared_ptr<const TimelineClip> clip;
    TimelineMode mode=TimelineMode::loop;
    bool reverse=false,paused=false;
    std::uint64_t phase=0;
    std::uint32_t numerator=1,denominator=1,fraction=0;
};
struct TimelinePosition {
    std::size_t entry=0,scheduleEntry=0;
    std::uint64_t localTick=0;
    bool finished=false;
};
struct TimelineEvent {std::size_t entry=0;std::int32_t value=0;};
// Host-supplied integer time. No widget, clock, original object or callback.
// Events are opt-in, bounded and returned only after complete admission.
class SpriteTimeline final {
    TimelineCheckpoint state_;
    std::vector<std::size_t> order_;
    std::vector<std::uint64_t> starts_;
    std::uint64_t duration_=0;
    void assign(TimelineCheckpoint);
public:
    SpriteTimeline(std::shared_ptr<const TimelineClip>,TimelineMode,bool reverse=false);
    TimelinePosition position() const;
    std::size_t frame() const;
    std::uint64_t duration() const {return duration_;}
    TimelineCheckpoint checkpoint() const {return state_;}
    void restore(TimelineCheckpoint);
    void pause(bool value) {state_.paused=value;}
    // Positive rates1..1e6; changing a rate drops the old fractional remainder.
    void rate(std::uint32_t numerator,std::uint32_t denominator);
    // Virtual clip time, independent of rate/pause; no events, clears fraction.
    void seek(std::uint64_t ticks);
    void seekEntry(std::size_t scheduleEntry,std::uint64_t localTick);
    std::vector<TimelineEvent> advance(std::uint64_t hostTicks,bool events=false,std::size_t eventBudget=4096);
    // Retains virtual phase, rate, fraction and pause. Failed lookup is atomic.
    void reload(std::shared_ptr<const SpriteTable>);
    render::SpriteDrawPlan plan(std::int32_t x,std::int32_t y,
                               std::optional<render::SpriteColourTable> colours={}) const;
};
}
