#pragma once
#include "actor_session.hpp"

namespace mnm::reconstruction::actor_services {
// Raw value-address adapters. Lower file, palette, task, support and platform
// services remain explicit calls through M; no host pointer enters game
// storage.
template <class M> class FrontierProviders {
  M &m;
  U r(U a) { return m.get32(a); }
  void w(U a, U v) { m.set32(a, v); }
  static int i(U v) { return v <= 0x7fffffff ? int(v) : -1 - int(~v); }
  U call(U e, U p, std::initializer_list<U> a = {}) {
    return m.invoke(e, p, a);
  }
  U fast(U e, U p, U q, std::initializer_list<U> a = {}) {
    return m.fastcall(e, p, q, a);
  }
  void copy(U d, U s, U n) {
    for (U k = 0; k < n; ++k)
      m.set8(d + k, m.get8(s + k));
  }
  void text(U d, U s) {
    for (U k = 0;; ++k) {
      U c = m.get8(s + k);
      m.set8(d + k, c);
      if (!c)
        break;
    }
  }
  U length(U p) {
    U n = 0;
    while (m.get8(p + n))
      ++n;
    return n;
  }
  U alloc(U n) { return call(0x597880, 0, {n}); }
  void release(U p) {
    if (p)
      call(0x5979b0, 0, {p});
  }
  U read(U p, U n, U f) { return call(0x59c359, 0, {p, 1, n, f}); }
  void close(U f) { call(0x59c4c3, 0, {f}); }
  U actor(U index) {
    return index < r(0x6def5c) ? r(0x6def58) + index * 0xe4b : 0;
  }
  U cell(U x, U y, U z) {
    return r(0x6c54dc) + (x + r(0x6cb942 + y * 4) + r(0x6cb8c2 + z * 4)) * 12;
  }
  U wrap(U v, U size) {
    if (i(v) < 0)
      v += size;
    if (i(v) >= i(size))
      v -= size;
    return v;
  }
  U animation(U p, U name) {
    for (U off : {0u, 4u, 8u})
      release(r(p + off));
    for (U off : {0u, 4u, 8u})
      w(p + off, 0);
    text(p + 16, name);
    U f = fast(0x4a1120, name, 0x5d96e0);
    if (!f)
      return 0;
    w(p, alloc(44));
    if (read(r(p), 44, f) != 44) {
      close(f);
      return 0;
    }
    if (r(p + 12)) {
      U path = m.temporary(256), part = m.temporary(4);
      if (!call(0x5c50c0, 0, {name, 256, path, part}))
        return 0;
      m.set8(r(part), 0);
      text(path + length(path), r(p) + 24);
      if (!call(0x57d310, r(p + 0x110), {path, 2})) {
        call(0x5c520c, 0, {0, 0x5d96a0, 0x5d96d8, 0x42030});
        close(f);
        call(0x5c5208, 0, {r(0x656614)});
      }
    }
    U h = r(p), count = r(h + 20), bytes = count * 4;
    if (!bytes) {
      close(f);
      return 0;
    }
    w(p + 4, alloc(bytes));
    if (read(r(p + 4), bytes, f) != bytes) {
      close(f);
      return 0;
    }
    U remaining = call(0x59d9eb, 0, {call(0x5b74d8, 0, {f})}) - bytes - 44;
    if (!remaining) {
      close(f);
      return 0;
    }
    U version = r(h + 12);
    if (version == 1 || version == 2) {
      call(0x5c520c, 0, {0, 0x5d9668, 0x5d9698, 0x42030});
      close(f);
      call(0x5c5208, 0, {r(0x656614)});
    }
    if (version == 3 || version == 4) {
      U old = alloc(remaining);
      if (read(old, remaining, f) != remaining) {
        close(f);
        return 0;
      }
      U stride = version == 3 ? 28 : 36;
      w(p + 8, alloc((remaining / stride) * 44));
      for (U bank = 0; bank + 1 < count; ++bank) {
        U index = r(r(p + 4) + bank * 4);
        for (;; ++index) {
          U src = old + index * stride, dst = r(p + 8) + index * 44;
          copy(dst, src, 16);
          text(dst + 16, src + 16);
          w(dst + 24, r(src + 24));
          w(dst + 28, version == 4 ? r(src + 28) : 0);
          w(dst + 32, version == 4 ? r(src + 32) : 0);
          w(dst + 36, 0);
          w(dst + 40, 0);
          if (r(src) == 6)
            break;
        }
      }
      w(h + 12, 5);
      release(old);
    } else if (version == 5) {
      w(p + 8, alloc(remaining));
      if (read(r(p + 8), remaining, f) != remaining) {
        close(f);
        return 0;
      }
    }
    if (r(h + 4) != remaining + 44 + bytes) {
      close(f);
      return 0;
    }
    close(f);
    for (U bank = 1; bank < count; ++bank)
      if (r(r(p + 8) + r(r(p + 4) + bank * 4) * 44 - 44) != 6) {
        call(0x5c520c, 0, {0, 0x5d9650, 0x5d9698, 0x42030});
        call(0x5c5208, 0, {r(0x656614)});
        break;
      }
    return 1;
  }
  U sprite(U p, U name, U shades) {
    if (r(p)) {
      release(r(p));
      for (U off : {0u, 4u, 8u, 12u})
        w(p + off, 0);
    }
    for (U bank = 0; bank < 4; ++bank) {
      U n = r(p + 16 + bank * 4);
      while (n) {
        U next = r(n);
        release(n);
        n = next;
      }
      w(p + 16 + bank * 4, 0);
    }
    if (r(p + 32)) {
      release(r(p + 32));
      w(p + 32, 0);
    }
    U f = fast(0x4a1120, name, 0x5d96e0);
    if (!f)
      return 0;
    U size = call(0x59d9eb, 0, {call(0x5b74d8, 0, {f})});
    if (!size) {
      close(f);
      return 0;
    }
    U header = m.temporary(24);
    if (read(header, 24, f) != 24) {
      close(f);
      return 0;
    }
    call(0x59cf25, 0, {f, 0, 0});
    if (r(header + 8) != 4) {
      call(0x5c520c, 0, {0, name, 0x5f1430, 0x42030});
      close(f);
      return 0;
    }
    w(p, alloc(size));
    if (!r(p)) {
      close(f);
      return 0;
    }
    if (read(r(p), size, f) != size) {
      close(f);
      return 0;
    }
    U palettes = r(r(p) + 16), offset = r(p) + 24 + palettes * 0x300;
    w(p + 4, palettes ? r(p) + 24 : 0);
    w(p + 8, offset);
    w(p + 12, offset + r(r(p) + 12) * 4);
    close(f);
    if ((palettes && !shades) || palettes >= 5)
      return 0;
    if (!palettes) {
      if (r(0x6e1f88))
        call(0x57d560, p, {});
      return 1;
    }
    if (!call(0x57d690, p, {0, shades}))
      return 0;
    for (U j = 0; j < r(r(p) + 12); ++j) {
      U frame = r(p + 12) + r(r(p + 8) + j * 4);
      w(frame + 28, r(p + 16 + r(frame + 28) * 4));
    }
    return 1;
  }
  U palettes(U p, U mode, U shades) {
    constexpr U builders[] = {0x582440, 0x582a80, 0x583330, 0x583bb0, 0x586660,
                              0x5870d0, 0x585d80, 0x587e70, 0x588730, 0x588bf0};
    if (mode > 9)
      return 1;
    for (U j = 0; j < r(r(p) + 16); ++j) {
      U slot = p + 16 + j * 4;
      while (r(slot))
        slot = r(slot);
      if (!fast(builders[mode], shades, slot, {r(p + 4) + j * 0x300}))
        return 0;
    }
    return 1;
  }
  void unlink(U p, U c) {
    U head = m.get16(c + 2);
    if (head != 0xffff) {
      if (head == r(p))
        m.set16(c + 2, r(p + 0x1a0));
      else {
        U manager = r(p + 36),
          n = head < r(manager + 4) ? r(manager) + head * 0x22e : 0;
        for (;;) {
          U next = m.get16(n + 0x1a0);
          if (next == r(p)) {
            m.set16(n + 0x1a0, r(p + 0x1a0));
            break;
          }
          if (next == 0xffff)
            return;
          n = next < r(manager + 4) ? r(manager) + next * 0x22e : 0;
        }
      }
      m.set16(p + 0x1a0, 0xffff);
    }
    if (!m.get16(c) && m.get16(c + 2) == 0xffff && m.get16(c + 4) == 0xffff &&
        m.get16(c + 6) == 0xffff && !(m.get8(c + 11) & 32) &&
        actor_lifecycle::emptyColumn(m, c))
      m.set8(c + 8, m.get8(c + 8) | 128);
  }
  void frame(U c, U value) {
    if (value > r(r(0x656608) + 12))
      return;
    U catalog = r(0x65660c);
    if (r(catalog + m.get16(c) * 0x164 + 0x94) == 16)
      m.set16(c + 10, m.get16(c + 10) & 0xfffe);
    m.set16(c, value);
    if (r(catalog + value * 0x164 + 0x94) == 16) {
      m.set8(c + 10, m.get8(c + 10) | 1);
      return;
    }
    if (!m.get16(c)) {
      m.set16(c + 10, m.get16(c + 10) & 0xfffc);
      call(0x4ed440, 0x6c5490, {c});
    }
  }
  U support(U p, U x, U y, U z, U profile) {
    auto normalized = [&](U value, U size) {
      while (i(value) < 0)
        value += size;
      while (i(value) >= i(size))
        value -= size;
      return value;
    };
    auto query = [&](U a, U b, U c) {
      return call(
          0x4f3320, p,
          {normalized(a, r(0x6c5494)), normalized(b, r(0x6c5498)), c, profile});
    };
    if (r(profile + 4) == 1)
      return call(0x4f3320, p, {x, y, z, profile});
    int hits = 0, sx = 0, sy = 0;
    bool level = false;
    for (U dy = 0; dy < 2; ++dy)
      for (U dx = 0; dx < 2; ++dx)
        for (U depth = 0; depth < 2 && i(z - depth) >= 0; ++depth) {
          U q = z - depth;
          if (query(x + dx, y + dy, q)) {
            ++hits;
            sx += dx;
            sy += dy;
            if (!depth)
              level = true;
            break;
          }
          if (r(profile + 12) == 2 && i(q) >= i(r(p + 0x7f0)) &&
              i(q) < i(r(p + 0x7f0) + 2)) {
            ++hits;
            sx += dx;
            sy += dy;
            break;
          }
        }
    auto abs = [](int n) { return n < 0 ? -n : n; };
    return abs((2 * sx - hits) * 2) < hits && abs((2 * sy - hits) * 2) < hits &&
           level;
  }
  U transform(U type) {
    U n = call(0x464b20, 0x6dbf00 + type * 0x114, {});
    if (n < 8)
      throw std::invalid_argument("5050d0 uninitialized original result for "
                                  "fewer than eight sequences");
    U good = fast(0x504cc0, type, 0);
    if (n >= 184 && !fast(0x504cc0, type, 176))
      good = 0;
    for (U bank = 0; bank <= n - 8; bank += 8) {
      if (!fast(0x504ec0, type, bank))
        good = 0;
      if (!fast(0x504ec0, type, bank + 1))
        good = 0;
    }
    return good;
  }
  void profile(U p, U type) {
    U descriptor = 0x6dbf00 + type * 0x114;
    auto sequence = [&](U bank) { return call(0x464b30, descriptor, {bank}); };
    for (U bank = 0; bank < 8; ++bank)
      for (U family = 0; family < 2; ++family) {
        if (family && !r(p + 60))
          continue;
        U first = sequence(bank + family * 176);
        while (r(first) != 0)
          first += 44;
        U last = first;
        while (r(last) != 6)
          last += 44;
        do {
          last -= 44;
        } while (r(last) != 0);
        if (bank < 2) {
          U delta = r(first + 8) - r(last + 8);
          U magnitude = i(delta) < 0 ? 0 - delta : delta;
          U total = (magnitude * 3) & 0x3ffffff;
          if (!bank)
            total <<= 1;
          U out = p + 0xd8 + family * 0x60 + bank * 0x30;
          U sum = total;
          for (U j = 0; j < 12; ++j, sum += total)
            w(out + j * 4, sum / 12);
          for (U j = 11; j; --j)
            w(out + j * 4, r(out + j * 4) - r(out + (j - 1) * 4));
        }
      }
    call(0x505160, p, {});
    U control = m.temporary(64);
    call(0x464c50, control, {});
    call(0x464ca0, control, {descriptor});
    for (U family = 0; family < 2; ++family) {
      U out = p + 0x5b5 + family * 4;
      w(out, 0);
      if (family && r(p + 0x74) == ~0u)
        continue;
      U count = 0;
      U lo = family ? 256 : 24, hi = family ? 264 : 48;
      for (U bank = lo; bank < hi; ++bank)
        if (r(sequence(bank)) != 6) {
          call(0x464cb0, control, {bank, 0});
          while (call(0x464ec0, control, {}) != 1)
            w(out, r(out) + 1);
          ++count;
        }
      if (count)
        w(out, U(i(r(out)) / int(count)) + (family ? r(p + 48) : 3));
    }
  }
  void stun(U p, U duration, U status, U force) {
    if (r(p + 0xa8) == 27) {
      call(0x50ab30, p, {});
      return;
    }
    U old = r(p + 0x5ec);
    if (!r(p + 0xe4) || r(p + 0xa0) == 3 ||
        (!U(force & 255) && (old == 15 || old == 16)))
      return;
    if (status != 3 && i(duration) < 1)
      duration = 1;
    if (r(p + 0xa0) && status) {
      w(p + 0x9c, duration);
      return;
    }
    if (r(p + 0x798) && status) {
      w(p + 0xa0, status);
      w(p + 0x798, 1);
      w(p + 0x9c, duration);
      return;
    }
    bool airborne = r(p + 0x108) && status;
    w(p + 0xb8b, 0);
    w(p + 0x164, 0);
    if (airborne) {
      w(p + 0xa4, r(p + 16));
      w(p + 0xde3, 0);
      m.set8(p + 0x962, 0);
      w(p + 0x963, 0);
    }
    if (r(p + 0x664)) {
      U targetKind = r(p + 0x5fc);
      if (targetKind == 10 && !airborne) {
        U t = actor(r(p + 0x64c));
        w(p + 0x664, 0);
        U n = r(t + 0x668) - 1;
        w(t + 0x668, i(n) < 0 ? 0 : n);
      } else if (targetKind == 20 || (targetKind == 10 && airborne)) {
        U peer = call(0x4970b0, 0x6def58,
                      {r(p + (targetKind == 10 ? 0x64c : 0x6a8))});
        w(p + 0x664, 0);
        call(0x50eb60, peer, {0});
      }
    }
    if (airborne) {
      call(0x50aa30, p, {16});
      w(p + 0xa0, status);
      w(p + 0x9c, duration);
      return;
    }
    if (!r(p + 0xa0))
      w(p + 0xa0, status);
    old = r(p + 0x5ec);
    if (old != 8 || i(r(p + 0x9c)) < i(duration))
      w(p + 0x9c, duration);
    if (r(p + 0xe4) && old != 8) {
      if (old == 2 || old == 6 || old == 3)
        w(p + 0x5f0, old);
      call(0x50aa30, p, {8});
    }
    if (r(p + 0x6e8)) {
      call(0x4943f0, r(p + 0x6e8), {});
      w(p + 0x6e8, 0);
    }
    U list = r(p + 0x8f9);
    if (r(list + 8)) {
      U head = r(list + 4);
      for (U node = r(head); node != head; node = r(node))
        if (r(node + 8) == 2 && r(node + 40)) {
          call(0x4943f0, r(node + 40), {});
          w(node + 40, 0);
        }
    }
    if (r(p + 0x68))
      call(0x5120c0, p, {});
  }
  void attack(U p, U target) {
    U selected = 24, definition = r(p + 0xac);
    if (r(definition + 8) == 2) {
      U x, y;
      bool valid = true;
      if (target) {
        x = r(target + 8);
        y = r(target + 12);
      } else {
        x = r(p + 0x96b);
        y = r(p + 0x96f);
        U h = m.get16(cell(x, y, r(p + 0x973)) + 2);
        valid = h != 0xffff;
        if (valid) {
          U effect = h < r(0x68949c) ? r(0x689498) + h * 0x22e : 0;
          valid = r(effect + 40) == 26;
        }
      }
      if (valid) {
        U dx = r(p + 8) - x, dy = r(p + 12) - y;
        U width = r(0x6c5494), height = r(0x6c5498);
        if (i(dx) > i(width) / 2)
          dx -= width;
        if (i(dx) < -i(width) / 2)
          dx += width;
        if (i(dy) > i(height) / 2)
          dy -= height;
        if (i(dy) < -i(height) / 2)
          dy += height;
        U ix = 2 - dx, iy = dy + 2;
        if (ix < 5 && iy < 5) {
          U index = ix * 5 + iy;
          if (!target) {
            U facing = r(0x5e52ac + index * 4);
            if (facing < 8) {
              w(p + 0x608, facing);
              U screen = (facing - 2 * r(0x6e9850)) & 7;
              if (m.get8(p + 0x722))
                screen = (screen - 4) & 7;
              w(p + 0x60c, screen);
              if (r(p + 0x7cd) == 3)
                call(0x513ff0, p, {});
              else if (r(p + 0x7cd) == 2 && r(p + 0x7d1))
                call(0x494a50, r(p + 0x7d1), {screen});
              if (r(p + 0x855))
                call(0x464e20, p + 0x825, {screen + 8});
            }
          }
          U table =
              target && r(r(target + 0xac) + 8) == 2 ? 0x5e5248 : 0x5e51e4;
          U candidate = r(table + index * 4);
          if ((!(m.get8(p + 0x60c) & 1) || candidate == 24) && candidate != ~0u)
            selected = candidate;
        }
      }
    }
    w(p + 0xe0, selected);
    call(0x464cb0, p + 0xb0, {r(p + 0x60c) + selected, 0});
  }
  void placement(U p, U type, U source) {
    if (type > 88)
      return;
    if (r(0x5fed7c) && r(p) > 124 && m.get8(0x6c5040))
      throw std::invalid_argument(
          "Effect diagnostic file branch is outside this owned-store scope");
    for (U off : {0x1b2u, 0x1aeu, 0x1aau, 0x1beu, 0x1bau, 0x1b6u, 0x1cau,
                  0x1c6u, 0x1c2u, 0x212u, 0x216u, 0x21au})
      w(p + off, ~0u);
    copy(p + 44, source, 252);
    U top = r(0x6c549c) * 16 - 1;
    for (U off : {52u, 64u})
      if (i(r(p + off)) >= i(top))
        w(p + off, top);
    for (U off : {44u, 56u})
      w(p + off, wrap(r(p + off), r(0x6c5494) * 32));
    for (U off : {48u, 60u})
      w(p + off, wrap(r(p + off), r(0x6c5498) * 32));
    if (r(p + 68) != ~0u) {
      U owner = actor(r(p + 68));
      w(p + 0x212, r(owner + 0x174));
      call(0x522720, owner, {p + 0x216});
    }
    bool endpoint = type == 9 || type == 11 || type == 31 || type == 32 ||
                    type == 21 || type == 30,
         position = true;
    if (type == 21 && !call(0x48a270, p, {}))
      return;
    if (type == 30 && i(r(0x5e1478)) > 0) {
      U block = call(0x597850, 0, {100});
      U particle = block ? call(0x546c20, block,
                                {r(p + 20), r(p + 24), r(0x6c549c) << 4})
                         : 0;
      if (particle) {
        if (r(p + 72) != ~0u) {
          U profile = r(p + 76) * 0x2d1, kind = r(0x6b13cd + profile);
          w(particle + 20, 0);
          w(particle + 24, (r(0x6b1270 + profile) * r(0x5e1478)) / 100);
          w(particle + 28, 0);
          for (U j = 0; j < 3; ++j)
            w(particle + 40 + j * 4, r(0x5e143c + kind * 12 + j * 4));
          w(particle + 52, 64);
          w(particle + 84, actor(r(p + 72)));
          w(particle + 88, 1);
          w(particle + 92, 1);
        }
        if (!call(0x5457e0, 0x6cbb8a, {particle})) {
          call(r(r(particle)), particle, {1});
          position = false;
        }
      }
    }
    if (position) {
      U off = endpoint ? 56 : 44;
      U x = r(p + off), y = r(p + off + 4), z = r(p + off + 8);
      w(p + 8, x >> 5);
      w(p + 12, y >> 5);
      w(p + 16, z >> 4);
      w(p + 20, x);
      w(p + 24, y);
      w(p + 28, z);
    }
    w(p + 0x198, 0);
    U c = cell(r(p + 8), r(p + 12), r(p + 16));
    w(p + 0x194, c);
    if (!(m.get8(c + 11) & 64)) {
      U h = m.get16(c + 2);
      if (h == 0xffff)
        m.set16(c + 2, r(p));
      else {
        U manager = r(p + 36), tail;
        do {
          tail = h < r(manager + 4) ? r(manager) + h * 0x22e : 0;
          h = m.get16(tail + 0x1a0);
        } while (h != 0xffff);
        m.set16(tail + 0x1a0, r(p));
        m.set16(p + 0x1a0, 0xffff);
      }
      m.set16(c + 8, m.get16(c + 8) & 0xff7f);
    }
    w(p + 400, r(0x65660c) + m.get16(c) * 0x164);
    w(p + 4, 1);
    if (i(r(0x6898dc)) < i(r(p) + 1))
      w(0x6898dc, r(p) + 1);
    w(p + 0x22a, 1);
    for (U off : {0x20eu, 0x21eu, 0x19cu, 0x222u})
      w(p + off, 0);
    call(0x494ab0, p, {type});
    for (U j = 0; j < 3; ++j) {
      w(p + 0x202 + j * 4, r(p + 8 + j * 4));
      w(p + 0x1f6 + j * 4, r(p + 8 + j * 4));
    }
  }
  void rectangle(U src, U dst, U rect, U x, U y) {
    U bounds = m.temporary(16);
    w(bounds, x);
    w(bounds + 4, y);
    w(bounds + 8, r(rect + 8) - r(rect) + x);
    w(bounds + 12, r(rect + 12) - r(rect + 4) + y);
    if (!r(0x6f68d8) && dst == r(0x6f98c0)) {
      U point = m.temporary(8), window = m.temporary(4);
      w(point, 0);
      w(point + 4, 0);
      U iface = r(0x6f98d0), hwnd = 0;
      if (iface && !call(r(r(iface) + 16), 0, {iface, window}))
        hwnd = r(window);
      call(0x5c51fc, 0, {hwnd, point});
      for (U j = 0; j < 4; ++j)
        w(bounds + j * 4, r(bounds + j * 4) + r(point + (j & 1) * 4));
    }
    U key = m.temporary(8);
    for (;;) {
      U iface = r(dst + 8), result;
      if (!r(0x6f68d8))
        result = call(r(r(iface) + 20), 0,
                      {iface, bounds, r(src + 8), rect,
                       r(0x6f98d8) ? 0x8000u : 0x1008000u, 0});
      else
        result = call(r(r(iface) + 28), 0,
                      {iface, x, y, r(src + 8), rect, r(0x6f98d8) ? 1u : 17u});
      if (!result)
        return;
      call(0x484c00, 0x6f8158, {result});
      if (result != 0x887601c2)
        continue;
      for (U surface : {src, dst}) {
        U handle = r(surface + 8);
        if (handle && !call(r(r(handle) + 0x6c), 0, {handle})) {
          if (r(0x6de6dd)) {
            w(key, m.get16(surface + 44));
            w(key + 4, m.get16(surface + 44));
            call(r(r(handle) + 0x74), 0, {handle, 8, key});
          }
          U reload = r(surface + 40);
          if (reload == ~0u)
            call(0x4a2f90, 0, {});
          else if (reload)
            call(r(r(reload)), reload, {0});
        }
      }
    }
  }

public:
  explicit FrontierProviders(M &memory) : m(memory) {}
  U run(U entry, U self, std::initializer_list<U> args) {
    U a[4]{};
    if (args.size() > 4)
      throw std::invalid_argument("Frontier argument extent");
    std::copy(args.begin(), args.end(), a);
    switch (entry) {
    case 0x4644d0:
      return animation(self, a[0]);
    case 0x57d310:
      return sprite(self, a[0], a[1]);
    case 0x57d690:
      return palettes(self, a[0], a[1]);
    case 0x488330:
      unlink(self, a[0]);
      break;
    case 0x493f20:
      placement(self, a[0], a[1]);
      break;
    case 0x4eb820:
      frame(self, a[0]);
      break;
    case 0x4f4330:
      return support(self, a[0], a[1], a[2], a[3]);
    case 0x5050d0:
      return transform(self);
    case 0x505220:
      profile(self, a[0]);
      break;
    case 0x50d060:
      stun(self, a[0], a[1], a[2]);
      break;
    case 0x50e400:
      attack(self, a[0]);
      break;
    case 0x58c8a0:
      rectangle(self, a[0], a[1], a[2], a[3]);
      break;
    default:
      throw std::invalid_argument("Unknown frontier root");
    }
    return 0;
  }
};
template <class M> class FrontierComposition {
  M &m;

public:
  explicit FrontierComposition(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U self, std::initializer_list<U> args = {},
                          U second = 0) {
    switch (entry) {
    case 0x4644d0:
    case 0x488330:
    case 0x493f20:
    case 0x4eb820:
    case 0x4f4330:
    case 0x5050d0:
    case 0x505220:
    case 0x50d060:
    case 0x50e400:
    case 0x57d310:
    case 0x57d690:
    case 0x58c8a0:
      return FrontierProviders<M>(m).run(entry, self, args);
    default:
      return SessionComposition<M>(m).tryRun(entry, self, args, second);
    }
  }
};
} // namespace mnm::reconstruction::actor_services
