#pragma once
#include "minimap.hpp"
#include <array>
#include <cstddef>

namespace mnm::render {
struct MinimapOverlayView {
  int gridWidth, gridHeight, centerX, centerY;
  // Original object +103/+107 rotation extents, independent of grid counts.
  int rotationWidth, rotationHeight, originX, originY;
  unsigned orientation;
  bool rgb555;
};
struct MinimapCellMarker {
  int x, y;
  unsigned paletteIndex;
  bool emphasized;
};
struct MinimapCreatureMarker { int x, y; bool hidden; };
struct MinimapCreatureResult { unsigned drawn; int gridWidth, gridHeight; };
struct MinimapCorner { int xDirection, yDirection; };
struct MinimapCameraOutline {
  int viewportWidth, viewportHeight;
  std::array<MinimapCorner, 4> corners;
  bool skipOuterBorder;
};
struct MinimapOutlineResult { int originX, originY; };

// Returned index is the original EAX destination pointer normalized to a word
// offset. All operations reject unsupported input atomically before writing.
std::size_t drawMinimapCellMarker(MinimapPlane &, const MinimapOverlayView &,
    const MinimapCellMarker &, const std::array<std::uint16_t, 9> &palette,
    bool flashEnabled);
MinimapCreatureResult drawMinimapCreatureMarkers(MinimapPlane &,
    const MinimapOverlayView &, const std::vector<MinimapCreatureMarker> &, bool fog);
// Selected camera-corner path; terrain, outer border and subsequent marker
// list composition remain separate. Returned origin preserves entry selection.
MinimapOutlineResult drawMinimapCameraOutline(MinimapPlane &,
    const MinimapOverlayView &, const MinimapCameraOutline &);
}
