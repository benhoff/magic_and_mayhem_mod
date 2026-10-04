#pragma once
#include "manager_contract.hpp"
#include "voice_scheduler.hpp"
#include <vector>
namespace mnm::reconstruction::audio {
// Decoded sorted ID tables; no process addresses or file-loading policy.
struct SoundGroup {std::int32_t id;std::vector<std::int32_t> members;};
struct AdmissionCatalog {
    std::vector<SoundGroup> groups;
    std::vector<std::int32_t> sourceIds;
};
struct AdmissionBackend:SchedulerBackend {
    virtual std::uint32_t randomWord()=0; // 0x54e200; RNG itself not reconstructed.
    virtual Status loadSource(std::int32_t sound,VoiceWrapper*& output)=0; // 0x56f400
    virtual Status duplicateBuffer(std::uint32_t source,std::uint32_t& output)=0;
    // Return a fresh default-initialized wrapper with unique nonzero identity.
    // Own it until lifetime teardown. Allocation failure is outside recovered valid domain.
    virtual VoiceWrapper& allocateWrapper()=0;
};
std::int32_t admissionSoundId(AdmissionBackend&,const AdmissionCatalog&,std::int32_t requested);
// 0x572180 under active manager gates: successful buffer duplication appends/publishes
// even on control failure.
Status duplicateAndStart(AdmissionBackend&,VoiceWrapper&,std::int32_t volume,std::int32_t pan,
                         bool looping,VoiceWrapper*& output);
struct AdmissionRequest {
    std::int32_t sound=0,volume=0,pan=0,x=-1,y=-1;
    bool looping=false;
    std::uint32_t outputAddress=0;std::uint32_t* output=nullptr;
};
// Selected 0x56f000 manager start path. Nonempty valid source and schedule rings;
// callbacks must preserve topology (loader may supply an existing ring member).
Status admitVoice(AdmissionBackend&,const ManagerState&,const AdmissionCatalog&,
                 VoiceWrapper*& sourceHead,ScheduleNode*& scheduleHead,
                 std::uint32_t disabledIdentity,const AdmissionRequest&);
}
