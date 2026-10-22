#pragma once
#include <new>
// Owned allocator and insertion-result boundary, shared as fixture INPUT. The
// original updater/constructors/destructors and native implementations differ.
struct EmissionAllocation {
  alignas(16) unsigned char bytes[2048]{};
  U requested{};
  bool plane{}, live{}, adopted{};
};
struct EmissionArena {
  std::array<EmissionAllocation, 128> allocations{};
  U count{}, objectAttempts{}, insertAttempts{}, failAllocate{}, failInsert{};
  std::vector<U> events;
  ParticleConstructorConfig config;
  ParticleTerrainFacts &terrain;
  Kind8Random &random;
  EmissionArena(ParticleTerrainFacts &facts, Kind8Random &stream)
      : terrain(facts), random(stream) {}
  U allocateRecord(U size, bool plane) {
    if (!plane) {
      ++objectAttempts;
      if (failAllocate == 3 || (failAllocate == 1 && objectAttempts == 1) ||
          (failAllocate == 2 && objectAttempts % 2 == 0)) {
        events.insert(events.end(), {2, objectAttempts, size, 0});
        return 0;
      }
    }
    if (count >= 128 || !size || size > 1536)
      throw std::runtime_error("Emission allocation budget");
    auto &allocation = allocations[count++];
    allocation.requested = size;
    allocation.plane = plane;
    allocation.live = true;
    if (!plane)
      new (allocation.bytes) ParticleObject;
    std::memset(allocation.bytes, 0xa5, sizeof(allocation.bytes));
    events.insert(events.end(), {1, count, size, U(plane)});
    return count;
  }
  U allocate(U size) { return allocateRecord(size, false); }
  ParticleObject &object(U token) {
    if (!token || token > count || !allocations[token - 1].live ||
        allocations[token - 1].plane)
      throw std::runtime_error("Emission dead object generation");
    return *reinterpret_cast<ParticleObject *>(allocations[token - 1].bytes);
  }
  U resolve(const void *pointer) const {
    for (U i = 0; i < count; ++i)
      if (allocations[i].live && pointer == allocations[i].bytes)
        return i + 1;
    throw std::runtime_error("Emission unknown allocation identity");
  }
  bool insert(U token) {
    object(token);
    ++insertAttempts;
    const bool accepted = failInsert == 0 ||
                          (failInsert == 1 && insertAttempts != 1) ||
                          (failInsert == 2 && insertAttempts % 2 != 0);
    allocations[token - 1].adopted = accepted;
    events.insert(events.end(), {3, token, U(accepted), insertAttempts});
    return accepted;
  }
  void release(U token) {
    if (!token || token > count || !allocations[token - 1].live)
      throw std::runtime_error("Emission duplicate free");
    auto &allocation = allocations[token - 1];
    allocation.live = false;
    events.insert(events.end(),
                  {4, token, U(allocation.plane), allocation.requested});
  }
  ParticleChildKind kind(U token) const {
    switch (allocations.at(token - 1).requested) {
    case 0x45:
      return ParticleChildKind::randomAdditive;
    case 100:
      return ParticleChildKind::ownerAdditive;
    case 0x32:
      return ParticleChildKind::colour;
    case 0x5a:
      return ParticleChildKind::terrainColour;
    default:
      throw std::runtime_error("Unknown emission object kind");
    }
  }
  void construct(U token, ParticleChildKind type,
                 const std::array<std::int32_t, 3> &position,
                 const std::array<std::int32_t, 3> &velocity) {
    auto &instance = object(token);
    auto plane =
        constructEmittedParticle(type, instance, {128, 96, 64}, position,
                                 velocity, config, terrain, random);
    if (!plane.empty()) {
      const auto id = allocateRecord(U(plane.size() * 2), true);
      std::memcpy(allocations[id - 1].bytes, plane.data(), plane.size() * 2);
      instance.put(0x2e, 0x3000 + id);
    }
  }
  void colour(U token, const std::array<std::int32_t, 3> &rgb) {
    auto &instance = object(token);
    const U pointer = U(instance.get(0x2e));
    std::vector<std::uint16_t> plane;
    U id = 0;
    if (pointer) {
      id = pointer - 0x3000;
      if (!id || id > count || !allocations[id - 1].live ||
          !allocations[id - 1].plane)
        throw std::runtime_error("Emission plane generation");
      const auto &record = allocations[id - 1];
      plane.resize(record.requested / 2);
      std::memcpy(plane.data(), record.bytes, record.requested);
    }
    colourEmittedParticle(kind(token), instance, plane, rgb, config.rgb555);
    if (id)
      std::memcpy(allocations[id - 1].bytes, plane.data(), plane.size() * 2);
  }
  void destroy(U token) {
    auto &instance = object(token);
    const auto type = kind(token);
    if (type == ParticleChildKind::colour ||
        type == ParticleChildKind::terrainColour) {
      const U pointer = U(instance.get(0x2e));
      if (pointer) {
        release(pointer - 0x3000);
        instance.put(0x2e, 0);
      }
    }
    instance.put(0, 0x5c73a8);
    release(token);
  }
  void checkRedzones() const {
    for (U index = 0; index < count; ++index) {
      const auto &allocation = allocations[index];
      for (U i = allocation.requested; i < sizeof(allocation.bytes); ++i)
        if (allocation.bytes[i] != 0xa5)
          throw std::runtime_error(
              "Emission write exceeded requested allocation");
    }
  }
};
#ifdef MNM_ORIGINAL_REFERENCE
static EmissionArena *originalEmissionArena;
extern "C" __attribute__((force_align_arg_pointer)) void *
emission_allocate(U size) {
  try {
    const bool plane =
        size != 0x45 && size != 100 && size != 0x32 && size != 0x5a;
    const auto token = originalEmissionArena->allocateRecord(size, plane);
    return token ? originalEmissionArena->allocations[token - 1].bytes
                 : nullptr;
  } catch (...) {
    _exit(70);
  }
}
extern "C" __attribute__((force_align_arg_pointer)) void
emission_free(void *memory) {
  try {
    originalEmissionArena->release(originalEmissionArena->resolve(memory));
  } catch (...) {
    _exit(71);
  }
}
extern "C" __attribute__((thiscall, force_align_arg_pointer)) U
emission_insert(void *receiver, void *memory) {
  if (U(reinterpret_cast<std::uintptr_t>(receiver)) != 0x6cbb8a)
    _exit(72);
  try {
    return originalEmissionArena->insert(
        originalEmissionArena->resolve(memory));
  } catch (...) {
    _exit(73);
  }
}
static void emission_boundary(U address, void *callback,
                              const unsigned char *expected) {
  auto *entry = reinterpret_cast<unsigned char *>(address);
  if (std::memcmp(entry, expected, 6))
    throw std::runtime_error("Emission boundary signature");
  const U rel = U(reinterpret_cast<std::uintptr_t>(callback)) - address - 5;
  entry[0] = 0xe9;
  std::memcpy(entry + 1, &rel, 4);
  entry[5] = 0x90;
}
#endif
