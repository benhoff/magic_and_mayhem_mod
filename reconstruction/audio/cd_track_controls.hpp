#pragma once
#include <array>
#include <cstdint>
namespace mnm::reconstruction::audio {
// Host annotations for the pinned manager's selected CD control fields, not
// a packed object or process-pointer wire layout. All values are raw DWORDs.
struct CdTrackState {
    std::uint32_t initialized=0,active=0;
    std::uint32_t paused=0,device=0;
    std::array<std::uint32_t,4> playback{}; // callback, from, to, pause callback
    std::array<std::uint32_t,4> status{}; // callback, result, item, track
};
struct CdCommandBackend {
    virtual ~CdCommandBackend()=default;
    virtual std::uint32_t command(std::uint32_t device,std::uint32_t message,
                                  std::uint32_t flags,std::array<std::uint32_t,4>& parameters)=0;
};
std::uint32_t playCdTrack(CdCommandBackend&,CdTrackState&,std::uint32_t track);
std::uint32_t playCdFromTrack(CdCommandBackend&,CdTrackState&,std::uint32_t track);
std::uint32_t pauseResumeCd(CdCommandBackend&,CdTrackState&);
std::uint32_t cdLengthByte(CdCommandBackend&,CdTrackState&);
}
