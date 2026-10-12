#pragma once
#include "cd_track_controls.hpp"
namespace mnm::reconstruction::audio {
struct CdStopState {
    CdTrackState cd;
    std::uint32_t mixerCount=0,firstMixer=0;
};
// Scriptable callee boundaries. This model does not implement desktop mixers,
// the volume setter's nested stop behavior, or the clock implementation.
struct CdStopBackend:CdCommandBackend {
    virtual std::int32_t firstMixerVolume(std::uint32_t mixer,std::uint32_t& raw)=0;
    virtual std::int32_t volume(std::int32_t level)=0;
    virtual std::int32_t ticks()=0;
};
// Recovered caller scope: raw mixer levels0..65535 and nonwrapping positive
// scripted clock intervals. Errors in STOP/restore are intentionally ignored.
std::uint32_t stopCd(CdStopBackend&,CdStopState&,std::uint32_t fade);
}
