#pragma once
#include "voice_lifetime.hpp"
#include <optional>
namespace mnm::reconstruction::audio {
struct PositionalControls {
    std::int32_t volume=-5000,pan=0;
    bool panWritten=false; // Original leaves output pan untouched on inaudible return.
};
struct PositionalInput {
    std::int32_t sourceX=0,sourceY=0,listenerX=0,listenerY=0;
    std::int32_t mapWidth=0,mapHeight=0,range=0,panWidth=0;
    unsigned orientation=0;
    std::optional<std::int8_t> mapByte;
    std::int32_t byteThreshold=0;
};
// Inputs use decoded map coordinates. Listener is the output of 0x4f7d00;
// screen/camera projection and map-byte lookup ownership are separate.
std::int32_t wrappedDifference(std::int32_t source,std::int32_t listener,std::int32_t extent);
struct PositionalByteSource {
    virtual ~PositionalByteSource()=default;
    virtual std::optional<std::int8_t> read(std::int32_t x,std::int32_t y,std::uint32_t z) const=0;
};
PositionalControls positionalControls(const PositionalInput&);
// Deferred lookup: outside-range calls do not read storage; provider replaces mapByte.
PositionalControls positionalControls(const PositionalInput&,const PositionalByteSource&,std::uint32_t sourceZ);
// Selected 0x571460 update block, receiving recovered geometry outputs.
// Scheduler notifications resolve wrapper identity; no game addresses here.
struct PositionalBackend:LifetimeBackend {
    virtual void positionalVolumeRecord(std::uint32_t identity,std::int32_t volume)=0;
};
Status updatePositionalVoice(PositionalBackend&,VoiceWrapper*& slot,const PositionalControls&,
                             bool worldPresent,bool enabled);
}
