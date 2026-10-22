#include "../reconstruction/rendering/particle_publication.hpp"
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  const auto pointer = [](const void *p) {
    return U(reinterpret_cast<std::uintptr_t>(p));
  };
  std::vector<std::pair<U, Bytes>> unchanged;
  for (auto range : std::vector<std::pair<U, U>>{{0x545990, 0x545adc},
                                                 {0x4f7ff0, 0x4f80be},
                                                 {0x4f80f0, 0x4f8212},
                                                 {0x4ffd30, 0x4fff49}}) {
    const auto *p = reinterpret_cast<const unsigned char *>(range.first);
    unchanged.push_back(
        {range.first, Bytes(p, p + range.second - range.first)});
  }
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native inputs");
#endif
  U id = 0;
  for (U view = 0; view < 4; ++view)
    for (U cameraCase = 0; cameraCase < 3; ++cameraCase)
      for (U clipping = 0; clipping < 5; ++clipping)
        for (U visibility = 0; visibility < 5; ++visibility)
          for (U capacity : {0u, 1u, 3u, 8u})
            for (U initial : {0u, 1u})
              for (U population : {0u, 1u, 4u, 13u})
                for (bool framePresent : {false, true}) {
                  ParticleCamera camera;
                  camera.view = view;
                  camera.centreX = cameraCase == 0 ? 0 : 3;
                  camera.centreY = cameraCase == 0 ? 0 : 2;
                  camera.offsetX = int(cameraCase) - 1;
                  camera.offsetY = 1 - int(cameraCase);
                  camera.offsetZ = int(cameraCase) * 2 - 1;
                  camera.viewportWidth = cameraCase == 2 ? -65 : 65;
                  camera.viewportHeight = 49;
                  camera.left = -7;
                  camera.top = 3;
                  const ConsumerClip clips[]{{-512, -512, 512, 512},
                                             {-8, -8, 24, 24},
                                             {0, 0, 32, 32},
                                             {32, 24, 64, 48},
                                             {100, 100, 100, 100}};
                  ParticleTerrainFacts terrain;
                  for (U i = 0; i < 48; ++i)
                    terrain.cells[0][i].bytes[6] =
                        static_cast<unsigned char>(i % 4);
                  std::vector<ParticleObject> objects(population);
                  for (U i = 0; i < population; ++i) {
                    auto &o = objects[i];
                    o.bytes.fill(0xa5);
                    o.put(0, originalParticleClasses[3 + i].table);
                    o.put(4, int((i * 31) % 128));
                    o.put(8, int((i * 23) % 96));
                    o.put(12, int((i * 17) % 128));
                    o.put(0x20, i % 2 ? 3 : -3);
                    o.put(0x24, i % 3 ? 4 : -4);
                    o.put(0x30, i % 2 ? 2 : -2);
                    o.put(0x34, i % 2 ? 3 : -3);
                    o.put(0x38, i % 3 ? 4 : -4);
                    o.put(0x5a, framePresent ? 0x4000 : 0);
                  }
                  ParticlePublicationQueue queue;
                  for (auto &row : queue.records)
                    row.fill(0xa5a5a5a5);
                  queue.capacity = capacity;
                  queue.count = std::min(initial, capacity);
                  const auto originalObjects = objects;
#ifdef MNM_ORIGINAL_REFERENCE
                  U frame[]{0, 5, 7, 0}, world[]{0, 4, 3};
                  global(0x689930 + 0x25, pointer(world));
                  global(0x689930 + 0x31, view);
                  global(0x689930 + 0x39, U(camera.centreX));
                  global(0x689930 + 0x3d, U(camera.centreY));
                  global(0x689930 + 0x41, U(camera.layer));
                  global(0x689930 + 0x45, U(camera.offsetX));
                  global(0x689930 + 0x49, U(camera.offsetY));
                  global(0x689930 + 0x4d, U(camera.offsetZ));
                  global(0x689930 + 0x11, U(camera.left));
                  global(0x689930 + 0x15, U(camera.top));
                  global(0x689930 + 0x51, U(camera.viewportWidth));
                  global(0x689930 + 0x55, U(camera.viewportHeight));
                  global(0x689930 + 0x5d, U(camera.radius));
                  global(0x6c549c, 4);
                  global(0x6c54dc, pointer(terrain.cells[0].data()));
                  global(0x689995, visibility);
                  for (U i = 0; i < 4; ++i)
                    global(0x6cb8c2 + i * 4, i * 12);
                  for (U i = 0; i < 3; ++i)
                    global(0x6cb942 + i * 4, i * 4);
                  global(0x6e0008, U(clips[clipping].left));
                  global(0x6cbb6c, U(clips[clipping].top));
                  global(0x6a49b8, U(clips[clipping].right));
                  global(0x656618, U(clips[clipping].bottom));
                  global(0x5e41ac, 1);
                  for (U i = 0; i < 500; ++i)
                    global(0x6e818c + i * 4, i * 82 / 100);
                  global(0x6e8db0, 0);
                  global(0x689b59, pointer(queue.records.data()));
                  global(0x689b5d,
                         pointer(queue.records.data()) + queue.count * 36);
                  global(0x689b61, queue.count);
                  global(0x689b65, queue.capacity);
                  global(0x689b69, 0);
                  global(0x689b6d, view);
                  U pool[1794]{};
                  for (U i = 0; i < population; ++i) {
                    pool[i] = pointer(objects[i].bytes.data());
                    if (framePresent)
                      objects[i].put(0x5a, particleSigned(pointer(frame)));
                  }
                  pool[1792] = population;
                  using Publish = void(__attribute__((thiscall)) *)(void *);
                  reinterpret_cast<Publish>(0x545990)(pool);
                  queue.count = *reinterpret_cast<U *>(0x689b61);
                  queue.attempts = *reinterpret_cast<U *>(0x6e8db0);
                  if (*reinterpret_cast<U *>(0x689b5d) !=
                      pointer(queue.records.data()) + queue.count * 36)
                    throw std::runtime_error("Original queue cursor");
                  for (U row = std::min(initial, capacity); row < queue.count;
                       ++row) {
                    bool found = false;
                    for (U i = 0; i < population; ++i)
                      if (queue.records[row][1] ==
                          pointer(objects[i].bytes.data())) {
                        queue.records[row][1] = i + 1;
                        found = true;
                        break;
                      }
                    if (!found)
                      throw std::runtime_error("Unknown published owner");
                  }
                  for (auto &o : objects)
                    if (framePresent)
                      o.put(0x5a, 0x4000);
#else
                  publishOriginalParticles(objects, terrain, camera,
                                           clips[clipping], visibility, {5, 7},
                                           queue);
#endif
                  if (std::memcmp(objects.data(), originalObjects.data(),
                                  objects.size() * sizeof(ParticleObject)))
                    throw std::runtime_error(
                        "Publication mutated particle bytes");
                  const U h[]{id++,        view,          cameraCase,
                              clipping,    visibility,    capacity,
                              initial,     population,    U(framePresent),
                              queue.count, queue.attempts};
                  output(h, sizeof(h));
                  output(queue.records.data(), sizeof(queue.records));
                }
#ifdef MNM_ORIGINAL_REFERENCE
  for (const auto &[address, body] : unchanged)
    if (std::memcmp(reinterpret_cast<void *>(address), body.data(),
                    body.size()))
      throw std::runtime_error("Original publication bodies changed");
#endif
  if (id != 19200)
    throw std::runtime_error("Publication matrix denominator");
  return std::cout ? 0 : 1;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
