#pragma once
#include "voice_contract.hpp"
#include <vector>
namespace mnm::reconstruction::audio {
// Host annotations for +0x1c, +0x18, +0x0c, +8 and +0x258. Not a game layout.
struct ManagerState {
    bool initialized=false,active=false;
    std::uint32_t primary=0,device=0;
    std::int32_t savedVolume=0;
    Status lastPrimaryStop=0;
};
struct ScheduledVoice {Schedule32* record;std::uint32_t buffer;std::uint32_t* output;};
struct ManagerBackend:VoiceBackend {
    virtual Status getVolume(std::uint32_t,std::int32_t&)=0;
    virtual std::uint32_t tickCount()=0;
    // Shutdown delegates the wrapper/list operation at 0x004de090. Its entire
    // implementation is deliberately not inferred from this call alone.
    virtual void retireVoice(std::uint32_t voiceAddress)=0;
    virtual void releaseVoice(std::uint32_t buffer)=0;
    virtual void releaseDevice(std::uint32_t device)=0;
};
Status getPrimaryVolume(ManagerBackend&,const ManagerState&,std::int32_t&);
Status setPrimaryVolume(ManagerBackend&,const ManagerState&,std::int32_t);
Status startPrimary(ManagerBackend&,ManagerState&);
// Post-setup block: capture volume, then start; outer init failure cleanup separate.
Status initializePrimaryControls(ManagerBackend&,ManagerState&);
// Inline manager global-stop contract, excludes engine list ownership/allocation.
Status disableAudio(ManagerBackend&,ManagerState&,const std::vector<ScheduledVoice>&);
// Selected shutdown ordering, with caller-supplied owned-buffer release inventory.
// Heap strings, wrapper/list disposal and device-specific destructor work excluded.
Status shutdownAudio(ManagerBackend&,ManagerState&,const std::vector<ScheduledVoice>&,
                     const std::vector<std::uint32_t>& ownedBuffers);
}
