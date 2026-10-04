#pragma once
#include "voice_lifetime.hpp"
namespace mnm::reconstruction::audio {
struct SchedulerBackend:VoiceBackend {
    virtual std::uint32_t tickCount()=0;
    virtual std::uint32_t bufferForVoice(std::uint32_t identity)=0;
};
// Valid nonempty circular host ring; callbacks must not mutate its topology.
// 0x571c30 returns the former tail, which can differ from the requested node.
ScheduleNode& evictSchedule(SchedulerBackend&,ScheduleNode*& head,ScheduleNode& requested,bool enabled);
// 0x571ab0: free, expired, then first strictly lower-volume candidate.
ScheduleNode* selectSchedule(SchedulerBackend&,ScheduleNode*& head,std::int32_t volume,bool enabled);
// 0x571be0: stores slot address; it does not write the caller's slot itself.
void assignSchedule(SchedulerBackend&,ScheduleNode&,const VoiceWrapper&,std::uint32_t outputAddress,
                    std::uint32_t* output,bool looping,std::int32_t x,std::int32_t y);
// 0x571d80: scans using OLD volume, then writes NEW volume. Not a generic sort.
void updateScheduleVolume(ScheduleNode*& head,ScheduleNode&,std::int32_t volume);
// 0x571ec0: clear whole ring, stop only nonexpired/looping voices; no reordering.
void clearSchedules(SchedulerBackend&,ScheduleNode* head,bool enabled);
}
