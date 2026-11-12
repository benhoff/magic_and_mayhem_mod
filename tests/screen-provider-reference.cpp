#include "../reconstruction/rendering/screen_resource_providers.hpp"
#include "../reconstruction/rendering/world_screen_retirement.hpp"
#include <array>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#include "original-linux-seh.hpp"
extern "C" unsigned screen_provider_cleanup(unsigned, unsigned);
extern "C" unsigned actor_services_call(unsigned, unsigned, unsigned, unsigned,
                                        const unsigned *, unsigned);
#endif
using U = std::uint32_t;
constexpr U Base = 0x10000000, Size = 0x40000, Object = Base + 0x100,
            Label = Base + 0x200, Vector = Base + 0x300, Text = Base + 0x400,
            Rect = Base + 0x500, Heap = Base + 0x4000;
static void check(bool v, const char *why) {
  if (!v)
    throw std::runtime_error(why);
}
struct WireFault {};
struct Memory {
  std::vector<unsigned char> owned = std::vector<unsigned char>(Size),
                             globals = std::vector<unsigned char>(0x140000);
  bool original = false;
  U top = Heap, attempt = 0, fail = 0, temporaryTop = 0, codecResult = 1,
    dcResult = Base + 0xa00;
  std::set<U> alive;
  std::vector<U> events;
  unsigned char *at(U p, U n = 4) {
    if (p >= Base && p - Base <= Size && n <= Size - (p - Base)) {
#ifdef MNM_ORIGINAL_REFERENCE
      if (original)
        return reinterpret_cast<unsigned char *>(p);
#endif
      return owned.data() + p - Base;
    }
    if (p >= 0x5c0000 && p - 0x5c0000 <= globals.size() &&
        n <= globals.size() - (p - 0x5c0000)) {
#ifdef MNM_ORIGINAL_REFERENCE
      if (original)
        return reinterpret_cast<unsigned char *>(p);
#endif
      return globals.data() + p - 0x5c0000;
    }
    throw WireFault{};
  }
  U get32(U p) {
    U v;
    std::memcpy(&v, at(p), 4);
    return v;
  }
  U get16(U p) {
    std::uint16_t v;
    std::memcpy(&v, at(p, 2), 2);
    return v;
  }
  U get8(U p) { return *at(p, 1); }
  void set32(U p, U v) { std::memcpy(at(p), &v, 4); }
  void set16(U p, U v) {
    std::uint16_t word = v;
    std::memcpy(at(p, 2), &word, 2);
  }
  void set8(U p, U v) { *at(p, 1) = v; }
  U temporary(U n) {
    U p = Base + 0x1c000 + temporaryTop;
    temporaryTop += (n + 3) & ~3u;
    check(temporaryTop < 0x3000, "scratch limit");
    return p;
  }
  void event(U f, U p, U second, std::initializer_list<U> args) {
    events.insert(events.end(), {f, p, second, U(args.size())});
    events.insert(events.end(), args.begin(), args.end());
  }
  U endpoint(U f, U p, std::initializer_list<U> args = {}, U second = 0) {
    U a[8]{};
    std::copy(args.begin(), args.end(), a);
    if (f == 0x5977d0)
      event(0x597850, 0, 0, {a[0]});
    else if (f == 0x598f70)
      event(0x597870, 0, 0, {a[0]});
    else
      event(f, p, second, args);
    if (f == 0x597850 || f == 0x5977d0) {
      if (++attempt == fail)
        return 0;
      U out = top;
      top += (a[0] + 15) & ~15u;
      check(top < Heap + 0x14000, "heap limit");
      alive.insert(out);
      return out;
    }
    if (f == 0x597870 || f == 0x598f70) {
      if (a[0])
        check(alive.erase(a[0]) == 1, "double or foreign free");
      return 0;
    }
    if (f == 0x4a5a90)
      return 0;
    if (f == 0x4a5000)
      return 0;
    if (f == 0x58b660)
      return dcResult;
    if (f == 0x1004f040)
      return codecResult;
    if (f == 0x1004f080)
      return 0;
    if (f == 0x584430 || f == 0x589280)
      return codecResult;
    if (f == 0x4711d0 || f == 0x4ffad0 || f == 0x52baf0 || f == 0x56d0a0 ||
        f == 0x535970 || f == 0x4ff9a0 || f == 0x464ad0 || f == 0x549230 ||
        f == 0x5496b0 || f == 0x549b20 || f == 0x4879f0 || f == 0x5458b0 ||
        f == 0x4ecbe0 || f == 0x573960 || f == 0x58bc10 || f == 0x46a930 ||
        f == 0x52a670)
      return 0;
    throw std::runtime_error("unknown provider endpoint " + std::to_string(f));
  }
  U invoke(U f, U p, std::initializer_list<U> a = {}) {
#ifdef MNM_ORIGINAL_REFERENCE
    if (original) {
      U values[8]{};
      std::copy(a.begin(), a.end(), values);
      U pop = a.size() * 4;
      if (f == 0x597850 || f == 0x597870)
        pop = 0;
      return actor_services_call(f, p, 0, a.size(), values, pop);
    }
#endif
    if (auto value =
            mnm::reconstruction::ScreenResourceProviders<Memory>(*this).tryRun(
                f, p, a))
      return *value;
    return endpoint(f, p, a);
  }
  U fastcall(U f, U p, U second, std::initializer_list<U> a = {}) {
    return endpoint(f, p, a, second);
  }
  U com(U f, U p, std::initializer_list<U> a = {}) { return endpoint(f, p, a); }
  U get(U p) { return get32(p); }
  void set(U p, U v) { set32(p, v); }
  U getByte(U p) { return get8(p); }
  void byte(U p, U v) { set8(p, v); }
  void initialize() {
    std::memset(at(Base, Size), 0x93, Size);
    std::memset(at(Base + 0x1000, Size - 0x1000), 0, Size - 0x1000);
    for (auto region : std::array<std::array<U, 2>, 6>{{{{0x642034, 1600}},
                                                        {{0x642c68, 4}},
                                                        {{0x5e04cc, 4}},
                                                        {{0x6f6950, 512}},
                                                        {{0x6f8154, 4}},
                                                        {{0x6de558, 36}}}})
      std::memset(at(region[0], region[1]), 0, region[1]);
    set32(0x5e04cc, ~0u);
    set32(0x5c6c6c + 0x20, 0x4ca980);
    set32(0x5c6c6c + 0x24, 0x4ca990);
    std::memcpy(at(Text, 32), "Owned screen control", 21);
    for (U i = 0; i < 4; ++i)
      set32(Rect + i * 4, i * 137 - 57);
    set32(Vector + 4, 0);
    set32(Vector + 8, 0);
    set32(Vector + 12, 0);
    set32(Vector + 16, 0);
  }
  std::vector<unsigned char> snapshot() {
    std::vector<unsigned char> result(at(Base, 0x1c000),
                                      at(Base, 0x1c000) + 0x1c000);
    for (auto region : std::array<std::array<U, 2>, 3>{
             {{{0x642034, 1600}}, {{0x642c68, 4}}, {{0x5e04cc, 4}}}})
      result.insert(result.end(), at(region[0], region[1]),
                    at(region[0], region[1]) + region[1]);
    return result;
  }
};
#ifdef MNM_SCREEN_PROTECTED
#include "screen-provider-protected.hpp"
#else
static void guardBodies(bool = false) {}
#endif
static Memory *active;
#ifdef MNM_ORIGINAL_REFERENCE
#define TC __attribute__((thiscall, force_align_arg_pointer))
#define CC __attribute__((cdecl, force_align_arg_pointer))
static U CC allocate(U n) { return active->endpoint(0x597850, 0, {n}); }
static U CC freeBlock(U p) {
  return p ? active->endpoint(0x597870, 0, {p}) : 0;
}
static U TC font(void *p, U a, U b) {
  return active->endpoint(0x4a5a90, U(p), {a, b});
}
static U TC dirty(void *p) { return active->endpoint(0x4a5000, U(p)); }
static U TC getDC(void *p) { return active->endpoint(0x58b660, U(p)); }
static U __attribute__((fastcall, force_align_arg_pointer))
codec(U p, U second, U a, U b, U c, U d) {
  return active->endpoint(0x1004f040, p, {a, b, c, d}, second);
}
static U __attribute__((stdcall, force_align_arg_pointer)) releaseDC(U p,
                                                                     U dc) {
  return active->endpoint(0x1004f080, p, {dc});
}
static U __attribute__((fastcall, force_align_arg_pointer))
effectWord(U p, U second, U rgb, U a, U b, U c, U d, U e, U f) {
  return active->endpoint(0x584430, p, {rgb, a, b, c, d, e, f}, second);
}
static U __attribute__((fastcall, force_align_arg_pointer))
effectDword(U p, U second, U rgb, U a, U b, U c, U d, U e, U f) {
  return active->endpoint(0x589280, p, {rgb, a, b, c, d, e, f}, second);
}
static void adapt(U address, void *function) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  U delta = U(function) - address - 5;
  std::memcpy(p + 1, &delta, 4);
}
static sigjmp_buf jump;
static volatile std::sig_atomic_t trapping = 0;
static void fault(int signal) {
  if (trapping)
    siglongjmp(jump, signal);
  std::_Exit(128 + signal);
}
#endif
struct Result {
  std::vector<unsigned char> bytes;
  std::vector<U> events;
  std::set<U> alive;
  bool fault = false;
  bool operator==(const Result &o) const {
    return bytes == o.bytes && events == o.events && alive == o.alive &&
           fault == o.fault;
  }
};
static void run(Memory &m, U suite, U variant) {
  auto call = [&](U f, U p, std::initializer_list<U> args = {}) {
    return m.invoke(f, p, args);
  };
  if (suite == 0) {
    call(0x559820, Base + 0x800);
    call(0x4cc720, Object);
    call(0x4d27b0, Label);
    call(0x4cb040, Object, {Text});
    call(0x4d28c0, Label, {Text});
    call(0x4cb040, Object, {variant & 1 ? 0 : Text});
    call(0x4d28c0, Label, {variant & 2 ? 0 : Text});
    call(0x4cb030, Object, {variant});
    call(0x4ca9b0, Object, {variant + 17});
    call(0x4d28b0, Label, {0x656680});
    call(0x4d2960, Label, {71});
    call(0x4d2930, Label, {Rect});
    call(0x4d2970, Label, {variant});
    call(0x4cb0b0, Object, {19, Rect, 0x12345678, 0x87654321, variant & 1});
    call(0x4cabd0, Vector, {8});
    call(0x4cad00, Vector, {Object, 1});
    call(0x4caf00, Vector, {Object});
    call(0x475c10, Object);
    call(0x4d27d0, Label);
    call(0x559880, Base + 0x800);
    call(0x597870, 0, {m.get32(Vector + 4)});
    return;
  }
  if (suite == 1) {
    U begin = Heap;
    for (U i = 0; i < 8; ++i)
      m.set32(begin + 4 * i, 0x100 + i);
    m.alive.insert(begin);
    m.top += 64;
    const U v = variant % 490;
    m.fail = variant >= 490 ? 1 : 0;
    const U size = v % 7, count = (v / 7) % 5, offset = v / 35 % (size + 1),
            capacity = v / 245 ? 16 : 8;
    m.set32(Vector + 4, begin);
    m.set32(Vector + 8, begin + 4 * size);
    m.set32(Vector + 12, begin + 4 * capacity);
    m.set32(Text, 0xf0f0);
    U value = variant & 1 && size ? begin + 4 * (size - 1) : Text;
    call(0x4def80, Vector, {begin + offset * 4, count, value});
    call(0x597870, 0, {m.get32(Vector + 4)});
    return;
  }
  if (suite == 2) {
    call(0x4d27b0, Label);
    m.fail = variant + 1;
    call(0x4d28c0, Label, {Text});
    call(0x4d28c0, Label, {Text});
    call(0x4d27d0, Label);
    return;
  }
  if (suite == 3) {
    m.set32(Object + 4, Base + 0xb00);
    m.set32(Base + 0xb00, Base + 0xc00);
    m.set32(Base + 0xc00 + 0x80, 0x1004f080);
    m.set32(0x5c5308, 0x1004f040);
    m.set32(Object + 16, 23);
    m.set32(Object + 20, 37);
    m.set32(Object + 24, 41);
    m.set32(0x6e1f88, variant & 1);
    m.dcResult = variant & 2 ? 0 : Base + 0xa00;
    m.codecResult = variant & 4 ? 0x102 : 0x100;
    check(call(0x58d320, Object, {Text}) ==
              (m.dcResult ? (m.codecResult & 255) : 0),
          "JPEG result");
    return;
  }
  if (suite == 4) {
    m.set32(Object, Base + 0x800);
    m.set32(Base + 0x810, variant % 5);
    m.set32(Object + 4, Base + 0x1000);
    for (U i = 0; i < 4; ++i) {
      U node = Base + 0x2000 + i * 128;
      m.set32(Object + 16 + i * 4, node);
      for (U j = 0; j < 4; ++j)
        m.set32(node + j * 16, j < 3 ? node + (j + 1) * 16 : 0);
    }
    m.codecResult = variant & 8 ? 0 : 1;
    call(0x57daa0, Object, {variant & 4 ? 2u : 1u, 32, 1, 2, 3, 4, 5, 6});
    return;
  }
  if (suite == 5) {
    const U entries[] = {0x5c0510, 0x5c051e, 0x5bf610, 0x5bf61e,
                         0x5bf62c, 0x5bf63a, 0x5c1b50};
    const U frame = Base + 0x1800, entry = entries[variant];
    if (variant < 6) {
      const U allocation = m.endpoint(0x597850, 0, {64});
      m.set32(frame - (variant < 2   ? 0x398
                       : variant < 5 ? 0x38c
                                     : 0x37c),
              allocation);
    } else {
      call(0x4cc720, Object);
      call(0x4cb0b0, Object, {1, Rect, 0, 0, 1});
      m.set32(frame - 0x34, Object);
    }
#ifdef MNM_ORIGINAL_REFERENCE
    if (m.original)
      screen_provider_cleanup(entry, frame);
    else
#endif
      check(mnm::reconstruction::ScreenResourceProviders<Memory>(m)
                .tryRun(entry, frame)
                .has_value(),
            "cleanup entry");
    return;
  }
  if (suite == 6) {
    const U array = Base + 0x1000, count = variant;
    call(0x59c441, 0, {array, 0x141, count, 0x559820, 0x559880});
    for (U i = 0; i < count; ++i) {
      const U list = m.endpoint(0x597850, 0, {12});
      m.set32(array + i * 0x141 + 0x139, list);
    }
    call(0x59bf87, 0, {array + 0x141 * count, 0x141, count, 0x559880});
    return;
  }
  if (suite == 7) {
    const U begin = Heap;
    m.alive.insert(begin);
    m.top += 64;
    const U size = variant % 8, capacity = 8,
            requested = ((variant >= 48 ? variant - 24 : variant) / 8) * 4;
    m.fail = variant >= 24 && variant < 48 ? 1 : 0;
    m.set32(Vector + 4, begin);
    m.set32(Vector + 8, begin + size * 4);
    m.set32(Vector + 12, begin + capacity * 4);
    m.set32(Vector + 16, 7);
    for (U i = 0; i < 16; ++i)
      m.set32(begin + 4 * i, 0x100 + i);
    call(0x4cabd0, Vector, {requested});
    call(0x597870, 0, {m.get32(Vector + 4)});
    return;
  }
  if (suite == 8) {
    const U counts[] = {0, 1, 399, 400};
    const U count = counts[variant % 4];
    m.set32(0x642c68, count);
    for (U i = 0; i < count; ++i) {
      U widget = Base + 0x8000 + i * 0x80;
      m.set32(0x642034 + 4 * i, widget);
      m.set32(widget + 4, i);
      m.set32(widget + 8, 1);
    }
    m.set32(0x5e04cc, count ? variant % count : ~0u);
    call(0x4cc720, Object);
    check(call(0x4a4e20, Object, {Rect, 1}) == U(count < 400),
          "registry capacity result");
    call(0x4ca910, Object);
    if (count)
      call(0x4a4f70, variant / 4 % count);
    return;
  }
  throw std::runtime_error("unknown suite");
}
static Result execute(bool original, U suite, U variant) {
  Memory m;
  m.original = original;
  active = &m;
  m.initialize();
  volatile bool failed = false;
#ifdef MNM_ORIGINAL_REFERENCE
  if (original) {
    const int signal = sigsetjmp(jump, 1);
    if (!signal) {
      trapping = 1;
      run(m, suite, variant);
      trapping = 0;
    } else {
      trapping = 0;
      check(signal == SIGSEGV, "unexpected original signal");
      failed = true;
    }
  } else
#endif
    try {
      run(m, suite, variant);
    } catch (const WireFault &) {
      failed = true;
    }
  guardBodies(true);
  return {m.snapshot(), m.events, m.alive, failed};
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  check(argc == 2, "pinned executable");
  map_image(read(argv[1]));
  check(mmap((void *)Base, Size, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "fixture map");
  check(mmap((void *)0x1004f000, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "endpoint code");
  OriginalLinuxSeh seh;
  adapt(0x597850, (void *)allocate);
  adapt(0x597870, (void *)freeBlock);
  adapt(0x4a5a90, (void *)font);
  adapt(0x4a5000, (void *)dirty);
  adapt(0x58b660, (void *)getDC);
  adapt(0x1004f040, (void *)codec);
  adapt(0x1004f080, (void *)releaseDC);
  adapt(0x584430, (void *)effectWord);
  adapt(0x589280, (void *)effectDword);
  std::signal(SIGSEGV, fault);
  std::signal(SIGFPE, fault);
#else
  (void)argc;
  (void)argv;
#endif
  U cases = 0, faults = 0;
  std::uint64_t checksum = 1469598103934665603ULL;
  const U counts[] = {32, 980, 3, 8, 32, 7, 24, 72, 16};
  for (U suite = 0; suite < std::size(counts); ++suite)
    for (U variant = 0; variant < counts[suite]; ++variant) {
      const auto native = execute(false, suite, variant);
#ifdef MNM_ORIGINAL_REFERENCE
      const auto original = execute(true, suite, variant);
      if (!(native == original)) {
        std::cerr << "mismatch suite " << suite << " variant " << variant
                  << " faults " << native.fault << ' ' << original.fault
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
      if (!native.fault)
        check(native.alive.empty(), "normal provider leaked owned allocation");
      ++cases;
      faults += native.fault;
      for (U v : native.events)
        checksum = (checksum ^ v) * 1099511628211ULL;
      for (U v : native.bytes)
        checksum = (checksum ^ v) * 1099511628211ULL;
    }
  std::cout << "{\"cases\":" << cases << ",\"faults\":" << faults
            << ",\"checksum\":" << checksum << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
