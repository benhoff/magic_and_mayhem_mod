#pragma once
#include <cstdint>
#include <initializer_list>
#include <optional>

namespace mnm::reconstruction {
// No-CD 40209ca7 surface creation, pixel locking, and optional JPEG backend.
// COM and OS calls are explicit value-address endpoints; HRESULTs, publication
// order, callback rereads and the original failed-retry quirk are preserved.
template <class Memory> class ScreenSurfaceProviders {
  using U = std::uint32_t;
  Memory &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  U call(U f, U p = 0, std::initializer_list<U> a = {}) {
    return m.invoke(f, p, a);
  }
  U com(U p, U slot, std::initializer_list<U> a = {}) {
    return m.com(r(r(p) + slot), p, a);
  }
  void diagnostic(U error) { call(0x484c00, 0x6f8158, {error}); }
  void teardown() {
    if (r(0x6f98c8)) {
      while (r(0x6f6950))
        call(0x58b3e0, r(0x6f6950));
      if (!r(0x6f68d8) && r(0x6f98d0)) {
        com(r(0x6f98d0), 8);
        w(0x6f98d0, 0);
      }
      if (r(0x6f68d8)) {
        com(r(0x6f98c8), 0x4c);
        w(0x6f68d8, 0);
      }
      com(r(0x6f98c4), 8);
      com(r(0x6f98c8), 8);
      com(r(0x6f98cc), 8);
      for (U field : {0x6f98c4u, 0x6f98c8u, 0x6f98ccu, 0x6f98c0u})
        w(field, 0);
    }
    call(0x5c5208, 0, {r(0x656614)});
  }
  U restore(U p) {
    const U surface = r(p + 8);
    if (!surface)
      return 0x80004005;
    const U result = com(surface, 0x6c);
    if (result)
      return result;
    if (r(0x6de6dd)) {
      const U key = m.temporary(8), value = m.get16(p + 44);
      w(key, value);
      w(key + 4, value);
      com(r(p + 8), 0x74, {8, key});
    }
    const U callback = r(p + 40);
    if (callback == ~0u)
      call(0x4a2f90, 0);
    else if (callback)
      call(r(r(callback)), callback, {0});
    return result;
  }

public:
  explicit ScreenSurfaceProviders(Memory &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U p, std::initializer_list<U> args = {}) {
    const U argument = args.size() ? *args.begin() : 0;
    if (entry == 0x58b270) {
      if (r(0x6f98bc))
        return U(r(0x6f98b0) != 0);
      const U library = call(0x5c50f4, 0, {0x5f1a5c});
      bool ready = library != 0;
      const U keys[] = {0x5f1a48, 0x5f1a34, 0x5f1a24, 0x5f1a10,
                        0x5f19fc, 0x5f19ec, 0x5f19d8};
      const U slots[] = {0x6f98a0, 0x6f98a4, 0x6f98a8, 0x6f98ac,
                         0x6f98b4, 0x6f98b0, 0x6f98b8};
      for (U i = 0; ready && i < 7; ++i) {
        U value = call(0x5c50d4, 0, {library, keys[i]});
        w(slots[i], value);
        ready = value != 0;
      }
      if (!ready)
        for (U slot : slots)
          w(slot, 0);
      w(0x6f98bc, 1);
      return U(ready);
    }
    if (entry == 0x58b660) {
      for (U o = 0; o < 108; o += 4)
        w(0x6f68e0 + o, 0);
      w(0x6f68e0, 108);
      if (!r(p + 8))
        return 0;
      const U result = com(r(p + 8), 0x64, {0, 0x6f68e0, 1, 0});
      w(0x6f8150, result);
      diagnostic(result);
      w(p + 24, r(0x6f68f0));
      if (r(0x6f8150) == 0x887601c2) {
        w(0x6f8150, restore(p));
        // The original ignores this second Lock HRESULT. Its global
        // status still contains Restore's result when it decides failure.
        com(r(p + 8), 0x64, {0, 0x6f68e0, 1, 0});
      }
      if (r(0x6f8150))
        teardown();
      w(p + 24, r(0x6f68f0));
      const U pixels = r(0x6f6904);
      if (r(0x6e1f1c))
        com(r(p + 4), 0x80, {pixels});
      return pixels;
    }
    if (entry != 0x58ad90)
      return std::nullopt;
    if (r(0x6f8154) >= 128)
      return 0;
    U result = com(r(0x6f98c8), 0x18, {argument, p + 4, 0});
    if (result)
      diagnostic(result);
    result = com(r(p + 4), 0, {0x5c7990, p + 8});
    if (result) {
      diagnostic(result);
      teardown();
    }
    if (r(0x6e1f1c))
      result = com(r(p + 8), 0x94, {0});
    if (result == 0x887601c2) {
      restore(p);
      teardown();
    } else if (result)
      teardown();
    w(p + 28, r(0x6f8154));
    w(0x6f6950 + r(0x6f8154) * 4, p);
    w(0x6f8154, r(0x6f8154) + 1);
    if (r(argument + 4) == 1 && r(argument + 104) == 0x200) {
      w(0x6f98c0, p);
      if (r(0x6f8174)) {
        const U colors = m.temporary(1024);
        for (U i = 0; i < 256; ++i)
          for (U channel = 0; channel < 3; ++channel)
            m.set8(colors + 4 * i + channel, static_cast<std::uint8_t>(i));
        if (com(r(0x6f98c8), 0x14, {4, colors, p + 12, 0}))
          teardown();
        com(r(p + 8), 0x7c, {r(p + 12)});
      }
      if (!r(0x6f68d8)) {
        result = com(r(p + 8), 0x70, {r(0x6f98d0)});
        if (result)
          diagnostic(result);
      }
    }
    result = com(r(p + 4), 0x58, {argument});
    if (result) {
      diagnostic(result);
      teardown();
    }
    if (!r(argument + 12)) {
      const U metric = r(0x5c5280);
      U ready = call(0x58b270);
      w(p + 16, call(ready ? r(0x6f98a0) : metric, 0, {0}));
      ready = call(0x58b270);
      w(p + 20, call(ready ? r(0x6f98a0) : metric, 0, {1}));
    } else {
      w(p + 16, r(argument + 12));
      w(p + 20, r(argument + 8));
    }
    w(p, U((r(argument + 104) & 0x4000) != 0));
    w(p + 46, 0);
    m.set16(p + 44, 0);
    w(p + 40, 0);
    w(p + 46, r(p + 20) * r(p + 16) * 2);
    return 0;
  }
};
} // namespace mnm::reconstruction
