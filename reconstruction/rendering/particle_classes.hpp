#pragma once
#include "particle_children.hpp"

namespace mnm::reconstruction {
struct OriginalParticleClass {
  std::uint32_t table;
  std::array<std::uint32_t, 6>
      slots; // deleting,width,height,draw,update,priority
  bool abstract;
};
inline constexpr std::array<OriginalParticleClass, 16> originalParticleClasses{
    {{0x5c73a8,
      {0x54aa10, 0x54a9e0, 0x54a9f0, 0x59b823, 0x59b823, 0x54aa00},
      true},
     {0x5c73c0,
      {0x545690, 0x54a9e0, 0x54a9f0, 0x59b823, 0x5456c0, 0x54aa00},
      true},
     {0x5c73d8,
      {0x5457b0, 0x54a9e0, 0x54a9f0, 0x59b823, 0x54aa30, 0x54aa00},
      true},
     {0x5c73f0,
      {0x545e60, 0x54aa60, 0x54aa70, 0x54aaa0, 0x546120, 0x54aa80},
      false},
     {0x5c7408,
      {0x546270, 0x54aa60, 0x54aa70, 0x546540, 0x546350, 0x54aa80},
      false},
     {0x5c7420,
      {0x546630, 0x54aac0, 0x54aad0, 0x54ab00, 0x5468a0, 0x54aae0},
      false},
     {0x5c7438,
      {0x546cb0, 0x54a9e0, 0x54a9f0, 0x54ac30, 0x546ce0, 0x54aa00},
      false},
     {0x5c7450,
      {0x547280, 0x54a9e0, 0x54a9f0, 0x54ac40, 0x547380, 0x54ac80},
      false},
     {0x5c7470,
      {0x547340, 0x54a9e0, 0x54a9f0, 0x54ac40, 0x547380, 0x54ac80},
      false},
     {0x5c7494,
      {0x548600, 0x54ab20, 0x54ab30, 0x54ab60, 0x54aa30, 0x54aa00},
      false},
     {0x5c74b0,
      {0x5487f0, 0x54ab20, 0x54ab30, 0x54ab60, 0x548860, 0x548840},
      false},
     {0x5c74cc,
      {0x548b90, 0x54a9e0, 0x54a9f0, 0x54ac00, 0x548bc0, 0x54ac10},
      false},
     {0x5c74e4,
      {0x5491e0, 0x54a9e0, 0x54a9f0, 0x5492b0, 0x549380, 0x54acc0},
      false},
     {0x5c74fc,
      {0x5499d0, 0x54a9e0, 0x54a9f0, 0x549730, 0x549820, 0x54acd0},
      false},
     {0x5c7514,
      {0x549ad0, 0x54a9e0, 0x54a9f0, 0x549ba0, 0x549c90, 0x54ace0},
      false},
     {0x5c752c,
      {0x549e30, 0x54ad10, 0x54ad20, 0x54a4a0, 0x54a140, 0x54acf0},
      false}}};
inline const OriginalParticleClass &originalParticleClass(std::uint32_t table) {
  for (const auto &type : originalParticleClasses)
    if (type.table == table)
      return type;
  throw std::invalid_argument("Unknown original particle virtual table");
}
inline std::int32_t
originalParticleProperty(std::uint32_t method, const ParticleObject &object,
                         std::int32_t duration,
                         std::array<std::int32_t, 2> frameSize = {}) {
  switch (method) {
  case 0x54a9e0:
  case 0x54a9f0:
    return 4;
  case 0x54aa00:
    return 1024;
  case 0x54aa60:
  case 0x54aad0:
    return object.get(0x34);
  case 0x54aa70:
    return object.get(0x38);
  case 0x54aac0:
    return object.get(0x30);
  case 0x54ab20:
    return object.get(0x20);
  case 0x54ab30:
    return object.get(0x24);
  case 0x54aa80:
    return particleDivide(
        particleMul(particleDivide(object.get(0x28), object.get(0x2c)), 1024),
        20);
  case 0x54aae0:
    return particleAdd(particleDivide(particleMul(object.get(0x2c), 1024), 4),
                       object.get(0x28));
  case 0x548840:
    return particleAdd(particleSub(object.get(0x56), duration), 1024);
  case 0x54ac10:
    return particleMul(
        particleMul(
            particleDivide(
                particleSub(particleMul(object.get(0x14), 2),
                            particleMul(particleSub(object.get(0x10), 1),
                                        object.get(0x18))),
                2),
            object.get(0x10)),
        1024);
  case 0x54ac80:
    return 0x200000;
  case 0x54acc0:
    return 0x100000;
  case 0x54acd0:
  case 0x54ace0:
    return particleAdd(object.get(0x14), 1024);
  case 0x54acf0:
    return particleSub(
        particleAdd(particleMul(particleAdd(object.get(0x5e), 1), 1024),
                    object.get(0x56)),
        duration);
  case 0x54ad10:
    return object.get(0x5a) ? frameSize[0] : 0;
  case 0x54ad20:
    return object.get(0x5a) ? frameSize[1] : 0;
  default:
    throw std::invalid_argument("Unrecovered particle virtual property");
  }
}
inline void
setOriginalParticleParameters(ParticleObject &object,
                              const std::array<std::int32_t, 6> &values) {
  for (unsigned i = 0; i < 6; ++i)
    object.put(0x24 + i * 4, values[i]);
}
inline std::uint16_t drawNonRasterParticle(std::uint32_t method,
                                           ParticleObject &object) {
  if (method == 0x54ac00) {
    object.bytes[0x40] = 1;
    return 0;
  }
  if (method == 0x54ac30 || method == 0x54ac40)
    return 0;
  throw std::invalid_argument("Particle method requires a raster backend");
}
} // namespace mnm::reconstruction
