#pragma once
#include "dsound_setup.hpp"
#include <cstddef>
#include <cstdint>

namespace mnm::reconstruction::audio {
// Host annotations for selected engine blocks, NOT native playback state or COM pointers.
struct VoiceContract {
    std::uint32_t buffer=0;
    std::int32_t requestedVolume=99, cachedVolume=99;
    VoiceContract* duplicate=nullptr; // Original wrapper +0x20; require an acyclic chain.
};
struct Schedule32 {
    std::uint32_t next=0,previous=0,deadline=0;
    std::int32_t volume=-5000;
    std::uint32_t voiceAddress=0,outputAddress=0;
    std::int32_t x=-1,y=-1;
};
static_assert(sizeof(Schedule32)==32);
static_assert(offsetof(Schedule32,voiceAddress)==0x10);
static_assert(offsetof(Schedule32,x)==0x18);
// Clearing returns the external slot address to zero; never dereferences a game pointer.
std::uint32_t clearSchedule(Schedule32&);
struct VoiceBackend {
    virtual ~VoiceBackend()=default;
    virtual Status getStatus(std::uint32_t buffer,std::uint32_t& flags)=0;
    virtual Status stop(std::uint32_t buffer)=0;
    virtual Status position(std::uint32_t buffer,std::uint32_t byte)=0;
    virtual Status volume(std::uint32_t buffer,std::int32_t value)=0;
    virtual Status pan(std::uint32_t buffer,std::int32_t value)=0;
    virtual Status play(std::uint32_t buffer,std::uint32_t reserved1,
                        std::uint32_t reserved2,std::uint32_t flags)=0;
    // Engine scheduler notifications; these do not generate samples.
    virtual void volumeRecord(VoiceContract&,std::int32_t value)=0;
};
// 0x49c530: errors count as busy; absent source buffer short-circuits duplicates.
bool voiceBusy(VoiceBackend&,const VoiceContract&,bool includeDuplicates);
// Shared status/Stop/reset block in 0x571c30/0x571ec0/0x56fd90.
// This narrow block does not include linked-list retirement or record clearing.
Status stopAndReset(VoiceBackend&,std::uint32_t buffer,bool enabled);
// 0x4ff640: cache writes precede the call; identical cache suppresses propagation.
Status setVoiceVolume(VoiceBackend&,VoiceContract&,std::int32_t value,
                     bool includeDuplicates,bool notifyScheduler,bool enabled);
// 0x5722b0: no pan cache; duplicate failures short-circuit subsequent calls.
Status setVoicePan(VoiceBackend&,VoiceContract&,std::int32_t value,
                  bool includeDuplicates,bool enabled);
// Common start block 0x56f24d/0x5721fb, after selection/duplication succeeds.
Status startVoice(VoiceBackend&,VoiceContract&,std::int32_t volume,std::int32_t pan,
                  bool looping,bool enabled);
// 0x571be0 and unsigned deadline comparisons in 0x571c30/0x571ab0.
std::uint32_t retirementDeadline(std::uint32_t now,std::uint32_t duration,bool looping);
bool retirementDue(std::uint32_t now,std::uint32_t deadline);
// 0x58ffb0: x86 wraps the dataBytes*1000 product before unsigned division.
std::uint32_t wavDuration(std::uint32_t dataBytes,std::uint32_t bytesPerSecond);
// Final integer pan mapping 0x57189f..0x571905, for a positive width.
// Inputs restricted to products representable in signed x86 arithmetic.
std::int32_t positionalPan(std::int32_t lateral,std::int32_t width);
}
