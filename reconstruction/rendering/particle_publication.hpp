#pragma once
#include "particle_classes.hpp"
#include "sprite_queue.hpp"
#include "world_consumer.hpp"

namespace mnm::reconstruction {
struct ParticleCamera {
  std::uint32_t view{};
  std::int32_t width{4}, height{3}, centreX{}, centreY{}, layer{1};
  std::int32_t offsetX{}, offsetY{}, offsetZ{}, left{}, top{},
      viewportWidth{64}, viewportHeight{48}, radius{1};
};
inline std::array<std::int32_t, 2>
projectOriginalParticle(const ParticleCamera &c, const ParticleObject &object) {
  if (c.view > 3 || c.width <= 0 || c.height <= 0)
    throw std::invalid_argument("Owned particle camera domain");
  auto dx = particleSub(particleAdd(particleAdd(c.centreX * 32, c.offsetX), 16),
                        object.get(4));
  auto dy = particleSub(particleAdd(particleAdd(c.centreY * 32, c.offsetY), 16),
                        object.get(8));
  const auto wrap = [](std::int32_t value, std::int32_t period) {
    if (value >= period / 2)
      value = particleSub(value, period);
    if (value < -period / 2)
      value = particleAdd(value, period);
    return value;
  };
  dx = wrap(dx, c.width * 32);
  dy = wrap(dy, c.height * 32);
  auto x = particleAdd(c.left, c.viewportWidth / 2);
  auto y = particleAdd(
      particleAdd(c.top, c.viewportHeight / 2),
      particleSub(particleAdd(c.layer * 16, c.offsetZ), object.get(12)));
  const auto sum = particleAdd(dx, dy), difference = particleSub(dx, dy);
  switch (c.view) {
  case 0:
    x = particleSub(x, difference);
    y = particleSub(y, sum / 2);
    break;
  case 1:
    x = particleSub(x, sum);
    y = particleAdd(y, difference / 2);
    break;
  case 2:
    x = particleAdd(x, difference);
    y = particleAdd(y, sum / 2);
    break;
  default:
    x = particleAdd(x, sum);
    y = particleSub(y, difference / 2);
    break;
  }
  return {x, y};
}
inline std::int32_t particlePublicationPriority(const ParticleCamera &c,
                                                std::int32_t x,
                                                std::int32_t y) {
  auto cx = c.centreX + ((c.view == 1 || c.view == 2) ? c.radius : -c.radius);
  auto cy = c.centreY + (c.view >= 2 ? c.radius : -c.radius);
  if (cx < 0)
    cx += c.width;
  else if (cx >= c.width)
    cx -= c.width;
  if (cy < 0)
    cy += c.height;
  else if (cy >= c.height)
    cy -= c.height;
  const auto dx = cx - x, dy = cy - y;
  return (((c.view == 0 || c.view == 3) ? dx > 0 : dx < 0) ? c.width * 32 : 0) +
         ((c.view < 2 ? dy > 0 : dy < 0) ? c.height * 32 : 0);
}
struct ParticlePublicationQueue {
  std::array<std::array<std::uint32_t, 9>, 8> records;
  std::uint32_t count{}, capacity{}, attempts{};
};
inline void
publishOriginalParticles(const std::vector<ParticleObject> &objects,
                         const ParticleTerrainFacts &terrain,
                         const ParticleCamera &camera, const ConsumerClip &clip,
                         std::uint32_t visibility,
                         const std::array<std::int32_t, 2> &frameSize,
                         ParticlePublicationQueue &queue) {
  if (objects.size() > 1792 || queue.capacity > queue.records.size() ||
      queue.count > queue.capacity)
    throw std::invalid_argument("Owned particle publication extent");
  for (auto cursor = objects.size(); cursor;) {
    const auto &object = objects[--cursor];
    const auto x = std::uint32_t(object.get(4)) >> 5,
               y = std::uint32_t(object.get(8)) >> 5,
               z = std::uint32_t(object.get(12)) >> 4;
    if (x >= 4 || y >= 3)
      throw std::invalid_argument("Owned publication terrain coordinate");
    const auto &cell = terrain.cells[0][std::min(z, 3u) * 12 + y * 4 + x];
    if ((cell.bytes[6] & 3u) > visibility)
      continue;
    const auto xy = projectOriginalParticle(camera, object);
    const auto &type = originalParticleClass(std::uint32_t(object.get(0)));
    const auto width =
        originalParticleProperty(type.slots[1], object, 100, frameSize);
    if (particleAdd(xy[0], width) < clip.left || xy[0] >= clip.right)
      continue;
    const auto height =
        originalParticleProperty(type.slots[2], object, 100, frameSize);
    if (particleAdd(xy[1], height) < clip.top || xy[1] >= clip.bottom)
      continue;
    const auto priority = particlePublicationPriority(camera, int(x), int(y));
    ++queue.attempts;
    if (queue.count >= queue.capacity)
      continue;
    auto &row = queue.records[queue.count++];
    row[0] = std::uint32_t(spriteDepthKey(
        {int(x * 32 + 16), int(y * 32 + 16), int(z * 16 + 1), priority},
        camera.view));
    row[1] = std::uint32_t(cursor + 1);
    row[2] = std::uint32_t(xy[0]);
    row[3] = std::uint32_t(xy[1]);
    row[4] = 0;
    row[5] = 0;
    row[6] = 8;
    row[7] = 0x8ad08ad0;
    // +32 is intentionally untouched by4ffd30.
  }
}
} // namespace mnm::reconstruction
