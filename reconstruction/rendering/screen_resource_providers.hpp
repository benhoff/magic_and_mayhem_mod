#pragma once
#include "actor_service_construction.hpp"
#include <algorithm>
#include <initializer_list>
#include <optional>
#include <stdexcept>

namespace mnm::reconstruction {
// Build-specific PE32 providers. Memory owns admitted wire storage, platform
// calls and temporary scratch; this dispatcher does not manufacture
// allocations.
template <class Memory> class ScreenResourceProviders {
  using U = std::uint32_t;
  Memory &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  U call(U f, U p = 0, std::initializer_list<U> a = {}) {
    return m.invoke(f, p, a);
  }
  U virt(U p, U slot, std::initializer_list<U> a = {}) {
    return call(r(r(p) + slot), p, a);
  }
  U length(U p) {
    U n = 0;
    while (m.get8(p + n))
      ++n;
    return n;
  }
  void copy(U d, U s, U n) {
    while (n >= 4) {
      w(d, r(s));
      d += 4;
      s += 4;
      n -= 4;
    }
    while (n--) {
      m.set8(d++, m.get8(s++));
    }
  }
  U ownText(U p, U offset, U source) {
    if (const U old = r(p + offset)) {
      call(0x597870, 0, {old});
      w(p + offset, 0);
    }
    if (source) {
      const U out = call(0x597850, 0, {length(source) + 1});
      w(p + offset, out);
      copy(out, source, length(source) + 1);
    }
    return 0;
  }

public:
  explicit ScreenResourceProviders(Memory &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U p, std::initializer_list<U> args = {},
                          U second = 0) {
    U a[8]{};
    if (args.size() > 8)
      throw std::invalid_argument("screen provider argument extent");
    std::copy(args.begin(), args.end(), a);
    switch (entry) {
    case 0x559820:
      w(p, 0);
      w(p + 0x130, 0);
      w(p + 0x134, 0);
      m.set8(p + 0x138, 0);
      w(p + 0x139, 0);
      w(p + 0x13d, 1);
      for (U o = 4; o < 0x24; o += 4)
        w(p + o, 0);
      m.set8(p + 0x24, 0);
      for (U o = 0x124; o < 0x130; o += 4)
        w(p + o, 0);
      return p;
    case 0x559880:
      if (r(p + 0x139))
        call(0x597870, 0, {r(p + 0x139)});
      return 0;
    case 0x4cc720: {
      call(0x4a4df0, p);
      w(p + 0x2d, ~0u);
      w(p + 0x25, 0);
      w(p + 0x29, 0);
      w(p + 0x21, 0);
      w(p + 0x35, ~0u);
      w(p, 0x5c5f24);
      // The original temporary widget is constructed then removed at -1;
      // it has no publication or heap ownership in the normal-return path.
      const U temporary = m.temporary(0x21);
      call(0x4a4df0, temporary);
      w(temporary, 0x5c5c20);
      call(0x4a4f70, r(temporary + 4));
      for (U o : {0x39u, 0x3du, 0x45u, 0x4du})
        w(p + o, 0);
      w(p + 0x49, ~0u);
      w(p + 0x51, 0x321);
      w(p, 0x5c6c6c);
      w(p + 0x55, 0x14);
      return p;
    }
    case 0x4d27b0:
      for (U o : {0u, 4u, 8u, 0x1du, 0x21u, 0x25u})
        w(p + o, 0);
      m.set8(p + 0x1c, 0);
      return p;
    case 0x4ca910:
      w(p, 0x5c5c20);
      call(0x4a4f70, r(p + 4));
      w(p + 4, ~0u);
      return 0;
    case 0x475c10: {
      const U text = r(p + 0x45);
      w(p, 0x5c5f24);
      if (text)
        call(0x597870, 0, {text});
      call(0x4ca910, p);
      return 0;
    }
    case 0x4d27d0: {
      const U text = r(p + 8);
      w(p, 0);
      w(p + 4, 0);
      if (text) {
        call(0x597870, 0, {text});
        w(p + 8, 0);
      }
      return 0;
    }
    case 0x4cb040:
      return ownText(p, 0x45, a[0]);
    case 0x4d28c0:
      return ownText(p, 8, a[0]);
    case 0x4cb030:
      w(p + 0x41, a[0]);
      return 0;
    case 0x4ca9b0:
      w(p + 0x31, a[0]);
      return 0;
    case 0x4d28b0:
      w(p + 4, a[0]);
      return 0;
    case 0x4d2960:
      w(p, a[0]);
      return 0;
    case 0x4d2930:
      copy(p + 12, a[0], 16);
      return 1;
    case 0x4d2970:
      m.set8(p + 0x1c, a[0]);
      call(0x4a5a90, r(p + 4), {p + 0x1c, 8});
      call(0x4a5a90, r(p + 4), {p + 0x1c, 16});
      return 0;
    case 0x4a4e20: {
      if (r(0x642c68) >= 400)
        return 0;
      copy(p + 0x11, a[0], 16);
      w(p + 8, a[1]);
      w(p + 4, r(0x642c68));
      w(0x642c68, r(0x642c68) + 1);
      w(0x642034 + 4 * r(p + 4), p);
      return 1;
    }
    case 0x4cb0b0:
      w(p + 0x25, a[2]);
      w(p + 0x29, a[3]);
      w(p + 0x2d, a[0]);
      call(0x4a4e20, p, {a[1], a[4]});
      w(p + 0x39, a[4] ? 0 : 3);
      w(p + 0x3d, 0);
      return 0;
    case 0x4ca970:
      w(p + 0x39, 2);
      w(p + 0x3d, 2);
      return call(0x4a5000, p);
    case 0x4ca980:
      w(p + 0x3d, 1);
      return 0;
    case 0x4ca990:
      w(p + 0x3d, 0);
      return 0;
    case 0x4cabd0: {
      U begin = r(p + 4);
      if (begin && ((r(p + 8) - begin) & ~3u)) {
        const U end = r(p + 8);
        w(p + 8, begin);
        (void)end;
      }
      w(p + 16, 0);
      begin = r(p + 4);
      const U capacity = begin ? (r(p + 12) - begin) / 4 : 0;
      if (capacity < a[0]) {
        const U out = call(
            0x597850, 0, {(actor_services::signed32(a[0]) < 0 ? 0 : a[0]) * 4});
        U source = r(p + 4), end = r(p + 8), dest = out;
        while (source != end) {
          if (dest)
            w(dest, r(source));
          source += 4;
          dest += 4;
        }
        call(0x597870, 0, {r(p + 4)});
        w(p + 12, out + a[0] * 4);
        const U old = r(p + 4), count = old ? (r(p + 8) - old) / 4 : 0;
        w(p + 4, out);
        w(p + 8, out + count * 4);
      }
      return 0;
    }
    case 0x4def80: {
      const U position = a[0], count = a[1], value = a[2];
      const U begin = r(p + 4), end = r(p + 8), capacity = r(p + 12);
      auto forward = [&](U first, U last, U destination) {
        while (first != last) {
          if (destination)
            w(destination, r(first));
          first += 4;
          destination += 4;
        }
        return destination;
      };
      auto fill = [&](U first, U n) {
        while (n--) {
          if (first)
            w(first, r(value));
          first += 4;
        }
        return first;
      };
      if ((capacity - end) / 4 < count) {
        const U size = begin ? (end - begin) / 4 : 0,
                newCount = size + std::max(size, count);
        const U out =
            call(0x597850, 0,
                 {(actor_services::signed32(newCount) < 0 ? 0 : newCount) * 4});
        U destination = forward(begin, position, out);
        destination = fill(destination, count);
        forward(position, end, destination);
        call(0x597870, 0, {r(p + 4)});
        w(p + 12, out + newCount * 4);
        const U old = r(p + 4), oldSize = old ? (r(p + 8) - old) / 4 : 0;
        w(p + 4, out);
        w(p + 8, out + (oldSize + count) * 4);
      } else {
        const U remaining = (end - position) / 4;
        if (remaining < count) {
          forward(position, end, position + count * 4);
          fill(end, count - remaining);
          fill(position, remaining);
          w(p + 8, r(p + 8) + count * 4);
        } else if (count) {
          forward(end - count * 4, end, end);
          U source = end - count * 4, dest = end;
          while (source != position) {
            source -= 4;
            dest -= 4;
            w(dest, r(source));
          }
          fill(position, count);
          w(p + 8, r(p + 8) + count * 4);
        }
      }
      return 0;
    }
    case 0x57daa0: {
      if (a[0] != 1)
        return 1;
      for (U i = 0; i < r(r(p) + 16); ++i) {
        const U slot = p + 16 + 4 * i;
        U node = r(slot);
        if (r(node))
          node = r(node);
        if (!m.fastcall(
                0x584430, a[1], node,
                {r(p + 4) + i * 0x300, a[2], a[3], a[4], a[5], a[6], a[7]}))
          return 0;
        node = r(slot);
        for (U j = 0; j < 3; ++j)
          if (r(node))
            node = r(node);
        if (!m.fastcall(
                0x589280, a[1], node,
                {r(p + 4) + i * 0x300, a[2], a[3], a[4], a[5], a[6], a[7]}))
          return 0;
      }
      return 1;
    }
    case 0x4cad00: {
      const U argument = m.temporary(4);
      w(argument, a[0]);
      call(0x4def80, p, {r(p + 8), 1, argument});
      w(a[0] + 0x21, p);
      if (a[1]) {
        virt(a[0], 0x24);
        const U begin = r(p + 4);
        w(p + 16, begin ? (r(p + 8) - begin) / 4 - 1 : ~0u);
        virt(a[0], 0x20);
      }
      return 0;
    }
    case 0x4caf00: {
      const U current = r(r(p + 4) + r(p + 16) * 4);
      virt(current, 0x24);
      for (U i = 0;; ++i) {
        const U begin = r(p + 4);
        if (!begin || i >= (r(p + 8) - begin) / 4)
          break;
        const U object = r(begin + 4 * i);
        if (object == a[0]) {
          virt(object, 0x20);
          w(p + 16, i);
        }
      }
      return 0;
    }
    case 0x58d320: {
      const U dc = call(0x58b660, p);
      if (!dc)
        return 0;
      const U result =
          m.fastcall(r(0x5c5308), a[0], dc,
                     {r(p + 16), r(p + 24), r(p + 20), r(0x6e1f88)}) &
          255;
      const U surface = r(p + 4);
      m.com(r(r(surface) + 0x80), surface, {dc});
      return result;
    }
    case 0x5c0510:
    case 0x5c051e:
      call(0x597870, 0, {r(p - 0x398)});
      return 0;
    case 0x5bf610:
    case 0x5bf61e:
    case 0x5bf62c:
      call(0x597870, 0, {r(p - 0x38c)});
      return 0;
    case 0x5bf63a:
      call(0x597870, 0, {r(p - 0x37c)});
      return 0;
    case 0x5c1b50:
      call(0x4ca910, r(p - 0x34));
      return 0;
    case 0x4a4df0:
    case 0x4a4f70:
    case 0x58b3e0:
    case 0x59bf87:
    case 0x59c4ab:
      return actor_services::ConstructionServices<Memory>(m).run(entry, p,
                                                                 args);
    case 0x597850:
    case 0x597870:
    case 0x5979b0:
    case 0x57d0d0:
    case 0x59c441:
      return actor_services::Services<Memory>(m).run(entry, p, args);
    default:
      (void)second;
      return std::nullopt;
    }
  }
};
} // namespace mnm::reconstruction
