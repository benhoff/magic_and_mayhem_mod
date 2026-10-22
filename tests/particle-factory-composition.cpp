#include "../reconstruction/rendering/particle_children.hpp"
#include "../reconstruction/rendering/particle_factory.hpp"
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
#include "particle-emitter-arena.hpp"
struct FactoryArena : EmissionArena {
  std::array<ParticleResourcePreparation, 3> resources;
  U loadMode{};
  using EmissionArena::EmissionArena;
  bool load(unsigned id, ParticleResourcePreparation &resource) {
    const bool ok = loadMode != 2;
    events.insert(events.end(), {5, id, 8, U(ok)});
    if (ok)
      resource.loaded = true;
    return ok;
  }
  void palette(unsigned id, unsigned index) {
    events.insert(events.end(), {6, id, index, 8});
  }
  void prepare(unsigned id) {
    prepareParticleResource(resources[id], id, *this);
  }
  std::int32_t pieces() const { return resources[2].pieces; }
  void construct(U token, const ParticleFactoryArguments &in, int ordinal,
                 const std::array<std::int32_t, 3> &velocity,
                 Kind8Random &rng) {
    ParticleConstructorArguments args;
    args.category = in.category;
    args.ordinal = ordinal;
    args.position = in.position;
    args.velocity = velocity;
    args.palette = in.palette;
    auto plane =
        constructParticleVariant(ParticleVariant::fadingSprite, object(token),
                                 {128, 96, 64}, args, config, {}, terrain, rng);
    if (!plane.empty()) {
      const U id = allocateRecord(U(plane.size() * 2), true);
      std::memcpy(allocations[id - 1].bytes, plane.data(), plane.size() * 2);
      object(token).put(0x2e, 0x3000 + id);
    }
  }
  void destroy(U token) {
    auto &value = object(token);
    const U plane = U(value.get(0x2e));
    if (plane) {
      release(plane - 0x3000);
      value.put(0x2e, 0);
    }
    value.put(0, 0x5c73a8);
    release(token);
  }
};
#ifdef MNM_ORIGINAL_REFERENCE
static FactoryArena *factory;
static U category;
static U headers[3][4];
static U pointer(const void *p) {
  return U(reinterpret_cast<std::uintptr_t>(p));
}
static U resourceSlot(U id) {
  return id == 0 ? 0x6f2218 : id == 1 ? 0x6f2240 : 0x6f1d58 + category * 36;
}
static U resourceCount(U id) {
  return id == 0 ? 0x6f220c : id == 1 ? 0x6f2148 : 0x6f2174 + category * 4;
}
extern "C" __attribute__((force_align_arg_pointer)) void *
factory_allocate(U size) {
  try {
    const U id = factory->allocateRecord(size, size != 0x82);
    return id ? factory->allocations[id - 1].bytes : nullptr;
  } catch (...) {
    _exit(74);
  }
}
extern "C" __attribute__((thiscall, force_align_arg_pointer)) U
factory_load(void *receiver, const char *name, U flags) {
  for (U id = 0; id < 3; ++id)
    if (pointer(receiver) == resourceSlot(id)) {
      const U expected = id == 0 ? 0x5eb4ec
                         : id == 1
                             ? 0x5eb4c8
                             : *reinterpret_cast<U *>(0x5c7338 + category * 4);
      if (pointer(name) != expected || flags != 8)
        _exit(75);
      const bool ok = factory->load(id, factory->resources[id]);
      if (ok)
        global(resourceSlot(id), pointer(headers[id]));
      return ok;
    }
  _exit(76);
}
extern "C" __attribute__((thiscall, force_align_arg_pointer)) U
factory_palette(void *receiver, U index, U flags) {
  for (U id = 0; id < 3; ++id)
    if (pointer(receiver) == resourceSlot(id)) {
      if (flags != 8)
        _exit(77);
      factory->palette(id, index);
      return 1;
    }
  _exit(78);
}
#endif
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  OriginalLinuxSeh seh;
  const unsigned char allocation[]{0x8b, 0x44, 0x24, 4, 0x6a, 0},
      release[]{0x8b, 0x44, 0x24, 4, 0x85, 0xc0},
      insert[]{0x51, 0x53, 0x55, 0x56, 0x8b, 0xf1},
      load[]{0x83, 0xec, 0x1c, 0x53, 0x55, 0x56},
      palette[]{0x8b, 0x44, 0x24, 4, 0x53, 0x55};
  emission_boundary(0x597850, reinterpret_cast<void *>(&factory_allocate),
                    allocation);
  emission_boundary(0x597870, reinterpret_cast<void *>(&emission_free),
                    release);
  emission_boundary(0x5457e0, reinterpret_cast<void *>(&emission_insert),
                    insert);
  emission_boundary(0x57d310, reinterpret_cast<void *>(&factory_load), load);
  emission_boundary(0x57d690, reinterpret_cast<void *>(&factory_palette),
                    palette);
  std::uint16_t control = 0x037f;
  __asm__ volatile("fldcw %0" ::"m"(control));
  std::vector<std::pair<U, Bytes>> unchanged;
  for (auto range : std::vector<std::pair<U, U>>{{0x549e80, 0x54a140},
                                                 {0x54a5f0, 0x54a791},
                                                 {0x548650, 0x548860},
                                                 {0x549e30, 0x549e80}}) {
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
  for (U cat : {0u, 4u, 13u, 24u, 25u, 26u, 27u})
    for (int pieces : {-1, 0, 1, 3})
      for (U vectors = 0; vectors < 3; ++vectors)
        for (U loading = 0; loading < 3; ++loading)
          for (U alloc = 0; alloc < 4; ++alloc)
            for (U insertFailure = 0; insertFailure < 3; ++insertFailure)
              for (bool rgb555 : {false, true})
                for (U seed : {0u, 1u, 0xdeadbeefu})
                  for (bool plane : {false, true}) {
                    Kind8Random random(seed);
                    ParticleTerrainFacts terrain;
                    for (auto &cell : terrain.cells[0])
                      cell.block = 1;
                    terrain.shades[0].fill(-8);
                    FactoryArena arena(terrain, random);
                    arena.failAllocate = alloc;
                    arena.failInsert = insertFailure;
                    arena.loadMode = loading;
                    arena.config.duration = 100;
                    arena.config.dimension = 3;
                    arena.config.planeEnabled = plane;
                    arena.config.rgb555 = rgb555;
                    arena.config.effectCount = 49;
                    for (U bank = 0; bank < 3; ++bank)
                      arena.resources[bank] = {loading == 0,
                                               bank == 2 ? 27u : 18u,
                                               bank == 2 ? pieces : -7};
                    ParticleFactoryArguments in;
                    in.category = int(cat);
                    in.palette = 1;
                    if (vectors == 2)
                      in.velocities = {{{-7.75f, 3.5f, -1.9f}},
                                       {{8.9f, -6.2f, 10.5f}}};
#ifdef MNM_ORIGINAL_REFERENCE
                    factory = &arena;
                    originalEmissionArena = &arena;
                    category = cat;
                    global(0x5eb480, 128);
                    global(0x5eb484, 96);
                    global(0x5eb488, 64);
                    global(0x6e1f88, rgb555);
                    global(0x5e1428, 100);
                    global(0x5e142c, 3);
                    global(0x5e1430, plane);
                    global(0x5e1424, 49);
                    global(0x6c54dc, pointer(terrain.cells[0].data()));
                    global(0x6c5c5c, pointer(terrain.shades[0].data()));
                    global(0x65660c, pointer(terrain.descriptors.data()));
                    for (U i = 0; i < 4; ++i)
                      global(0x6cb8c2 + i * 4, i * 12);
                    for (U i = 0; i < 3; ++i)
                      global(0x6cb942 + i * 4, i * 4);
                    for (U bank = 0; bank < 3; ++bank) {
                      headers[bank][3] = arena.resources[bank].frameCount;
                      global(resourceSlot(bank),
                             loading == 0 ? pointer(headers[bank]) : 0);
                      global(resourceCount(bank),
                             U(arena.resources[bank].pieces));
                    }
                    U entries[2][7]{};
                    for (U v = 0; v < in.velocities.size(); ++v)
                      std::memcpy(&entries[v][3], in.velocities[v].data(), 12);
                    U vectorHeader[]{0, vectors == 2 ? pointer(entries) : 0,
                                     vectors == 2 ? pointer(entries) + 56 : 0};
                    global(0x6574c8, random.first);
                    global(0x6574cc, random.second);
                    global(0x6f2a9c, random.lcg);
                    std::memcpy(reinterpret_cast<void *>(0x6574d0),
                                random.words.data(), 1000);
                    using Factory = void(__attribute__((fastcall)) *)(
                        int, int, int, int, int, int, int, int, U, void *);
                    reinterpret_cast<Factory>(0x549e80)(
                        int(cat), 31, 31, 15, 255, 127, 65, 4, 1,
                        vectors ? vectorHeader : nullptr);
                    seh.check();
                    random.first = *reinterpret_cast<U *>(0x6574c8);
                    random.second = *reinterpret_cast<U *>(0x6574cc);
                    random.lcg = *reinterpret_cast<U *>(0x6f2a9c);
                    std::memcpy(random.words.data(),
                                reinterpret_cast<void *>(0x6574d0), 1000);
                    for (U bank = 0; bank < 3; ++bank) {
                      arena.resources[bank].loaded =
                          *reinterpret_cast<U *>(resourceSlot(bank)) != 0;
                      arena.resources[bank].pieces = particleSigned(
                          *reinterpret_cast<U *>(resourceCount(bank)));
                    }
#else
                    createFadingParticles(in, random, arena);
#endif
                    for (U stage = 0; stage < 2; ++stage) {
                      if (stage)
                        for (U i = 0; i < arena.count; ++i)
                          if (arena.allocations[i].live &&
                              !arena.allocations[i].plane) {
#ifdef MNM_ORIGINAL_REFERENCE
                            auto &value = arena.object(i + 1);
                            using Destroy =
                                void *(__attribute__((thiscall)) *)(void *, U);
                            reinterpret_cast<Destroy>(*reinterpret_cast<U *>(
                                U(value.get(0))))(value.bytes.data(), 1);
                            seh.check();
#else
                            arena.destroy(i + 1);
#endif
                          }
                      arena.checkRedzones();
                      const U h[]{id,
                                  stage,
                                  cat,
                                  U(pieces),
                                  vectors,
                                  loading,
                                  alloc,
                                  insertFailure,
                                  U(rgb555),
                                  seed,
                                  U(plane),
                                  arena.count,
                                  U(arena.events.size())};
                      output(h, sizeof(h));
                      output(&random, sizeof(random));
                      for (const auto &resource : arena.resources) {
                        const U facts[]{U(resource.loaded), resource.frameCount,
                                        U(resource.pieces)};
                        output(facts, sizeof(facts));
                      }
                      for (U i = 0; i < arena.count; ++i) {
                        const auto &a = arena.allocations[i];
                        const U facts[]{i + 1, a.requested, U(a.plane),
                                        U(a.live), U(a.adopted)};
                        output(facts, sizeof(facts));
                        if (a.plane)
                          output(a.bytes, a.requested);
                        else {
                          ParticleObject value =
                              *reinterpret_cast<const ParticleObject *>(
                                  a.bytes);
#ifdef MNM_ORIGINAL_REFERENCE
                          const U cell = U(value.get(0x3e)),
                                  base = pointer(terrain.cells[0].data());
                          if (cell >= base && cell < base + 48 * 12 &&
                              (cell - base) % 12 == 0)
                            value.put(0x3e, 1 + (cell - base) / 12);
                          const U desc = U(value.get(0x42)),
                                  start = pointer(terrain.descriptors.data());
                          if (desc >= start && desc < start + 3 * 356 &&
                              (desc - start) % 356 == 0)
                            value.put(0x42, 0x2000 + (desc - start) / 356);
                          for (U j = 0; j < arena.count; ++j)
                            if (U(value.get(0x2e)) ==
                                pointer(arena.allocations[j].bytes))
                              value.put(0x2e, 0x3000 + j + 1);
#endif
                          output(value.bytes.data(), a.requested);
                        }
                      }
                      output(arena.events.data(), arena.events.size() * 4);
                    }
                    ++id;
                  }
#ifdef MNM_ORIGINAL_REFERENCE
  for (const auto &[address, body] : unchanged)
    if (std::memcmp(reinterpret_cast<void *>(address), body.data(),
                    body.size()))
      throw std::runtime_error(
          "Original factory/preparation/constructor changed");
#endif
  if (id != 36288)
    throw std::runtime_error("Factory matrix denominator");
  return std::cout ? 0 : 1;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
