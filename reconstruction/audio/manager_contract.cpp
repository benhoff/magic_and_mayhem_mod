#include "manager_contract.hpp"
namespace mnm::reconstruction::audio {
Status getPrimaryVolume(ManagerBackend& b,const ManagerState& m,std::int32_t& value){return m.initialized?b.getVolume(m.primary,value):0;}
Status setPrimaryVolume(ManagerBackend& b,const ManagerState& m,std::int32_t value){return m.initialized?b.volume(m.primary,value):0;}
Status startPrimary(ManagerBackend& b,ManagerState& m){
    if(!m.initialized || m.active)return 0;
    const auto result=b.play(m.primary,0,0,1);if(!result)m.active=true;return result;
}
Status initializePrimaryControls(ManagerBackend& b,ManagerState& m){
    if(!m.initialized)return 0;
    const auto result=b.getVolume(m.primary,m.savedVolume);return result?result:startPrimary(b,m);
}
namespace {
bool selected(ManagerBackend& b,const Schedule32& r){
    if(!r.voiceAddress)return false;
    return r.deadline==0xffffffff || !retirementDue(b.tickCount(),r.deadline);
}
void clear(const ScheduledVoice& v){if(v.record->outputAddress && v.output)*v.output=0;clearSchedule(*v.record);}
}
Status disableAudio(ManagerBackend& b,ManagerState& m,const std::vector<ScheduledVoice>& voices){
    if(!m.initialized || !m.active)return 0;
    for(const auto& v:voices){
        if(selected(b,*v.record))stopAndReset(b,v.buffer,true);
        clear(v); // Secondary failures do not abort record retirement.
    }
    m.lastPrimaryStop=0;m.active=false;
    m.lastPrimaryStop=b.stop(m.primary);return m.lastPrimaryStop;
}
Status shutdownAudio(ManagerBackend& b,ManagerState& m,const std::vector<ScheduledVoice>& voices,
                     const std::vector<std::uint32_t>& owned){
    if(!m.initialized)return 0;
    b.volume(m.primary,m.savedVolume); // Return is ignored by original shutdown.
    if(m.active){
        for(const auto& v:voices){if(selected(b,*v.record))b.retireVoice(v.record->voiceAddress);clear(v);}
        m.lastPrimaryStop=0;m.active=false;m.lastPrimaryStop=b.stop(m.primary);
    }
    for(auto buffer:owned)b.releaseVoice(buffer);
    if(m.device){b.releaseDevice(m.device);m.device=0;}
    m.initialized=false;return 0;
}
}
