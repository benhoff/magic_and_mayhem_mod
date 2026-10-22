#pragma once
#include "particle_emitters.hpp"

namespace mnm::reconstruction {
inline std::vector<std::uint16_t> constructEmittedParticle(
    ParticleChildKind type, ParticleObject &object, ParticleWorld world,
    const std::array<std::int32_t, 3> &position,
    const std::array<std::int32_t, 3> &velocity,
    const ParticleConstructorConfig &config,
    const ParticleTerrainFacts &terrain, Kind8Random &random) {
  if (type == ParticleChildKind::randomAdditive) {
    ParticleConstructorArguments arguments;
    arguments.position = position;
    return constructParticleVariant(ParticleVariant::randomAdditive, object,
                                    world, arguments, config, {}, terrain,
                                    random);
  }
  constructParticleMotion(object, world, position, velocity, true);
  if (type == ParticleChildKind::ownerAdditive) {
    object.put(0, 0x5c7408);
    object.put(0x1c, 128);
    object.put(0x20, 128);
    object.put(0x24, 32);
    object.put(0x28, 100);
    object.put(0x2c, 5);
    object.put(0x30, 0);
    object.put(0x34, 2);
    object.put(0x38, 2);
    object.put(0x3c, 16);
    object.put(0x40, config.rgb555 ? 16 : 32);
    object.put(0x44, 4);
    for (unsigned at : {0x54u, 0x58u, 0x5cu, 0x60u})
      object.put(at, 0);
    return {};
  }
  const bool cached = type == ParticleChildKind::terrainColour;
  object.put(0, cached ? 0x5c74b0 : 0x5c7494);
  object.put(0x1c, cached ? 0 : 2);
  object.put(0x20, 2);
  object.put(0x24, 2);
  object.put(0x2e, 0);
  const std::uint16_t rgb[]{24, 0, 0};
  std::memcpy(object.bytes.data() + 0x28, rgb, 6);
  if (!cached)
    return {};
  if (world.width != 128 || world.height != 96 || world.depth != 64 ||
      terrain.generation > 1)
    throw std::invalid_argument("Owned emitted terrain extent");
  const auto x = std::uint32_t(object.get(4)) >> 5,
             y = std::uint32_t(object.get(8)) >> 5,
             z = std::uint32_t(object.get(12));
  object.put(0x32, x);
  object.put(0x36, y);
  object.put(0x3a, z >> 4);
  if (z < 64) {
    const auto index = (z >> 4) * 12 + y * 4 + x;
    object.put(0x3e, terrain.generation * 48 + index + 1);
    object.put(0x42, 0x2000 + terrain.cells[terrain.generation][index].block);
    object.put(0x46,
               terrain.shades[terrain.generation][(z >> 5) * 12 + y * 4 + x]);
  }
  object.put(0x56, config.duration);
  const auto dimension = std::clamp(config.dimension, 1, 16);
  object.put(0x20, dimension);
  object.put(0x24, dimension);
  return config.planeEnabled
             ? kind8Plane(dimension, dimension, {24, 0, 0}, config.rgb555)
             : std::vector<std::uint16_t>{};
}
inline void colourEmittedParticle(ParticleChildKind type,
                                  ParticleObject &object,
                                  std::vector<std::uint16_t> &plane,
                                  std::array<std::int32_t, 3> requested,
                                  bool rgb555) {
  if (type != ParticleChildKind::colour &&
      type != ParticleChildKind::terrainColour)
    throw std::invalid_argument("Emitted particle colour method");
  if (type == ParticleChildKind::terrainColour) {
    const auto factor = particleAdd(object.get(0x46), 127);
    for (unsigned axis = 0; axis < 3; ++axis) {
      object.put(0x4a + axis * 4, requested[axis]);
      requested[axis] =
          particleDivide(particleMul(factor, requested[axis]), 127);
    }
  }
  OwnedKind8 shape;
  std::copy_n(object.bytes.begin(), 128, shape.bytes.begin());
  shape.plane = plane;
  setOwnedKind8Colour(shape, requested, rgb555);
  std::copy(shape.bytes.begin(), shape.bytes.end(), object.bytes.begin());
  plane = std::move(shape.plane);
}
} // namespace mnm::reconstruction
