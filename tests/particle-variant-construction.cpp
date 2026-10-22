#include "../reconstruction/rendering/particle_constructors.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#include <cstdlib>
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
#ifdef MNM_ORIGINAL_REFERENCE
static void *allocatedPlane;
static U allocations, frees, requested;
extern "C" __attribute__((force_align_arg_pointer)) void *
allocate_variant(U size) {
  if (allocatedPlane || size > 1536 || !size)
    _exit(70);
  allocatedPlane = std::malloc(size);
  if (!allocatedPlane)
    _exit(71);
  std::memset(allocatedPlane, 0xcd, size);
  requested = size;
  ++allocations;
  return allocatedPlane;
}
extern "C" __attribute__((force_align_arg_pointer)) void
free_variant(void *memory) {
  if (memory != allocatedPlane || !memory)
    _exit(72);
  std::free(memory);
  allocatedPlane = nullptr;
  ++frees;
}
static void boundary(U address, void *target, const unsigned char *expected) {
  auto *entry = reinterpret_cast<unsigned char *>(address);
  if (std::memcmp(entry, expected, 6))
    throw std::runtime_error("Allocator entry signature");
  const U relative = U(reinterpret_cast<std::uintptr_t>(target)) - address - 5;
  entry[0] = 0xe9;
  std::memcpy(entry + 1, &relative, 4);
  entry[5] = 0x90;
}
#endif
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  OriginalLinuxSeh seh;
  const unsigned char allocSignature[]{0x8b, 0x44, 0x24, 0x04, 0x6a, 0},
      freeSignature[]{0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0};
  boundary(0x597850, reinterpret_cast<void *>(&allocate_variant),
           allocSignature);
  boundary(0x597870, reinterpret_cast<void *>(&free_variant), freeSignature);
  std::uint16_t control = 0x037f;
  __asm__ volatile("fldcw %0" ::"m"(control));
  std::vector<std::pair<U, Bytes>> original;
  for (auto range : std::vector<std::pair<U, U>>{{0x545e90, 0x546120},
                                                 {0x546660, 0x5468a0},
                                                 {0x546c20, 0x546cb0},
                                                 {0x547210, 0x547380},
                                                 {0x5491b0, 0x5492b0},
                                                 {0x549900, 0x549ba0},
                                                 {0x549d70, 0x549e80},
                                                 {0x548650, 0x548860},
                                                 {0x547890, 0x547940},
                                                 {0x547b70, 0x547e00},
                                                 {0x54a5f0, 0x54a680}}) {
    const auto *p = reinterpret_cast<const unsigned char *>(range.first);
    original.push_back({range.first, Bytes(p, p + range.second - range.first)});
  }
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native arguments");
#endif
  const U seeds[]{0, 1, 0x12345678, 0xffffffff, 0x80000000, 0xdeadbeef};
  const std::array<int, 2> positions[]{{-129, 257}, {0, 0}, {127, 95}};
  const int heights[]{0, 15, 63, 64, 129};
  const int phases[]{-1, 0, 1, 17, 31, 100}, speeds[]{1, 2, 16, 3, 9, 32},
      spreads[]{1, 2, 17, 31, 7, 128};
  U id = 0;
  for (U type = 0; type < 9; ++type)
    for (U configIndex = 0; configIndex < 6; ++configIndex)
      for (U pos = 0; pos < 3; ++pos)
        for (U zi = 0; zi < 5; ++zi)
          for (bool format : {false, true})
            for (U seed : seeds)
              for (int dimension : {-1, 2, 17})
                for (bool enabled : {false, true}) {
                  ParticleObject object;
                  for (U i = 0; i < object.bytes.size(); ++i)
                    object.bytes[i] = unsigned(i * 17 + 13);
                  ParticleConstructorConfig config;
                  config.additivePhase = phases[configIndex];
                  config.additiveSpeed = speeds[configIndex];
                  config.emitterPeriod = phases[configIndex];
                  config.ownerEmitter = {int(configIndex) - 2,
                                         int(configIndex) * 3, 5, -7};
                  config.lifetimeSpread = spreads[configIndex];
                  config.duration = spreads[configIndex];
                  config.effectCount = phases[configIndex];
                  config.dimension = dimension;
                  config.planeEnabled = enabled;
                  config.rgb555 = format;
                  ParticleConstructorArguments in;
                  in.position = {positions[pos][0], positions[pos][1],
                                 heights[zi]};
                  in.velocity = {-7, 9, -1};
                  in.emitterParameters = {int(configIndex), 9, -10, 3};
                  in.lifetime = configIndex % 2 ? INT32_MAX : -7;
                  in.category = configIndex % 2 ? 13 : 4;
                  in.ordinal = int(configIndex);
                  in.palette = configIndex % 2;
                  ParticleOwnerFacts owner{true,
                                           true,
                                           {-12, 17, 7},
                                           configIndex % 2 ? -31 : 3,
                                           configIndex % 2 ? -9 : 5,
                                           1};
                  ParticleTerrainFacts terrain;
                  for (U gen = 0; gen < 2; ++gen)
                    for (U index = 0; index < 48; ++index) {
                      terrain.cells[gen][index].block = index % 3;
                      terrain.shades[gen][index] =
                          std::int8_t((index * 37) & 255);
                    }
                  Kind8Random random(seed);
                  std::vector<std::uint16_t> plane;
                  ParticleObject constructed, retired;
                  U allocationCount, freeCount, requestedSize;
#ifdef MNM_ORIGINAL_REFERENCE
                  const auto ptr = [](const void *p) {
                    return U(reinterpret_cast<std::uintptr_t>(p));
                  };
                  global(0x5eb480, 128);
                  global(0x5eb484, 96);
                  global(0x5eb488, 64);
                  global(0x6e1f88, format);
                  global(0x5e1464, config.additivePhase);
                  global(0x5e1468, config.additiveSpeed);
                  global(0x5e147c, config.emitterPeriod);
                  for (U i = 0; i < 4; ++i)
                    global(0x5e149c + i * 4, config.ownerEmitter[i]);
                  global(0x6dbe99, config.lifetimeSpread);
                  global(0x5e1428, config.duration);
                  global(0x5e142c, dimension);
                  global(0x5e1430, enabled);
                  global(0x5e1424, config.effectCount);
                  global(0x6c54dc, ptr(terrain.cells[0].data()));
                  global(0x6c5c5c, ptr(terrain.shades[0].data()));
                  global(0x65660c, ptr(terrain.descriptors.data()));
                  for (U i = 0; i < 4; ++i)
                    global(0x6cb8c2 + i * 4, i * 12);
                  for (U i = 0; i < 3; ++i)
                    global(0x6cb942 + i * 4, i * 4);
                  U resource[4]{0, 0, 0, 9};
                  global(0x6f1d58 + in.category * 36, ptr(resource));
                  U actor[72]{}, descriptor[104]{};
                  actor[0xac / 4] = ptr(descriptor);
                  descriptor[0x198 / 4] = U(owner.horizontalOffset);
                  descriptor[0x19c / 4] = U(owner.verticalOffset);
                  for (U axis = 0; axis < 3; ++axis)
                    actor[0x14 / 4 + axis] = U(owner.position[axis]);
                  global(0x6574c8, random.first);
                  global(0x6574cc, random.second);
                  global(0x6f2a9c, random.lcg);
                  std::memcpy(reinterpret_cast<void *>(0x6574d0),
                              random.words.data(), 1000);
                  allocations = frees = requested = 0;
                  if (allocatedPlane)
                    throw std::runtime_error(
                        "Prior constructor allocation leaked");
                  using Ctor3 = void *(__attribute__((thiscall)) *)(void *, int,
                                                                    int, int);
                  using Ctor7 =
                      void *(__attribute__((thiscall)) *)(void *, int, int, int,
                                                          int, int, int, int);
                  using Ctor5 =
                      void *(__attribute__((thiscall)) *)(void *, int, int, int,
                                                          void *, int);
                  using Ctor9 = void *(
                      __attribute__((thiscall)) *)(void *, int, int, int, int,
                                                   int, int, int, int, U);
                  const auto &p = in.position;
                  void *result = nullptr;
                  const U entries[]{0x545e90, 0x546660, 0x546c20,
                                    0x547210, 0x5472c0, 0x5491b0,
                                    0x549900, 0x549a00, 0x549d70};
                  if (type == 4) {
                    const auto &a = in.emitterParameters;
                    result = reinterpret_cast<Ctor7>(entries[type])(
                        object.bytes.data(), p[0], p[1], p[2], a[0], a[1], a[2],
                        a[3]);
                  } else if (type == 6 || type == 7)
                    result = reinterpret_cast<Ctor5>(entries[type])(
                        object.bytes.data(), p[0], p[1], p[2], actor,
                        in.lifetime);
                  else if (type == 8)
                    result = reinterpret_cast<Ctor9>(entries[type])(
                        object.bytes.data(), in.category, in.ordinal, p[0],
                        p[1], p[2], -7, 9, -1, in.palette);
                  else
                    result = reinterpret_cast<Ctor3>(entries[type])(
                        object.bytes.data(), p[0], p[1], p[2]);
                  if (result != object.bytes.data())
                    throw std::runtime_error(
                        "Constructor receiver return changed");
                  seh.check();
                  constructed = object;
                  auto normalize = [&](ParticleObject &state) {
                    if (type == 6 || type == 7) {
                      if (U(state.get(0x10)) != ptr(actor))
                        throw std::runtime_error("Owner identity");
                      state.put(0x10, particleSigned(in.ownerToken));
                    }
                    if (type == 8) {
                      if (state.get(12) < 64) {
                        const U x = U(state.get(4)) >> 5,
                                y = U(state.get(8)) >> 5, z = U(state.get(12));
                        const U index = (z >> 4) * 12 + y * 4 + x;
                        if (U(state.get(0x3e)) != ptr(&terrain.cells[0][index]))
                          throw std::runtime_error("Constructor cell identity");
                        state.put(0x3e, index + 1);
                        if (U(state.get(0x42)) !=
                            ptr(terrain
                                    .descriptors[terrain.cells[0][index].block]
                                    .data()))
                          throw std::runtime_error(
                              "Constructor descriptor identity");
                        state.put(0x42, 0x2000 + terrain.cells[0][index].block);
                      }
                      if (state.get(0x2e)) {
                        if (U(state.get(0x2e)) != ptr(allocatedPlane))
                          throw std::runtime_error(
                              "Constructor plane identity");
                        state.put(0x2e, 0x3000);
                      }
                    }
                  };
                  normalize(constructed);
                  if (allocatedPlane)
                    plane.assign(static_cast<std::uint16_t *>(allocatedPlane),
                                 static_cast<std::uint16_t *>(allocatedPlane) +
                                     requested / 2);
                  const U table = U(object.get(0));
                  using Destroy =
                      void *(__attribute__((thiscall)) *)(void *, U);
                  if (reinterpret_cast<Destroy>(*reinterpret_cast<U *>(table))(
                          object.bytes.data(), 0) != object.bytes.data())
                    throw std::runtime_error("Nondeleting receiver return");
                  seh.check();
                  retired = object;
                  normalize(retired);
                  allocationCount = allocations;
                  freeCount = frees;
                  requestedSize = requested;
                  if (allocatedPlane || allocations != frees)
                    throw std::runtime_error("Constructor plane not retired");
                  random.first = *reinterpret_cast<U *>(0x6574c8);
                  random.second = *reinterpret_cast<U *>(0x6574cc);
                  random.lcg = *reinterpret_cast<U *>(0x6f2a9c);
                  std::memcpy(random.words.data(),
                              reinterpret_cast<void *>(0x6574d0), 1000);
#else
                  plane = constructParticleVariant(
                      ParticleVariant(type), object, {128, 96, 64}, in, config,
                      owner, terrain, random);
                  constructed = object;
                  auto livePlane = plane;
                  destroyParticleVariantNonDeleting(ParticleVariant(type),
                                                    object, livePlane);
                  retired = object;
                  allocationCount = freeCount = !plane.empty();
                  requestedSize = plane.size() * 2;
#endif
                  const U header[]{id++,
                                   type,
                                   configIndex,
                                   pos,
                                   zi,
                                   U(format),
                                   seed,
                                   U(dimension),
                                   U(enabled),
                                   U(plane.size()),
                                   allocationCount,
                                   freeCount};
                  std::array<std::uint16_t, 768> words{};
                  std::copy(plane.begin(), plane.end(), words.begin());
                  output(header, sizeof(header));
                  output(constructed.bytes.data(), 160);
                  output(retired.bytes.data(), 160);
                  output(&random, sizeof(random));
                  output(words.data(), 1536);
                  output(&requestedSize, 4);
                }
  if (id != 58320)
    throw std::runtime_error("Variant denominator " + std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
  for (const auto &range : original)
    if (std::memcmp(reinterpret_cast<const void *>(range.first),
                    range.second.data(), range.second.size()))
      throw std::runtime_error(
          "Original constructor/shape/resource body changed");
#endif
  return 0;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
