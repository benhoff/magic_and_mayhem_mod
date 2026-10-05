#pragma once
#include "simulation/movement.hpp"
#include "sprite.hpp"
#include "terrain_submission.hpp"
#include "animation.hpp"
#include "terrain_catalog.hpp"
#include "placement.hpp"

namespace mnm::scene {
// Explicit diagnostic camera, without legacy map/camera pointer ownership.
struct Camera { std::uint32_t view=0; game::Point origin{}; int x=256,y=64; };
struct Tile { std::uint32_t definition=0; game::Point surface; };
struct Draw { bool creature=false; std::uint32_t frame=0; int x=0,y=0; std::int32_t key=0; };
struct Frame { render::Image pixels; QImage image; std::vector<Draw> queue; };
reconstruction::AnimationOffset project(game::Point,const Camera&);
// Reads the exact owned controller display; never advances a second ANI clock.
std::optional<assets::AnimationRecord> displayed(const assets::Animation&,const game::Entity&);
std::vector<Draw> compose(const game::MovementSession&,const assets::Animation&,
                         const assets::TerrainCatalog&,const std::vector<Tile>&,const Camera&,
                         std::size_t terrainFrames,std::size_t creatureFrames);
Frame render(render::GlBlitter&,const assets::Sprite& terrain,const assets::Sprite& creature,
             const std::vector<Draw>&);
}
