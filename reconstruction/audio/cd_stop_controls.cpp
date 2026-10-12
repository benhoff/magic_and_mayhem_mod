#include "cd_stop_controls.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::audio {
std::uint32_t stopCd(CdStopBackend& backend,CdStopState& state,std::uint32_t fade){
    auto& cd=state.cd;
    if(!cd.initialized || !cd.active)return 0;
    cd.status[2]=4;
    const auto status=backend.command(cd.device,0x814,0x102,cd.status);
    if(!status && (cd.status[1]==0x212 || cd.status[1]==0x20d || cd.status[1]==0x20c))return 0;
    std::int32_t saved=0;
    if(fade){
        saved=15;std::int32_t queried=0;
        if(cd.initialized && cd.active){
            std::uint32_t raw=0;
            if(state.mixerCount)queried=backend.firstMixerVolume(state.firstMixer,raw);
            if(!queried){
                if(raw>65535)throw std::out_of_range("Outside recovered CD mixer range");
                saved=std::int32_t(raw/4369);
            }
        }else saved=0;
        if(!queried){
            auto level=saved;
            while(level>0){
                const auto set=backend.volume(--level);
                const auto begin=backend.ticks();const auto delay=(1000/saved)/2;
                if(begin<0 || begin>std::numeric_limits<std::int32_t>::max()-delay)
                    throw std::out_of_range("Outside recovered nonwrapping CD clock domain");
                const auto end=begin+delay;
                // The original checks once, then repeats its separate polling loop.
                if(end>backend.ticks())while(end>backend.ticks()){}
                if(set)break;
            }
        }
    }
    std::array<std::uint32_t,4> generic{cd.playback[3],0,0,0};
    backend.command(cd.device,0x808,0,generic);
    if(fade)backend.volume(saved);
    cd.paused=0;return 0;
}
}
