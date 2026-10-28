#pragma once
#include "scene_snapshot.hpp"
namespace mnm::legacy {
// Validate a diagnostic-only refusal. Successful packets return no refusal;
// malformed packets and partial refusal payloads remain errors.
std::optional<std::uint32_t> decodeWorldRefusal(const QByteArray&);
struct WorldDraw {
    SnapshotFrame frame;
    std::int32_t x=0,y=0;
    render::Rect clip;
    std::optional<render::SpriteColourTable> colours;
    render::SpriteComposite composite;
    std::uint32_t backend=0;
    std::optional<render::AdditiveRectangle> additive;
    std::optional<render::ColourRectangle> colourRectangle;
    std::optional<render::SpriteRowEffect> rowEffect;
};
struct WorldFrame {
    std::uint32_t sequence=0,width=0,height=0,stride=0;
    std::vector<WorldDraw> draws;
    bool colourRgb555=false;
};
// Existing/live consumers retain RGB565-only admission. The offline caller must
// explicitly request the bounded colour-only RGB555 policy.
enum class WorldInputPolicy { rgb565, colourRgb555 };
WorldFrame decodeWorldFrame(const QByteArray&,WorldInputPolicy=WorldInputPolicy::rgb565);
render::SceneDraw worldPrimitiveDraw(const WorldDraw&);
render::SceneDraw worldSpriteDraw(const WorldDraw&,const BoundFrame&);
// Strict, all-or-nothing asset binding. Neither oracle canvas is an input.
std::vector<render::SceneDraw> worldDisplay(const WorldFrame&,const SnapshotResources&);
}
