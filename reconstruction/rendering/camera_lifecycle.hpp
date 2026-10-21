#pragma once
#include "terrain_camera.hpp"
#include <vector>
namespace mnm::reconstruction {
struct TerrainCameraLifecycle {
 TerrainCamera camera;
 std::uint32_t requestedView=0,rendered=0,redrawFrames=0;
 bool clockwise=false,counterclockwise=false;
 std::uint32_t width=0,height=0,layers=0,parameter29=0,parameter2d=0;
};
// 004f7860 clears pending rotation even when base registration refuses. The
// registry owns admission; native inputs are validated before any mutation.
bool initializeTerrainCamera(TerrainCameraLifecycle&,unsigned width,unsigned height,
 unsigned layers,TerrainViewport,std::uint32_t parameter29,std::uint32_t parameter2d,
 bool registrationAccepted);
enum class CameraTurn {clockwise,counterclockwise};
struct CameraRefresh {
 enum class Kind {creature,effect,minimap};
 Kind kind;std::size_t ordinal;unsigned view;
};
// Callback-free ordered requests: application orchestration applies them to
// owned animation/minimap state. A distinct primary camera view is explicit.
std::vector<CameraRefresh> rotateTerrainCamera(TerrainCameraLifecycle&,CameraTurn,
 const std::vector<bool>& creatures,const std::vector<bool>& effects,
 std::optional<unsigned> distinctPrimaryView={});
void beginTerrainCameraFrame(TerrainCameraLifecycle&);
// Complete the selected post-frame branch: clockwise wins and only its pending
// request clears. A simultaneous counterclockwise request remains for next time.
std::vector<CameraRefresh> finishTerrainCameraFrame(TerrainCameraLifecycle&,
 const std::vector<bool>& creatures,const std::vector<bool>& effects,
 std::optional<unsigned> distinctPrimaryView={});
struct ViewportRegistration {
 std::uint64_t identity=0;TerrainViewport rectangle;std::uint32_t flags=0;
};
// Dense owned receiver records replace the original pointer registry. Identities
// remain stable while dense indices move; no borrowed receiver pointer survives.
class TerrainViewportRegistry {
 std::vector<ViewportRegistration> entries_;
 std::optional<std::size_t> selected_;
public:
 bool append(ViewportRegistration);
 void remove(std::optional<std::size_t>);
 void clear();
 std::optional<std::size_t> hit(std::int32_t x,std::int32_t y) const;
 void select(std::optional<std::size_t>);
 const auto& entries() const{return entries_;}
 auto selected() const{return selected_;}
};
}
