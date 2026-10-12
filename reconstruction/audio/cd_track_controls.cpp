#include "cd_track_controls.hpp"
namespace mnm::reconstruction::audio {
std::uint32_t playCdTrack(CdCommandBackend& backend,CdTrackState& state,std::uint32_t track){
    if(!state.initialized || !state.active)return 0;
    state.playback[1]=track&255u;state.playback[2]=(track+1u)&255u;
    const auto first=backend.command(state.device,0x806,13,state.playback);
    if(!first){state.paused=0;return 0;}
    state.status[2]=3;
    if(!backend.command(state.device,0x814,0x102,state.status) && state.status[1]==track)
        return backend.command(state.device,0x806,5,state.playback);
    // The original retry-return path does not clear paused even on success.
    return first;
}
std::uint32_t playCdFromTrack(CdCommandBackend& backend,CdTrackState& state,std::uint32_t track){
    if(!state.initialized || !state.active)return 0;
    state.playback[1]=track&255u;
    const auto result=backend.command(state.device,0x806,5,state.playback);
    if(!result)state.paused=0;
    return result;
}
std::uint32_t pauseResumeCd(CdCommandBackend& backend,CdTrackState& state){
    if(!state.initialized || !state.active)return 0;
    std::array<std::uint32_t,4> parameters{state.playback[3],0,0,0};
    const auto result=backend.command(state.device,state.paused?0x855:0x809,2,parameters);
    if(!result)state.paused=state.paused?0:1;
    return result;
}
std::uint32_t cdLengthByte(CdCommandBackend& backend,CdTrackState& state){
    state.status[2]=1;
    if(backend.command(state.device,0x814,0x102,state.status))return 0;
    return state.status[1]&255u;
}
}
