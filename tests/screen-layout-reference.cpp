#define main unused_screen_provider_main
#include "screen-provider-reference.cpp"
#undef main
#include "../reconstruction/rendering/screen_layout_literals.hpp"
#include "../reconstruction/rendering/screen_layout_provider.hpp"
#include <cstdarg>
using mnm::reconstruction::ScreenLayoutLiterals;
struct LayoutMemory : Memory {
  U variant = 0;
  std::string text(U p) {
    std::string s;
    while (get8(p))
      s += char(get8(p++));
    return s;
  }
  void text(U p, const std::string &s) {
    std::memcpy(at(p, s.size() + 1), s.c_str(), s.size() + 1);
  }
  void record(U id, std::initializer_list<U> args,
              const std::vector<std::string> &strings = {}) {
    event(id, 0, 0, args);
    for (auto &s : strings) {
      events.push_back(s.size());
      for (unsigned char c : s)
        events.push_back(c);
    }
  }
  U platform(U f, std::initializer_list<U> args) {
    U a[6]{};
    std::copy(args.begin(), args.end(), a);
    if (f == 0x5c50bc) {
      const std::string directory = variant & 1 ? "C:/game/" : "C:\\game";
      record(f, {a[0], a[1]});
      text(a[1], directory);
      return directory.size();
    }
    if (f == 0x5c50b0) {
      record(f, {}, {text(a[0]), text(a[1]), text(a[2]), text(a[3])});
      return 1;
    }
    if (f == 0x5c507c) {
      const std::string key = text(a[1]);
      std::string result;
      if (variant != 0) {
        if (key == "Font")
          result = variant % 3 == 0   ? "LaRgE ; comment"
                   : variant % 3 == 1 ? "tiny"
                                      : "other";
        if (key == "Rect2")
          result = variant == 15 ? "1,2" : "-17,23,201,599";
        if (key == "Rect1" && variant & 2)
          result = "-7,19,103,401";
        if (key == "Text")
          result = variant % 4 == 0   ? "123suffix ;comment"
                   : variant % 4 == 1 ? "\"Quoted value\" ;comment"
                   : variant % 4 == 2 ? "Plain value"
                                      : "-1 literal";
        if (key == "SpriteIndexes")
          result = variant & 4 ? "11" : "11,22,33";
        if (key == "minValue")
          result = " -23suffix";
        if (key == "maxValue")
          result = "100";
        if (key == "TextFlags")
          result = variant % 3 == 0   ? "LEFT ;comment"
                   : variant % 3 == 1 ? "right"
                                      : "CENTER";
        if (key == "Colours")
          result = variant != 14 && (variant & 8) ? "0" : "2,1,2,3,4,5,6";
        if (key == "step")
          result = " +7suffix";
      }
      record(f, {a[4]}, {text(a[0]), key, text(a[2]), text(a[5]), result});
      text(a[3], result);
      return result.size();
    }
    throw std::runtime_error("unknown layout platform");
  }
  U invoke(U f, U p, std::initializer_list<U> a = {}) {
    if (f == 0x5c50bc || f == 0x5c50b0 || f == 0x5c507c)
      return platform(f, a);
    if (f == 0x4a6970) {
      record(f, {p});
      return 0;
    }
    if (f == 0x4a5a90) {
      record(f, {p, *a.begin(), *(a.begin() + 1)});
      set8(*a.begin(), *(a.begin() + 1));
      return 0;
    }
    return Memory::invoke(f, p, a);
  }
  U fastcall(U f, U p, U second, std::initializer_list<U> a = {}) {
    if (f == 0x58a6b0) {
      record(f, {p, second});
      text(Base + 0x3000, "Localized " + std::to_string(p));
      return Base + 0x3000;
    }
    return Memory::fastcall(f, p, second, a);
  }
  void initialize(U v) {
    Memory::initialize();
    variant = v;
    fail = v == 14 ? 1 : 0;
    for (auto literal : ScreenLayoutLiterals)
      text(literal.address, literal.text);
    set8(0x5faf28, 0);
    text(Text, "Interface\\screen.cfg");
    for (U p : {0x6f2d40u, 0x6f3044u, 0x6f326cu, 0x6f336cu})
      std::memset(at(p, 256), 0x79, 256);
  }
  std::vector<unsigned char> snapshot() {
    auto bytes = Memory::snapshot();
    for (U p : {0x6f2d40u, 0x6f3044u, 0x6f326cu, 0x6f336cu})
      bytes.insert(bytes.end(), at(p, 256), at(p, 256) + 256);
    return bytes;
  }
};
static LayoutMemory *layoutMemory;
#ifdef MNM_ORIGINAL_REFERENCE
static U
    __attribute__((stdcall, force_align_arg_pointer)) directory(U n, U out) {
  return layoutMemory->platform(0x5c50bc, {n, out});
}
static U __attribute__((stdcall, force_align_arg_pointer))
profileString(U s, U k, U d, U out, U n, U p) {
  return layoutMemory->platform(0x5c507c, {s, k, d, out, n, p});
}
static U __attribute__((stdcall, force_align_arg_pointer))
writeProfile(U s, U k, U v, U p) {
  return layoutMemory->platform(0x5c50b0, {s, k, v, p});
}
static U TC realizeFont(void *p) {
  return layoutMemory->invoke(0x4a6970, U(p));
}
static U TC textFlags(void *p, U a, U b) {
  return layoutMemory->invoke(0x4a5a90, U(p), {a, b});
}
static U __attribute__((fastcall, force_align_arg_pointer))
localized(U id, U fallback) {
  return layoutMemory->fastcall(0x58a6b0, id, fallback);
}
static U CC formatLayout(char *out, const char *fmt, ...) {
  va_list a;
  va_start(a, fmt);
  int n = std::vsprintf(out, fmt, a);
  va_end(a);
  return n;
}
#endif
static Result executeLayout(bool original, U kind, U variant, U index) {
  LayoutMemory m;
  m.original = original;
  m.initialize(variant);
  layoutMemory = &m;
  active = &m;
  m.invoke(0x559820, Object);
  volatile bool failed = false;
#ifdef MNM_ORIGINAL_REFERENCE
  if (original) {
    const int signal = sigsetjmp(jump, 1);
    if (!signal) {
      trapping = 1;
      U args[] = {kind, index};
      actor_services_call(0x558580, Text, Object, 2, args, 8);
      trapping = 0;
    } else {
      trapping = 0;
      check(signal == SIGSEGV, "unexpected layout signal");
      failed = true;
    }
  } else
#endif
    try {
      mnm::reconstruction::ScreenLayoutProvider<LayoutMemory>(m).tryRun(
          0x558580, Text, Object, {kind, index});
    } catch (const WireFault &) {
      failed = true;
    }
  if (!failed)
    m.invoke(0x559880, Object);
  guardBodies();
  return {m.snapshot(), m.events, m.alive, failed};
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  check(argc == 2, "pinned executable");
  map_image(read(argv[1]));
  check(mmap((void *)Base, Size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "layout map");
  OriginalLinuxSeh seh;
  *reinterpret_cast<U *>(0x5c50bc) = U(&directory);
  *reinterpret_cast<U *>(0x5c507c) = U(&profileString);
  *reinterpret_cast<U *>(0x5c50b0) = U(&writeProfile);
  adapt(0x4a6970, (void *)realizeFont);
  adapt(0x4a5a90, (void *)textFlags);
  adapt(0x58a6b0, (void *)localized);
  adapt(0x59c1fd, (void *)formatLayout);
  adapt(0x597850, (void *)allocate);
  adapt(0x597870, (void *)freeBlock);
  std::signal(SIGSEGV, fault);
  std::signal(SIGFPE, fault);
  for (auto literal : ScreenLayoutLiterals)
    check(std::string(reinterpret_cast<const char *>(literal.address)) ==
              literal.text,
          "layout literal changed");
#else
  (void)argc;
  (void)argv;
#endif
  U cases = 0, faults = 0;
  std::uint64_t checksum = 1469598103934665603ULL;
  for (U kind = 0; kind < 11; ++kind)
    for (U variant = 0; variant < 16; ++variant)
      for (U index : {0u, 1u, ~0u}) {
        auto native = executeLayout(false, kind, variant, index);
#ifdef MNM_ORIGINAL_REFERENCE
        auto original = executeLayout(true, kind, variant, index);
        if (!(native == original)) {
          std::cerr << "layout mismatch " << kind << ' ' << variant << ' '
                    << index << " fault " << native.fault << ' '
                    << original.fault << " events " << native.events.size()
                    << ' ' << original.events.size() << '\n';
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
          check(native.alive.empty(), "layout list leaked");
        for (U v : native.events)
          checksum = (checksum ^ v) * 1099511628211ULL;
        for (U v : native.bytes)
          checksum = (checksum ^ v) * 1099511628211ULL;
        ++cases;
        faults += native.fault;
      }
  std::cout << "{\"cases\":" << cases << ",\"faults\":" << faults
            << ",\"checksum\":" << checksum << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
