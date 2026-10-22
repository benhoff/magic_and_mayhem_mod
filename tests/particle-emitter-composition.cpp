#include "../reconstruction/rendering/particle_children.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
#include "particle-emitter-arena.hpp"
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  OriginalLinuxSeh seh;
  const unsigned char allocation[]{0x8b, 0x44, 0x24, 0x04, 0x6a, 0},
      release[]{0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0},
      insert[]{0x51, 0x53, 0x55, 0x56, 0x8b, 0xf1};
  emission_boundary(0x597850, reinterpret_cast<void *>(&emission_allocate),
                    allocation);
  emission_boundary(0x597870, reinterpret_cast<void *>(&emission_free),
                    release);
  emission_boundary(0x5457e0, reinterpret_cast<void *>(&emission_insert),
                    insert);
  std::uint16_t control = 0x037f;
  __asm__ volatile("fldcw %0" ::"m"(control));
  std::vector<std::pair<U, Bytes>> original;
  for (auto range : std::vector<std::pair<U, U>>{{0x5468a0, 0x546c20},
                                                 {0x546ce0, 0x547210},
                                                 {0x547380, 0x547890},
                                                 {0x548bc0, 0x5491b0},
                                                 {0x54a140, 0x54a4a0},
                                                 {0x54aa30, 0x54aa60},
                                                 {0x546660, 0x5468a0},
                                                 {0x548650, 0x548860},
                                                 {0x547890, 0x547940},
                                                 {0x547b70, 0x547e00}}) {
    const auto *p = reinterpret_cast<const unsigned char *>(range.first);
    original.push_back({range.first, Bytes(p, p + range.second - range.first)});
  }
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native inputs");
#endif
  const U seeds[]{0, 1, 0xdeadbeef};
  const int counts[]{0, 1, 3};
  U id = 0;
  for (U type = 0; type < 7; ++type)
    for (U configIndex = 0; configIndex < 3; ++configIndex)
      for (U state = 0; state < 3; ++state)
        for (U ownerState = 0; ownerState < 3; ++ownerState)
          for (bool flag : {false, true})
            for (U failAllocate = 0; failAllocate < 4; ++failAllocate)
              for (U failInsert = 0; failInsert < 3; ++failInsert)
                for (bool format : {false, true})
                  for (U seed : seeds)
                    for (bool enabled : {false, true}) {
                      Kind8Random random(seed);
                      ParticleTerrainFacts terrain;
                      for (auto &generation : terrain.cells)
                        for (auto &cell : generation) {
                          cell.block = 1;
                          const std::int16_t material =
                              configIndex == 2 ? 0 : -1;
                          std::memcpy(cell.bytes.data() + 2, &material, 2);
                        }
                      for (auto &plane : terrain.shades)
                        plane.fill(configIndex == 0   ? -127
                                   : configIndex == 1 ? -8
                                                      : 127);
                      if (configIndex == 2)
                        terrain.descriptors[1].fill(0xff);
                      ParticleOwnerFacts owner{ownerState != 0,
                                               ownerState == 2,
                                               {-12, 17, 7},
                                               3,
                                               5,
                                               ownerState == 2 ? 0x12345678u
                                                               : 0};
                      ParticleEmissionConfig emission;
                      emission.rgb555 = format;
                      emission.levelLoss = configIndex == 0   ? 0
                                           : configIndex == 1 ? 3
                                                              : 10;
                      emission.colourThreshold = configIndex == 0   ? -1
                                                 : configIndex == 1 ? 50
                                                                    : 100;
                      emission.trails = int(configIndex);
                      emission.childDimensions = {configIndex == 0   ? -1
                                                  : configIndex == 1 ? 2
                                                                     : 17,
                                                  configIndex == 2 ? 0 : 3};
                      ParticleAnimationFacts animation;
                      animation.fallbackB = configIndex == 2;
                      for (U bank = 0; bank < 3; ++bank)
                        for (U i = 0; i < 64; ++i)
                          (bank == 0   ? animation.primary
                           : bank == 1 ? animation.fallbackA
                                       : animation.fallback)[i] =
                              0x4000 + bank * 64 + i;
                      EmissionArena arena(terrain, random);
                      arena.failAllocate = failAllocate;
                      arena.failInsert = failInsert;
                      arena.config.rgb555 = format;
                      arena.config.dimension = 3;
                      arena.config.planeEnabled = enabled;
                      arena.config.duration = 100;
                      ParticleObject object;
                      object.bytes.fill(0xa5);
                      object.put(4, configIndex == 2 ? 127 : 31);
                      object.put(8, configIndex == 2 ? 95 : 31);
                      object.put(12, state == 0 ? 0 : state == 1 ? 15 : 63);
                      object.put(16, -1);
                      object.put(20, 1);
                      object.put(24, -1);
                      unsigned ownerOffset = 0;
                      if (type == 0) {
                        object.put(0, 0x5c7420);
                        object.put(0x1c, 128);
                        object.put(0x20, 128);
                        object.put(0x24, 32);
                        object.put(0x28, state == 0 ? 5 : state == 1 ? 11 : 75);
                        object.put(0x2c, counts[configIndex]);
                        object.bytes[0x44] = flag;
                      } else if (type == 1) {
                        object.put(0, 0x5c7438);
                        object.put(0x10, int(configIndex) + 1);
                        object.put(0x14, counts[state]);
                        object.put(0x18, counts[(state + 1) % 3]);
                        object.put(0x1c, counts[(state + 2) % 3]);
                        object.put(0x20, int(state) + 1);
                        object.put(0x24, flag ? 1 : 0);
                        object.put(0x28, 192);
                        object.put(0x2c, 128);
                        object.put(0x30, 32);
                        object.put(0x34, configIndex ? 4 : 0);
                        object.put(0x38, -3);
                        object.put(0x3c, 2);
                        object.put(0x40, -12);
                        object.put(0x44, configIndex ? 4 : 0);
                        object.put(0x48, configIndex ? 4 : 0);
                        object.put(0x4c, configIndex ? 3 : 0);
                        object.put(0x50, int(configIndex) * 2);
                        ownerOffset = 0x54;
                        for (unsigned axis = 0; axis < 3; ++axis)
                          object.put(0x58 + axis * 4,
                                     owner.present && flag ? 1 : 0);
                      } else if (type == 2 || type == 3) {
                        object.put(0, type == 2 ? 0x5c7450 : 0x5c7470);
                        object.put(0x10, counts[state]);
                        object.put(0x14, 128);
                        object.put(0x18, 64);
                        object.put(0x1c, 32);
                        object.put(0x20, configIndex ? 4 : 0);
                        object.put(0x24, -3);
                        object.put(0x28, 2);
                        object.put(0x2c, type == 2 ? 12 : -12);
                        object.put(0x30, configIndex ? 4 : 0);
                        object.put(0x34, configIndex ? 4 : 0);
                        object.put(0x38, configIndex ? 3 : 0);
                        object.put(0x3c, type == 2 ? 0x5e14b0 : 0x5e1494);
                        object.put(0x40, int(configIndex) * 2);
                        ownerOffset = 0x44;
                        for (unsigned axis = 0; axis < 3; ++axis)
                          object.put(0x48 + axis * 4,
                                     owner.present && flag ? 1 : 0);
                      } else if (type == 4) {
                        object.put(0, 0x5c74cc);
                        object.put(0x10, counts[state]);
                        object.put(0x14, counts[state]);
                        object.put(0x18, int(configIndex) * 2 - 2);
                        object.put(0x1c, -7);
                        object.put(0x20, 9);
                        object.put(0x24, -1);
                        object.put(0x28, configIndex == 0   ? 1
                                         : configIndex == 1 ? 3
                                                            : 6);
                        object.put(0x2c, 224);
                        object.put(0x30, 32);
                        object.put(0x34, 0);
                        object.put(0x38, configIndex ? 4 : 0);
                        ownerOffset = 0x3c;
                        object.bytes[0x40] = flag;
                      } else if (type == 5) {
                        object.put(0, 0x5c752c);
                        object.put(0x1c, 0);
                        object.put(0x20, 2);
                        object.put(0x24, 2);
                        object.put(0x2e, 0);
                        object.put(0x42, 0x2001);
                        object.put(0x46, 0);
                        const std::uint16_t colour[]{24, 0, 0};
                        std::memcpy(object.bytes.data() + 0x28, colour, 6);
                        object.put(0x4a, 192);
                        object.put(0x4e, 0);
                        object.put(0x52, 0);
                        object.put(0x56, state == 0   ? 100
                                         : state == 1 ? 99
                                                      : 1);
                        object.put(0x5a, 0x4000);
                        object.put(0x5e, counts[configIndex]);
                        object.put(0x62, 255);
                        object.put(0x66, 127);
                        object.put(0x6a, 65);
                        object.put(0x6e, flag ? 4 : 0);
                        object.put(0x72, configIndex % 2);
                        object.put(0x76, flag ? 13 : 4);
                        object.put(0x7a, int(configIndex) + int(flag));
                        object.put(0x7e, state == 0 ? -1 : state == 1 ? 0 : 7);
                        const U x = U(object.get(4)) >> 5,
                                y = U(object.get(8)) >> 5,
                                z = U(object.get(12)) >> 4,
                                index = std::min(47u, z * 12 + y * 4 + x);
                        object.put(0x3e, index + 1);
                        for (unsigned axis = 0; axis < 3; ++axis)
                          object.put(0x32 + axis * 4,
                                     flag        ? particleSigned(UINT32_MAX)
                                     : axis == 0 ? x
                                     : axis == 1 ? y
                                                 : z);
                      } else
                        object.put(0, 0x5c7494);
                      if (ownerOffset)
                        object.put(ownerOffset, owner.present ? 0x1010 : 0);
                      ParticleTerrainValidity validity;
                      U result;
#ifdef MNM_ORIGINAL_REFERENCE
                      const auto ptr = [](const void *p) {
                        return U(reinterpret_cast<std::uintptr_t>(p));
                      };
                      originalEmissionArena = &arena;
                      U actor[512]{}, descriptor[104]{};
                      actor[0xac / 4] = ptr(descriptor);
                      actor[0xe4 / 4] = owner.active;
                      actor[0x114 / 4] = owner.keepAlive;
                      actor[0x110 / 4] = owner.keepAlive;
                      actor[0x798 / 4] = owner.keepAlive;
                      descriptor[0x198 / 4] = U(owner.horizontalOffset);
                      descriptor[0x19c / 4] = U(owner.verticalOffset);
                      for (U axis = 0; axis < 3; ++axis)
                        actor[0x14 / 4 + axis] = U(owner.position[axis]);
                      if (ownerOffset && owner.present)
                        object.put(ownerOffset, particleSigned(ptr(actor)));
                      U frameHeaders[3][4]{}, offsets[3][64]{},
                          frames[3][64][10]{};
                      for (U bank = 0; bank < 3; ++bank) {
                        frameHeaders[bank][3] = bank == 2 ? 27 : 18;
                        for (U i = 0; i < 64; ++i) {
                          frames[bank][i][1] = i + 1;
                          frames[bank][i][2] = i + 2;
                          offsets[bank][i] = i * 40;
                        }
                      }
                      if (type == 5) {
                        object.put(
                            0x3e,
                            particleSigned(ptr(
                                &terrain.cells[0][U(object.get(0x3e)) - 1])));
                        object.put(0x42, particleSigned(ptr(
                                             terrain.descriptors[1].data())));
                        object.put(0x5a, particleSigned(ptr(frames[0][0])));
                      }
                      global(0x5eb480, 128);
                      global(0x5eb484, 96);
                      global(0x5eb488, 64);
                      global(0x6e1f88, format);
                      global(0x5e1464, arena.config.additivePhase);
                      global(0x5e1468, emission.additiveSpeed);
                      global(0x5e146c, emission.fallLimit);
                      global(0x5e1470, emission.levelLoss);
                      global(0x5e1474, emission.levelRestore);
                      global(0x5e1424, emission.colourThreshold);
                      global(0x6e1f5c, emission.trails);
                      global(0x5e1428, 100);
                      global(0x5e142c, 3);
                      global(0x5e1430, enabled);
                      global(0x6c54dc, ptr(terrain.cells[0].data()));
                      global(0x6c5c5c, ptr(terrain.shades[0].data()));
                      global(0x65660c, ptr(terrain.descriptors.data()));
                      for (U i = 0; i < 4; ++i)
                        global(0x6cb8c2 + i * 4, i * 12);
                      for (U i = 0; i < 3; ++i)
                        global(0x6cb942 + i * 4, i * 4);
                      global(type == 2 ? 0x5e14b0 : 0x5e1494,
                             U(emission.childDimensions[0]));
                      global((type == 2 ? 0x5e14b0 : 0x5e1494) + 4,
                             U(emission.childDimensions[1]));
                      const U category = type == 5 ? U(object.get(0x76)) : 4;
                      global(0x6f1d58 + category * 36, ptr(frameHeaders[0]));
                      global(0x6f1d60 + category * 36, ptr(offsets[0]));
                      global(0x6f1d64 + category * 36, ptr(frames[0]));
                      global(0x6f2174 + category * 4, 2);
                      global(0x6a5f88 + category * 0x5c9,
                             animation.fallbackB ? 2 : 1);
                      global(0x6f2148, 2);
                      global(0x6f220c, 3);
                      global(0x6f2240, ptr(frameHeaders[1]));
                      global(0x6f2248, ptr(offsets[1]));
                      global(0x6f224c, ptr(frames[1]));
                      global(0x6f2218, ptr(frameHeaders[2]));
                      global(0x6f2220, ptr(offsets[2]));
                      global(0x6f2224, ptr(frames[2]));
                      global(0x6574c8, random.first);
                      global(0x6574cc, random.second);
                      global(0x6f2a9c, random.lcg);
                      std::memcpy(reinterpret_cast<void *>(0x6574d0),
                                  random.words.data(), 1000);
                      const U entries[]{0x5468a0, 0x546ce0, 0x547380, 0x547380,
                                        0x548bc0, 0x54a140, 0x54aa30};
                      using Update = U(__attribute__((thiscall)) *)(void *);
                      result = reinterpret_cast<Update>(entries[type])(
                          object.bytes.data());
                      seh.check();
                      random.first = *reinterpret_cast<U *>(0x6574c8);
                      random.second = *reinterpret_cast<U *>(0x6574cc);
                      random.lcg = *reinterpret_cast<U *>(0x6f2a9c);
                      std::memcpy(random.words.data(),
                                  reinterpret_cast<void *>(0x6574d0), 1000);
                      auto normalize = [&](ParticleObject value,
                                           unsigned ownerAt, bool cached,
                                           bool frame) {
                        if (ownerAt && U(value.get(ownerAt)) == ptr(actor))
                          value.put(ownerAt, 0x1010);
                        if (cached) {
                          const U cell = U(value.get(0x3e)),
                                  base = ptr(terrain.cells.data());
                          if (cell >= base && cell < base + 96 * 12 &&
                              (cell - base) % 12 == 0)
                            value.put(0x3e, 1 + (cell - base) / 12);
                          const U desc = U(value.get(0x42)),
                                  start = ptr(terrain.descriptors.data());
                          if (desc >= start && desc < start + 3 * 356 &&
                              (desc - start) % 356 == 0)
                            value.put(0x42, 0x2000 + (desc - start) / 356);
                          const U plane = U(value.get(0x2e));
                          if (plane)
                            for (U i = 0; i < arena.count; ++i)
                              if (plane == ptr(arena.allocations[i].bytes))
                                value.put(0x2e, 0x3000 + i + 1);
                        }
                        if (frame) {
                          for (U bank = 0; bank < 3; ++bank)
                            for (U i = 0; i < 64; ++i)
                              if (U(value.get(0x5a)) == ptr(frames[bank][i]))
                                value.put(0x5a, 0x4000 + bank * 64 + i);
                        }
                        return value;
                      };
#else
                      switch (type) {
                      case 0:
                        result = updateAdditiveEmitter(object, {128, 96, 64},
                                                       emission, random, arena);
                        break;
                      case 1:
                        result = updateInterpolatingEmitter(
                            object, {128, 96, 64}, owner, emission, random,
                            arena);
                        break;
                      case 2:
                      case 3:
                        result = updateOwnerEmitter(object, {128, 96, 64},
                                                    owner, owner.keepAlive,
                                                    emission, random, arena);
                        break;
                      case 4:
                        result = updateFlagEmitter(object, {128, 96, 64}, owner,
                                                   emission, random, arena);
                        break;
                      case 5:
                        result = updateFadingEmitter(
                            object, {128, 96, 64}, terrain, validity, animation,
                            emission, random, arena);
                        break;
                      default:
                        result = updateGravityParticle(object, {128, 96, 64});
                        break;
                      }
                      auto normalize = [](ParticleObject value, unsigned, bool,
                                          bool) { return value; };
#endif
                      for (U stage = 0; stage < 2; ++stage) {
                        if (stage == 1)
                          for (U i = 0; i < arena.count; ++i)
                            if (arena.allocations[i].live &&
                                !arena.allocations[i].plane) {
#ifdef MNM_ORIGINAL_REFERENCE
                              auto &child = arena.object(i + 1);
                              const U table = U(child.get(0));
                              using Destroy = void *(
                                  __attribute__((thiscall)) *)(void *, U);
                              if (reinterpret_cast<Destroy>(
                                      *reinterpret_cast<U *>(table))(
                                      child.bytes.data(), 1) !=
                                  child.bytes.data())
                                throw std::runtime_error(
                                    "Deleting receiver return");
                              seh.check();
#else
                              arena.destroy(i + 1);
#endif
                            }
                        arena.checkRedzones();
                        if (stage == 1)
                          for (U i = 0; i < arena.count; ++i)
                            if (arena.allocations[i].live)
                              throw std::runtime_error(
                                  "Emission allocation not retired");
                        const U header[]{id,
                                         stage,
                                         type,
                                         configIndex,
                                         state,
                                         ownerState,
                                         U(flag),
                                         failAllocate,
                                         failInsert,
                                         U(format),
                                         seed,
                                         U(enabled),
                                         result,
                                         arena.count,
                                         U(arena.events.size()),
                                         U(validity.descriptorDefined)};
                        output(header, sizeof(header));
                        const auto parent = normalize(object, ownerOffset,
                                                      type == 5, type == 5);
                        output(parent.bytes.data(), 160);
                        output(&random, sizeof(random));
                        for (U i = 0; i < arena.count; ++i) {
                          const auto &allocation = arena.allocations[i];
                          const U facts[]{
                              i + 1, allocation.requested, U(allocation.plane),
                              U(allocation.live), U(allocation.adopted)};
                          output(facts, sizeof(facts));
                          if (allocation.plane)
                            output(allocation.bytes, allocation.requested);
                          else {
                            const auto child = normalize(
                                *reinterpret_cast<const ParticleObject *>(
                                    allocation.bytes),
                                allocation.requested == 100 ? 0x60 : 0,
                                allocation.requested == 50 ||
                                    allocation.requested == 90,
                                false);
                            output(child.bytes.data(), allocation.requested);
                          }
                        }
                        output(arena.events.data(), arena.events.size() * 4);
                      }
                      ++id;
                    }
  if (id != 54432)
    throw std::runtime_error("Emitter denominator " + std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
  for (const auto &range : original)
    if (std::memcmp(reinterpret_cast<const void *>(range.first),
                    range.second.data(), range.second.size()))
      throw std::runtime_error(
          "Original emitting updater/constructor/shape body changed");
#endif
  return 0;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
