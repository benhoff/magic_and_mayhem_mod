#pragma once
#include "blit.hpp"

namespace mnm::render {
constexpr std::size_t maxClipRegions=32;
struct ClipperState {
    bool attached=false;
    // Attached nullopt means NOCLIPLIST; an empty vector means full exclusion.
    std::optional<std::vector<Rect>> regions;
};
enum class SurfaceCopyApi {Blt, BltFast};
struct SurfaceCopyRequest {
    SurfaceCopyApi api=SurfaceCopyApi::Blt;
    Rect source, destination;
    std::uint32_t flags=0;
    // Explicit caller admission observations, not native lock/loss lifecycle.
    bool sourceBusy=false,destinationBusy=false;
};
namespace surfaceStatus {
constexpr std::uint32_t ok=0,invalidRect=0x88760096,noClipList=0x887600cd;
constexpr std::uint32_t busy=0x887601ae,fastCannotClip=0x8876023e;
constexpr std::uint32_t invalidArgument=0x80070057,notImplemented=0x80004001;
}
struct SurfaceCopyPiece {Rect source;int x,y;};
struct SurfaceCopyResult {std::uint32_t hresult=0;unsigned pieces=0;};
struct SurfaceCopyPlan {
    std::vector<SurfaceCopyPiece> pieces;
    // A failure can follow earlier valid pieces. Execute them before returning.
    std::uint32_t hresult=0;
};
void validateClipper(const ClipperState& clipper,int width,int height);
SurfaceCopyPlan planSurfaceCopy(int sourceWidth,int sourceHeight,
                                int destinationWidth,int destinationHeight,
                                const ClipperState& clipper,const SurfaceCopyRequest& request);
}
