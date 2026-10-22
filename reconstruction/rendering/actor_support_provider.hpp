#pragma once
#include "actor_font_providers.hpp"

namespace mnm::reconstruction::actor_services {
// NoCD4f3320: borrow initialized map/terrain/parameter storage. The query
// neither allocates nor transfers ownership. Address arithmetic is PE32 DWORD.
template <class M> class SupportProvider {
  M &m;
  U r(U p) { return m.get32(p); }
  static int i(U n) { return n < 0x80000000u ? int(n) : -1 - int(~n); }
public:
  explicit SupportProvider(M &memory) : m(memory) {}
  U run(U self, U x, U y, U z, U parameters) {
    U index = r(self + 0x64b2 + y * 4) + r(self + 0x6432 + z * 4) + x;
    U cell = r(self + 0x4c) + index * 12;
    U special = m.get8(parameters + 0x10);
    auto restricted = [special](U flags) {
      return !special && (flags & 0x20) && !(flags & 0x10);
    };
    auto eligible = [&](U record) {
      return (m.get8(record + 0xb0) & 8) || r(parameters + 0x11) == 22;
    };
    U flags = m.get16(cell + 10);
    if (restricted(flags) || (flags & 0x4000))
      return 0;
    U id = m.get16(cell);
    if (id && !(flags & 0x80)) {
      U record = r(0x65660c) + id * 0x164;
      return r(record + 0x94) < 16 && eligible(record) ? 1 : 0;
    }
    if (i(z) <= 0)
      return 0;
    cell -= r(self + 0x10) * 12;
    flags = m.get16(cell + 10);
    if (restricted(flags))
      return 0;
    if (flags & 0x4000)
      return 1;
    id = m.get16(cell);
    if (!id || (flags & 0x80))
      return 0;
    U record = r(0x65660c) + id * 0x164;
    return r(record + 0x94) == 16 && eligible(record) ? 1 : 0;
  }
};
template <class M> class SupportComposition {
  M &m;
public:
  explicit SupportComposition(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U self, std::initializer_list<U> args = {}, U second = 0) {
    switch (entry) {
    case 0x4f3320: {
      if (args.size() != 4)
        throw std::invalid_argument("Support query arguments");
      auto a = args.begin();
      return SupportProvider<M>(m).run(self, a[0], a[1], a[2], a[3]);
    }
    default:
      return FontComposition<M>(m).tryRun(entry, self, args, second);
    }
  }
};
} // namespace mnm::reconstruction::actor_services
