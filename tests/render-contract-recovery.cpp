// Original private PE mapping versus independently reset native caller models.
// Suppliers are deliberately authored, including their HRESULTs and flags.
#include "../reconstruction/rendering/dc_image_wrappers.hpp"
#include "../reconstruction/rendering/font_screen_installation.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#include "original-linux-seh.hpp"
extern "C" {
std::uint32_t contract_host_sp, contract_function, contract_self, contract_argc;
std::uint32_t contract_call_sp, contract_gpr[8], contract_flags,
    contract_first_arg, contract_guards[2];
void contract_original_call(std::uint32_t, std::uint32_t, std::uint32_t,
                            const std::uint32_t *);
}
#endif
using U = std::uint32_t;
using Bytes = std::vector<unsigned char>;
constexpr U Owned = 0x10000000, Self = Owned + 0x1000, Surface = Owned + 0x2000,
            OtherSurface = Owned + 0x2100, Source = Owned + 0x4000,
            Scratch = Owned + 0x80000;
constexpr U GetDc = Owned + 0x100, ReleaseDc = Owned + 0x140;
struct Spec {
  U id, argc, pop;
  bool receiver, second;
};
static const Spec specs[] = {
    {0x486950, 0, 0, true, false},  {0x486960, 0, 0, true, false},
    {0x486ae0, 1, 4, true, false},  {0x486a80, 3, 12, true, false},
    {GetDc, 2, 8, false, false},    {ReleaseDc, 2, 8, false, false},
    {0x59c1fd, 3, 0, false, false}, {0x4a5ae0, 2, 8, true, false},
    {0x57d310, 2, 8, true, false},  {0x49e4e0, 0, 0, true, false},
    {0x4dba20, 1, 4, true, false},  {0x4db650, 0, 0, true, false},
    {0x4dc0f0, 1, 4, true, false},  {0x4d8630, 1, 4, true, false},
    {0x4d85f0, 0, 0, true, false},  {0x4d82f0, 0, 0, true, false},
    {0x49c280, 0, 0, true, false},  {0x4a30b0, 0, 0, false, false},
    {0x534aa0, 2, 8, true, false},  {0x534ad0, 1, 4, true, false},
    {0x502b20, 1, 4, true, true},   {0x54d070, 0, 0, false, false},
    {0x4f7860, 5, 20, true, false}, {0x4db4f0, 1, 4, true, false},
    {0x4dad50, 0, 0, true, false},  {0x4d9360, 1, 4, true, false},
    {0x554830, 0, 0, true, false},  {0x4d5e50, 1, 4, true, false},
    {0x4d3690, 0, 0, true, false},  {0x4da200, 0, 0, true, false},
    {0x4dc830, 0, 0, true, false},  {0x5c52e8, 0, 0, false, false},
    {0x54e170, 1, 4, true, false},  {0x4d28b0, 1, 4, true, false},
    {0x4d2960, 1, 4, true, false},  {0x58a680, 1, 4, true, false},
    {0x4d28c0, 1, 4, true, false},  {0x4d2970, 1, 4, true, false},
    {0x4d2930, 1, 4, true, false},  {0x4d2800, 0, 0, true, false},
    {0x58bf40, 3, 12, true, false}, {0x4ca370, 0, 0, true, false}};
