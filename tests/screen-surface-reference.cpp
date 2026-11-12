#define main unused_screen_provider_main
#include "screen-provider-reference.cpp"
#undef main
#include "../reconstruction/rendering/screen_surface_providers.hpp"
namespace {
constexpr U Draw = Base + 0x1000, Raw = Base + 0x2000, Surface = Base + 0x3000,
            Descriptor = Base + 0x600, DrawTable = Base + 0x1100,
            RawTable = Base + 0x2100, SurfaceTable = Base + 0x3100;
constexpr U Create = 0x1004f100, Query = 0x1004f140, Release = 0x1004f180,
            Page = 0x1004f1c0, Restore = 0x1004f200, Color = 0x1004f240,
            Lock = 0x1004f280, Unlock = 0x1004f2c0, Describe = 0x1004f300,
            Palette = 0x1004f340, AttachPalette = 0x1004f380,
            AttachClipper = 0x1004f3c0, Cooperative = 0x1004f400,
            Metric = 0x1004f440, Library = 0x1004f480, Procedure = 0x1004f4c0,
            PageUnlock = 0x1004f540;
struct SurfaceMemory : Memory {
  U variant = 0, mode = 0, locks = 0, procedures = 0, comReferences = 0;
  U readLocal(U p) {
#ifdef MNM_ORIGINAL_REFERENCE
    U value;
    if (original) {
      std::memcpy(&value, reinterpret_cast<void *>(p), 4);
      return value;
    }
#endif
    return get32(p);
  }
  void writeLocal(U p, U value) {
#ifdef MNM_ORIGINAL_REFERENCE
    if (original) {
      std::memcpy(reinterpret_cast<void *>(p), &value, 4);
      return;
    }
#endif
    set32(p, value);
  }
  U endpoint(U f, U p, std::initializer_list<U> args = {}) {
    U a[8]{};
    std::copy(args.begin(), args.end(), a);
    std::vector<U> normalized(args);
    if (f == Color) {
      normalized[1] = 0x20000000;
      normalized.push_back(readLocal(a[1]));
      normalized.push_back(readLocal(a[1] + 4));
    }
    if (f == Palette) {
      normalized[1] = 0x20000000;
      for (U i = 0; i < 256; ++i)
        for (U j = 0; j < 3; ++j) {
#ifdef MNM_ORIGINAL_REFERENCE
          if (original)
            normalized.push_back(
                reinterpret_cast<unsigned char *>(a[1])[i * 4 + j]);
          else
#endif
            normalized.push_back(get8(a[1] + i * 4 + j));
        }
    }
    events.insert(events.end(), {f, p, 0, U(normalized.size())});
    events.insert(events.end(), normalized.begin(), normalized.end());
    switch (f) {
    case Create:
      if (variant == 20) {
        set32(a[1], 0);
        return 0x88760020;
      }
      set32(a[1], Raw);
      ++comReferences;
      return variant == 1 ? 0x88760001 : 0;
    case Query:
      if (variant == 21) {
        set32(a[1], 0);
        return 0x88760021;
      }
      set32(a[1], Surface);
      ++comReferences;
      return variant == 2 ? 0x88760002 : 0;
    case Release:
      if (p == Raw || p == Surface || p == Base + 0x3200)
        comReferences -= comReferences ? 1 : 0;
      return 0;
    case Page:
      return variant == 3 || variant == 23 ? 0x887601c2
             : variant == 4                ? 0x88760004
                                           : 0;
    case Restore:
      return variant == 5 || variant == 23 ? 0x88760005 : 0;
    case Color:
      return 0;
    case Lock:
      ++locks;
      set32(0x6f68f0, 1280 + locks * 16);
      set32(0x6f6904, Base + 0x8000 + locks * 16);
      return (variant == 5 || variant == 6 || variant == 8 || variant == 23) &&
                     locks == 1
                 ? 0x887601c2
             : variant == 7               ? 0x88760007
             : variant == 8 && locks == 2 ? 0x88760008
                                          : 0;
    case PageUnlock:
    case Unlock:
      return 0;
    case Describe:
      set32(a[0] + 8, mode == 3 ? 0 : 480);
      set32(a[0] + 12, mode == 3 ? 0 : 640);
      return variant == 9 ? 0x88760009 : 0;
    case Palette:
      ++comReferences;
      set32(a[2], Base + 0x3200);
      set32(Base + 0x3200, SurfaceTable);
      return variant == 10 ? 1 : 0;
    case AttachPalette:
    case AttachClipper:
    case Cooperative:
      return 0;
    case 0x484c00:
      return 0;
    case 0x5c5208:
      return 1;
    case 0x5c50f4:
      return variant == 11 ? 0 : 0x12345678;
    case 0x5c50d4:
      return ++procedures == variant - 11 ? 0 : Metric;
    case Metric:
      return a[0] ? 600 : 800;
    case 0x4a2f90:
      return 0;
    default:
      throw std::runtime_error("unknown surface endpoint " + std::to_string(f));
    }
  }
  U invoke(U f, U p, std::initializer_list<U> args = {}) {
#ifdef MNM_ORIGINAL_REFERENCE
    if (original) {
      U values[8]{};
      std::copy(args.begin(), args.end(), values);
      return actor_services_call(f, p, 0, args.size(), values, args.size() * 4);
    }
#endif
    if (auto value =
            mnm::reconstruction::ScreenSurfaceProviders<SurfaceMemory>(*this)
                .tryRun(f, p, args))
      return *value;
    if (auto value =
            mnm::reconstruction::ScreenResourceProviders<SurfaceMemory>(*this)
                .tryRun(f, p, args))
      return *value;
    return endpoint(f, p, args);
  }
  U com(U f, U p, std::initializer_list<U> args = {}) {
    return endpoint(f, p, args);
  }
  void initialize(U seed) {
    Memory::initialize();
    variant = seed % 24;
    mode = seed / 24;
    for (auto region : std::array<std::array<U, 2>, 8>{{{{0x6f68d8, 120}},
                                                        {{0x6f6950, 512}},
                                                        {{0x6f8150, 4}},
                                                        {{0x6f8154, 4}},
                                                        {{0x6f8174, 4}},
                                                        {{0x6f98a0, 52}},
                                                        {{0x6e1f1c, 4}},
                                                        {{0x6de6dd, 4}}}})
      std::memset(at(region[0], region[1]), 0, region[1]);
    std::memset(at(Object, 64), 0, 64);
    std::memset(at(Descriptor, 108), 0, 108);
    set32(Descriptor, 108);
    set32(Descriptor + 4, 7);
    set32(Descriptor + 8, 480);
    set32(Descriptor + 12, 640);
    set32(Descriptor + 104, 0x840);
    set32(Draw, DrawTable);
    set32(Raw, RawTable);
    set32(Surface, SurfaceTable);
    set32(DrawTable + 0x18, Create);
    set32(DrawTable + 8, Release);
    set32(DrawTable + 0x14, Palette);
    set32(DrawTable + 0x4c, Cooperative);
    set32(RawTable, Query);
    set32(RawTable + 8, Release);
    set32(RawTable + 0x58, Describe);
    set32(RawTable + 0x80, Unlock);
    set32(SurfaceTable + 8, Release);
    set32(SurfaceTable + 0x94, Page);
    set32(SurfaceTable + 0x98, PageUnlock);
    set32(SurfaceTable + 0x6c, Restore);
    set32(SurfaceTable + 0x74, Color);
    set32(SurfaceTable + 0x64, Lock);
    set32(SurfaceTable + 0x80, Unlock);
    set32(SurfaceTable + 0x7c, AttachPalette);
    set32(SurfaceTable + 0x70, AttachClipper);
    set32(0x6f98c8, Draw);
    set32(0x6f98c4, Draw);
    set32(0x6f98cc, Draw);
    set32(0x6f98d0, Draw);
    set32(0x6f68d8, variant & 1);
    set32(0x6e1f1c, variant & 1);
    set32(0x6de6dd, 1);
    set32(0x5c50f4, Library);
    set32(0x5c50d4, Procedure);
    set32(0x5c5280, Metric);
    set32(0x656614, 0x1234);
    if (mode == 1 || mode == 2) {
      set32(Descriptor + 4, 1);
      set32(Descriptor + 104, 0x200);
      set32(0x6f8174, mode == 2);
    }
    if (variant == 22)
      set32(0x6f8154, 128);
  }
  std::vector<unsigned char> snapshot() {
    auto result = Memory::snapshot();
    for (auto region : std::array<std::array<U, 2>, 8>{{{{0x6f68d8, 120}},
                                                        {{0x6f6950, 512}},
                                                        {{0x6f8150, 4}},
                                                        {{0x6f8154, 4}},
                                                        {{0x6f8174, 4}},
                                                        {{0x6f98a0, 52}},
                                                        {{0x6e1f1c, 4}},
                                                        {{0x6de6dd, 4}}}})
      result.insert(result.end(), at(region[0], region[1]),
                    at(region[0], region[1]) + region[1]);
    return result;
  }
};
static SurfaceMemory *current;
#ifdef MNM_ORIGINAL_REFERENCE
static U bridge(U f, U self, U count, const U *args) {
  U values[8]{};
  std::copy(args, args + count, values);
  const bool com = (f >= Create && f <= Cooperative) || f == PageUnlock;
  if (com) {
    self = values[0];
    std::move(values + 1, values + count, values);
    --count;
  } else if (f != 0x484c00 && f != 0x4a2f90)
    self = 0;
  switch (count) {
  case 0:
    return current->endpoint(f, self);
  case 1:
    return current->endpoint(f, self, {values[0]});
  case 2:
    return current->endpoint(f, self, {values[0], values[1]});
  case 3:
    return current->endpoint(f, self, {values[0], values[1], values[2]});
  case 4:
    return current->endpoint(f, self,
                             {values[0], values[1], values[2], values[3]});
  default:
    throw std::runtime_error("surface endpoint arity");
  }
}
static void stub(U address, U f, U count) {
  unsigned char *code = reinterpret_cast<unsigned char *>(address);
  U i = 0;
  auto byte = [&](U v) { code[i++] = v; };
  auto word = [&](U v) {
    std::memcpy(code + i, &v, 4);
    i += 4;
  };
  byte(0x55);
  byte(0x89);
  byte(0xe5); // frame
  byte(0x8d);
  byte(0x45);
  byte(8);
  byte(0x50); // argument pointer
  byte(0x68);
  word(count);
  byte(0x51);
  byte(0x68);
  word(f);
  byte(0xb8);
  word(U(&bridge));
  byte(0xff);
  byte(0xd0);
  byte(0x83);
  byte(0xc4);
  byte(16);
  byte(0xc9);
  byte(0xc2);
  byte(count * 4);
  byte(0);
}
#endif
static Result executeSurface(bool original, U suite, U variant) {
  SurfaceMemory m;
  m.original = original;
  m.initialize(variant);
  current = &m;
  auto execute = [&] {
    if (suite == 0) {
      m.invoke(0x58ad90, Object, {Descriptor, 0, 0});
      if (m.get32(Object + 8))
        m.invoke(0x58b3e0, Object);
    }
    if (suite == 1) {
      m.set32(Object + 4, Raw);
      m.set32(Object + 8, Surface);
      m.set32(Object + 40, ~0u);
      m.invoke(0x58b660, Object);
    }
    if (suite == 2) {
      m.invoke(0x58b270, 0);
      m.invoke(0x58b270, 0);
    }
  };
  volatile bool failed = false;
#ifdef MNM_ORIGINAL_REFERENCE
  if (original) {
    const int signal = sigsetjmp(jump, 1);
    if (!signal) {
      trapping = 1;
      execute();
      trapping = 0;
    } else {
      trapping = 0;
      check(signal == SIGSEGV, "unexpected surface signal");
      failed = true;
    }
  } else
#endif
    try {
      execute();
    } catch (const WireFault &) {
      failed = true;
    }
  if (suite == 0 && !failed)
    check(m.comReferences == 0, "surface reference retirement leaked");

  guardBodies();
  auto bytes = m.snapshot();
  for (U i = 0; i < 4; ++i)
    bytes.push_back((m.comReferences >> (8 * i)) & 255);
  return {bytes, m.events, {}, failed};
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  check(argc == 2, "pinned executable");
  map_image(read(argv[1]));
  check(mmap((void *)Base, Size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "surface map");
  check(mmap((void *)0x1004f000, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "surface endpoints");
  OriginalLinuxSeh seh;
  std::signal(SIGSEGV, fault);
  std::signal(SIGFPE, fault);
  for (auto spec :
       std::array<std::array<U, 3>, 19>{{{{Create, Create, 4}},
                                         {{Query, Query, 3}},
                                         {{Release, Release, 1}},
                                         {{Page, Page, 2}},
                                         {{Restore, Restore, 1}},
                                         {{Color, Color, 3}},
                                         {{Lock, Lock, 5}},
                                         {{Unlock, Unlock, 2}},
                                         {{Describe, Describe, 2}},
                                         {{Palette, Palette, 5}},
                                         {{AttachPalette, AttachPalette, 2}},
                                         {{AttachClipper, AttachClipper, 2}},
                                         {{Cooperative, Cooperative, 1}},
                                         {{Metric, Metric, 1}},
                                         {{Library, 0x5c50f4, 1}},
                                         {{Procedure, 0x5c50d4, 2}},
                                         {{0x484c00, 0x484c00, 1}},
                                         {{PageUnlock, PageUnlock, 2}},
                                         {{0x4a2f90, 0x4a2f90, 0}}}})
    stub(spec[0], spec[1], spec[2]);
  // 5c5208 is an IAT cell, not executable storage.
  *reinterpret_cast<U *>(0x5c5208) = 0x1004f500;
  stub(0x1004f500, 0x5c5208, 1);
#else
  (void)argc;
  (void)argv;
#endif
  U cases = 0, faults = 0;
  std::uint64_t checksum = 1469598103934665603ULL;
  const U variants[] = {96, 24, 24};
  for (U suite = 0; suite < 3; ++suite)
    for (U variant = 0; variant < variants[suite]; ++variant) {
      auto native = executeSurface(false, suite, variant);
#ifdef MNM_ORIGINAL_REFERENCE
      auto original = executeSurface(true, suite, variant);
      if (!(native == original)) {
        std::cerr << "surface mismatch " << suite << ' ' << variant
                  << " events " << native.events.size() << ' '
                  << original.events.size() << '\n';
        for (U i = 0;
             i < std::min(native.events.size(), original.events.size()); ++i)
          if (native.events[i] != original.events[i]) {
            std::cerr << "event " << i << ' ' << std::hex << native.events[i]
                      << ' ' << original.events[i] << '\n';
            break;
          }
        for (U i = 0; i < native.bytes.size(); ++i)
          if (native.bytes[i] != original.bytes[i]) {
            std::cerr << "byte " << std::hex << i << ' ' << U(native.bytes[i])
                      << ' ' << U(original.bytes[i]) << '\n';
            break;
          }
        return 1;
      }
#endif
      for (U v : native.events)
        checksum = (checksum ^ v) * 1099511628211ULL;
      for (U v : native.bytes)
        checksum = (checksum ^ v) * 1099511628211ULL;
      ++cases;
      faults += native.fault;
    }
  std::cout << "{\"cases\":" << cases << ",\"faults\":" << faults
            << ",\"checksum\":" << checksum << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
