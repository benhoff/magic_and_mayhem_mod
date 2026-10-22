#pragma once
#include "particle_constructors.hpp"
#include <cmath>

namespace mnm::reconstruction {
struct ParticleResourcePreparation {
  bool loaded{};
  std::uint32_t frameCount{};
  std::int32_t pieces{};
};
// The loader owns publication of a successful resource. A failed request leaves
// the prior piece count intact; palette requests are ordered and unchecked.
template <class Loader>
inline void prepareParticleResource(ParticleResourcePreparation &resource,
                                    unsigned identity, Loader &loader) {
  if (resource.loaded || !loader.load(identity, resource))
    return;
  resource.pieces = particleSigned(resource.frameCount / 9);
  for (unsigned palette : {1u, 2u, 6u, 4u, 5u, 7u})
    loader.palette(identity, palette);
}
struct ParticleFactoryArguments {
  std::int32_t category{4};
  std::array<std::int32_t, 3> position{31, 31, 15};
  std::array<std::int32_t, 3> rgb{255, 127, 65};
  std::int32_t extra{4};
  std::uint32_t palette{};
  // Owned finite values in the original float-to-integer conversion domain.
  std::vector<std::array<float, 3>> velocities;
};
template <class Sink>
inline void createFadingParticles(const ParticleFactoryArguments &in,
                                  Kind8Random &random, Sink &sink) {
  if (in.category == 0 || (in.category >= 24 && in.category <= 26))
    return;
  sink.prepare(0);
  sink.prepare(1);
  sink.prepare(2);
  const auto count =
      std::max(sink.pieces(), static_cast<std::int32_t>(in.velocities.size()));
  std::size_t remaining = in.velocities.size();
  for (std::int32_t ordinal = 0; ordinal < count; ++ordinal) {
    std::array<std::int32_t, 3> velocity;
    if (remaining) {
      const auto &source = in.velocities[--remaining];
      for (unsigned axis = 0; axis < 3; ++axis) {
        const double v = source[axis];
        if (!std::isfinite(v) || v < INT32_MIN || v > INT32_MAX)
          throw std::invalid_argument("Owned factory float conversion domain");
        velocity[axis] = static_cast<std::int32_t>(v);
      }
    } else {
      auto x = static_cast<std::int32_t>((random.next() & 0x7fffffffu) % 6 + 2);
      if (random.next() & 1)
        x = -x;
      auto y = static_cast<std::int32_t>((random.next() & 0x7fffffffu) % 6 + 2);
      if (random.next() & 1)
        y = -y;
      velocity = {x, y, static_cast<std::int32_t>((random.next() & 7) + 5)};
    }
    const auto token = sink.allocate(0x82);
    if (!token)
      continue;
    sink.construct(token, in, ordinal, velocity, random);
    sink.prepare(2);
    auto &object = sink.object(token);
    for (unsigned axis = 0; axis < 3; ++axis)
      object.put(0x62 + axis * 4, in.rgb[axis]);
    object.put(0x6e, in.extra);
    if (!sink.insert(token))
      sink.destroy(token);
  }
}
} // namespace mnm::reconstruction
