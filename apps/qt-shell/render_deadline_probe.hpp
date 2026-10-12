#pragma once
#include "../../renderer/commands.hpp"
#include <array>
#include <algorithm>
#include <chrono>
#include <cstdint>
// Observation only. Opportunities are deduplicated per reason in global
// 16.667ms monotonic buckets; reasons may overlap and must not be summed.
struct RenderDeadlineProbe {
    enum Reason { UiPublication, IncompleteUi, PendingWorldImport, Hidden,
                  SourceCompleted, RenderAttempt, NoRecipe, FirstSeal, StaticUi, GenerationMismatch, ReasonCount };
    std::array<std::uint64_t,ReasonCount> turns{},opportunities{};
    std::array<std::int64_t,ReasonCount> lastBucket=[] {std::array<std::int64_t,ReasonCount> result;result.fill(-1);return result;}();
    double maxLateMs=0;
    void observe(std::chrono::steady_clock::time_point deadline,
                 std::chrono::steady_clock::time_point now,Reason reason){
        if(!deadline.time_since_epoch().count() || now<deadline)return;
        const auto us=std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        const auto bucket=us/16667;
        ++turns[reason];if(lastBucket[reason]!=bucket){lastBucket[reason]=bucket;++opportunities[reason];}
        maxLateMs=std::max(maxLateMs,std::chrono::duration<double,std::milli>(now-deadline).count());
    }
};
struct RenderStageTiming {
    std::uint64_t samples=0;double totalMs=0,maxMs=0;
    std::array<std::uint64_t,8> bins{};
    void add(double ms){
        static constexpr std::array<double,7> limits{{1,2,4,8,16.667,33.334,100}};
        ++samples;totalMs+=ms;maxMs=std::max(maxMs,ms);
        std::size_t bucket=0;while(bucket<limits.size() && ms>limits[bucket])++bucket;++bins[bucket];
    }
};

// New source consumption/import already draws a World: never redraw it twice
// in that owner turn. UI publications do not cancel a due display deadline.
inline bool nativeDisplayEligible(bool visible,bool pending,bool sourceCompleted,
                                  std::uint64_t sealedGeneration,std::uint64_t worldGeneration){
    return visible && !pending && !sourceCompleted && sealedGeneration && sealedGeneration==worldGeneration;
}

inline RenderDeadlineProbe::Reason nativeUiBlockReason(const mnm::render::CommandDisplayState& ui,std::uint64_t world){
    if(!ui.generation)return RenderDeadlineProbe::NoRecipe;
    if(ui.generation!=world)return RenderDeadlineProbe::GenerationMismatch;
    if(!ui.sealed)return RenderDeadlineProbe::FirstSeal;
    if(!ui.worldOutput)return RenderDeadlineProbe::StaticUi;
    return RenderDeadlineProbe::RenderAttempt;
}
