// Reuse the established value-address storage and provider ABIs without
// changing its historical fixture or routing. This suite supplies its own owned
// session, allocator, complete snapshots and original callback bridge.
#define main unused_actor_services_reference_main
#include "actor-services-reference.cpp"
#undef main
#include "../reconstruction/rendering/actor_frontier_providers.hpp"
#include <map>

namespace {
constexpr U Heap = Base + 0x40000, HeapLimit = Base + 0xf0000;
constexpr std::array<std::array<U, 2>, 9> SessionGlobals{{{{0x690600, 0x12000}},
                                                          {{0x644550, 0x12000}},
                                                          {{0x5d854c, 4}},
                                                          {{0x5d865c, 4}},
                                                          {{0x656634, 4}},
                                                          {{0x6e1ac8, 1008}},
                                                          {{0x5c7264, 8}},
                                                          {{0x6e9218, 1008}},
                                                          {{0x6a29d8, 1008}}}};
constexpr int FrontierSmall[] = {-1, -1, -1, -1, -1, 24, 32, 40, 24,
                                 -1, 40, -1, -1, 32, -1, 32, -1, -1,
                                 40, -1, 24, 40, 32, 24, -1};
constexpr int FrontierLarge[] = {24, 32, 24, 40, 24, 40, -1, -1, -1,
                                 32, 24, -1, -1, -1, 24, 32, -1, -1,
                                 -1, 40, 24, 40, 24, 32, 24};
constexpr int FrontierFacing[] = {-1, -1, -1, -1, -1, 5, 6,  6, 7, -1, 4, 4, 6,
                                  0,  -1, 4,  2,  0,  0, -1, 3, 2, 2,  1, -1};

constexpr U Map = Base + 0x15000;
constexpr std::array<std::array<U, 2>, 9> ExtraGlobals{{{{0x656608, 4}},
                                                        {{0x5e143c, 36}},
                                                        {{0x5e51e4, 300}},
                                                        {{0x5e1478, 4}},
                                                        {{0x6f68d8, 4}},
                                                        {{0x6f98c0, 32}},
                                                        {{0x6de6dd, 4}},
                                                        {{0x6de0e8, 0x500}},
                                                        {{0x6e9850, 4}}}};
struct FrontierMemory : Memory {
  struct Block {
    U at, size;
    bool alive;
  };
  std::vector<Block> blocks;
  std::map<U, U> originalHints;
  U cursor = Heap + 32, allocations = 0, failAllocation = 0, frames = 0;
  FrontierMemory() {
    std::vector<unsigned char> space(0x100000);
    owned.swap(space);
  }
  U hash() {
    U value = 2166136261u;
    auto add = [&](U at, U size, bool node = false) {
      for (U i = 0; i < size; ++i)
        value = (value ^ (node && i == 0x25 ? 0 : get8(at + i))) * 16777619u;
    };
    add(Base, 0x18000);
    for (auto range : Globals)
      add(range[0], range[1]);
    for (auto range : ExtraGlobals)
      add(range[0], range[1]);
    for (const auto &f : files) {
      value = (value ^ f.id) * 16777619u;
      value = (value ^ f.cursor) * 16777619u;
      value = (value ^ U(f.open)) * 16777619u;
    }
    for (const auto &block : blocks) {
      value = (value ^ block.alive) * 16777619u;
      add(block.at, block.size, false);
    }
    return value;
  }
  U allocate(U size, U originalHint = ~0u) {
    ++allocations;
    if (allocations == failAllocation)
      return 0;
    U aligned = (size + 15) & ~15u;
    if ((cursor & 0xffff) < 32)
      cursor = (cursor & ~65535u) + 32;
    if ((cursor & 0xffff) + aligned > 65536)
      cursor = (cursor + 65535) & ~65535u, cursor += 32;
    if (cursor + aligned > HeapLimit)
      throw std::runtime_error("Session heap extent");
    if ((cursor & 0xffff) < 32 || (cursor & 0xffff) + aligned > 65536)
      throw std::runtime_error("Session allocation overlaps a heap header");
    U at = cursor;
    cursor += aligned;
    U page = at & ~65535u;
    set32(page + 8, 0x1001f200);
    set16(page + 14, 0xcad0);
    std::memset(pointer(at, size), 0, size);
    blocks.push_back({at, size, true});
    if (size == 0x8d && originalHint != ~0u)
      originalHints.emplace(at, originalHint);
    return at;
  }
  void free(U address) {
    if (!address)
      return;
    for (auto &block : blocks)
      if (block.at == address && block.alive) {
        block.alive = false;
        return;
      }
    throw std::runtime_error("Session free requires one live allocation");
  }
  void verifyHints() {
#ifdef MNM_ORIGINAL_REFERENCE
    for (auto [node, value] : originalHints)
      if ((get32(node + 0x41) | get32(node + 0x45)) &&
          get8(node + 0x25) != value)
        throw std::runtime_error("Original source byte25 machine provenance");
#endif
  }
  std::vector<unsigned char> ani, spr;
  struct File {
    U id, cursor;
    bool open;
    bool sprite;
  };
  std::vector<File> files;
  U readCount = 0, builderCount = 0, blits = 0;
  U localRead(U p) {
    U v;
    std::memcpy(&v, localPointer(p, 4), 4);
    return v;
  }
  void localWrite(U p, U v) { std::memcpy(localPointer(p, 4), &v, 4); }
  U external(U id, U self, U second, std::initializer_list<U> args) {
    U a[12]{};
    std::copy(args.begin(), args.end(), a);
    U rawSelf = self;
    std::vector<U> normalized(args), payload;
    auto temp = [&](U v) { return isTemporary(v) ? 0x20000000 : v; };
    auto bytes = [&](U p, U n) {
      for (U k = 0; k < n; ++k)
        payload.push_back(localPointer(p + k, 1)[0]);
    };
    auto string = [&](U p) {
      U n = 0;
      while (localPointer(p + n, 1)[0])
        ++n;
      payload.push_back(n + 1);
      bytes(p, n + 1);
    };
    bool handled = true;
    switch (id) {
    case 0x4a1120:
      string(self);
      self = temp(self);
      break;
    case 0x4f3320:
      if (isTemporary(a[3])) {
        normalized[3] = 0x20000000;
        bytes(a[3], 22);
      }
      break;
    case 0x59c359:
      normalized[0] = temp(a[0]);
      break;
    case 0x5c50c0:
      string(a[0]);
      normalized[0] = temp(a[0]);
      normalized[2] = 0x20000000;
      normalized[3] = 0x20000004;
      break;
    case 0x5c520c:
      normalized[1] = temp(a[1]);
      break;
    case 0x5c51fc:
      normalized[1] = 0x20000000;
      bytes(a[1], 8);
      break;
    case 0x1001f500:
      normalized[1] = 0x20000000;
      break;
    case 0x1001f540:
      normalized[1] = 0x20000000;
      normalized[3] = temp(a[3]);
      bytes(a[1], 16);
      bytes(a[3], 16);
      break;
    case 0x1001f580:
      normalized[4] = temp(a[4]);
      bytes(a[4], 16);
      break;
    case 0x1001f600:
      normalized[2] = 0x20000000;
      bytes(a[2], 8);
      break;
    case 0x1001f5c0:
    case 0x1001f640:
    case 0x484c00:
    case 0x4a2f90:
    case 0x59c4c3:
    case 0x59cf25:
    case 0x59d9eb:
    case 0x5b74d8:
    case 0x5c5208:
    case 0x598a10:
    case 0x1001f200:
    case 0x597df0:
    case 0x504cc0:
    case 0x504ec0:
    case 0x505160:
    case 0x4ed440:
    case 0x522720:
    case 0x48a270:
    case 0x546c20:
    case 0x513ff0:
    case 0x5120c0:
    case 0x4970b0:
    case 0x57d560:
      break;
    default:
      handled = false;
    }
    bool builder = id == 0x582440 || id == 0x582a80 || id == 0x583330 ||
                   id == 0x583bb0 || id == 0x586660 || id == 0x5870d0 ||
                   id == 0x585d80 || id == 0x587e70 || id == 0x588730 ||
                   id == 0x588bf0;
    if (builder) {
      handled = true;
      bytes(a[0], 768);
    }
    for (U k = 1; k <= 16; ++k)
      if (id && id == s::behaviorCallback(k))
        handled = true;
    if (!handled) {
      events.insert(events.end(), {0xface0010, hash()});
      return Memory::external(id, self, second, args);
    }
    events.insert(events.end(),
                  {0xface0020, hash(), id, self, second, U(args.size())});
    events.insert(events.end(), normalized.begin(), normalized.end());
    events.insert(events.end(), payload.begin(), payload.end());
    if (builder) {
      ++builderCount;
      if (variant % 16 == 15 && builderCount == 2)
        return 0;
      U n = allocate(20);
      set32(second, n);
      set32(n, 0);
      set32(n + 4, id);
      set32(n + 8, self);
      set32(n + 12, a[0]);
      return 1;
    }
    switch (id) {
    case 0x598a10:
      return allocate(a[1]);
    case 0x1001f200:
      free(self);
      return 0;
    case 0x597df0:
      return 0;
    case 0x4a1120: {
      if (variant % 16 == 14)
        return 0;
      bool sprite = false;
      for (U k = 0; localPointer(rawSelf + k, 1)[0]; ++k)
        if (localPointer(rawSelf + k, 1)[0] == 's')
          sprite = true;
      U f = Other + 0x1c00 + U(files.size()) * 16;
      files.push_back({f, 0, true, sprite});
      return f;
    }
    default:
      break;
    }
    if (id == 0x59c359 || id == 0x59cf25 || id == 0x59c4c3 || id == 0x5b74d8 ||
        id == 0x59d9eb) {
      U f = id == 0x59c359 ? a[3] : a[0];
      for (auto &file : files)
        if (file.id == f) {
          auto &data = file.sprite ? spr : ani;
          if (id == 0x5b74d8)
            return f;
          if (id == 0x59d9eb)
            return U(data.size());
          if (id == 0x59c4c3) {
            file.open = false;
            return 0;
          }
          if (id == 0x59cf25) {
            file.cursor = a[1];
            return 0;
          }
          U n = std::min(a[1] * a[2], U(data.size()) - file.cursor);
          ++readCount;
          if (variant % 16 == 13 && readCount == 2 && n)
            n -= 1;
          std::memcpy(localPointer(a[0], n), data.data() + file.cursor, n);
          file.cursor += n;
          return n / a[1];
        }
      throw std::runtime_error("Unknown supplied file handle");
    }
    switch (id) {
    case 0x504cc0:
    case 0x504ec0:
      set32(Other + 0x1b00, get32(Other + 0x1b00) + id + second);
      return (variant + second) % 7 != 0;
    case 0x4970b0:
      return get32(0x6def58) + a[0] * 0xe4b;
    case 0x505160:
      set32(self + 0x1b0, get32(self + 0xd8) + get32(self + 0x108));
      return 0;
    case 0x4f3320:
      return variant % 8 == 0
                 ? 0
                 : ((a[0] + a[1] * 3 + a[2] + variant) % 5 ? 0x12345678u : 0);
    case 0x522720:
      set16(a[0], 7);
      set32(a[0] + 2, get32(self + 0x174));
      return 0;
    case 0x48a270:
      return variant & 1;
    case 0x546c20:
      set32(self, Other + 0x1a00);
      set32(Other + 0x1a00, 0x545e60);
      set32(Other + 0x1a00 + 20, 0x54aa00);
      return self;
    case 0x5c50c0: {
      const char *value = "C:/fixture/game.ani";
      std::memcpy(localPointer(a[2], 20), value, 20);
      localWrite(a[3], a[2] + 11);
      return variant % 16 == 12 ? 0 : 19;
    }
    case 0x5c51fc:
      localWrite(a[1], 37);
      localWrite(a[1] + 4, U(-11));
      return 1;
    case 0x1001f500:
      localWrite(a[1], 0x1234);
      return variant & 8 ? 1 : 0;
    case 0x1001f540:
    case 0x1001f580:
      ++blits;
      return blits <= variant % 3 ? (blits == 1 ? 0x887601c2u : 0x8876021cu)
                                  : 0;
    case 0x1001f5c0:
      return variant & 16 ? 1 : 0;
    default:
      return 0;
    }
  }
  U invokeFast(U entry, U self, U second) {
    return dispatch(entry, self, {}, second);
  }
  U fastcall(U entry, U self, U second, std::initializer_list<U> args) {
    return dispatch(entry, self, args, second);
  }
  U invoke(U entry, U self, std::initializer_list<U> args) {
    return dispatch(entry, self, args);
  }
  U com(U id, U buffer, std::initializer_list<U> args) {
    return args.size() ? external(id, 0, 0, {buffer, *args.begin()})
                       : external(id, 0, 0, {buffer});
  }
  U dispatch(U entry, U self, std::initializer_list<U> args, U second = 0) {
#ifndef MNM_ORIGINAL_REFERENCE
    if (entry == 0x520790 && args.size() == 1 && *args.begin() == 0x20000000)
      return invoke(entry, self, {temporary(20)});
#endif
#ifdef MNM_ORIGINAL_REFERENCE
    U values[12]{}, i = 0;
    for (U value : args)
      values[i++] = value;
    for (const auto &abi : SessionAbis)
      if (abi.id == entry)
        return actor_services_call(entry, self, second, U(args.size()), values,
                                   abi.pop);
    for (const auto &spec : ServiceProviders)
      if (spec.id == entry) {
        U target = entry >= 0x5c0000 && entry < Base ? get32(entry) : entry;
        return actor_services_call(target, self, second, U(args.size()), values,
                                   spec.pop);
      }
    throw std::runtime_error("Missing original session ABI: " +
                             std::to_string(entry));
#else
    if (auto result = s::FrontierComposition<FrontierMemory>(*this).tryRun(
            entry, self, args, second)) {
      if (std::find(completedServices.begin(), completedServices.end(),
                    entry) == completedServices.end())
        completedServices.push_back(entry);
      return *result;
    }
    if (entry == 0x59cb60) {
      auto it = args.begin();
      U dst = *it++, src = *it++, count = *it;
      std::memmove(pointer(dst, count), pointer(src, count), count);
      return dst;
    }
    return external(entry, self, second, args);
#endif
  }
  void initialize(U seed) {
    reset(seed);
    std::memset(pointer(Base, 0x18000), 0, 0x18000);
    for (auto range : ExtraGlobals)
      std::memset(pointer(range[0], range[1]), 0, range[1]);
    set32(0x5f2094, Other);
    set32(0x5f1ec8, 0);
    set8(0x5f20e8, 0);
    set32(0x6c5494, 6);
    set32(0x6c5498, 6);
    set32(0x6c549c, 4);
    set32(0x6c54a0, 36);
    set32(0x6c54dc, Map);
    for (U k = 0; k < 6; ++k)
      set32(0x6cb942 + k * 4, k * 6);
    for (U k = 0; k < 4; ++k)
      set32(0x6cb8c2 + k * 4, k * 36);
    set32(0x6cb8be, U(-36));
    for (U k = 0; k < 144; ++k) {
      U c = Map + k * 12;
      set16(c, 0);
      set16(c + 2, 0xffff);
      set16(c + 4, 0xffff);
      set16(c + 6, 0xffff);
    }
    set32(0x65660c, Catalog);
    set32(0x656608, Other + 0x800);
    set32(Other + 0x80c, 8);
    set32(Catalog + 0x164 + 0x94, 16);
    set32(Catalog + 0x2c8 + 0x94, 8);
    set32(0x6def58, Pool);
    set32(0x6def5c, 4);
    set32(0x6df180, 4);
    set32(0x5fed7c, 0);
    set32(0x6898dc, 0);
    set32(0x6e1f88, 0);
    set32(0x5e1478, seed & 8 ? 60 : 0);
    set32(0x5e143c, 31);
    set32(0x5e1440, 67);
    set32(0x5e1444, 127);
    for (U k = 0; k < 4; ++k) {
      U p = Pool + k * 0xe4b;
      set32(p, k);
      set32(p + 4, 1);
      set32(p + 0xe4, 1);
      set32(p + 0xac, Catalog);
      set32(p + 0x174, k + 3);
    }
    set32(Object + 0xac, Catalog);
    set32(Object + 0xe4, 1);
    set32(Object + 0x8f9, Other + 0x900);
    set32(Other + 0x904, Nodes);
    set32(Nodes, Nodes);
    const char *name = "game.ani";
    std::memcpy(pointer(Buffer, 9), name, 9);
    const char *sn = "game.spr";
    std::memcpy(pointer(Buffer + 32, 9), sn, 9);
    prepareAnimation(seed);
    prepareSprite(seed);
    for (U j = 0; j < 25; ++j) {
      set32(0x5e51e4 + j * 4, U(FrontierSmall[j]));
      set32(0x5e5248 + j * 4, U(FrontierLarge[j]));
      set32(0x5e52ac + j * 4, U(FrontierFacing[j]));
    }
    set32(0x6e9850, seed % 4);
  }
  void prepareAnimation(U seed) {
    U version = 3 + seed % 3,
      stride = version == 3   ? 28
               : version == 4 ? 36
                              : 44,
      count = 265;
    ani.assign(44 + count * 4 + count * 3 * stride, 0);
    auto put = [&](U at, U value) { std::memcpy(ani.data() + at, &value, 4); };
    put(4, U(ani.size()));
    put(12, version);
    put(20, count);
    std::memcpy(ani.data() + 24, "game.spr", 9);
    for (U k = 0; k < count; ++k) {
      put(44 + k * 4, k * 3);
      U at = 44 + count * 4 + k * 3 * stride;
      put(at, 0);
      put(at + 8, k + 10);
      put(at + stride, 0);
      put(at + stride + 8, k + 14 + (seed % 11));
      put(at + 2 * stride, 6);
    }
    U descriptor = 0x6dbf00;
    set32(descriptor, Other + 0x1000);
    set32(Other + 0x1014, count);
    set32(descriptor + 4, Base + 0x25000);
    set32(descriptor + 8, Base + 0x27000);
    for (U k = 0; k < count; ++k) {
      set32(Base + 0x25000 + k * 4, k * 3);
      U at = Base + 0x27000 + k * 132;
      set32(at, 0);
      set32(at + 8, k + 10);
      set32(at + 44, 0);
      set32(at + 52, k + 14 + seed % 11);
      set32(at + 88, 6);
    }
    set32(Object + 0xb0 + 28, descriptor);
    set32(Object + 0x825 + 28, descriptor);
  }
  void prepareSprite(U seed) {
    U palettes = seed % 6, frames = 2;
    U total = 24 + palettes * 768 + frames * 4 + frames * 32;
    spr.assign(total, 0);
    auto put = [&](U at, U v) { std::memcpy(spr.data() + at, &v, 4); };
    put(4, total);
    put(8, seed % 16 == 11 ? 3 : 4);
    put(12, frames);
    put(16, palettes);
    for (U k = 24; k < 24 + palettes * 768; ++k)
      spr[k] = static_cast<unsigned char>(k * 17 + seed);
    U offsets = 24 + palettes * 768;
    for (U k = 0; k < frames; ++k) {
      put(offsets + k * 4, k * 32);
      put(offsets + frames * 4 + k * 32 + 28, palettes ? k % palettes : 0);
    }
  }
  void snapshotFrontier(U root, U result, U phase) {
    auto emit = [](const void *data, std::size_t size) {
      auto *p = static_cast<const unsigned char *>(data);
      while (size) {
        ssize_t n = write(STDOUT_FILENO, p, size);
        if (n < 0 && errno == EINTR)
          continue;
        if (n <= 0)
          _exit(89);
        p += n;
        size -= std::size_t(n);
      }
    };
    U header[] = {0x46525031,       root,
                  variant,          phase,
                  result,           U(events.size()),
                  U(blocks.size()), U(files.size())};
    emit(header, sizeof(header));
    emit(pointer(Base, 0x18000), 0x18000);
    emit(pointer(Base + 0x25000, 0x10000), 0x10000);
    for (auto range : Globals)
      emit(pointer(range[0], range[1]), range[1]);
    for (auto range : ExtraGlobals)
      emit(pointer(range[0], range[1]), range[1]);
    for (const auto &b : blocks) {
      U fields[] = {b.at, b.size, U(b.alive)};
      emit(fields, sizeof(fields));
      emit(pointer(b.at, b.size), b.size);
    }
    for (auto &f : files) {
      U fields[] = {f.id, f.cursor, U(f.open), U(f.sprite)};
      emit(fields, sizeof(fields));
    }
    emit(events.data(), events.size() * 4);
    events.clear();
  }
};
static FrontierMemory *frontierActive;
#ifdef MNM_ORIGINAL_REFERENCE
extern "C" __attribute__((force_align_arg_pointer))
U frontier_provider(U id, U self, U *args, U second) {
  try {
    const ServiceProvider *spec = nullptr;
    for (const auto &s : ServiceProviders)
      if (s.id == id)
        spec = &s;
    if (!spec)
      _exit(84);
    if (!spec->receiver)
      self = 0;
    if (!spec->second)
      second = 0;
    switch (spec->arguments) {
    case 0:
      return frontierActive->external(id, self, second, {});
    case 1:
      return frontierActive->external(id, self, second, {args[0]});
    case 2:
      return frontierActive->external(id, self, second, {args[0], args[1]});
    case 3:
      return frontierActive->external(id, self, second,
                                      {args[0], args[1], args[2]});
    case 4:
      return frontierActive->external(id, self, second,
                                      {args[0], args[1], args[2], args[3]});
    case 5:
      return frontierActive->external(
          id, self, second, {args[0], args[1], args[2], args[3], args[4]});
    case 6:
      return frontierActive->external(
          id, self, second,
          {args[0], args[1], args[2], args[3], args[4], args[5]});
    case 7:
      return frontierActive->external(
          id, self, second,
          {args[0], args[1], args[2], args[3], args[4], args[5], args[6]});
    case 8:
      return frontierActive->external(id, self, second,
                                      {args[0], args[1], args[2], args[3],
                                       args[4], args[5], args[6], args[7]});
    case 11:
      return frontierActive->external(id, self, second,
                                      {args[0], args[1], args[2], args[3],
                                       args[4], args[5], args[6], args[7],
                                       args[8], args[9], args[10]});
    default:
      throw std::runtime_error("Session provider arity");
    }
  } catch (const std::exception &error) {
    std::fprintf(stderr, "session provider %x: %s\n", id, error.what());
    _exit(86);
  }
}
void installFrontierProviders() {
  std::vector<std::array<U, 2>> ranges;
  U index = 0;
  for (const auto &spec : ServiceProviders) {
    U at = spec.id >= Base ? spec.id : Code + index * 32;
    ++index;
    if (at < Base + 0x1f000 || at + 26 > Base + 0x24000)
      throw std::runtime_error("Session trampoline extent");
    for (auto range : ranges)
      if (at < range[1] && range[0] < at + 26)
        throw std::runtime_error("Session trampoline overlap");
    ranges.push_back({at, at + 26});
  }
  for (const auto &spec : ServiceProviders)
    if (spec.id < Base &&
        std::memcmp(reinterpret_cast<const void *>(spec.id), spec.bytes, 8))
      throw std::runtime_error("Frontier provider preflight guard: " +
                               std::to_string(spec.id));
  index = 0;
  for (const auto &spec : ServiceProviders) {
    U at = spec.id >= Base ? spec.id : Code + index * 32;
    ++index;
    auto *p = reinterpret_cast<unsigned char *>(at);
    const unsigned char prefix[] = {0x55, 0x89, 0xe5, 0x52, 0x8d,
                                    0x45, 0x08, 0x50, 0x51, 0x68};
    std::memcpy(p, prefix, sizeof(prefix));
    std::memcpy(p + 10, &spec.id, 4);
    p[14] = 0xe8;
    U relative =
        U(reinterpret_cast<std::uintptr_t>(frontier_provider)) - at - 19;
    std::memcpy(p + 15, &relative, 4);
    const unsigned char tail[] = {0x83, 0xc4, 0x10, 0xc9, 0xc2};
    std::memcpy(p + 19, tail, sizeof(tail));
    std::uint16_t pop = std::uint16_t(spec.pop);
    std::memcpy(p + 24, &pop, 2);
    if (spec.id < Base) {
      auto *target = reinterpret_cast<unsigned char *>(spec.id);
      if (spec.id >= 0x5c0000)
        std::memcpy(target, &at, 4);
      else {
        target[0] = 0xe9;
        relative = at - spec.id - 5;
        std::memcpy(target + 1, &relative, 4);
      }
    }
  }
}
#endif
void frontierFault(int signal, siginfo_t *info, void *context) {
#ifdef MNM_ORIGINAL_REFERENCE
  U pc = U(static_cast<ucontext_t *>(context)->uc_mcontext.gregs[REG_EIP]);
#else
  U pc = U(static_cast<ucontext_t *>(context)->uc_mcontext.gregs[REG_RIP]);
#endif
  std::fprintf(stderr, "frontier_fault %x %u signal %d pc %x address %p\n",
               U(runningEntry), U(runningVariant), signal, pc, info->si_addr);
  _exit(128 + signal);
}
} // namespace
#include "actor-frontier-seeds.inc"
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected pinned original");
  map_image(read(argv[1]));
  OriginalLinuxSeh seh;
  if (mmap(reinterpret_cast<void *>(Base), 0x100000,
           PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
           0) == MAP_FAILED)
    throw std::runtime_error("Frontier owned map");
  if (std::memcmp(reinterpret_cast<const void *>(0x5e51e4), FrontierSmall,
                  100) ||
      std::memcmp(reinterpret_cast<const void *>(0x5e5248), FrontierLarge,
                  100) ||
      std::memcmp(reinterpret_cast<const void *>(0x5e52ac), FrontierFacing,
                  100))
    throw std::runtime_error("Pinned original attack-selection tables differ");
  installFrontierProviders();
  if (!bodiesUnchanged())
    throw std::runtime_error("Compared bodies changed at installation");
