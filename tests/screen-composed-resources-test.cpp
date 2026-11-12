#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#define main unused_acquisition_main
#include "screen-acquisition-reference.cpp"
#undef main
#pragma GCC diagnostic pop
#include "../reconstruction/rendering/screen_layout_literals.hpp"
#include "../reconstruction/rendering/screen_provider_composition.hpp"
#include <set>
struct OwnedWire {
  Memory &m;
  std::map<U, U> sizes;
  std::set<U> alive;
  U top = Heap, scratch = Base + 0x3c000, fileCursor = 0, opens = 0;
  std::vector<unsigned char> spr;
  explicit OwnedWire(Memory &memory) : m(memory) {
    for (auto literal : mnm::reconstruction::ScreenLayoutLiterals)
      std::memcpy(m.at(literal.address, std::strlen(literal.text) + 1),
                  literal.text, std::strlen(literal.text) + 1);
    for (const auto &literal : literals)
      std::memcpy(m.at(literal.first, literal.second.size() + 1),
                  literal.second.c_str(), literal.second.size() + 1);
    spr.resize(24 + 768 + 4 + 32, 0);
    auto put = [&](U at, U value) { std::memcpy(spr.data() + at, &value, 4); };
    put(4, spr.size());
    put(8, 4);
    put(12, 1);
    put(16, 1);
    put(24 + 768, 0);
    put(24 + 768 + 4 + 28, 0);
    for (U i = 24; i < 24 + 768; ++i)
      spr[i] = i * 31;
    for (U p = 0x6f6950; p < 0x6f6b50; p += 4)
      m.set(p, 0);
    m.set(0x6f8154, 0);
    m.set(0x6e1f1c, 0);
    m.set(0x6de6dd, 0);
    m.set(0x6f98bc, 1);
    m.set(0x6f98b0, 0);
    m.set(0x6f8174, 0);
    m.set(0x6f98c8, Base + 0x11000);
    m.set(Base + 0x11000, Base + 0x11100);
    m.set(Base + 0x11100 + 0x18, 0x7f000200);
    for (auto item : std::initializer_list<std::pair<U, U>>{{0, 0x7f000240},
                                                            {8, 0x7f000280},
                                                            {0x58, 0x7f0002c0},
                                                            {0x64, 0x7f000300},
                                                            {0x80, 0x7f000340}})
      m.set(Base + 0x11200 + item.first, item.second);
    m.set(0x5c5308, 0x7f000380);
    m.set(0x5f2094, Base + 0x10000);
    m.byte(0x5f20e8, 0);
    m.set(0x5f1ec8, 0);
    for (U settings : {0x5f18d0u, 0x5f18f8u}) {
      m.set(settings, 30);
      m.set(settings + 4, 40);
      real(settings + 8, 1);
      real(settings + 16, 1);
    }
    m.set(0x6de6e5, 4);
    m.set(0x6e1f88, 0);
    m.set(0x642c68, 0);
    m.set(0x5e04cc, ~0u);
    m.set(0x5c6c6c + 0x20, 0x4ca980);
    m.set(0x5c6c6c + 0x24, 0x4ca990);
    for (U base : {0x6de558u, 0x6a28f8u, 0x644528u, 0x6593e0u, 0x6ddd30u})
      for (U o = 0; o < 36; o += 4)
        m.set(base + o, 0);
  }
  U get32(U p) { return m.get(p); }
  U get16(U p) { return m.getByte(p) | (m.getByte(p + 1) << 8); }
  U get8(U p) { return m.getByte(p); }
  void set32(U p, U v) { m.set(p, v); }
  void set16(U p, U v) {
    m.byte(p, v);
    m.byte(p + 1, v >> 8);
  }
  void set8(U p, U v) { m.byte(p, v); }
  void real(U p, double v) {
    std::uint64_t bits;
    std::memcpy(&bits, &v, 8);
    m.set(p, U(bits));
    m.set(p + 4, U(bits >> 32));
  }
  U temporary(U n) {
    U p = scratch;
    scratch += (n + 3) & ~3u;
    need(scratch < Base + Size, "scratch capacity");
    return p;
  }
  U allocate(U n) {
    U p = top;
    top += (n + 15) & ~15u;
    need(top < Base + 0x3b000, "owned composed heap");
    alive.insert(p);
    sizes[p] = n;
    U page = p & 0xffff0000u;
    m.set(page + 8, 0x7f000100);
    set16(page + 14, 0xcad0);
    std::memset(m.at(p, n), 0, n);
    return p;
  }
  void free(U p) {
    if (p)
      need(alive.erase(p) == 1, "composed provider double or foreign free");
  }
  std::string text(U p) {
    std::string result;
    while (get8(p))
      result += char(get8(p++));
    return result;
  }
  void text(U p, const std::string &value) {
    std::memcpy(m.at(p, value.size() + 1), value.c_str(), value.size() + 1);
  }
  std::map<std::string, std::string> profileWrites;
  U invoke(U f, U p, std::initializer_list<U> args = {}) {
    if (auto v =
            mnm::reconstruction::ScreenProviderComposition<OwnedWire>(*this)
                .tryRun(f, p, args))
      return *v;
    U a[8]{};
    std::copy(args.begin(), args.end(), a);
    if (f == 0x5c50bc) {
      text(a[1], "C:\\game");
      return 7;
    }
    if (f == 0x5c50b0) {
      profileWrites[text(a[0]) + "/" + text(a[1])] = text(a[2]);
      return 1;
    }
    if (f == 0x5c507c) {
      std::string key = text(a[1]), value;
      const auto found = profileWrites.find(text(a[0]) + "/" + key);
      if (found != profileWrites.end())
        value = found->second;
      else if (key == "Font")
        value = "LARGE";
      else if (key == "Rect2")
        value = "1,2,30,40";
      else if (key == "Text")
        value = "Control " + text(a[0]);
      else if (key == "Colours")
        value = "1,1,2,3";
      else if (key == "TextFlags")
        value = "LEFT";
      text(a[3], value);
      return value.size();
    }
    if (f == 0x484c00) {
      need(a[0] == 0, "unexpected surface failure");
      return 0;
    }
    if (f == 0x4a6970)
      return 0;
    if (f == 0x598a10)
      return allocate(a[1]);
    if (f == 0x597df0)
      return 0;
    if (f == 0x7f000100) {
      free(p);
      return 0;
    }
    if (f == 0x5b74d8)
      return a[0];
    if (f == 0x59d9eb)
      return spr.size();
    if (f == 0x59c359) {
      U n = std::min<U>(a[1] * a[2], spr.size() - fileCursor);
      std::memcpy(m.at(a[0], n), spr.data() + fileCursor, n);
      fileCursor += n;
      return n / a[1];
    }
    if (f == 0x59cf25) {
      fileCursor = a[1];
      return 0;
    }
    if (f == 0x59c4c3) {
      need(opens == 1, "file ownership");
      --opens;
      return 0;
    }
    if (f == 0x4a5a90 || f == 0x4a5000)
      return 0;
    throw std::runtime_error("unrecovered composed leaf " + std::to_string(f));
  }
  U fastcall(U f, U p, U second, std::initializer_list<U> a = {}) {
    if (f == 0x7f000380)
      return 1;
    if (f == 0x4a1120) {
      need(opens == 0, "file reopen");
      fileCursor = 0;
      ++opens;
      return 19;
    }
    if (auto v =
            mnm::reconstruction::ScreenProviderComposition<OwnedWire>(*this)
                .tryRun(f, p, a, second))
      return *v;
    throw std::runtime_error("composed fastcall leaf " + std::to_string(f));
  }
  U com(U f, U object, std::initializer_list<U> args) {
    U a[4]{};
    std::copy(args.begin(), args.end(), a);
    if (f == 0x7f000200) {
      U raw = allocate(16);
      m.set(raw, Base + 0x11200);
      m.set(raw + 4, m.get(a[0] + 12));
      m.set(raw + 8, m.get(a[0] + 8));
      m.set(a[1], raw);
      return 0;
    }
    if (f == 0x7f000240) {
      U next = allocate(16);
      m.set(next, Base + 0x11200);
      m.set(next + 4, m.get(object + 4));
      m.set(next + 8, m.get(object + 8));
      m.set(a[1], next);
      return 0;
    }
    if (f == 0x7f000280) {
      free(object);
      return 0;
    }
    if (f == 0x7f0002c0) {
      m.set(a[0] + 12, m.get(object + 4));
      m.set(a[0] + 8, m.get(object + 8));
      return 0;
    }
    if (f == 0x7f000300) {
      m.set(0x6f68f0, m.get(object + 4) * 2);
      m.set(0x6f6904, Base + 0x11600);
      return 0;
    }
    if (f == 0x7f000340)
      return 0;
    throw std::runtime_error(
        "unexpected COM endpoint in CPU ownership composition");
  }
};
struct ConcreteServices : Services {
  OwnedWire &wire;
  explicit ConcreteServices(OwnedWire &memory) : wire(memory) {}
  U allocate(U n) { return wire.invoke(0x597850, 0, {n}); }
  void free(U p) { wire.invoke(0x597870, 0, {p}); }
  void freeArray(U p) { wire.invoke(0x5979b0, 0, {p}); }
  void constructArray(U p, U stride, U count, U ctor, U dtor) {
    wire.invoke(0x59c441, 0, {p, stride, count, ctor, dtor});
  }
  void destroyArray(U p, U stride, U count, U dtor) {
    wire.invoke(0x59bf87, 0, {p + stride * count, stride, count, dtor});
  }
  void destruct(U f, U p) { wire.invoke(f, p); }
  void bitmapPlane(U p, U node) { wire.invoke(0x57d0d0, p, {node}); }
  U call(U f, U p = 0, std::initializer_list<U> args = {}) {
    if (auto result =
            mnm::reconstruction::ScreenProviderComposition<OwnedWire>(wire)
                .tryRun(f, p, args))
      return *result;
    return Services::call(f, p, args);
  }
  U text(U f, U p, const std::string &s) {
    if (f == 0x4cb040 || f == 0x4d28c0) {
      U text = wire.temporary(s.size() + 1);
      storeText(text, s);
      return wire.invoke(f, p, {text});
    }
    if (f == 0x58d320) {
      U filename = wire.temporary(s.size() + 1);
      storeText(filename, s);
      return wire.invoke(f, p, {filename});
    }
    return Services::text(f, p, s);
  }
  U sprite(U p, const std::string &name, U count) {
    U text = wire.temporary(name.size() + 1);
    storeText(text, name);
    return wire.invoke(0x57d310, p, {text, count});
  }
  void disable(U p) { wire.invoke(0x4ca980, p); }
  void loadLayout(U p, U filename) {
    mnm::reconstruction::acquireOriginalScreenLayout(*memory, *this, p,
                                                     filename, p == Report);
  }
  void loadLayoutText(U p, const std::string &name) {
    U filename = wire.temporary(name.size() + 1);
    storeText(filename, name);
    loadLayout(p, filename);
  }
  void layout(U filename, U destination, U kind, U index) {
    wire.fastcall(0x558580, filename, destination, {kind, index});
  }
  void surface(U owner, const std::array<U, 27> &descriptor, U file, U line) {
    U at = wire.temporary(108);
    for (U i = 0; i < 27; ++i)
      wire.set32(at + i * 4, descriptor[i]);
    wire.invoke(0x58ad90, owner, {at, file, line});
  }

