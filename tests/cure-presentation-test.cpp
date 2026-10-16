#include "../reconstruction/casting/cure_presentation.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CURE_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
using namespace mnm::reconstruction::casting;
using namespace mnm::reconstruction;
namespace {
void require(bool b, const char *why) {
  if (!b)
    throw std::runtime_error(why);
}
unsigned fixture = 0;
std::vector<unsigned> queries;
bool query(unsigned cell) {
  queries.push_back(cell);
  return fixture % 2 != 0;
}
std::uint64_t digest = 1469598103934665603ull;
void hash(unsigned x) { digest = (digest ^ x) * 1099511628211ull; }
CureAnimationDefinition definition(unsigned n) {
  CureAnimationDefinition d;
  d.entry = {3, 2, n * 17, 0xabcdef00u + n};
  auto add = [&](unsigned op, int argument) {
    mnm::assets::AnimationRecord r{op, argument, {}};
    for (unsigned i = 0; i < 9; ++i)
      r.metadata[i] = n * 101 + i;
    d.sequence.push_back(r);
  };
  switch (n % 6) {
  case 0:
    add(0, 7);
    add(0, 8);
    break;
  case 1:
    add(1, 3);
    add(5, -7);
    add(0, 9);
    add(5, 1);
    add(0, 11);
    break;
  case 2:
    add(2, 3);
    add(0, 12);
    add(3, -1);
    break;
  case 3:
    add(0, 13);
    add(4, -1);
    break;
  case 4:
    add(99, 4);
    add(1, 0);
    add(0, 14);
    break;
  case 5:
    break; // Initial stop remains inactive, returning0 on later ticks.
  }
  add(6, 0);
  return d;
}
void hashState(const CurePresentationState &s) {
  hash(s.scan);
  for (const auto &c : s.cells) {
    hash(c.head);
    hash(c.flags);
  }
  for (const auto &o : s.records) {
    const auto &r = o.placement;
    hash(r.active);
    hash(r.type);
    hash(r.next);
    for (auto w : r.parameters)
      hash(w);
    for (auto w : r.position)
      hash(w);
    for (auto w : r.units)
      hash(w);
    for (auto w : r.sentinels)
      hash(w);
    hash(o.reservation);
    hash(o.property);
    if (o.animation) {
      const auto &a = o.animation->player.state();
      hash(unsigned(a.pc));
      hash(unsigned(a.displayedRecord.value_or(UINT32_MAX)));
      hash(a.active);
      hash(a.delay);
      hash(a.elapsed);
      hash(a.repeats);
      hash(a.breakFlag);
    }
  }
  for (auto q : queries)
    hash(q);
}
#ifdef MNM_CURE_REFERENCE
unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
void put(void *p, unsigned at, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + at, &v, 4);
}
void word(void *p, unsigned at, unsigned v) {
  std::uint16_t w = std::uint16_t(v);
  std::memcpy(static_cast<unsigned char *>(p) + at, &w, 2);
}
struct PrivateFs {
  unsigned head = UINT32_MAX;
  unsigned short saved = 0;
  user_desc descriptor{};
  PrivateFs() {
    descriptor.entry_number = unsigned(-1);
    descriptor.base_addr = address(&head);
    descriptor.limit = 3;
    descriptor.seg_32bit = 1;
    descriptor.useable = 1;
    require(syscall(SYS_set_thread_area, &descriptor) == 0,
            "private FS allocation");
    asm volatile("mov %%fs,%0" : "=rm"(saved));
    const unsigned short selector = (descriptor.entry_number << 3) | 3;
    asm volatile("mov %0,%%fs" ::"rm"(selector) : "memory");
  }
  ~PrivateFs() {
    asm volatile("mov %0,%%fs" ::"rm"(saved) : "memory");
    user_desc empty{};
    empty.entry_number = descriptor.entry_number;
    empty.seg_not_present = 1;
    empty.read_exec_only = 1;
    (void)syscall(SYS_set_thread_area, &empty);
  }
};
Bytes records, cells, actors, nodes, values, catalog, metadata, terrain;
std::vector<unsigned> starts;
std::vector<mnm::assets::AnimationRecord> rows;
unsigned width = 0;
extern "C" unsigned __attribute__((thiscall)) recycle(void *receiver,
                                                      void *cell) {
  require(address(receiver) == 0x6a49c0, "cell recycler receiver");
  const auto delta = address(cell) - address(cells.data() + 32);
  require(delta % 12 == 0 && delta / 12 < cells.size() / 12,
          "cell recycler index");
  return query(delta / 12);
}
std::vector<std::pair<unsigned, Bytes>> bodies;
void install(const char *path) {
  const auto input = read(path);
  map_image(input);
  for (auto r : {std::pair<unsigned, unsigned>{0x493f20, 0x4943e4},
                 {0x4943f0, 0x4945e3},
                 {0x494ab0, 0x497068},
                 {0x464c80, 0x465100},
                 {0x488330, 0x4883e7},
                 {0x489200, 0x4896b0},
                 {0x522720, 0x522769}}) {
    Bytes bytes(r.second - r.first);
    std::memcpy(bytes.data(), reinterpret_cast<void *>(r.first), bytes.size());
    bodies.push_back({r.first, bytes});
  }
  auto *p = reinterpret_cast<unsigned char *>(0x534520);
  p[0] = 0xe9;
  put(p, 1, address((void *)recycle) - 0x534525);
  // Retain the whole private executable text after the single checked lower
  // recycler redirect. All target and transitive animation code remains intact.
  const auto pe = u32(input, 0x3c);
  const auto sections =
      pe + 24 + (input.at(pe + 20) | (unsigned(input.at(pe + 21)) << 8));
  const auto count = input.at(pe + 6) | (unsigned(input.at(pe + 7)) << 8);
  for (unsigned i = 0; i < count; ++i) {
    const auto row = sections + i * 40;
    if (u32(input, row + 36) & 0x20000000u) {
      const auto address = 0x400000 + u32(input, row + 12);
      Bytes bytes(u32(input, row + 16));
      std::memcpy(bytes.data(), reinterpret_cast<void *>(address),
                  bytes.size());
      bodies.push_back({address, std::move(bytes)});
    }
  }
  global(0x5fed7c, 0);
  global(0x5e1478, 0);
}
unsigned recordPointer(unsigned slot) {
  return address(records.data() + 32 + slot * 0x22e);
}
void initialize(const CurePresentationState &s,
                const std::vector<CureCreator> &creators,
                const std::vector<CureContribution> &contributions,
                const CureAnimationDefinition &d) {
  width = s.width;
  records.assign(s.records.size() * 0x22e + 64, 0xa5);
  cells.assign(s.cells.size() * 12 + 64, 0xa7);
  actors.assign(creators.size() * 0xe4b + 64, 0xa9);
  nodes.assign(contributions.size() * 64 + 64, 0xab);
  values.assign(contributions.size() * 48 + 64, 0xad);
  catalog.assign(3 * 0x114 + 64, 0xaf);
  metadata.assign(75 * 16 + 64, 0xb1);
  terrain.assign(4 * 356 + 64, 0xb3);
  for (unsigned i = 0; i < s.records.size(); ++i) {
    auto *p = records.data() + 32 + i * 0x22e;
    put(p, 0, i);
    put(p, 4, 0);
    put(p, 0x24, 0x689498);
    word(p, 0x1a0, NoEffect);
  }
  for (unsigned i = 0; i < s.cells.size(); ++i) {
    auto *p = cells.data() + 32 + i * 12;
    const auto &c = s.cells[i];
    word(p, 0, c.terrain);
    word(p, 2, c.head);
    word(p, 4, c.otherA);
    word(p, 6, c.otherB);
    put(p, 8, c.flags);
  }
  for (unsigned i = 0; i < creators.size(); ++i) {
    auto *p = actors.data() + 32 + i * 0xe4b;
    put(p, 0x174, creators[i].owner);
    for (unsigned j = 0; j < 2; ++j)
      put(p, 0x678 + j * 4,
          creators[i].contributions[j]
              ? address(nodes.data() + 32 + *creators[i].contributions[j] * 64)
              : 0);
  }
  for (unsigned i = 0; i < contributions.size(); ++i) {
    auto *p = nodes.data() + 32 + i * 64;
    const auto &n = contributions[i];
    put(p, 0x39, n.next ? address(nodes.data() + 32 + *n.next * 64) : 0);
    put(p, 5, n.value ? address(values.data() + 32 + i * 48) : 0);
    put(values.data() + 32 + i * 48, 0x1f, n.value.value_or(0));
  }
  rows.assign(3, {0, 1, {}});
  rows.insert(rows.end(), d.sequence.begin(), d.sequence.end());
  starts = {0, 1, 2, 3, unsigned(rows.size())};
  auto *asset = catalog.data() + 32 + 2 * 0x114;
  put(asset, 4, address(starts.data()));
  put(asset, 8, address(rows.data()));
  auto *entry = metadata.data() + 32 + 74 * 16;
  put(entry, 0, d.entry.sequence);
  put(entry, 4, d.entry.assetIndex);
  put(entry, 8, d.entry.property);
  put(entry, 12, d.entry.opaque);
  global(0x689498, address(records.data() + 32));
  global(0x68949c, unsigned(s.records.size()));
  global(0x6898dc, s.scan);
  global(0x6894a0, address(catalog.data() + 32));
  global(0x6898d0, address(metadata.data() + 32)); // collection+8/+438
  global(0x6def58, address(actors.data() + 32));
  global(0x6def5c, unsigned(creators.size()));
  global(0x6c5494, s.width);
  global(0x6c5498, s.height);
  global(0x6c549c, s.layers);
  global(0x6c54dc, address(cells.data() + 32));
  global(0x65660c, address(terrain.data() + 32));
  for (unsigned i = 0; i < s.height; ++i)
    global(0x6cb942 + i * 4, i * s.width);
  for (unsigned i = 0; i < s.layers; ++i)
    global(0x6cb8c2 + i * 4, i * s.width * s.height);
}
void compare(const CurePresentationState &s, const std::function<void()> &call,
             const std::vector<unsigned> &expectedQueries) {
  auto wantedRecords = records, wantedCells = cells;
  const auto savedActors = actors, savedNodes = nodes, savedValues = values,
             savedCatalog = catalog, savedMetadata = metadata,
             savedTerrain = terrain;
  const auto savedStarts = starts;
  const auto savedRows = rows;
  constexpr unsigned offsets[] = {0x1aa, 0x1ae, 0x1b2, 0x1b6, 0x1ba, 0x1be,
                                  0x1c2, 0x1c6, 0x1ca, 0x212, 0x216, 0x21a};
  for (unsigned i = 0; i < s.records.size(); ++i) {
    const auto &o = s.records[i];
    const auto &r = o.placement;
    auto *p = wantedRecords.data() + 32 + i * 0x22e;
    put(p, 4, r.active);
    word(p, 0x1a0, r.next);
    if (!r.initialized)
      continue;
    put(p, 0x28, r.type);
    std::memcpy(p + 0x2c, r.parameters.data(), 252);
    for (unsigned k = 0; k < 3; ++k) {
      put(p, 8 + k * 4, r.position[k]);
      put(p, 0x14 + k * 4, r.units[k]);
      put(p, 0x1f6 + k * 4, r.initialPosition[k]);
      put(p, 0x202 + k * 4, r.initialPosition[k]);
    }
    for (unsigned k = 0; k < 12; ++k)
      put(p, offsets[k], r.sentinels[k]);
    put(p, 0x194, address(cells.data() + 32 + r.cell * 12));
    put(p, 0x190, address(terrain.data() + 32 + s.cells[r.cell].terrain * 356));
    put(p, 0x198, 0);
    put(p, 0x19c, 0);
    put(p, 0x20e, o.reservation);
    put(p, 0x21e, o.property);
    put(p, 0x222, 0);
    put(p, 0x22a, 1);
    auto *q = p + 0x160;
    if (o.animationReset) {
      for (unsigned offset : {4u, 8u, 12u, 16u, 20u, 24u, 32u})
        put(q, offset, 0);
    } else {
      const auto &a = o.animation->player.state();
      put(q, 0, 3);
      put(q, 4,
          a.displayedRecord ? address(rows.data() + 3 + *a.displayedRecord)
                            : 0);
      put(q, 8, a.active);
      put(q, 12, a.delay);
      put(q, 16, a.elapsed);
      put(q, 20, a.repeats);
      put(q, 24, address(rows.data() + 3 + a.pc));
      put(q, 28, address(catalog.data() + 32 + 2 * 0x114));
      put(q, 32, a.breakFlag);
      put(q, 36, 0);
    }
  }
  for (unsigned i = 0; i < s.cells.size(); ++i) {
    auto *p = wantedCells.data() + 32 + i * 12;
    word(p, 2, s.cells[i].head);
    put(p, 8, s.cells[i].flags);
  }
  queries.clear();
  static PrivateFs fs;
  call();
  require(fs.head == UINT32_MAX, "original FS restored");
  require(records == wantedRecords, "whole effect records/guards");
  require(cells == wantedCells, "whole spatial cells/guards");
  require(queries == expectedQueries, "cell release query order");
  require(*reinterpret_cast<unsigned *>(0x6898dc) == s.scan,
          "effect high water");
  require(actors == savedActors && nodes == savedNodes &&
              values == savedValues && catalog == savedCatalog &&
              metadata == savedMetadata && terrain == savedTerrain &&
              starts == savedStarts && rows.size() == savedRows.size() &&
              !std::memcmp(rows.data(), savedRows.data(), rows.size() * 44),
          "immutable creator/catalog/ANI guards");
}
#endif
void refusals() {
  auto s = makeCurePresentation(8, 8, 3, 2);
  CastEffectDescriptor p{};
  p[6] = UINT32_MAX;
  p[8] = 41;
  for (unsigned n = 0; n < 6; ++n) {
    auto bad = p;
    auto before = s;
    std::vector<CureCreator> creators(1);
    std::vector<CureContribution> nodes(1);
    if (n == 0)
      bad[8] = 42;
    if (n == 1)
      bad[0] = unsigned(-257);
    if (n == 2)
      bad[2] = UINT32_MAX;
    if (n == 3)
      bad[6] = 1;
    if (n == 4) {
      bad[6] = 0;
      creators[0].contributions[0] = 0;
      nodes[0].next = 0;
    }
    if (n == 5) {
      bad[6] = 0;
      creators[0].contributions[0] = 2;
    }
    bool failed = false;
    try {
      createCurePresentation(s, 0, bad, creators, nodes);
    } catch (const std::invalid_argument &) {
      failed = true;
    }
    require(failed && !s.records[0].placement.active && s.scan == before.scan,
            "native creation refusal before publication");
  }
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CURE_REFERENCE
  require(argc == 2, "expected pinned PE");
  install(argv[1]);
#else
  (void)argv;
  require(argc == 1, "unexpected argument");
#endif
  refusals();
  unsigned creations = 0, bindings = 0, ticks = 0, releases = 0,
           recyclerCalls = 0;
  for (fixture = 0; fixture < 192; ++fixture) {
    auto s = makeCurePresentation(8, 8, 3, 5);
    for (auto &c : s.cells) {
      c.terrain = fixture % 3;
      c.flags = (fixture % 5 == 0 ? 0x40000000u : 0) |
                (fixture % 7 == 0 ? 0x20000000u : 0) | 0x12380;
      c.otherA = fixture % 11 == 0 ? 0 : NoEffect;
    }
    s.scan = fixture % 6;
    std::vector<CureCreator> creators(2);
    creators[0].owner = fixture % 8;
    creators[1].owner = 0x80000000u + fixture;
    std::vector<CureContribution> contributions{
        {1, 99}, {std::nullopt, fixture}, {std::nullopt, std::nullopt}};
    creators[0].contributions = {0, 2};
    creators[1].contributions = {std::nullopt, 1};
    auto d = definition(fixture);
#ifdef MNM_CURE_REFERENCE
    initialize(s, creators, contributions, d);
#endif
    for (unsigned slot : {4u, 2u, 0u, 3u, 1u}) {
      CastEffectDescriptor p{};
      for (unsigned i = 0; i < 63; ++i)
        p[i] = fixture * 7919 + slot * 41 + i;
      p[0] = unsigned(16 + (int(slot % 3) - 1) * 256);
      p[1] = unsigned(17 + (int(slot % 3) - 1) * 256);
      p[2] = INT32_MAX;
      p[3] = unsigned(144 + (int(fixture % 3) - 1) * 256);
      p[4] = unsigned(145 + (int(fixture % 3) - 1) * 256);
      p[5] = fixture % 2 ? 47 : 0;
      p[6] = fixture % 4 == 0 ? UINT32_MAX : fixture % 2;
      p[7] = 1;
      p[8] = 41;
      queries.clear();
      createCurePresentation(s, slot, p, creators, contributions);
      ++creations;
#ifdef MNM_CURE_REFERENCE
      const auto expected = queries;
      compare(
          s,
          [&] {
            using Create = void(__attribute__((thiscall)) *)(void *, unsigned,
                                                             const void *);
            reinterpret_cast<Create>(0x493f20)(
                reinterpret_cast<void *>(recordPointer(slot)), 11, p.data());
          },
          expected);
#endif
      queries.clear();
      bindCurePresentation(s, slot, d);
      ++bindings;
#ifdef MNM_CURE_REFERENCE
      compare(s,
              [&] {
                using Setup =
                    void(__attribute__((thiscall)) *)(void *, unsigned);
                reinterpret_cast<Setup>(0x494ab0)(
                    reinterpret_cast<void *>(recordPointer(slot)), 77);
              },
              {});
#endif
      for (unsigned t = 0; t < 24; ++t) {
        if (t == 8 && fixture % 6 == 3) {
          s.records[slot].animation->player.requestBreak();
#ifdef MNM_CURE_REFERENCE
          put(reinterpret_cast<void *>(recordPointer(slot) + 0x160), 32, 1);
#endif
        }
        const auto event = tickCurePresentation(s, slot);
        hash(unsigned(event));
        ++ticks;
#ifdef MNM_CURE_REFERENCE
        compare(s,
                [&] {
                  using Tick = int(__attribute__((thiscall)) *)(void *);
                  require(
                      reinterpret_cast<Tick>(0x464ec0)(reinterpret_cast<void *>(
                          recordPointer(slot) + 0x160)) == event,
                      "original raw ANI event");
                },
                {});
#endif
      }
      hashState(s);
    }
    for (unsigned slot : {2u, 4u, 0u, 1u, 3u}) {
      queries.clear();
      releaseCurePresentation(s, slot, query);
      ++releases;
      recyclerCalls += unsigned(queries.size());
      hashState(s);
#ifdef MNM_CURE_REFERENCE
      const auto expected = queries;
      compare(
          s,
          [&] {
            using Release = void(__attribute__((thiscall)) *)(void *);
            reinterpret_cast<Release>(0x4943f0)(
                reinterpret_cast<void *>(recordPointer(slot)));
          },
          expected);
#endif
    }
    require(s.scan == 0, "all released high water");
#ifdef MNM_CURE_REFERENCE
    for (const auto &body : bodies)
      require(!std::memcmp(reinterpret_cast<void *>(body.first),
                           body.second.data(), body.second.size()),
              "original target body/table changed");
#endif
  }
  require(recyclerCalls > 0, "recycler branch exercised");
  std::cout << "{\"success\":true,\"fixtures\":192,\"creations\":" << creations
            << ",\"bindings\":" << bindings << ",\"ticks\":" << ticks
            << ",\"releases\":" << releases
            << ",\"recycler_calls\":" << recyclerCalls
            << ",\"cases\":" << creations + bindings + ticks + releases
            << ",\"outcome_digest\":\"" << std::hex << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << "fixture " << fixture << ": " << e.what() << '\n';
  return 1;
}
