#include "../reconstruction/creature-ai/owner_visibility.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_VISIBILITY_REFERENCE
#define main unused_visibility_mapper_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
namespace m = mnm::reconstruction::combat;
namespace {
void need(bool ok, const char *s) {
  if (!ok)
    throw std::runtime_error(s);
}
struct Map {
  m::Extent dimensions{32, 32, 8};
  std::vector<unsigned char> cells =
                                 std::vector<unsigned char>(32 * 32 * 8 * 12),
                             terrain = std::vector<unsigned char>(4 * 356),
                             columns = std::vector<unsigned char>(32 * 32 * 6);
  unsigned attenuation = 7;
  unsigned at(m::Cell p) const {
    need(p.x >= 0 && p.x < 32 && p.y >= 0 && p.y < 32 && p.z >= 0 && p.z < 8,
         "sample domain");
    return unsigned((p.z * 32 + p.y) * 32 + p.x) * 12;
  }
  m::SightSample sample(m::Cell p, unsigned x, unsigned y, unsigned z) const {
    unsigned off = at(p);
    unsigned kind = cells[off] | unsigned(cells[off + 1]) << 8;
    unsigned occupant = cells[off + 4] | unsigned(cells[off + 5]) << 8;
    const auto c = unsigned(p.y * 32 + p.x) * 6;
    return {kind != 0 || occupant != 65535,
            (terrain.at(kind * 356 + z * 8 + y) & (128u >> x)) != 0,
            (columns[c] || columns[c + 1]) &&
                p.z >= std::int8_t(columns[c + 3]) &&
                p.z <= std::int8_t(columns[c + 2])};
  }
  void fill(unsigned n) {
    for (unsigned i = 0; i < cells.size(); i += 12) {
      const unsigned kind = (i / 12 + n * 13) % 37 ? 0 : 1 + (i / 12 + n) % 3;
      cells[i] = kind;
      cells[i + 4] = cells[i + 5] = 255;
    }
    for (unsigned i = 0; i < terrain.size(); ++i)
      terrain[i] = std::uint8_t(i * 37 + n * 17);
    for (unsigned i = 0; i < columns.size(); i += 6) {
      columns[i] = std::uint8_t((i / 6 + n) % 13 == 0);
      columns[i + 2] = 5;
      columns[i + 3] = 1;
    }
  }
};
std::uint64_t digest = 1469598103934665603ull;
unsigned checks = 0;
void check(unsigned a, unsigned b, const char *s) {
  need(a == b, s);
  ++checks;
  digest = (digest ^ a) * 1099511628211ull;
}
#ifdef MNM_VISIBILITY_REFERENCE
void put(void *p, unsigned off, unsigned value) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &value, 4);
}
unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
void publish(Map &map) {
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  global(0x6c54dc, address(map.cells.data()));
  global(0x65660c, address(map.terrain.data()));
  global(0x6a49c0, address(map.columns.data()));
  global(0x6becc6, map.attenuation);
  global(0x644520, unsigned(-1));
  global(0x6c5c84, 0);
  for (unsigned i = 0; i < 32; ++i)
    global(0x6cb942 + i * 4, i * 32);
  for (unsigned i = 0; i < 8; ++i)
    global(0x6cb8c2 + i * 4, i * 32 * 32);
}
std::vector<unsigned char> actors, configs;
void pack(const std::vector<m::VisibilityActor> &in,
          std::vector<unsigned char> &out) {
  for (unsigned i = 0; i < in.size(); ++i) {
    const auto &a = in[i];
    auto *p = out.data() + i * 0xe4b;
    put(p, 0, unsigned(a.slot));
    put(p, 4, a.active);
    put(p, 8, a.position.x);
    put(p, 12, a.position.y);
    put(p, 16, a.position.z);
    put(p, 20, a.finePosition.x);
    put(p, 24, a.finePosition.y);
    put(p, 28, a.finePosition.z);
    put(p, 0xac, address(configs.data() + i * 0x5c9));
    put(p, 0xa8, a.type);
    put(p, 0xe4, a.health);
    put(p, 0x174, a.owner);
    put(p, 0x8c, a.hidden);
    put(p, 0x664, a.engaged);
    put(p, 0x668, a.incoming);
    put(p, 0x164, a.readiness);
    put(p, 0x64c, a.target);
    put(p, 0x608, a.direction);
    for (unsigned owner = 0; owner < 8; ++owner) {
      put(p, 0x24 + 4 * owner, a.visible[owner]);
      put(p, 0x48 + 4 * owner,
          a.observer[owner] < 0
              ? 0
              : address(actors.data() + unsigned(a.observer[owner]) * 0xe4b));
    }
  }
}
void compare(const m::VisibilityState &before,
             const std::vector<m::VisibilityActor> &in,
             const m::VisibilityState &after,
             const std::vector<m::VisibilityActor> &out,
             const m::VisionRelations &r, const m::VisionRelations &s) {
  actors.assign(in.size() * 0xe4b, 0);
  configs.assign(in.size() * 0x5c9, 0);
  pack(in, actors);
  for (unsigned i = 0; i < in.size(); ++i) {
    auto *p = configs.data() + i * 0x5c9;
    const auto &d = in[i].definition;
    put(p, 0x38, d.range);
    put(p, 0x198, d.aimXY);
    put(p, 0x19c, d.aimZ);
    for (unsigned j = 0; j < 8; ++j) {
      put(p, 0x520 + 12 * j, d.offsets[j].x);
      put(p, 0x524 + 12 * j, d.offsets[j].y);
      put(p, 0x528 + 12 * j, d.offsets[j].z);
    }
  }
  for (unsigned a = 0; a < 8; ++a)
    for (unsigned b = 0; b < 8; ++b) {
      *reinterpret_cast<unsigned char *>(0x643a80 + 340 * a + 0x18 + b) =
          r[a][b];
      *reinterpret_cast<unsigned char *>(0x643a80 + 340 * a + 0x20 + b) =
          s[a][b];
    }
  std::array<unsigned char, 0x270> context{};
  put(context.data(), 0, address(actors.data()));
  put(context.data(), 4, before.capacity);
  put(context.data(), 0x228, in.size());
  put(context.data(), 0x260, before.phase);
  auto expected = actors;
  pack(out, expected);
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(0x52b540)(context.data());
  unsigned phase;
  std::memcpy(&phase, context.data() + 0x260, 4);
  check(phase, after.phase, "original phase");
  need(actors == expected, "original actor visibility/observer bytes");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_VISIBILITY_REFERENCE
  need(argc == 2, "PE");
  const auto pe = read(argv[1]);
  map_image(pe);
  // All original text remains intact and executable but read-only.
  const auto h = u32(pe, 60), opt = h + 24,
             count = pe[h + 6] | unsigned(pe[h + 7]) << 8;
  const auto sections = opt + (pe[h + 20] | unsigned(pe[h + 21]) << 8);
  for (unsigned i = 0; i < count; ++i) {
    auto at = sections + 40 * i;
    if (u32(pe, at + 36) & 0x20000000u) {
      auto *p = reinterpret_cast<void *>(0x400000 + u32(pe, at + 12));
      need(mprotect(p, (u32(pe, at + 8) + 4095) & ~4095,
                    PROT_READ | PROT_EXEC) == 0,
           "seal original text");
    }
  }
#else
  (void)argc;
  (void)argv;
#endif
  Map map;
  unsigned visible = 0, hidden = 0, shared = 0, expired = 0, locked = 0,
           rays = 0;
  for (unsigned fixture = 0; fixture < 96; ++fixture) {
    map.fill(fixture);
#ifdef MNM_VISIBILITY_REFERENCE
    publish(map);
#endif
    auto sample = [&](auto p, auto x, auto y, auto z) {
      return map.sample(p, x, y, z);
    };
    m::VisibilityState state{125, fixture % 12};
    m::VisionRelations relations{}, sharing{};
    std::vector<m::VisibilityActor> pool(24);
    for (unsigned i = 0; i < pool.size(); ++i) {
      auto &a = pool[i];
      a.slot = int(i);
      a.owner = int(i % 4);
      a.type = i % 11 == 0 ? 27 : i % 7 == 0 ? 22 : 17;
      a.active = (i + fixture) % 13 != 0;
      a.health = (i + fixture) % 17 ? 100 : 0;
      a.position = {int((i * 7 + fixture) % 32),
                    int((i * 11 + fixture / 3) % 32), 2};
      a.finePosition = {a.position.x * 32 + int(i % 8),
                        a.position.y * 32 + int(i % 8), 32 + int(i % 9)};
      a.definition.range = 3 + fixture % 13;
      a.definition.aimXY = 16;
      a.definition.aimZ = 8;
      for (unsigned j = 0; j < 8; ++j)
        a.definition.offsets[j] = {int(j % 3) - 1, int(j % 5) - 2, int(j % 3)};
      a.direction = i % 8;
      a.visible.fill(0);
      a.visible[unsigned(a.owner)] = 30;
    }
    for (unsigned tick = 0; tick < 192; ++tick) {
      if (tick == 48 || tick == 96)
        map.fill(fixture + tick / 48);
      for (unsigned a = 0; a < 8; ++a)
        for (unsigned b = 0; b < 8; ++b) {
          relations[a][b] =
              (a == b || ((a + b + fixture + tick / 48) % 5 == 0)) ? 2 : 0;
          sharing[a][b] = (fixture + tick / 24 + a * 3 + b) % 3;
        }
      for (unsigned i = 0; i < pool.size(); ++i) {
        auto &a = pool[i];
        a.hidden = (tick / 32 + i) % 4 == 0;
        a.engaged = (i + tick / 16) % 7 == 0;
        a.incoming = (i + tick / 13) % 9 == 0;
        a.readiness = (i + tick / 19) % 13 == 0;
        a.target = int((i + 1) % pool.size());
        if (tick == 64 && i % 3 == 0)
          a.health = 0;
        if (tick == 128 && i % 5 == 0)
          a.active = false;
      }
      auto before = pool;
      auto prior = state;
      m::updateOwnerVisibility(
          state, pool, relations, sharing, [&](const auto &a, const auto &b) {
            ++rays;
            return m::creatureSight(a, b, map.dimensions, map.attenuation,
                                    sample);
          });
#ifdef MNM_VISIBILITY_REFERENCE
      compare(prior, before, state, pool, relations, sharing);
#else
      check(state.phase, state.phase, "phase");
      (void)prior;
#endif
      for (unsigned i = 0; i < pool.size(); ++i)
        for (unsigned o = 0; o < 8; ++o) {
          const auto &a = pool[i];
          check(a.visible[o], a.visible[o], "visibility");
          check(unsigned(a.observer[o]), unsigned(a.observer[o]), "observer");
          visible += a.visible[o] != 0;
          hidden += a.hidden && !a.visible[o];
          shared += a.owner != int(o) && a.visible[o] &&
                    relations[unsigned(a.owner)][o] &&
                    sharing[unsigned(a.owner)][o];
          expired += before[i].visible[o] && !a.visible[o];
          locked += a.observer[o] >= 0 && pool[unsigned(a.observer[o])].engaged;
        }
    }
    // Direct ray corpus includes wrap seams, masks, attenuation, endpoint-first
    // admission, same-point and zero-budget exits independently of pool
    // filters.
    for (unsigned i = 0; i < 256; ++i) {
      m::Cell from{int((i * 31 + fixture) % 1024),
                   int((i * 19 + fixture) % 1024), 40 + int(i % 8)},
          to{int((i * 17 + fixture) % 1024), int((i * 23 + fixture) % 1024),
             40 + int((i / 3) % 8)};
      if (i % 17 == 0)
        to = from;
      else if (i % 11 == 0) {
        to.x = from.x;
        to.y = from.y;
        to.z = from.z + 64;
      }
      if (i % 7 == 0)
        from.x -= 1024;
      if (i % 7 == 1)
        to.y += 1024;
      const auto direction =
          m::fineCoordinateFacing(from, 16, to, map.dimensions);
#ifdef MNM_VISIBILITY_REFERENCE
      std::array<unsigned char, 0xe4b> facingActor{};
      std::array<unsigned char, 0x5c9> facingConfig{};
      put(facingActor.data(), 0xac, address(facingConfig.data()));
      put(facingConfig.data(), 0x198, 16);
      put(facingActor.data(), 0x14, from.x);
      put(facingActor.data(), 0x18, from.y);
      put(facingActor.data(), 0x608, 7);
      put(facingActor.data(), 0x60c, 7);
      global(0x6e9850, 0);
      using Face =
          void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
      reinterpret_cast<Face>(0x5084f0)(facingActor.data(), to.x, to.y);
      unsigned actual;
      std::memcpy(&actual, facingActor.data() + 0x608, 4);
      if (direction.value_or(7) != actual)
        throw std::runtime_error(
            "fine facing from=" + std::to_string(from.x) + "," +
            std::to_string(from.y) + " to=" + std::to_string(to.x) + "," +
            std::to_string(to.y) +
            " native=" + std::to_string(direction.value_or(7)) +
            " original=" + std::to_string(actual));
      check(direction.value_or(7), actual, "original fine facing");
#else
      check(direction.value_or(7), direction.value_or(7), "fine facing");
#endif
      m::VisibilityActor observer;
      observer.finePosition = from;
      observer.position = {int(unsigned(from.x + 1024) % 1024 / 32),
                           int(unsigned(from.y + 1024) % 1024 / 32), 2};
      observer.definition.range = 15;
      observer.definition.aimXY = 16;
      const bool point =
          m::pointSight(observer, to, map.dimensions, map.attenuation, sample);
#ifdef MNM_VISIBILITY_REFERENCE
      put(facingActor.data(), 8, observer.position.x);
      put(facingActor.data(), 12, observer.position.y);
      put(facingConfig.data(), 0x38, 15);
      put(facingActor.data(), 0x1c, from.z);
      put(facingActor.data(), 0x608, 0);
      using Point = unsigned(__attribute__((thiscall)) *)(void *, unsigned,
                                                          unsigned, unsigned);
      check(point,
            reinterpret_cast<Point>(0x5151b0)(facingActor.data(), to.x, to.y,
                                              to.z),
            "original point sight");
#else
      check(point, point, "point sight");
#endif
      int range = int(i % 24);
      bool special = i % 3 == 0;
      const bool value = m::sightRay(from, to, range, special, map.dimensions,
                                     map.attenuation, sample);
#ifdef MNM_VISIBILITY_REFERENCE
      std::array<unsigned, 14> context{};
      using F = unsigned(__attribute__((thiscall)) *)(
          void *, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned,
          unsigned, unsigned);
      const auto original = reinterpret_cast<F>(0x4e14a0)(
          context.data(), from.x, from.y, from.z, to.x, to.y, to.z,
          unsigned(range) * 32, special);
      check(value, original, "original sight ray");
#else
      check(value, value, "ray");
#endif
    }
  }
  need(visible && hidden && shared && expired && locked && rays,
       "missing visibility witnesses");
  unsigned refusals = 0;
  for (unsigned capacity : {0u, 9u, 126u}) {
    m::VisibilityState s{capacity, 0};
    std::vector<m::VisibilityActor> a;
    m::VisionRelations r{};
    try {
      m::updateOwnerVisibility(s, a, r, r, {});
      throw std::runtime_error("pool accepted");
    } catch (const std::invalid_argument &) {
      ++refusals;
    }
  }
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"ticks\":18432,\"rays\":" << rays
            << ",\"visible\":" << visible << ",\"hidden\":" << hidden
            << ",\"shared\":" << shared << ",\"expired\":" << expired
            << ",\"locked\":" << locked << ",\"refusals\":" << refusals
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
