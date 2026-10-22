#pragma once
#include "kind8_owned.hpp"
#include "particle_terrain.hpp"

namespace mnm::reconstruction {
enum class ParticleVariant {
  randomMotion,
  randomAdditive,
  emitter,
  defaultOwnerEmitter,
  customOwnerEmitter,
  seeker,
  attachedA,
  attachedB,
  fadingSprite
};
struct ParticleConstructorConfig {
  std::int32_t additivePhase{19}, additiveSpeed{16}, emitterPeriod{23};
  std::array<std::int32_t, 4> ownerEmitter{2, 3, 4, 5};
  std::int32_t lifetimeSpread{17}, duration{100}, dimension{3}, effectCount{40};
  bool planeEnabled{}, rgb555{};
};
struct ParticleConstructorArguments {
  std::array<std::int32_t, 3> position{}, velocity{};
  std::array<std::int32_t, 4> emitterParameters{};
  std::int32_t lifetime{}, category{4}, ordinal{};
  std::uint32_t ownerToken{0x1010}, palette{};
};
inline std::vector<std::uint16_t> constructParticleVariant(
    ParticleVariant type, ParticleObject &object, ParticleWorld world,
    const ParticleConstructorArguments &in,
    const ParticleConstructorConfig &config, const ParticleOwnerFacts &owner,
    const ParticleTerrainFacts &terrain, Kind8Random &random) {
  if (unsigned(type) > 8)
    throw std::invalid_argument("Unknown particle constructor variant");
  constructParticleMotion(object, world, in.position, in.velocity, false);
  auto put = [&](unsigned at, std::int32_t value) { object.put(at, value); };
  auto zeros = [&](std::initializer_list<unsigned> offsets) {
    for (auto at : offsets)
      put(at, 0);
  };
  auto positive = [&] { return random.next() & 0x7fffffffu; };
  std::vector<std::uint16_t> plane;
  if (type == ParticleVariant::randomMotion ||
      type == ParticleVariant::randomAdditive) {
    zeros({0x10, 0x14, 0x18});
    put(0x1c, 128);
    put(0x20, 128);
    put(0x24, 32);
    const bool additive = type == ParticleVariant::randomAdditive;
    put(0, additive ? 0x5c7420 : 0x5c73f0);
    put(0x28, additive ? 75 : 100);
    put(0x2c, additive ? config.additivePhase : 5);
    put(0x30, additive ? 2 : 0);
    put(0x34, 2);
    put(0x38, additive ? 16 : 2);
    put(additive ? 0x3c : 0x40, config.rgb555 ? 16 : 32);
    put(additive ? 0x40 : 0x44, 4);
    if (additive) {
      object.bytes[0x44] = 0;
      if (config.additiveSpeed <= 0)
        throw std::invalid_argument("Original additive speed divides by zero");
      auto x = std::int32_t(positive() % std::uint32_t(config.additiveSpeed));
      auto y =
          std::int32_t(positive() % std::uint32_t(config.additiveSpeed - x));
      if (positive() & 1)
        x = -x;
      if (positive() & 1)
        y = -y;
      put(0x10, x);
      put(0x14, y);
    } else {
      put(0x3c, 16);
      std::int32_t x, y;
      if (positive() & 1) {
        x = positive() % 16;
        y = positive() % (16 - x);
      } else {
        y = positive() % 16;
        x = positive() % (16 - y);
      }
      auto z = 16 - x - y;
      if (positive() & 1)
        x = -x;
      if (positive() & 1)
        y = -y;
      if (positive() & 1)
        z = -z;
      put(0x10, x);
      put(0x14, y);
      put(0x18, z);
    }
  } else if (type == ParticleVariant::emitter) {
    put(0, 0x5c7438);
    put(0x10, config.emitterPeriod);
    put(0x20, config.emitterPeriod);
    zeros({0x14, 0x1c, 0x24, 0x38, 0x3c, 0x44, 0x48, 0x54, 0x58, 0x5c, 0x60});
    put(0x28, 128);
    put(0x2c, 128);
    put(0x30, 32);
    put(0x34, 64);
    put(0x50, 16);
    put(0x18, 20);
    put(0x40, -12);
    put(0x4c, 3);
  } else if (type == ParticleVariant::defaultOwnerEmitter ||
             type == ParticleVariant::customOwnerEmitter) {
    const bool defaults = type == ParticleVariant::defaultOwnerEmitter;
    const auto &p = defaults ? config.ownerEmitter : in.emitterParameters;
    put(0, defaults ? 0x5c7450 : 0x5c7470);
    put(0x10, 8);
    for (unsigned i = 0; i < 4; ++i)
      put(0x14 + i * 4, p[i]);
    zeros({0x24, 0x28, 0x30, 0x34, 0x44, 0x48, 0x4c, 0x50});
    put(0x2c, defaults ? 12 : -12);
    put(0x38, 3);
    put(0x3c, defaults ? 0x5e14b0 : 0x5e1494);
    put(0x40, 16);
  } else if (type == ParticleVariant::seeker) {
    put(0, 0x5c74e4);
    zeros({0x10, 0x14, 0x18});
  } else if (type == ParticleVariant::attachedA ||
             type == ParticleVariant::attachedB) {
    if (!owner.present || config.lifetimeSpread <= 0)
      throw std::invalid_argument(
          "Original attached constructor requires owner and positive spread");
    put(0, type == ParticleVariant::attachedA ? 0x5c74fc : 0x5c7514);
    put(0x10, particleSigned(in.ownerToken));
    put(0x14, particleAdd(std::int32_t(positive() %
                                       std::uint32_t(config.lifetimeSpread)),
                          in.lifetime));
    for (unsigned axis = 0; axis < 3; ++axis)
      put(0x18 + axis * 4, particleAdd(particleSub(owner.horizontalOffset,
                                                   object.get(4 + axis * 4)),
                                       owner.position[axis]));
  } else {
    if (world.width != 128 || world.height != 96 || world.depth != 64 ||
        terrain.generation > 1)
      throw std::invalid_argument("Owned fading constructor terrain extent");
    put(0, 0x5c752c);
    for (unsigned axis = 0; axis < 3; ++axis)
      put(0x10 + axis * 4, in.velocity[axis]);
    put(0x1c, 0);
    const auto dimension = std::clamp(config.dimension, 1, 16);
    put(0x20, dimension);
    put(0x24, dimension);
    put(0x2e, 0);
    const std::uint16_t colour[]{24, 0, 0};
    std::memcpy(object.bytes.data() + 0x28, colour, 6);
    const auto x = std::uint32_t(object.get(4)) >> 5,
               y = std::uint32_t(object.get(8)) >> 5,
               z = std::uint32_t(object.get(12));
    put(0x32, x);
    put(0x36, y);
    put(0x3a, z >> 4);
    if (z < std::uint32_t(world.depth)) {
      const auto index = (z >> 4) * 12 + y * 4 + x;
      const auto &cell = terrain.cells[terrain.generation][index];
      put(0x3e, terrain.generation * 48 + index + 1);
      put(0x42, 0x2000 + cell.block);
      put(0x46, terrain.shades[terrain.generation][(z >> 5) * 12 + y * 4 + x]);
    }
    put(0x56, config.duration);
    if (config.planeEnabled) {
      plane = kind8Plane(dimension, dimension, {24, 0, 0}, config.rgb555);
      if (!plane.empty())
        put(0x2e, 0x3000);
    }
    put(0x5a, 0);
    put(0x5e, config.effectCount / 10);
    put(0x72, particleSigned(in.palette));
    put(0x76, in.category);
    put(0x7a, in.ordinal);
    put(0x7e, random.next() & 7);
    // Resource preparation54a5f0 is a no-op for this preloaded-resource
    // domain; lazy original asset/palette loading remains independent.
  }
  return plane;
}
inline void
destroyParticleVariantNonDeleting(ParticleVariant type, ParticleObject &object,
                                  std::vector<std::uint16_t> &plane) {
  if (unsigned(type) > 8)
    throw std::invalid_argument("Unknown particle variant destruction");
  if (!plane.empty()) {
    plane.clear();
    object.put(0x2e, 0);
  }
  object.put(0, 0x5c73a8);
}
} // namespace mnm::reconstruction
