#pragma once
#include "scene.hpp"
namespace mnm::scene {
// Use the exact presented order and decoded coverage, including terrain occlusion.
std::optional<game::Handle> pickActor(const std::vector<Draw>&,const assets::Sprite& terrain,const assets::Sprite& creature,int x,int y);
// Ignore bodies and choose the foremost opaque terrain draw's declared cell.
std::optional<game::Point> pickTerrain(const std::vector<Draw>&,const assets::Sprite& terrain,int x,int y);
}
