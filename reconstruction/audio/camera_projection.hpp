#pragma once
#include "positional_audio.hpp"
namespace mnm::reconstruction::audio {
struct CameraPoint {std::int32_t x=0,y=0;};
// Decoded host annotations, NOT the packed original camera object at 0x689930.
// Names preserve field offsets where physical semantics are not established.
struct CameraState {
    unsigned orientation=0;std::int32_t mapWidth=0,mapHeight=0;
    std::int32_t f11=0,f15=0,f39=0,f3d=0,f41=0,f45=0,f49=0,f4d=0,
                 f51=0,f55=0,f5d=0,f61=0;
};
CameraPoint cameraScreenOrigin(const CameraState&); // 0x4f8230
CameraPoint screenToMap(const CameraState&,CameraPoint screen); // 0x4f7d00
CameraPoint audioListener(const CameraState&,bool largeScreen); // 0x5716ae call site
// Provides decoded listener coordinates to the recovered positional arithmetic.
PositionalControls cameraPositionalControls(const CameraState&,bool largeScreen,PositionalInput source);
PositionalControls cameraPositionalControls(const CameraState&,bool largeScreen,PositionalInput source,
                                          const PositionalByteSource&,std::uint32_t sourceZ);
}
