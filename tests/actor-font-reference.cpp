// Reuse the established value-address storage and provider ABIs without
// changing its historical fixture or routing. This suite supplies its own owned
// session, allocator, complete snapshots and original callback bridge.
#define main unused_actor_services_reference_main
#include "actor-services-reference.cpp"
#undef main
#include "../reconstruction/rendering/actor_font_providers.hpp"
#include "actor-font-corpus.hpp"
#include <map>

namespace {
constexpr U Heap = Base + 0x200000, HeapLimit = Base + 0x1600000;
constexpr int FrontierSmall[] = {-1, -1, -1, -1, -1, 24, 32, 40, 24,
                                 -1, 40, -1, -1, 32, -1, 32, -1, -1,
                                 40, -1, 24, 40, 32, 24, -1};
constexpr int FrontierLarge[] = {24, 32, 24, 40, 24, 40, -1, -1, -1,
                                 32, 24, -1, -1, -1, 24, 32, -1, -1,
                                 -1, 40, 24, 40, 24, 32, 24};
constexpr int FrontierFacing[] = {-1, -1, -1, -1, -1, 5, 6,  6, 7, -1, 4, 4, 6,
                                  0,  -1, 4,  2,  0,  0, -1, 3, 2, 2,  1, -1};

constexpr U Map = Base + 0x15000, Program = Base + 0x100000,
            Rgb = Base + 0x150000;
constexpr std::array<std::array<U, 2>, 14> ExtraGlobals{{{{0x5f18d0, 0x50}},
                                                         {{0x6f56ac, 0xa10}},
                                                         {{0x6c007d, 4}},
                                                         {{0x5e15f0, 4}},
                                                         {{0x5c5a88, 8}},
                                                         {{0x656608, 4}},
                                                         {{0x5e143c, 36}},
                                                         {{0x5e51e4, 300}},
                                                         {{0x5e1478, 4}},
                                                         {{0x6f68d8, 4}},
                                                         {{0x6f98c0, 32}},
                                                         {{0x6de6dd, 4}},
                                                         {{0x6de0e8, 0x500}},
                                                         {{0x6e9850, 4}}}};
constexpr U FontCanvas = Base + 0x160000, FontObject = 0x656680;
constexpr std::array<std::array<U, 2>, 7> FontGlobals{{{{0x656680, 0xe3c}},
                                                       {{0x6b0400, 0xe3c}},
                                                       {{0x6df1c0, 0xe3c}},
                                                       {{0x6a5138, 0xe3c}},
                                                       {{0x6f5dac, 0x104}},
                                                       {{0x6e1ed4, 4}},
                                                       {{0x6e1f7c, 6}}}};
struct FontMemory : Memory {
  struct Block {
    U at, size;
    bool alive;
  };
  std::vector<Block> blocks;
  U cursor = Heap + 32, allocations = 0, failAllocation = 0, frames = 0;
  FontMemory() {
    std::vector<unsigned char> space(0x1800000);
    owned.swap(space);
  }
  U hash() {
    U value = 2166136261u;
    auto add = [&](U at, U size, bool node = false) {
      for (U i = 0; i < size; ++i)
        value = (value ^ (node && i == 0x25 ? 0 : get8(at + i))) * 16777619u;
    };
    add(Base, 0x18000);
    add(FontCanvas, 0x10000);
    for (auto range : FontGlobals)
      add(range[0], range[1]);
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
  U allocate(U size) {
    ++allocations;
    if (allocations == failAllocation)
      return 0;
    if (!size || size > 0x40000)
      throw std::runtime_error("Owned profile/palette allocation extent");
    cursor = ((cursor + 65535) & ~65535u) + 32;
    U at = cursor;
    cursor += (size + 15) & ~15u;
    if (cursor > HeapLimit)
      throw std::runtime_error("Owned profile/palette heap budget");
    U page = at & ~65535u;
    set32(page + 8, 0x1001f200);
    set16(page + 14, 0xcad0);
    std::memset(pointer(at, size), 0, size);
    blocks.push_back({at, size, true});
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
  std::vector<unsigned char> fontData;
  U fontFailure = 0;
  U external(U id, U self, U second, std::initializer_list<U> args) {
    U fa[12]{};
    std::copy(args.begin(), args.end(), fa);
    if (id == 0x58b660) {
      events.insert(events.end(), {0xfa170004, hash(), self});
      return variant & 2 ? 0 : FontCanvas;
    }
    if (id == 0x1001f680) {
      if (fa[1] != FontCanvas)
        throw std::runtime_error("Font unmap pointer");
      events.insert(events.end(), {0xfa170005, hash(), fa[0], fa[1]});
      return 0;
    }

    if (id == 0x59b7e2) {
      events.insert(events.end(), {0xfa170001, hash(), id, fa[0]});
      return variant & 1;
    }
    if (id == 0x59cae2 || id == 0x59c359 || id == 0x59c4c3 || id == 0x5b74d8 ||
        id == 0x59d9eb) {
      U actual[4]{};
      std::copy(args.begin(), args.end(), actual);
      if (id == 0x59cae2) {
        events.insert(events.end(), {0xfa170002, hash(), id});
        for (U k = 0; localPointer(fa[0] + k, 1)[0]; ++k)
          events.push_back(localPointer(fa[0] + k, 1)[0]);
        events.push_back(0);
        if (fontFailure == 1)
          return 0;
        U handle = Other + 0x1c00 + U(files.size()) * 16;
        files.push_back({handle, 0, true, false});
        return handle;
      }
      if (id == 0x59c359 && isTemporary(actual[0]))
        actual[0] = 0x20000000;
      events.insert(events.end(), {0xfa170003, hash(), id, U(args.size())});
      events.insert(events.end(), actual, actual + args.size());
      U handle = id == 0x59c359 ? fa[3] : fa[0];
      for (auto &file : files)
        if (file.id == handle) {
          if (id == 0x5b74d8)
            return handle;
          if (id == 0x59d9eb)
            return fontFailure == 2 ? 0 : U(fontData.size());
          if (id == 0x59c4c3) {
            if (!file.open)
              throw std::runtime_error("Font double fclose");
            file.open = false;
            return 0;
          }
          if (!file.open)
            throw std::runtime_error("Font closed handle read");
          U n = std::min(fa[1] * fa[2], U(fontData.size()) - file.cursor);
          if (fontFailure == 4 && n)
            --n;
          std::memcpy(localPointer(fa[0], n), fontData.data() + file.cursor, n);
          file.cursor += n;
          return n / fa[1];
        }
      throw std::runtime_error("Font unknown file handle");
    }

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
    case 0x59c1fd:
      if (a[1] != 0x5e9ab4)
        throw std::runtime_error("Unexpected formatted profile message");
      normalized[0] = temp(a[0]);
      break;
    case 0x5c50bc:
      break;
    case 0x5c507c:
      string(a[0]);
      string(a[1]);
      string(a[2]);
      string(a[5]);
      break;
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
      if (isTemporary(a[1]))
        string(a[1]);
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
    switch (id) {
    case 0x59c1fd: {
      std::string value = "type=" + std::to_string(a[2]) +
                          " bank=" + std::to_string(a[3]) +
                          " reason=" + std::to_string(a[4]);
      std::memcpy(localPointer(a[0], U(value.size() + 1)), value.c_str(),
                  value.size() + 1);
      return U(value.size());
    }
    case 0x5c50bc: {
      const char *directories[] = {"Z:\\", "C:/fixture", "", "C:/fixture/"};
      const char *value = directories[variant % 4];
      U size = U(std::strlen(value));
      std::memcpy(localPointer(a[1], size + 1), value, size + 1);
      return size;
    }
    case 0x5c507c: {
      std::string key, section;
      for (U p = a[0]; get8(p); ++p)
        section.push_back(char(get8(p)));
      for (U p = a[1]; get8(p); ++p)
        key.push_back(char(get8(p)));
      if (variant % 4 == 0) {
        set8(a[3], 0);
        return 0;
      }
      const char *value = key == "SpecalValue" ? (variant % 4 == 1 ? "1" : "8")
                          : key == "Darkest"   ? "7"
                          : key == "BlueTint"  ? "30"
                          : key == "Contrast"  ? (variant % 4 == 2 ? "0" : "50")
                                               : (variant % 4 == 1   ? "0.25"
                                                  : variant % 4 == 2 ? "-0.5"
                                                                     : "0.5");
      U n = U(std::strlen(value));
      std::memcpy(pointer(a[3], n + 1), value, n + 1);
      return n;
    }
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
    if (auto result = s::FontComposition<FontMemory>(*this).tryRun(
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
    for (const auto &bytes : ProfilePaletteRom)
      std::memcpy(pointer(bytes.address, bytes.size), bytes.data, bytes.size);
    for (U k = 0; k < 250; ++k)
      set32(0x5eb940 + k * 4, (k + 1) % 250);
    for (U settings : {0x5f18d0u, 0x5f18f8u}) {
      set32(settings, (settings == 0x5f18d0 ? 1 : 3) + seed % 5);
      set32(settings + 4, 1 + seed % 3);
      double power = seed & 4 ? 1. : 2.;
      std::memcpy(pointer(settings + 8, 8), &power, 8);
      power = seed & 8 ? 1. : 2.;
      std::memcpy(pointer(settings + 16, 8), &power, 8);
      std::memset(pointer(settings + 24, 16), 0, 16);
    }
    for (U k = 0; k < 3072; ++k)
      set8(Rgb + k, (k * 73 + seed * 17) ^ (k >> 2));
    for (U k = 0; k < 12; ++k)
      set8(Rgb + k, 0);
    set32(0x6c007d, 1234);
    float slope = float(int(seed % 5) - 2) / 16;
    std::memcpy(pointer(0x5e15f0, 4), &slope, 4);
  }
  void prepareAnimation(U seed) {
    constexpr U count = 265, stride = 44, records = 16;
    U total = 44 + count * 4 + count * records * stride;
    ani.assign(total, 0);
    auto put = [&](U at, U value) { std::memcpy(ani.data() + at, &value, 4); };
    put(4, total);
    put(12, 5);
    put(20, count);
    std::memcpy(ani.data() + 24, "game.spr", 9);
    U descriptor = 0x6dbf00;
    set32(descriptor, Other + 0x1000);
    set32(Other + 0x1014, count);
    set32(descriptor + 4, Base + 0x25000);
    set32(descriptor + 8, Program);
    for (U bank = 0; bank < count; ++bank) {
      put(44 + bank * 4, bank * records);
      set32(Base + 0x25000 + bank * 4, bank * records);
      U pos = 0;
      for (U frame = 0; frame < 12; ++frame) {
        U at = 44 + count * 4 + (bank * records + pos++) * stride;
        put(at, 0);
        put(at + 4, bank * 17 + frame);
        put(at + 8, 100 + frame * 3 + seed % 7);
        if (frame == 5) {
          at = 44 + count * 4 + (bank * records + pos++) * stride;
          put(at, 5);
          put(at + 4, 5);
        }
      }
      U at = 44 + count * 4 + (bank * records + 13) * stride;
      put(at, 5);
      put(at + 4, 2);
      at += 44;
      put(at, 0);
      put(at + 4, bank * 17);
      put(at + 8, 100);
      put(at + 44, 6);
      std::memcpy(pointer(Program + bank * records * 44, records * 44),
                  ani.data() + 44 + count * 4 + bank * records * 44,
                  records * 44);
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
  void snapshotProviders(U root, U result, U phase) {
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
    emit(pointer(FontCanvas, 0x10000), 0x10000);
    for (auto range : FontGlobals)
      emit(pointer(range[0], range[1]), range[1]);
    emit(pointer(Base, 0x18000), 0x18000);
    emit(pointer(Base + 0x25000, 0x1000), 0x1000);
    emit(pointer(Program, 0x40000), 0x40000);
    emit(pointer(Rgb, 0x1000), 0x1000);
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
static FontMemory *providerActive;
#ifdef MNM_ORIGINAL_REFERENCE
extern "C" __attribute__((force_align_arg_pointer))
U font_provider_callback(U id, U self, U *args, U second) {
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
      return providerActive->external(id, self, second, {});
    case 1:
      return providerActive->external(id, self, second, {args[0]});
    case 2:
      return providerActive->external(id, self, second, {args[0], args[1]});
    case 3:
      return providerActive->external(id, self, second,
                                      {args[0], args[1], args[2]});
    case 4:
      return providerActive->external(id, self, second,
                                      {args[0], args[1], args[2], args[3]});
    case 5:
      return providerActive->external(
          id, self, second, {args[0], args[1], args[2], args[3], args[4]});
    case 6:
      return providerActive->external(
          id, self, second,
          {args[0], args[1], args[2], args[3], args[4], args[5]});
    case 7:
      return providerActive->external(
          id, self, second,
          {args[0], args[1], args[2], args[3], args[4], args[5], args[6]});
    case 8:
      return providerActive->external(id, self, second,
                                      {args[0], args[1], args[2], args[3],
                                       args[4], args[5], args[6], args[7]});
    case 11:
      return providerActive->external(id, self, second,
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
void installFontProviders() {
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
        U(reinterpret_cast<std::uintptr_t>(font_provider_callback)) - at - 19;
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
#include "actor-font-seeds.inc"
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected pinned original");
  map_image(read(argv[1]));
  OriginalLinuxSeh seh;
  if (mmap(reinterpret_cast<void *>(Base), 0x1800000,
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
  for (const auto &bytes : ProfilePaletteRom)
    if (std::memcmp(reinterpret_cast<const void *>(bytes.address), bytes.data,
                    bytes.size))
      throw std::runtime_error("Pinned profile/palette ROM differs");
  installFontProviders();
  if (!bodiesUnchanged())
    throw std::runtime_error("Compared bodies changed at installation");
#else
  (void)argc;
  (void)argv;
#endif
  U variants =
      std::getenv("MNM_FONT_VARIANTS")
          ? U(std::strtoul(std::getenv("MNM_FONT_VARIANTS"), nullptr, 10))
          : 64;
  for (U root : FontRoots)
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
      FontMemory memory;
      providerActive = &memory;
      active = &memory;
      runningEntry = root;
      runningVariant = seed;
      memory.initialize(seed);
      initializeFontCase(memory, seed);
      std::fprintf(stderr, "provider_start %x %u\n", root, seed);
      U result = executeFonts(memory, root, seed);
      memory.snapshotProviders(root, result, 0);
#ifdef MNM_ORIGINAL_REFERENCE
      seh.check();
      if (!bodiesUnchanged())
        throw std::runtime_error("Compared frontier body changed");
#endif
      // Check original partial resources before explicit caller-owned teardown.
      releaseFontCase(memory, root);
      for (auto &f : memory.files)
        if (f.open)
          memory.invoke(0x59c4c3, 0, {f.id});
      for (auto &b : memory.blocks)
        if (b.alive)
          memory.invoke(0x5979b0, 0, {b.at});
      memory.snapshotProviders(root, 0, 1);
      std::fprintf(stderr, "provider_returned %x %u live 0\n", root, seed);
      _exit(0);
    }
  return 0;
} catch (const std::exception &e) {
  std::fprintf(stderr, "frontier: %s\n", e.what());
  return 1;
}
