#pragma once
#include "voice_contract.hpp"
namespace mnm::reconstruction::audio {
// Host annotations for the 40-byte wrapper; no process pointers are dereferenced.
struct VoiceWrapper {
    std::uint32_t identity=0,buffer=0;
    std::int32_t sourceIndex=-1;
    std::uint32_t field08=0,field0c=2;
    std::int32_t requestedVolume=99,cachedVolume=99;
    VoiceWrapper* duplicate=nullptr;
    std::uint32_t duration=0; // +0x24; +0x18/+0x1c links excluded from this model.
};
struct LifetimeBackend:VoiceBackend {
    virtual void clearVoiceSchedule(std::uint32_t identity)=0;
    virtual void releaseBuffer(std::uint32_t buffer)=0;
    virtual void freeWrapper(std::uint32_t identity)=0;
};
// Reset block 0x572150. Does not release a buffer or alter external list links.
void resetWrapper(VoiceWrapper&);
// 0x56df00: descendants destroyed first; only bit zero controls own heap free.
// Acyclic, uniquely owned duplicate chains required. Host objects remain readable
// for fixture inspection; freeWrapper marks logical disposal, not C++ delete.
void destroyWrapper(LifetimeBackend&,VoiceWrapper&,std::uint32_t flags);
// 0x5700e0: release/reset contents without freeing the source wrapper itself.
void releaseSourceContents(LifetimeBackend&,VoiceWrapper&);
Status retireVoice(LifetimeBackend&,VoiceWrapper&,bool includeDuplicates,
                   bool clearScheduler,bool enabled);
}
namespace mnm::reconstruction::audio {
// Host-linked view of Schedule32. Valid nonempty circular ring required.
struct ScheduleNode {
    Schedule32 record;
    ScheduleNode* next=nullptr;ScheduleNode* previous=nullptr;
    std::uint32_t* output=nullptr;
};
// 0x571d20 moves a middle record to the tail, advances head when retiring head,
// and leaves tail in place. Ring membership is retained, never deallocated.
void retireSchedule(ScheduleNode*& head,ScheduleNode& node);
}