#else
  (void)argc;
  (void)argv;
#endif
  U variants =
      std::getenv("MNM_FRONTIER_VARIANTS")
          ? U(std::strtoul(std::getenv("MNM_FRONTIER_VARIANTS"), nullptr, 10))
          : 64;
  for (U root : FrontierRoots)
    for (U seed = 0; seed < variants; ++seed) {
      std::cout.flush();
      pid_t child = fork();
      if (child < 0)
        throw std::runtime_error("Frontier fork");
      if (child) {
        int status = 0;
        if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
            WEXITSTATUS(status))
          throw std::runtime_error("Frontier case process failed");
        continue;
      }
      struct sigaction action{};
      action.sa_sigaction = frontierFault;
      action.sa_flags = SA_SIGINFO;
      sigemptyset(&action.sa_mask);
      for (int signal : {SIGSEGV, SIGFPE, SIGILL, SIGABRT})
        sigaction(signal, &action, nullptr);
      FrontierMemory memory;
      frontierActive = &memory;
      active = &memory;
      runningEntry = root;
      runningVariant = seed;
      memory.initialize(seed);
      std::fprintf(stderr, "frontier_start %x %u\n", root, seed);
      U result = executeFrontier(memory, root, seed);
      memory.snapshotFrontier(root, result, 0);
#ifdef MNM_ORIGINAL_REFERENCE
      seh.check();
      if (!bodiesUnchanged())
        throw std::runtime_error("Compared frontier body changed");
#endif
      // Check original partial resources before explicit caller-owned teardown.
      for (auto &f : memory.files)
        if (f.open)
          memory.invoke(0x59c4c3, 0, {f.id});
      for (auto &b : memory.blocks)
        if (b.alive)
          memory.invoke(0x5979b0, 0, {b.at});
      memory.snapshotFrontier(root, 0, 1);
      std::fprintf(stderr, "frontier_returned %x %u live 0\n", root, seed);
      _exit(0);
    }
  return 0;
} catch (const std::exception &e) {
  std::fprintf(stderr, "frontier: %s\n", e.what());
  return 1;
}