static void *ptr(U p) { return reinterpret_cast<void *>(std::uintptr_t(p)); }
static U get(U p) {
  U v;
  std::memcpy(&v, ptr(p), 4);
  return v;
}
static void put(U p, U v) { std::memcpy(ptr(p), &v, 4); }
static Bytes image;
static Bytes baseline;
static std::vector<std::pair<U, Bytes>> protectedBodies;
static U at32(const Bytes &b, std::size_t p) {
  U v;
  if (p + 4 > b.size())
    throw std::runtime_error("PE extent");
  std::memcpy(&v, b.data() + p, 4);
  return v;
}
static void mapImage(const char *path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    throw std::runtime_error("PE input");
  image = Bytes(std::istreambuf_iterator<char>(in), {});
  U pe = at32(image, 0x3c), opt = pe + 24;
  if (at32(image, opt + 28) != 0x400000)
    throw std::runtime_error("PE base");
  if (mmap(ptr(0x400000), 0x400000, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
           0) == MAP_FAILED ||
      mmap(ptr(Owned), 0x100000, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
           0) == MAP_FAILED)
    throw std::runtime_error("Private map");
  U n = image.at(pe + 6) | (U(image.at(pe + 7)) << 8),
    os = image.at(pe + 20) | (U(image.at(pe + 21)) << 8);
  for (U k = 0; k < n; ++k) {
    U s = opt + os + 40 * k, size = at32(image, s + 16),
      off = at32(image, s + 20), rva = at32(image, s + 12);
    if (std::uint64_t(off) + size > image.size() ||
        std::uint64_t(rva) + size > 0x400000)
      throw std::runtime_error("PE section");
    std::memcpy(ptr(0x400000 + rva), image.data() + off, size);
  }
  baseline = Bytes(static_cast<unsigned char *>(ptr(0x5d0000)),
                   static_cast<unsigned char *>(ptr(0x705000)));
  for (auto r : std::array<std::array<U, 2>, 5>{{{0x5575b0, 922},
                                                 {0x58d1a0, 152},
                                                 {0x58d240, 59},
                                                 {0x58d280, 156},
                                                 {0x58d370, 61}}}) {
    auto *p = static_cast<unsigned char *>(ptr(r[0]));
    protectedBodies.emplace_back(r[0], Bytes(p, p + r[1]));
  }
}
struct Memory {
  U seed = 0, next = Scratch, loadIndex = 0, localImage = 0, localDc = 0,
    dcSlot = 0;
  bool font = false;
  std::vector<U> events;
  U get32(U p) { return get(p); }
  void set32(U p, U v) { put(p, v); }
  unsigned char get8(U p) { return *static_cast<unsigned char *>(ptr(p)); }
  void set8(U p, unsigned char v) { *static_cast<unsigned char *>(ptr(p)) = v; }
  U scratch(U n) {
    U p = next;
    next += (n + 15) & ~15u;
    if (next > Scratch + 0x1000)
      throw std::runtime_error("Scratch budget");
    std::memset(ptr(p), 0xcc, n);
    return p;
  }
  void event(U id, U receiver, U second, std::initializer_list<U> args) {
    events.insert(events.end(), {id, receiver, second, U(args.size())});
    events.insert(events.end(), args);
  }
  std::string string(U p) {
    std::string s;
    for (U k = 0; k < 256; ++k) {
      char c = char(get8(p + k));
      if (!c)
        return s;
      s += c;
    }
    throw std::runtime_error("Supplier string bound");
  }
  void text(U p, const std::string &s) {
    if (s.size() >= 256)
      throw std::runtime_error("Supplier text bound");
    std::memcpy(ptr(p), s.c_str(), s.size() + 1);
  }
  void textEvent(U id, U receiver, U second, const std::string &s,
                 std::initializer_list<U> args = {}) {
    event(id, receiver, second, args);
    events.push_back(U(s.size()));
    for (unsigned char c : s)
      events.push_back(c);
  }
  U call(U id, U receiver, U second, std::initializer_list<U> args) {
    const auto *a = args.begin();
    if (id == 0x59c1fd) {
      char out[256];
      int n = std::snprintf(out, sizeof out, string(a[1]).c_str(), a[2]);
      if (n < 0 || n >= 256)
        throw std::runtime_error("Formatting extent");
      text(a[0], out);
      textEvent(id, 0, 0, out, {a[1], a[2]});
      return U(n);
    }
    if (id == 0x4a5ae0 || id == 0x57d310) {
      textEvent(id, receiver, 0, string(a[0]), {a[1]});
      U ordinal = loadIndex++;
      put(receiver, U(0x71300000 + ordinal * 256 + seed));
      if (ordinal == 0 && (seed & 256))
        put(0x6de6d5, get(0x6de6d5) ? 0u : 0xffffffffu);
      return (seed & (1u << (ordinal + 3))) ? 0u : 0x80000001u;
    }
    if (id == 0x49e4e0) {
      textEvent(id, 0, 0, string(receiver));
      return 0x99990000;
    }
    if (id == 0x4d2930) {
      event(id, receiver, 0,
            {get(a[0]), get(a[0] + 4), get(a[0] + 8), get(a[0] + 12)});
      return 0xdead0000;
    }
    if (id == 0x486950) {
      localImage = receiver;
      put(receiver, 0xaabbccdd);
      put(receiver + 4, 0x12345678);
      put(receiver + 8, 0x87654321);
      event(id, 0xf1000000, 0, {});
      return 0;
    }
    if (id == 0x486960) {
      if (receiver != localImage)
        throw std::runtime_error("Wrong image destruction");
      event(id, 0xf1000000, 0,
            {get(receiver), get(receiver + 4), get(receiver + 8)});
      return 0xeeeeeeee;
    }
    if (id == 0x486ae0) {
      if (receiver != localImage)
        throw std::runtime_error("Wrong image load");
      event(id, 0xf1000000, 0, {a[0]});
      put(receiver + 4, a[0]);
      return seed & 1 ? 0x80000001u : 0;
    }
    if (id == GetDc) {
      localDc = a[1];
      dcSlot = a[1];
      event(id, 0, 0, {a[0], get(a[1])});
      if (seed & 4)
        put(a[1], 0xdcdc0000 + seed);
      return seed & 8 ? 0x88760249u : 0;
    }
    if (id == 0x486a80) {
      if (a[0] != localDc)
        throw std::runtime_error("Wrong HDC slot");
      event(id, receiver == localImage ? 0xf1000000 : receiver, 0,
            {get(a[0]), a[1], a[2]});
      if (seed & 16)
        put(a[0], get(a[0]) ^ 0x1234);
      if (seed & 32)
        put(Self + 8, OtherSurface);
      return 0xffff0000;
    }
    if (id == ReleaseDc) {
      event(id, 0, 0, args);
      return seed & 64 ? 0x8876024au : 0x76543210;
    }
    event(id, receiver, second, args);
    if (id == 0x502b20)
      return seed & 128 ? 0 : 1;
    if (id == 0x5c52e8)
      return 0x77770000 + seed;
    if (id == 0x58a680)
      return 0x5ec060;
    // Deterministic visible supplier side effects, independently applied to
    // fresh native and original inputs; not recovered widget implementations.
    if (receiver >= 0x640000 && receiver < 0x705000)
      put(receiver + 0x10, get(receiver + 0x10) ^ id ^ seed);
    return 0x01020304;
  }
  U com(U object, U slot, std::initializer_list<U> args) {
    if (!object)
      throw std::runtime_error("Unadmitted null surface");
    U function = get(get(object) + slot);
    return call(function, 0, 0, args);
  }
  void initialize(U s, bool f) {
    seed = s;
    font = f;
    next = Scratch;
    loadIndex = localImage = localDc = dcSlot = 0;
    events.clear();
    std::memcpy(ptr(0x5d0000), baseline.data(), baseline.size());
    std::memset(ptr(Owned + 0x1000), 0xa5, 0x10000 - 0x1000);
    put(Self + 8, s & 2 ? Surface : 0);
    put(Surface, Owned + 0x3000);
    put(OtherSurface, Owned + 0x3100);
    for (U v : {Owned + 0x3000, Owned + 0x3100}) {
      put(v + 0x44, GetDc);
      put(v + 0x68, ReleaseDc);
    }
    text(Source, "authored-image");
    put(0x6de6d5, s & 1 ? 0xffffffffu : 0);
    put(0x6de6d9, (s >> 1) & 3);
    put(0x642c64, s & 4);
    set8(0x6de6c5, s & 2 ? 0xff : 0);
    put(0x6de6c0, s & 4 ? Owned + 0x5000 : 0);
    put(Owned + 0x5004, s & 1 ? 2 : 0);
  }
  Bytes snapshot() {
    constexpr std::size_t imageBytes = 0x705000 - 0x5d0000;
    Bytes out(imageBytes + 0xf000);
    std::memcpy(out.data(), ptr(0x5d0000), imageBytes);
    std::memcpy(out.data() + imageBytes, ptr(Owned + 0x1000), 0xf000);
    return out;
  }
};
static Memory *active;
#ifdef MNM_ORIGINAL_REFERENCE
extern "C" __attribute__((force_align_arg_pointer))
U contract_callback(U id, U receiver, U *args, U second) {
  try {
    const Spec *spec = nullptr;
    for (const auto &s : specs)
      if (s.id == id)
        spec = &s;
    if (!spec)
      throw std::runtime_error("Unknown supplier");
    if (!spec->receiver)
      receiver = 0;
    if (!spec->second)
      second = 0;
    switch (spec->argc) {
    case 0:
      return active->call(id, receiver, second, {});
    case 1:
      return active->call(id, receiver, second, {args[0]});
    case 2:
      return active->call(id, receiver, second, {args[0], args[1]});
    case 3:
      return active->call(id, receiver, second, {args[0], args[1], args[2]});
    case 5:
      return active->call(id, receiver, second,
                          {args[0], args[1], args[2], args[3], args[4]});
    default:
      throw std::runtime_error("Supplier arity");
    }
  } catch (const std::exception &e) {
    std::fprintf(stderr, "supplier %x: %s\n", id, e.what());
    std::abort();
  }
}
static void installSuppliers() {
  U index = 0;
  for (const auto &s : specs) {
    U dest = Owned + 0x60000 + index++ * 64;
    auto *p = static_cast<unsigned char *>(ptr(dest));
    const unsigned char prefix[] = {0x55, 0x89, 0xe5, 0x52, 0x8d,
                                    0x45, 0x08, 0x50, 0x51, 0x68};
    std::memcpy(p, prefix, sizeof prefix);
    put(dest + 10, s.id);
    p[14] = 0xe8;
    put(dest + 15,
        U(reinterpret_cast<std::uintptr_t>(contract_callback)) - dest - 19);
    const unsigned char tail[] = {0x83, 0xc4, 0x10, 0xc9, 0x68, 0x83,
                                  0x02, 0,    0,    0x9d, 0xc2};
    std::memcpy(p + 19, tail, sizeof tail);
    std::uint16_t pop = s.pop;
    std::memcpy(p + 30, &pop, 2);
    if (s.id == 0x5c52e8)
      put(s.id, dest);
    else {
      auto *site = static_cast<unsigned char *>(ptr(s.id));
      site[0] = 0xe9;
      put(s.id + 1, dest - s.id - 5);
    }
  }
}
static U addFlags(U a, U b) {
  U r = a + b;
  U parity = 0;
  for (U k = 0; k < 8; ++k)
    parity ^= (r >> k) & 1;
  return (r < a ? 1u : 0u) | (!parity ? 4u : 0u) | ((a ^ b ^ r) & 16u) |
         (!r ? 64u : 0u) | (r & 0x80000000u ? 128u : 0u) |
         ((~(a ^ b) & (a ^ r)) & 0x80000000u ? 2048u : 0u);
}
#endif
static std::uint64_t hash(const unsigned char *p, std::size_t n) {
  std::uint64_t h = 14695981039346656037ull;
  while (n--) {
    h ^= *p++;
    h *= 1099511628211ull;
  }
  return h;
}
int main(int argc, char **argv) try {
  if (argc != 2)
    throw std::runtime_error("Pinned PE required");
  mapImage(argv[1]);
#ifdef MNM_ORIGINAL_REFERENCE
  OriginalLinuxSeh seh;
  installSuppliers();
#endif
  Memory m;
  active = &m;
  U cases = 0;
  for (U family = 0; family < 2; ++family)
    for (U s = 0; s < (family ? 512u : 1024u); ++s) {
      U entry = family ? 0x5575b0
                       : std::array<U, 4>{0x58d1a0, 0x58d240, 0x58d280,
                                          0x58d370}[s % 4];
      U seed = family ? s : s / 4;
      bool positioned = entry == 0x58d280 || entry == 0x58d370;
      if (positioned)
        seed |= 2;
      U x = 0xfffffffdu, y = 0x80000001u;
      U entrySeh = seed & 128 ? Owned + 0x5000 : 0xffffffffu;
      m.initialize(seed, family);
      U expected = 0;
      mnm::reconstruction::rendering::DcImageResult predicted{};
      if (family)
        mnm::reconstruction::rendering::installScreenFonts(m, Self);
      else
        predicted = mnm::reconstruction::rendering::dcImageWrapper(
            m, entry, Self, Source, x, y, entrySeh);
      if (!family)
        expected = predicted.eax;
      Bytes native = m.snapshot();
      auto transcript = m.events;
#ifdef MNM_ORIGINAL_REFERENCE
      m.initialize(seed, family);
      if (!family && !positioned && (seed & 2) == 0)
        put(Self + 8, 0);
      U args[] = {Source, x, y};
      U n = family ? 0 : positioned ? 3 : 1;
      seh.chain[0] = entrySeh;
      contract_original_call(entry, Self, n, args);
      if (seh.chain[0] != entrySeh)
        throw std::runtime_error("SEH chain changed");
      seh.chain[0] = 0xffffffffu;
      seh.check();
      bool good = m.snapshot() == native && m.events == transcript &&
                  (family || contract_gpr[0] == expected);
      good = good && contract_gpr[3] == 0xb1b2b3b4 &&
             contract_gpr[5] == 0xe1e2e3e4 && contract_gpr[6] == 0x51525354 &&
             contract_gpr[7] == 0x71727374;
      good = good && contract_gpr[4] == contract_call_sp + n * 4 &&
             contract_guards[0] == 0x2468ace0 &&
             contract_guards[1] == 0x13579bdf;
      if (!family) {
        bool load = entry == 0x58d1a0 || entry == 0x58d280;
        good = good && contract_gpr[1] == predicted.ecx;
        if (load) {
          U sp = contract_call_sp - 4;
          good = good && (contract_flags & 0x8d5) == addFlags(sp - 24, 24);
          good = good && contract_first_arg == predicted.firstArgument;
        } else
          good = good && (contract_flags & 0x8c5) ==
                             ((!positioned && !(seed & 2)) ? 0x44u : 0x81u);
      }
      if (!good) {
        auto observed = m.snapshot();
        std::cerr << "mismatch " << std::hex << entry << " seed " << seed
                  << " eax " << contract_gpr[0] << " expected " << expected
                  << " ecx " << contract_gpr[1] << " flags " << contract_flags
                  << " args " << contract_first_arg << "/"
                  << predicted.firstArgument << " events " << m.events.size()
                  << "/" << transcript.size() << '\n';
        for (std::size_t k = 0, d = 0; k < native.size() && d < 12; ++k)
          if (native[k] != observed[k]) {
            std::cerr << "state " << std::hex << k << " " << U(native[k]) << "/"
                      << U(observed[k]) << '\n';
            ++d;
          }
        for (std::size_t k = 0;
             k < std::min(m.events.size(), transcript.size()); ++k)
          if (m.events[k] != transcript[k]) {
            std::cerr << "event " << k << " " << transcript[k] << "/"
                      << m.events[k] << '\n';
            break;
          }
        return 1;
      }
#endif
      std::cout << (family ? "font" : "dc") << ' ' << std::hex << entry << ' '
                << seed << ' ' << expected << ' '
                << hash(native.data(), native.size()) << ' '
                << hash(reinterpret_cast<const unsigned char *>(
                            transcript.data()),
                        transcript.size() * 4)
                << ' ' << std::dec << transcript.size() << '\n';
      ++cases;
    }
  for (const auto &r : protectedBodies)
    if (std::memcmp(ptr(r.first), r.second.data(), r.second.size()))
      throw std::runtime_error("Compared original body modified");
  std::cerr << "passed " << cases << " cases; five compared bodies unchanged\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 2;
}
