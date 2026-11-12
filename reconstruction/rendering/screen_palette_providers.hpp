#pragma once
#include "palette_shading.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <stdexcept>

namespace mnm::reconstruction {
// 584430/589280 mutate caller-owned palette nodes. The three tint coefficients
// are doubles supplied by World; these providers neither allocate nor relink.
template <class M> class ScreenPaletteProviders {
  using U = std::uint32_t;
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  static int signedValue(U v) {
    return v <= 0x7fffffff ? int(v) : -1 - int(~v);
  }
  double real(U p) {
    std::uint64_t bits = r(p) | (std::uint64_t(r(p + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, 8);
    return value;
  }
  void real(U p, double value) {
    std::uint64_t bits;
    std::memcpy(&bits, &value, 8);
    w(p, U(bits));
    w(p + 4, U(bits >> 32));
  }
  static int integer(long double v) {
    if (!std::isfinite(v) || v < -2147483648.L || v >= 2147483648.L)
      return -2147483647 - 1;
    return int(v);
  }
  static long double power(double base, double exponent) {
#if defined(__i386__) || defined(__x86_64__)
    long double result;
    asm volatile("fldl %2; fldl %1; fyl2x; fld %%st(0); frndint; fxch; "
                 "fsub %%st(1), %%st(0); f2xm1; fld1; faddp; fscale; fstp "
                 "%%st(1); fstpt %0"
                 : "=m"(result)
                 : "m"(base), "m"(exponent)
                 : "st", "st(1)", "st(2)");
    return result;
#else
    return std::pow(static_cast<long double>(base),
                    static_cast<long double>(exponent));
#endif
  }

public:
  explicit ScreenPaletteProviders(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U count, U node,
                          std::initializer_list<U> args) {
    if (entry != 0x584430 && entry != 0x589280)
      return std::nullopt;
    if (count < 2 || count > 256 || (count & (count - 1)) || !node)
      return 0;
    if (args.size() != 7)
      throw std::invalid_argument("Screen palette argument extent");
    U a[7];
    std::copy(args.begin(), args.end(), a);
    double tint[3];
    for (U i = 0; i < 3; ++i) {
      std::uint64_t bits = a[1 + 2 * i] | (std::uint64_t(a[2 + 2 * i]) << 32);
      std::memcpy(tint + i, &bits, 8);
      if (!std::isfinite(tint[i]) || std::abs(tint[i]) > 1000)
        throw std::invalid_argument("Screen palette finite coefficient policy");
    }
    U shift = 0;
    for (U n = count; n < 256; n *= 2)
      ++shift;
    const U half = (count - 1) / 2;
    w(node + 4, shift);
    w(node + 8, half);
    const double ip = real(0x5f18d8), sp = real(0x5f1908);
    const double intensity = double(power(signedValue(r(0x5f18f8)), ip)),
                 saturation = double(power(signedValue(r(0x5f18fc)), sp));
    real(0x5f1910, intensity);
    real(0x5f1918, saturation);
    if (!std::isfinite(intensity) || !std::isfinite(saturation) ||
        intensity <= 0 || saturation <= 0)
      throw std::invalid_argument("Screen palette lighting policy");
    const bool wide = entry == 0x589280, rgb555 = r(0x6e1f88) != 0;
    const U tables = wide ? half + 1 : half * 2 + 1;
    for (U table = 0; table < tables; ++table) {
      const long double factor =
          table < half ? power((127 * half - 127 * table) / half, ip) : 0.L;
      for (U color = 0; color < 256; ++color) {
        std::array<int, 3> v;
        for (U c = 0; c < 3; ++c)
          v[c] = m.get8(a[0] + color * 3 + c);
        if (table < half) {
          const long double red = v[0], green = v[1], blue = v[2];
          const double luminance =
              double(wide ? (blue * double(.11) + green * double(.59)) +
                                red * double(.3)
                          : (red * double(.3) + blue * double(.11)) +
                                green * double(.59));
          for (auto &ch : v)
            ch = ch ? integer(
                          (intensity /
                           (static_cast<long double>(intensity) + factor)) *
                          ((static_cast<long double>(luminance) - ch) *
                               (factor / (static_cast<long double>(saturation) +
                                          factor)) +
                           ch))
                    : 0;
        } else if (table > half)
          for (auto &ch : v)
            ch += ((255 - ch) * (table - half)) / half;
        const auto before = v;
        const long double sum = v[0] + v[1] + v[2], green = sum * tint[1];
        const double blue = double(sum * tint[2]);
        if (before[0])
          v[0] = integer((sum * tint[0]) *
                             (static_cast<long double>(before[0]) / sum) +
                         before[0]);
        if (before[1])
          v[1] = integer((static_cast<long double>(before[1]) / sum) * green +
                         before[1]);
        if (before[0] || before[1])
          v[2] = integer((static_cast<long double>(before[2]) / sum) * blue +
                         before[2]);
        for (auto &ch : v)
          ch = std::clamp(ch, 0, 255);
        const U value =
            rgb555 ? ((v[0] & 248) << 7) | ((v[1] & 248) << 2) | (v[2] >> 3)
                   : ((v[0] & 248) << 8) | ((v[1] & 252) << 3) | (v[2] >> 3);
        const U at = node + 12 + (table * 256 + color) * (wide ? 4 : 2);
        if (wide)
          w(at, value);
        else
          m.set16(at, value);
      }
    }
    return 1;
  }
};
} // namespace mnm::reconstruction