  void dispose(U p, U flags) {
    (void)flags;
    free(p);
  }
};
struct ConcreteRetirement {
  ConcreteServices &p;
  U call(U f, U self = 0, U a = 0) {
    if (f == 0x4711d0 || f == 0x56d0a0 || f == 0x58bc10)
      return p.call(f, self, {a});
    return p.call(f, self);
  }
  void free(U p0) { p.free(p0); }
  void freeArray(U p0) { p.freeArray(p0); }
  void bitmapPlane(U p0, U n) { p.bitmapPlane(p0, n); }
};
int main() try {
  unsigned journeys = 0;
  for (profile = 0; profile < 16; ++profile) {
    mutation = 0;
    auto m = input(0);
    reset(m);
    OwnedWire wire(m);
    ConcreteServices p(wire);
    need(mnm::reconstruction::constructOriginalCoreMenu(m, p, 0x5c63e4, Main),
         "main construction");
    mnm::reconstruction::prepareOriginalMainScreen(m, p, Main);
    need(mnm::reconstruction::releaseOriginalScreenResources(m, p, 0x5c63e4,
                                                             Main),
         "main release");
    // These two World-owned resources already exist at entry. Populate actual
    // file buffers and chains, then let the recovered exit retire them
    // directly.
    for (U resource : {0x6de558u, 0x6a28f8u}) {
      need(p.sprite(resource, "world.spr", 4) == 1, "world SPR acquisition");
      for (U mode : {1u, 8u, 9u})
        need(wire.invoke(0x57d690, resource, {mode, 4}) == 1,
             "world palette chains");
    }
    mnm::reconstruction::enterOriginalWorldScreen(m, p, World);
    const auto before = wire.alive.size();
    mnm::reconstruction::prepareOriginalCampaignReport(m, p, Report);
    mnm::reconstruction::exitOriginalCampaignReport(m, p, Report);
    ConcreteRetirement retirement{p};
    mnm::reconstruction::retireOriginalWorldScreen(m, retirement, World);
    need(wire.alive.size() < before,
         "populated World ownership was not retired");
    for (U resource : {0x6de558u, 0x6a28f8u})
      for (U o = 0; o < 36; o += 4)
        need(!m.get(resource + o), "World resource remains after exit");
    mnm::reconstruction::prepareOriginalMainScreen(m, p, Main);
    need(mnm::reconstruction::destroyOriginalCoreMenu(m, p, 0x5c63e4, Main),
         "main destruction");
    // Selection/markers/lines are global service resources, retained by World
    // exit and explicitly retired by their service owner at session shutdown.
    for (U resource : {0x644528u, 0x6593e0u, 0x6ddd30u}) {
      if (U bytes = m.get(resource))
        p.freeArray(bytes);
      for (U o = 16; o < 32; o += 4)
        if (U chain = m.get(resource + o))
          p.bitmapPlane(resource, chain);
      if (U tail = m.get(resource + 32))
        p.freeArray(tail);
    }
    // The report vector retains backing storage on release; enclosing lifetime
    // teardown owns that allocation, as it does for the Main vector destructor.
    if (U backing = m.get(Report + 0x23))
      p.free(backing);
    if (U scores = m.get(0x634754)) {
      p.free(scores);
      m.set(0x634754, 0);
    }
    for (U leaked : wire.alive)
      std::cerr << "leaked " << std::hex << leaked << std::dec << " bytes "
                << wire.sizes[leaked] << " first " << std::hex << m.get(leaked)
                << std::dec << "\n";
    need(wire.alive.empty() && wire.opens == 0, "composed ownership leaked");
    ++journeys;
  }
  std::cout << "{\"journeys\":" << journeys
            << ",\"world_fields_cleared_before_exit\":false,\"live_allocations_"
               "at_session_exit\":0}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
