#pragma once
#include "actor_profile_palette_providers.hpp"
#include "font_layout.hpp"
#include "glyph_backend_state.h"
#include <cstring>

namespace mnm::reconstruction::actor_services {
// Build-specific byte-address adapters over owned storage. Normal masked,
// round-to-nearest FP state only; no CPU ABI or malformed-pointer equivalence.
template <class M> class FontProviders {
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  static int i(U n) { return n < 0x80000000u ? int(n) : -1 - int(~n); }
  U call(U e, U p, std::initializer_list<U> a = {}) {
    return m.invoke(e, p, a);
  }
  void clear(U p) {
    for (U k : {0u, 8u, 12u, 16u, 20u})
      w(p + k, 0);
  }
  void chain(U p) {
    while (p) {
      U next = r(p);
      call(0x5979b0, 0, {p});
      p = next;
    }
  }
  void retire(U p) {
    if (r(p))
      call(0x5979b0, 0, {r(p)});
    if (r(p + 20))
      call(0x4a58c0, p, {r(p + 20)});
    clear(p);
  }
  void setup(U p, U destructor) {
    clear(p);
    for (U k : {0x220u, 0x228u, 0x22cu})
      w(p + k, 0);
    w(p + 0x224, 20);
    w(p + 0x230, 1);
    w(p + 0x234, 1);
    call(0x59b7e2, 0, {destructor});
  }
  U load(U p, U path, U conversion) {
    U file = call(0x59cae2, 0, {path, 0x5d96e0});
    if (!file)
      return 0;
    U descriptor = call(0x5b74d8, 0, {file}),
      size = call(0x59d9eb, 0, {descriptor});
    if (!size) {
      call(0x59c4c3, 0, {file});
      return 0;
    }
    U raw = call(0x597880, 0, {size});
    w(p, raw);
    if (!raw) {
      call(0x59c4c3, 0, {file});
      return 0;
    }
    if (call(0x59c359, 0, {raw, 1, size, file}) != size) {
      call(0x59c4c3, 0, {file});
      return 0;
    }
    if (r(raw + 4) != size) {
      retire(p);
      call(0x59c4c3, 0, {file});
      return 0;
    }
    U version = r(raw + 8), palette = r(raw + 32), rows = r(raw + 16),
      count = r(raw + 12), table = 0;
    if (version == 2 || version == 3) {
      if (version == 2)
        w(raw + 36, 0);
      U offset = version == 2 ? 36 : 40;
      w(p + 4, palette ? raw + offset : 0);
      w(p + 8, raw + offset + (palette ? 768 : 0));
      U records = version == 2 ? count : r(raw + 36);
      table = palette ? r(p + 8) + records * rows * 8
                      : r(p + 12) + records * rows * 8;
      w(p + 12, table);
      w(p + 16, table + count * 4);
    }
    call(0x59c4c3, 0, {file});
    w(p + 0x220, r(raw + 20) + r(raw + 24));
    if (palette && conversion) {
      m.fastcall(0x582440, conversion, p + 20, {r(p + 4)});
      for (U k = 0; k < count; ++k)
        w(r(p + 16) + r(r(p + 12) + k * 4) + 28, r(p + 20));
    }
    return 1;
  }
  rendering::ByteFont contours(U p) {
    rendering::ByteFont f;
    f.glyphCount = r(r(p) + 12);
    f.rows = r(r(p) + 16);
    if (f.rows > 128 || f.glyphCount > 256)
      throw std::invalid_argument("Owned font contour extent");
    f.contours.reserve(f.rows * f.glyphCount);
    for (U k = 0; k < f.rows * f.glyphCount; ++k)
      f.contours.push_back(
          {i(r(r(p + 8) + k * 8)), i(r(r(p + 8) + k * 8 + 4))});
    return f;
  }
  rendering::FontCursor cursor(U p, U rows) {
    rendering::FontCursor c;
    c.x = i(r(p + 0x218));
    c.y = i(r(p + 0x21c));
    c.tabAdvance = i(r(p + 0x224));
    c.tracking = i(r(p + 0x230));
    for (U k = 0; k < rows; ++k)
      c.trailing.push_back(i(r(p + 24 + k * 4)));
    return c;
  }
  void trailing(U p, const rendering::FontCursor &c) {
    for (U k = 0; k < c.trailing.size(); ++k)
      w(p + 24 + k * 4, U(c.trailing[k]));
  }
  U advance(U p, U byte, U update) {
    auto f = contours(p);
    auto c = cursor(p, f.rows);
    U result = U(rendering::fontByteAdvance(
        f, c, static_cast<unsigned char>(byte), update != 0, i(r(0x6e1ed4))));
    if (update)
      trailing(p, c);
    return result;
  }
  void glyph(U frame, U x, U y, U red, U green, U blue) {
    U table[64];
    if (!r(0x6f5eac)) {
      if (mnm_glyph_coverage_bits(0, table))
        throw std::runtime_error("Coverage generation");
      for (U k = 0; k < 64; ++k)
        w(0x6f5dac + k * 4, table[k]);
    }
    U width = r(frame + 4), height = r(frame + 8);
    int left = i(x) - i(r(frame + 12)), top = i(y) - i(r(frame + 16));
    int clipLeft = i(r(0x6e0008)), clipTop = i(r(0x6cbb6c)),
        clipRight = i(r(0x6a49b8)), clipBottom = i(r(0x656618));
    if (!width || !height || left + int(width) <= clipLeft ||
        top + int(height) <= clipTop || left >= clipRight ||
        top >= clipBottom || left < clipLeft || left + int(width) > clipRight)
      return;
    bool format = r(0x6e1f88) != 0;
    unsigned target[] = {red / 8, green / (format ? 8u : 4u), blue / 8};
    for (U row = 0; row < height; ++row) {
      int dy = top + int(row);
      if (dy < clipTop || dy >= clipBottom)
        continue;
      U control = frame + r(frame + 40 + row * 8),
        colour = frame + r(frame + 44 + row * 8), at = 0;
      bool opaque = false;
      while (at < width) {
        U n = m.get8(control++);
        if (n > width - at || (!n && opaque))
          throw std::invalid_argument("Font RLE run");
        if (opaque)
          for (U k = 0; k < n; ++k) {
            U index = m.get8(colour++);
            if (index >= 64)
              throw std::invalid_argument("Font coverage index");
            U bits = r(0x6f5dac + index * 4);
            float value;
            std::memcpy(&value, &bits, 4);
            long double weight = value, other = 1.L - weight;
            U dest = r(0x658174) + (U(dy) * r(0x6a2dc8) + U(left) + at + k) * 2,
              old = m.get16(dest);
            unsigned d[] = {format ? (old >> 10) & 31 : (old >> 11) & 31,
                            format ? (old >> 5) & 31 : (old >> 5) & 63,
                            old & 31},
                     out[3];
            for (U c = 0; c < 3; ++c)
              out[c] = unsigned(d[c] * other + target[c] * weight);
            m.set16(dest,
                    (out[0] << (format ? 10 : 11)) | (out[1] << 5) | out[2]);
          }
        at += n;
        opaque = !opaque;
      }
    }
  }
  void byte(U p, const U *a) {
    auto f = contours(p);
    auto c = cursor(p, f.rows);
    auto g = rendering::fontByteGlyph(f, c, static_cast<unsigned char>(a[0]),
                                      i(r(0x6e1ed4)));
    trailing(p, c);
    w(p + 0x218, U(c.x));
    if (g.frame)
      m.fastcall(0x581ec0, r(p + 16) + r(r(p + 12) + *g.frame * 4), U(g.x),
                 {U(g.y), a[1], a[2], a[3]});
  }
  U layout(U entry, U p, const U *a) {
    rendering::FontLayoutInput in;
    in.kind = entry == 0x4a5ce0   ? rendering::FontLayoutKind::rectangle
              : entry == 0x4a6190 ? rendering::FontLayoutKind::heightLimited
                                  : rendering::FontLayoutKind::measure;
    bool limited = entry == 0x4a6190, measure = entry == 0x4a6630;
    auto f = contours(p);
    in.cursor = cursor(p, f.rows);
    in.ascent = i(r(r(p) + 20));
    in.descent = i(r(r(p) + 24));
    in.lineHeight = i(r(p + 0x220));
    in.firstX = limited ? i(a[2]) : 0;
    in.punctuationMode = i(r(0x6e1ed4));
    in.flags = static_cast<unsigned char>(a[measure ? 3 : limited ? 6 : 5]);
    in.bounds = {i(r(a[1])), i(r(a[1] + 4)), i(r(a[1] + 8)), i(r(a[1] + 12))};
    in.nullSource = !a[0];
    if (!in.nullSource) {
      for (U k = 0; k < 65536; ++k) {
        auto b = static_cast<unsigned char>(m.get8(a[0] + k));
        in.text.push_back(b);
        if (!b)
          break;
      }
    }
    auto out = rendering::fontLineLayout(f, in);
    w(p + 0xe38, U(out.lines.size()));
    bool early = in.nullSource ||
                 (measure && U(in.bounds.bottom - in.bounds.top) <
                                 U(in.ascent + in.descent)) ||
                 (!measure && !limited && in.text.size() == 1);
    if (early)
      return out.width;
    for (U k = 0; k < 32; ++k)
      w(p + 24 + k * 4, 0);
    trailing(p, out.cursor);
    w(p + 0x218, U(out.cursor.x));
    if (!measure)
      w(p + 0x21c, U(out.cursor.y));
    for (U k = 0; k < out.lines.size(); ++k) {
      auto l = out.lines[k];
      w(p + 0x238 + k * 12, U(l.x));
      w(p + 0x23c + k * 12, l.count);
      w(p + 0x240 + k * 12, a[0] + l.source);
    }
    if (!measure) {
      U consumed = a[limited ? 7 : 6];
      if (consumed && out.consumed)
        w(consumed, a[0] + *out.consumed);
      for (auto g : out.glyphs)
        glyph(r(p + 16) + r(r(p + 12) + *g.frame * 4), U(g.x), U(g.y),
              a[limited ? 3 : 2], a[limited ? 4 : 3], a[limited ? 5 : 4]);
    }
    return out.width;
  }

public:
  explicit FontProviders(M &memory) : m(memory) {}
  U run(U entry, U self, std::initializer_list<U> args = {}, U second = 0) {
    U a[8]{};
    if (args.size() > 8)
      throw std::invalid_argument("Font arguments");
    std::copy(args.begin(), args.end(), a);
    switch (entry) {
    case 0x4be870:
      setup(0x6b0400, 0x4be8c0);
      return 0;
    case 0x4be910:
      setup(0x656680, 0x4be960);
      return 0;
    case 0x4be9b0:
      setup(0x6df1c0, 0x4bea00);
      return 0;
    case 0x4bea50:
      setup(0x6a5138, 0x4beaa0);
      return 0;
    case 0x4be8c0:
      retire(0x6b0400);
      return 0;
    case 0x4be960:
      retire(0x656680);
      return 0;
    case 0x4bea00:
      retire(0x6df1c0);
      return 0;
    case 0x4beaa0:
      retire(0x6a5138);
      return 0;
    case 0x4a58c0:
      chain(a[0]);
      return 0;
    case 0x4a58e0:
      return advance(self, a[0], a[1]);
    case 0x4a59e0:
      byte(self, a);
      return 0;
    case 0x4a5ae0:
      return load(self, a[0], a[1]);
    case 0x4a5ce0:
    case 0x4a6190:
    case 0x4a6630:
      return layout(entry, self, a);
    case 0x4a6970:
      return r(r(self) + 20) + r(r(self) + 24);
    case 0x581ec0:
      glyph(self, second, a[0], a[1], a[2], a[3]);
      return 0;
    default:
      throw std::invalid_argument("Unknown font provider");
    }
  }
};
template <class M> class FontComposition {
  M &m;

public:
  explicit FontComposition(M &memory) : m(memory) {}
  std::optional<U> tryRun(U e, U p, std::initializer_list<U> a = {},
                          U second = 0) {
    switch (e) {
    case 0x4be870:
    case 0x4be8c0:
    case 0x4be910:
    case 0x4be960:
    case 0x4be9b0:
    case 0x4bea00:
    case 0x4bea50:
    case 0x4beaa0:
    case 0x4a58c0:
    case 0x4a58e0:
    case 0x4a59e0:
    case 0x4a5ae0:
    case 0x4a5ce0:
    case 0x4a6190:
    case 0x4a6630:
    case 0x4a6970:
    case 0x581ec0:
      return FontProviders<M>(m).run(e, p, a, second);
    default:
      return ProfilePaletteComposition<M>(m).tryRun(e, p, a, second);
    }
  }
};
} // namespace mnm::reconstruction::actor_services
