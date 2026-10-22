#pragma once
#include "particle_constructors.hpp"

namespace mnm::reconstruction {
enum class ParticleChildKind {
  randomAdditive,
  ownerAdditive,
  colour,
  terrainColour
};
struct ParticleEmissionConfig {
  std::int32_t additiveSpeed{16}, fallLimit{4}, levelLoss{3}, levelRestore{25};
  std::int32_t colourThreshold{50}, trails{}, duration{100};
  std::array<std::int32_t, 2> childDimensions{2, 2};
  bool rgb555{};
};
inline std::int32_t particleRandomMod(Kind8Random &random,
                                      std::int32_t divisor) {
  if (!divisor)
    throw std::invalid_argument("Original emitting particle divides by zero");
  return std::int32_t(std::int64_t(random.next() & 0x7fffffffu) % divisor);
}
inline std::array<std::int32_t, 3>
particlePosition(const ParticleObject &object) {
  return {object.get(4), object.get(8), object.get(12)};
}
inline std::array<std::int32_t, 3>
particleVelocity(const ParticleObject &object) {
  return {object.get(16), object.get(20), object.get(24)};
}
inline void setParticleScaledColour(ParticleObject &object,
                                    unsigned destination,
                                    std::array<std::int32_t, 3> rgb,
                                    std::int32_t numerator,
                                    std::int32_t denominator, bool rgb555) {
  for (unsigned channel = 0; channel < 3; ++channel) {
    const auto value = std::clamp(
        particleDivide(particleMul(rgb[channel], numerator), denominator), 0,
        255);
    object.put(destination + channel * 4,
               value / (channel == 1 ? (rgb555 ? 8 : 4) : 8));
  }
}
// Backend owns allocation generations, independent constructor models, pool
// transfer decisions and deleting destruction. Failed transfers are explicitly
// destroyed by these original callers; failed allocations remain null.
template <class Backend>
bool updateAdditiveEmitter(ParticleObject &object, ParticleWorld world,
                           const ParticleEmissionConfig &config,
                           Kind8Random &random, Backend &backend) {
  const bool moving = updateParticleMotion(object, world);
  auto dz = particleSub(object.get(24), 1);
  object.put(24, dz);
  if (!moving || object.get(12) <= 0)
    return false;
  object.put(0x28, particleSub(object.get(0x28), config.levelLoss));
  bool alive = true;
  if (dz <= particleSub(0, config.fallLimit) || object.get(0x28) < 11) {
    const auto phase = particleSub(object.get(0x2c), 1);
    object.put(0x2c, phase);
    if (phase < 1)
      alive = false;
    else {
      object.put(0x28, particleAdd(object.get(0x28), config.levelRestore));
      auto x = particleRandomMod(random, config.additiveSpeed);
      auto y = particleRandomMod(random, particleSub(config.additiveSpeed, x));
      object.put(24, 0);
      if (random.next() & 1)
        x = particleSub(0, x);
      if (random.next() & 1)
        y = particleSub(0, y);
      object.put(16, x);
      object.put(20, y);
      if (object.bytes[0x44] == 1) {
        object.bytes[0x44] = 0;
        const auto token = backend.allocate(0x45);
        if (token) {
          backend.construct(token, ParticleChildKind::randomAdditive,
                            particlePosition(object),
                            std::array<std::int32_t, 3>{});
          auto &child = backend.object(token);
          child.put(0x28, object.get(0x28));
          child.put(0x2c, object.get(0x2c));
          child.put(16, particleSub(0, x));
          child.put(20, particleSub(0, y));
          std::array<std::int32_t, 3> rgb{object.get(0x1c), object.get(0x20),
                                          object.get(0x24)};
          const auto denominator =
              particleAdd(particleRandomMod(random, 21) - 10, child.get(0x28));
          for (unsigned axis = 0; axis < 3; ++axis)
            child.put(0x1c + axis * 4, rgb[axis]);
          setParticleScaledColour(child, 0x38, rgb, 100, denominator,
                                  config.rgb555);
          if (!backend.insert(token))
            backend.destroy(token);
        }
      }
    }
  }
  if (object.get(0x28) <= 10)
    return false;
  const auto level =
      particleAdd(particleRandomMod(random, 21) - 10, object.get(0x28));
  setParticleScaledColour(
      object, 0x38, {object.get(0x1c), object.get(0x20), object.get(0x24)},
      level, 100, config.rgb555);
  return alive;
}
inline std::array<std::int32_t, 3>
emittingVelocity(const ParticleObject &object, unsigned values,
                 unsigned spreads, Kind8Random &random) {
  std::array<std::int32_t, 3> velocity{};
  for (unsigned axis = 0; axis < 3; ++axis) {
    const auto spread = object.get(spreads + axis * 4);
    velocity[axis] = object.get(values + axis * 4);
    if (spread)
      velocity[axis] = particleAdd(
          velocity[axis],
          particleSub(particleRandomMod(
                          random, axis == 2 ? particleMul(spread, 2) : spread),
                      axis == 2 ? spread : spread / 2));
  }
  return velocity;
}
inline std::array<std::int32_t, 3>
emittingPosition(const ParticleObject &object, std::int32_t radius,
                 Kind8Random &random) {
  std::array<std::int32_t, 3> position{};
  const auto divisor = particleAdd(particleMul(radius, 2), 1);
  // Original pushes constructor arguments in Z,Y,X evaluation order.
  for (int axis = 2; axis >= 0; --axis)
    position[axis] = particleSub(particleAdd(particleRandomMod(random, divisor),
                                             object.get(4 + axis * 4)),
                                 radius);
  return position;
}
inline std::array<std::int32_t, 3> emittingColour(const ParticleObject &object,
                                                  unsigned rgbOffset,
                                                  std::int32_t spread,
                                                  Kind8Random &random) {
  std::array<std::int32_t, 3> rgb{};
  for (unsigned axis = 0; axis < 3; ++axis) {
    rgb[axis] = object.get(rgbOffset + axis * 4);
    if (spread)
      rgb[axis] =
          particleAdd(rgb[axis], particleSub(particleRandomMod(random, spread),
                                             spread / 2));
  }
  return rgb;
}
inline void inheritEmittingOwner(ParticleObject &child,
                                 const ParticleObject &object,
                                 const ParticleOwnerFacts &owner,
                                 unsigned ownerOffset, unsigned flagOffset,
                                 bool initializeZOffset) {
  child.put(0x60, object.get(ownerOffset));
  for (unsigned axis = 0; axis < (initializeZOffset ? 3u : 2u); ++axis) {
    const auto flag = object.get(flagOffset + axis * 4);
    child.put(0x54 + axis * 4, flag);
    if (flag && (axis < 2 || initializeZOffset)) {
      if (!owner.present)
        throw std::invalid_argument(
            "Original emitting owner flags dereference a null owner");
      child.put(16 + axis * 4, 0);
      const auto offset =
          axis == 2 ? owner.verticalOffset : owner.horizontalOffset;
      child.put(0x48 + axis * 4,
                particleAdd(particleSub(offset, child.get(4 + axis * 4)),
                            owner.position[axis]));
    }
  }
}
template <class Backend>
bool updateInterpolatingEmitter(ParticleObject &object, ParticleWorld world,
                                const ParticleOwnerFacts &owner,
                                const ParticleEmissionConfig &config,
                                Kind8Random &random, Backend &backend) {
  const auto stage = object.get(0x24);
  if (stage < 0 || stage > 1)
    throw std::invalid_argument(
        "Original interpolating emitter keyframe extent");
  const auto initial = object.get(0x14 + unsigned(stage) * 4);
  auto count = particleAdd(
      particleDivide(
          particleMul(
              particleSub(object.get(0x18 + unsigned(stage) * 4), initial),
              particleSub(object.get(0x10), object.get(0x20))),
          object.get(0x10)),
      initial);
  attachParticleAxes(object, world, owner, 0, 0x58, false);
  while (count > 0) {
    const auto velocity = emittingVelocity(object, 0x38, 0x44, random);
    const auto token = backend.allocate(100);
    if (!token)
      break;
    const auto position = emittingPosition(object, object.get(0x50), random);
    backend.construct(token, ParticleChildKind::ownerAdditive, position,
                      velocity);
    auto &child = backend.object(token);
    const auto rgb = emittingColour(object, 0x28, object.get(0x34), random);
    for (unsigned axis = 0; axis < 3; ++axis)
      child.put(0x1c + axis * 4, rgb[axis]);
    setParticleScaledColour(child, 0x3c, rgb, 100, child.get(0x28),
                            config.rgb555);
    child.put(0x30,
              particleSub(0, particleDivide(object.get(12), velocity[2])));
    inheritEmittingOwner(child, object, owner, 0x54, 0x58, true);
    if (!backend.insert(token)) {
      backend.destroy(token);
      break;
    }
    --count;
  }
  const auto remaining = particleSub(object.get(0x20), 1);
  object.put(0x20, remaining);
  if (remaining < 1) {
    object.put(0x20, object.get(0x10));
    const auto next = particleAdd(object.get(0x24), 1);
    object.put(0x24, next);
    if (next > 1)
      return false;
  }
  return true;
}
template <class Backend>
std::uint32_t updateOwnerEmitter(ParticleObject &object, ParticleWorld world,
                                 const ParticleOwnerFacts &owner,
                                 std::uint32_t ownerVirtualAlive,
                                 const ParticleEmissionConfig &config,
                                 Kind8Random &random, Backend &backend) {
  auto count = object.get(0x10);
  std::uint32_t alive = 1;
  if (owner.present) {
    if (!owner.active)
      alive = 0;
    else {
      attachParticleAxes(object, world, owner, 0, 0x48, false);
      alive = ownerVirtualAlive;
    }
  }
  while (count > 0) {
    const auto velocity = emittingVelocity(object, 0x24, 0x30, random);
    const auto token = backend.allocate(100);
    if (!token)
      break;
    const auto position = emittingPosition(object, object.get(0x40), random);
    backend.construct(token, ParticleChildKind::ownerAdditive, position,
                      velocity);
    auto &child = backend.object(token);
    const auto rgb = emittingColour(object, 0x14, object.get(0x20), random);
    child.put(0x34, std::clamp(config.childDimensions[0], 0, 16));
    child.put(0x38, std::clamp(config.childDimensions[1], 0, 16));
    for (unsigned axis = 0; axis < 3; ++axis)
      child.put(0x1c + axis * 4, rgb[axis]);
    setParticleScaledColour(child, 0x3c, rgb, 100, child.get(0x28),
                            config.rgb555);
    child.put(0x30,
              particleSub(0, particleDivide(object.get(12), velocity[2])));
    // Unlike546ce0,547380 leaves Z-follow, Z offset and Z velocity at their
    // constructed values; only the X/Y owner bindings are copied.
    inheritEmittingOwner(child, object, owner, 0x44, 0x48, false);
    if (!backend.insert(token)) {
      backend.destroy(token);
      break;
    }
    --count;
  }
  return alive;
}
template <class Backend>
bool updateFlagEmitter(ParticleObject &object, ParticleWorld world,
                       const ParticleOwnerFacts &owner,
                       const ParticleEmissionConfig &config,
                       Kind8Random &random, Backend &backend) {
  attachParticleAxes(object, world, owner, 0, 0, false);
  auto rgb = std::array<std::int32_t, 3>{object.get(0x2c), object.get(0x30),
                                         object.get(0x34)};
  for (std::int32_t index = 0; index < object.get(0x14); ++index) {
    std::array<std::int32_t, 3> position{}, velocity{};
    const auto spread = object.get(0x28);
    for (unsigned axis = 0; axis < 3; ++axis)
      position[axis] = particleAdd(
          particleSub(particleRandomMod(random, spread), spread / 2),
          object.get(4 + axis * 4));
    for (unsigned axis = 0; axis < 3; ++axis)
      velocity[axis] = particleAdd(
          particleSub(particleRandomMod(random, spread), spread / 2),
          object.get(0x1c + axis * 4));
    if (object.get(0x38))
      rgb = emittingColour(object, 0x2c, object.get(0x38), random);
    const bool simple = particleRandomMod(random, 100) > config.colourThreshold;
    auto create = [&](bool terrain, const auto &p, const auto &v, bool trail) {
      const auto token = backend.allocate(terrain ? 0x5a : 0x32);
      if (!token)
        return 0;
      backend.construct(token,
                        terrain ? ParticleChildKind::terrainColour
                                : ParticleChildKind::colour,
                        p, v);
      auto &child = backend.object(token);
      if (trail) {
        child.put(0x20, 1);
        child.put(0x24, 1);
        child.put(0x1c, 0);
      }
      backend.colour(token, rgb);
      if (!backend.insert(token)) {
        backend.destroy(token);
        return -1;
      }
      return 1;
    };
    if (simple) {
      if (object.bytes[0x40] && create(false, position, velocity, false) < 0)
        break;
    } else {
      const int status = create(true, position, velocity, false);
      if (status < 0)
        break;
      if (status > 0 && config.trails > 0 && object.bytes[0x40])
        for (std::int32_t trail = 0; trail < config.trails; ++trail) {
          position[0] = particleSub(position[0], velocity[0]);
          position[1] = particleSub(position[1], velocity[1]);
          velocity[2] = particleAdd(velocity[2], 1);
          position[2] = particleSub(position[2], velocity[2]);
          if (create(false, position, velocity, true) < 0)
            break;
        }
    }
  }
  const auto life = particleSub(object.get(0x10), 1),
             count = particleAdd(object.get(0x14), object.get(0x18));
  object.put(0x10, life);
  object.put(0x14, count);
  object.bytes[0x40] = 0;
  return count > 0 && life > 0;
}
struct ParticleAnimationFacts {
  std::int32_t primaryOrdinals{2}, fallbackPeriodA{2}, fallbackPeriodB{3};
  bool fallbackB{};
  std::array<std::uint32_t, 64> primary{}, fallbackA{}, fallback{};
  std::uint32_t primaryFrameCount{18}, fallbackFrameCountA{18},
      fallbackFrameCountB{27};
};
template <class Backend>
bool updateFadingEmitter(ParticleObject &object, ParticleWorld world,
                         const ParticleTerrainFacts &terrain,
                         ParticleTerrainValidity &validity,
                         const ParticleAnimationFacts &animation,
                         const ParticleEmissionConfig &config,
                         Kind8Random &random, Backend &backend) {
  if (object.get(0x56) == config.duration)
    for (std::int32_t index = 0; index < object.get(0x5e); ++index) {
      const auto token = backend.allocate(0x5a);
      if (!token)
        continue;
      std::array<std::int32_t, 3> position{}, velocity{};
      for (int axis = 2; axis >= 0; --axis)
        velocity[axis] = particleRandomMod(random, 5) - 2;
      for (int axis = 2; axis >= 0; --axis)
        position[axis] = particleAdd(particleRandomMod(random, 5) - 2,
                                     object.get(4 + axis * 4));
      backend.construct(token, ParticleChildKind::terrainColour, position,
                        velocity);
      auto &child = backend.object(token);
      if (particleRandomMod(random, 100) > config.colourThreshold)
        child.put(0x56, particleRandomMod(random, config.duration));
      auto rgb = std::array<std::int32_t, 3>{object.get(0x62), object.get(0x66),
                                             object.get(0x6a)};
      const auto spread = object.get(0x6e);
      if (spread)
        for (int axis = 2; axis >= 0; --axis)
          rgb[axis] = particleSub(
              particleAdd(particleRandomMod(random, spread), rgb[axis]),
              spread / 2);
      backend.colour(token, rgb);
      if (!backend.insert(token)) {
        backend.destroy(token);
        break;
      }
    }
  const auto alive = updateTerrainParticle(
      object, world, terrain, config.duration, config.rgb555, validity);
  if (!alive)
    return false;
  const auto &cell = terrain.cell(std::uint32_t(object.get(0x3e)));
  std::int16_t material;
  std::memcpy(&material, cell.bytes.data() + 2, 2);
  if (material != -1) {
    object.put(0x10, 0);
    object.put(0x14, 0);
  }
  const auto direction = object.get(0x56) == config.duration
                             ? particleAdd(object.get(0x7e), 1) % 8
                             : 8;
  object.put(0x7e, direction);
  const auto ordinal = object.get(0x7a);
  std::uint32_t index;
  const std::array<std::uint32_t, 64> *frames;
  std::uint32_t count;
  if (ordinal < animation.primaryOrdinals) {
    index = std::uint32_t(particleAdd(particleMul(ordinal, 9), direction));
    frames = &animation.primary;
    count = animation.primaryFrameCount;
  } else {
    const auto period = animation.fallbackB ? animation.fallbackPeriodB
                                            : animation.fallbackPeriodA;
    const auto remainder =
        particleDivide(particleSub(ordinal, animation.primaryOrdinals), period);
    const auto mod =
        particleSub(particleSub(ordinal, animation.primaryOrdinals),
                    particleMul(remainder, period));
    index = std::uint32_t(particleAdd(particleMul(mod, 9), direction));
    frames = animation.fallbackB ? &animation.fallback : &animation.fallbackA;
    count = animation.fallbackB ? animation.fallbackFrameCountB
                                : animation.fallbackFrameCountA;
  }
  if (count < index)
    index = 0; // Original strict comparison also admits index==count.
  if (index >= frames->size())
    throw std::invalid_argument(
        "Original particle frame table escaped owned storage");
  object.put(0x5a, particleSigned(frames->at(index)));
  return true;
}
inline bool updateGravityParticle(ParticleObject &object, ParticleWorld world) {
  const auto alive = updateParticleMotion(object, world);
  object.put(24, particleSub(object.get(24), 1));
  return alive && object.get(12) > 0;
}
} // namespace mnm::reconstruction
