#pragma once
#include "actor_frontier_providers.hpp"
#include "palette_effects.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>

namespace mnm::reconstruction::actor_services {
// Build-specific value-address providers, including the caller-visible
// ownership and configuration side effects absent from a table-value-only
// reconstruction.
template <class M> class ProfilePaletteProviders {
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  U call(U e, U p, std::initializer_list<U> a = {}) {
    return m.invoke(e, p, a);
  }
  static int i(U v) { return v <= 0x7fffffff ? int(v) : -1 - int(~v); }
  U sequence(U type, U bank) {
    return call(0x464b30, 0x6dbf00 + type * 0x114, {bank});
  }
  U diagnostic(U type, U bank, U message) {
    U buffer = m.temporary(256);
    call(0x59c1fd, 0, {buffer, 0x5e9ab4, type, bank, message});
    call(0x5c520c, 0, {0, buffer, 0x5e9a98, 0x42030});
    return 0;
  }
  U walking(U type, U bank) {
    if (r(sequence(type, bank)) == 6)
      return 1;
    for (U facing = 0; facing < 8; ++facing) {
      U start = sequence(type, bank + facing), record = start, count = 0;
      while (count < 12) {
        U tag = r(record);
        if (tag == 6)
          return diagnostic(type, bank, 0x5e9ba4);
        if (tag != 0 && tag != 5)
          return diagnostic(type, bank, 0x5e9b68);
        if (tag == 0)
          ++count;
        record += 44;
      }
      if (r(record) != 5)
        return diagnostic(type, bank, 0x5e9ae8);
      if (r(record + 4) == 5)
        record += 44;
      if (r(record) != 5 || r(record + 4) != 2)
        return diagnostic(type, bank, 0x5e9ae8);
      record += 44;
      if (r(record) != 0 || r(record + 4) != r(start + 4))
        return diagnostic(type, bank, 0x5e9b18);
      if (r(record + 44) != 6)
        return diagnostic(type, bank, 0x5e9b4c);
    }
    return 1;
  }
  U groups(U type, U bank) {
    U root = sequence(type, bank),
      peers[] = {sequence(type, bank + 2), sequence(type, bank + 4),
                 sequence(type, bank + 6)};
    for (;; root += 44) {
      U tag = r(root);
      if (tag <= 6) {
        for (U k = 0; k < 3; ++k) {
          if (r(peers[k]) != tag)
            return diagnostic(type, bank + 2 + k * 2, 0x5e9bf0);
          if (tag >= 1 && tag <= 5 && r(peers[k] + 4) != r(root + 4))
            return diagnostic(type, bank + 2 + k * 2, 0x5e9bc0);
        }
        if (tag == 6)
          return 1;
      }
      for (auto &p : peers)
        p += 44;
    }
  }
  static U ftol(long double v) {
    if (!std::isfinite(v) || v < -9223372036854775808.0L ||
        v >= 9223372036854775808.0L)
      return 0;
    return U(static_cast<std::int64_t>(v));
  }
  static U magnitude(U v) { return i(v) < 0 ? 0 - v : v; }
  static U metric(U x, U y, U z) {
    std::array<U, 3> v{magnitude(x * 2), magnitude(y * 2), magnitude(z)};
    std::sort(v.begin(), v.end(), [](U a, U b) { return i(a) < i(b); });
    return v[2] * 2 + v[1] + v[0];
  }
  U base(U p, U category, U x, U y, U z) {
    if (category >= 1 && category <= 3 && !r(p + 60))
      return r(0x6c007d);
    U parity = 0;
    if (!x && !y) {
      x = z;
      z = 0;
    } else {
      if (i(x) < -1 || i(x) > 1 || i(y) < -1 || i(y) > 1)
        throw std::invalid_argument(
            "Profile direction outside recovered table");
      constexpr U dir[] = {7, 0, 1, 6, 0, 2, 5, 4, 3};
      parity = dir[i(x) + 3 * i(y) + 4] & 1;
    }
    U sum = 0;
    if (category <= 4) {
      U bank = (category == 1 || category == 2 ? 2 : 0) + parity;
      for (U k = 0; k < 12; ++k)
        sum += r(p + 0xd8 + bank * 48 + k * 4);
    }
    U scaled = ((metric(x, y, z) * sum) >> 2) * 12;
    return scaled / 12;
  }
  void accelerate(U p, const U *a) {
    U divisor = a[0], category = a[1], x = a[2], y = a[3], z = a[4],
      prior = a[6], targetAt = a[7];
    if (!divisor)
      throw std::domain_error("Original profile scalar zero divisor");
    U target = (a[5] * 4) / divisor;
    w(targetAt, target);
    if (category != 4) {
      if (z == 1)
        target = U(i(target * 2) / 3);
      else if (z == ~0u)
        target = U(i(target * 3) / 2);
      w(targetAt, target);
    }
    U acceleration = r(p + 16);
    if (i(target) < i(r(prior)))
      acceleration = 0 - acceleration;
    if (category != 4 && (z == 1 || z == ~0u)) {
      U bits = r(0x5e15f0);
      float slope;
      std::memcpy(&slope, &bits, 4);
      long double adjustment =
          static_cast<long double>(slope) * 76.8000030517578125L;
      acceleration =
          ftol(i(acceleration) + (z == 1 ? -adjustment : adjustment));
    }
    U old = r(prior), distance = (metric(x, y, z) * 9 * 256) >> 2;
    int discriminant = i(old * old + distance * acceleration * 4);
    long double value;
    if (i(acceleration) >= 0) {
      value =
          (std::sqrt(static_cast<long double>(discriminant)) + i(old)) * 0.5L;
      if (value >= i(target))
        value = i(target);
    } else {
      value = (std::sqrt(static_cast<long double>(std::max(discriminant, 0))) +
               i(old)) *
              0.5L;
      if (value <= i(target))
        value = i(target);
    }
    w(prior, ftol(value));
  }
  void maximum(U p) {
    U values = m.temporary(8);
    w(values, 0);
    w(values + 4, 0);
    w(p + 0x589, 0);
    for (U category = 0; category < 5; ++category) {
      if (category >= 1 && category <= 3 && !r(p + 60))
        continue;
      if (category == 4 && r(p + 68) != 2)
        continue;
      for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
          for (int z = -1; z <= 1; ++z) {
            if (!x && !y)
              continue;
            U raw = call(0x505840, p, {category, U(x), U(y), U(z)});
            call(0x505920, p,
                 {4, category, U(x), U(y), U(z), raw, values, values + 4});
            if (i(r(p + 0x589)) < i(r(values + 4)))
              w(p + 0x589, r(values + 4));
          }
    }
  }
  std::string text(U p) {
    std::string s;
    while (m.get8(p)) {
      s.push_back(char(m.get8(p++)));
      if (s.size() > 255)
        throw std::invalid_argument("Palette profile string extent");
    }
    return s;
  }
  void text(U p, const std::string &s) {
    for (U k = 0; k < s.size(); ++k)
      m.set8(p + k, static_cast<unsigned char>(s[k]));
    m.set8(p + U(s.size()), 0);
  }
  double real(U p) {
    std::uint64_t v = r(p) | (std::uint64_t(r(p + 4)) << 32);
    double result;
    std::memcpy(&result, &v, 8);
    return result;
  }
  void real(U p, double value) {
    std::uint64_t bits;
    std::memcpy(&bits, &value, 8);
    w(p, U(bits));
    w(p + 4, U(bits >> 32));
  }
  PaletteEffectProfile profile(U mode) {
    PaletteEffectProfile out;
    if (mode != 4 && mode != 5 && mode != 7)
      return out;
    U directory = mode == 4   ? 0x6f56ac
                  : mode == 5 ? 0x6f5aac
                              : 0x6f5eb0,
      path = mode == 4   ? 0x6f58ac
             : mode == 5 ? 0x6f5cac
                         : 0x6f59ac,
      buffer = mode == 4   ? 0x6f5bac
               : mode == 5 ? 0x6f57ac
                           : 0x6f5fb0;
    call(0x5c50bc, 0, {256, directory});
    std::string value = text(directory);
    if (!value.empty() && value.back() != '/' && value.back() != '\\')
      value += '\\';
    value += "cfg\\tables.cfg";
    text(path, value);
    U section = mode == 4 ? 0x5f1934 : mode == 5 ? 0x5f1974 : 0x5f1984;
    auto field = [&](U key, double &result) {
      if (call(0x5c507c, 0, {section, key, 0x5faf28, buffer, 256, path})) {
        std::string source = text(buffer);
        result = std::strtod(source.c_str(), nullptr);
      }
    };
    if (mode == 4) {
      field(0x5f1944, out.stoneReducer);
      field(0x5f1928, out.stoneSpecalValue);
      field(0x5f1920, out.stoneDarkest);
    } else if (mode == 5) {
      field(0x5f1944, out.metalReducer);
      field(0x5f1928, out.metalSpecalValue);
      field(0x5f1920, out.metalDarkest);
      field(0x5f1968, out.metalBlueTint);
      field(0x5f195c, out.metalContrast);
    } else
      field(0x5f1944, out.redTintReducer);
    return out;
  }
  U palette(U mode, U count, U slot, U source) {
    PaletteEffectProfile effects = profile(mode);
    if (mode != 0 && mode != 8 &&
        (count < 2 || count > 256 || (count & (count - 1))))
      return 0;
    if (count < 2 || count > 256 || (mode == 8 && (count & 1)) ||
        (mode == 0 && (count & 1) && count != 3))
      throw std::invalid_argument("Original palette underallocation or "
                                  "undefined count is outside owned domain");
    U bytes = mode < 8 ? count * 512 - 496 : (count >> 1) * 1024 + 16;
    U node = call(0x597880, 0, {bytes});
    w(slot, node);
    if (!node)
      return 0;
    U shift = 0;
    if (!(count & (count - 1)))
      for (U n = count; n < 256; n *= 2)
        ++shift;
    w(node + 4, shift);
    w(node + 8, (count - 1) / 2);
    w(node, 0);
    U settings = mode == 1 || mode == 9 ? 0x5f18f8 : 0x5f18d0;
    PaletteShadingConfig lighting;
    lighting.count = count;
    lighting.intensityLevel = i(r(settings));
    lighting.saturationLevel = i(r(settings + 4));
    lighting.intensityPower = real(settings + 8);
    lighting.saturationPower = real(settings + 16);
    real(settings + 24,
         std::pow(lighting.intensityLevel, lighting.intensityPower));
    real(settings + 32,
         std::pow(lighting.saturationLevel, lighting.saturationPower));
    PaletteRgb rgb{};
    for (U n = 0; n < 256; ++n)
      for (U c = 0; c < 3; ++c)
        rgb[n][c] = static_cast<unsigned char>(m.get8(source + n * 3 + c));
    auto result =
        buildPaletteEffect(rgb, mode, lighting, effects, r(0x6e1f88) != 0);
    if (mode == 5)
      for (U n = 0; n < 256; ++n)
        for (U c = 0; c < 3; ++c)
          m.set8(source + n * 3 + c, result.source[n][c]);
    U at = node + 12, width = mode < 8 ? 2 : 4;
    for (const auto &table : result.palette.tables)
      for (auto word : table) {
        if (width == 2)
          m.set16(at, word);
        else
          w(at, word);
        at += width;
      }
    return 1;
  }

public:
  explicit ProfilePaletteProviders(M &memory) : m(memory) {}
  U run(U entry, U self, std::initializer_list<U> args = {}, U second = 0) {
    U a[8]{};
    if (args.size() > 8)
      throw std::invalid_argument("Profile/palette argument extent");
    std::copy(args.begin(), args.end(), a);
    switch (entry) {
    case 0x504cc0:
      return walking(self, second);
    case 0x504ec0:
      return groups(self, second);
    case 0x505160:
      maximum(self);
      return 0;
    case 0x505840:
      return base(self, a[0], a[1], a[2], a[3]);
    case 0x505920:
      accelerate(self, a);
      return 0;
    case 0x582440:
      return palette(0, self, second, a[0]);
    case 0x582a80:
      return palette(1, self, second, a[0]);
    case 0x583330:
      return palette(2, self, second, a[0]);
    case 0x583bb0:
      return palette(3, self, second, a[0]);
    case 0x586660:
      return palette(4, self, second, a[0]);
    case 0x5870d0:
      return palette(5, self, second, a[0]);
    case 0x585d80:
      return palette(6, self, second, a[0]);
    case 0x587e70:
      return palette(7, self, second, a[0]);
    case 0x588730:
      return palette(8, self, second, a[0]);
    case 0x588bf0:
      return palette(9, self, second, a[0]);
    default:
      throw std::invalid_argument("Unknown profile/palette provider");
    }
  }
};
template <class M> class ProfilePaletteComposition {
  M &m;

public:
  explicit ProfilePaletteComposition(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U self, std::initializer_list<U> args = {},
                          U second = 0) {
    switch (entry) {
    case 0x504cc0:
    case 0x504ec0:
    case 0x505160:
    case 0x505840:
    case 0x505920:
    case 0x582440:
    case 0x582a80:
    case 0x583330:
    case 0x583bb0:
    case 0x586660:
    case 0x5870d0:
    case 0x585d80:
    case 0x587e70:
    case 0x588730:
    case 0x588bf0:
      return ProfilePaletteProviders<M>(m).run(entry, self, args, second);
    default:
      return FrontierComposition<M>(m).tryRun(entry, self, args, second);
    }
  }
};
} // namespace mnm::reconstruction::actor_services
