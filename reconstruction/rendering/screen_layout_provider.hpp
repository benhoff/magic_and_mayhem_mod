#pragma once
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace mnm::reconstruction {
// 558580, No-CD 40209ca7. Profile API, localization and font realization are
// named platform/service boundaries. Unlike the stricter native config loader,
// this retains atoi prefixes, partial token handling and profile writeback.
template <class M> class ScreenLayoutProvider {
  using U = std::uint32_t;
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  U call(U f, U p = 0, std::initializer_list<U> a = {}) {
    return m.invoke(f, p, a);
  }
  U length(U p) {
    U n = 0;
    while (m.get8(p + n))
      ++n;
    return n;
  }
  std::string text(U p) {
    std::string s;
    while (m.get8(p))
      s += char(m.get8(p++));
    return s;
  }
  void text(U p, const std::string &s) {
    for (U i = 0; i < s.size(); ++i)
      m.set8(p + i, static_cast<unsigned char>(s[i]));
    m.set8(p + U(s.size()), 0);
  }
  void copy(U dst, U src, U n) {
    while (n >= 4) {
      w(dst, r(src));
      dst += 4;
      src += 4;
      n -= 4;
    }
    while (n--)
      m.set8(dst++, m.get8(src++));
  }
  void trim(char delimiter) {
    for (U p = 0x6f2d40; m.get8(p); ++p)
      if (m.get8(p) == U(delimiter)) {
        m.set8(p, 0);
        break;
      }
  }
  U atoi(const std::string &s) {
    U at = 0;
    while (at < s.size() && (s[at] == ' ' || s[at] == '\t' || s[at] == '\r' ||
                             s[at] == '\n' || s[at] == '\f' || s[at] == '\v'))
      ++at;
    bool negative = false;
    if (at < s.size() && (s[at] == '-' || s[at] == '+'))
      negative = s[at++] == '-';
    U value = 0;
    while (at < s.size() && s[at] >= '0' && s[at] <= '9')
      value = value * 10 + U(s[at++] - '0');
    return negative ? 0 - value : value;
  }
  static std::string upper(std::string s) {
    for (auto &c : s)
      if (c >= 'a' && c <= 'z')
        c -= 32;
    return s;
  }
  bool property(U key) {
    return call(0x5c507c, 0,
                {0x6f3044, key, 0x5faf28, 0x6f2d40, 256, 0x6f326c}) != 0;
  }
  struct Tokens {
    std::string source;
    std::size_t next = 0;
    std::optional<std::string> take() {
      const auto first = source.find_first_not_of(',', next);
      if (first == std::string::npos)
        return {};
      const auto end = source.find(',', first);
      next = end == std::string::npos ? source.size() : end + 1;
      return source.substr(first, end == std::string::npos ? end : end - first);
    }
  };
  U required(Tokens &tokens) {
    auto value = tokens.take();
    if (!value) {
      (void)m.get8(0);
      return 0;
    }
    return atoi(*value);
  }
  static int signedValue(U v) {
    return v <= 0x7fffffff ? int(v) : -1 - int(~v);
  }

public:
  explicit ScreenLayoutProvider(M &memory) : m(memory) {}
  std::optional<U> tryRun(U entry, U filename, U destination,
                          std::initializer_list<U> args) {
    if (entry != 0x558580)
      return std::nullopt;
    if (args.size() != 2)
      throw std::invalid_argument("Screen layout argument extent");
    U a[2]{};
    std::copy(args.begin(), args.end(), a);
    call(0x5c50bc, 0, {256, 0x6f336c});
    copy(0x6f326c, 0x6f336c, length(0x6f336c) + 1);
    auto directory = text(0x6f326c);
    if (!directory.empty() && directory.back() != '/' &&
        directory.back() != '\\')
      directory += '\\';
    text(0x6f326c, directory + text(filename));
    constexpr U sections[] = {0x5ec200, 0x5ec1f4, 0x5ec1e8, 0x5ec1d8,
                              0x5ec1c8, 0x5ec1bc, 0x5ec1b4, 0x5ec1a8,
                              0x5ec19c, 0x5ec18c, 0x5ec180};
    if (a[0] < 11)
      copy(0x6f3044, sections[a[0]], length(sections[a[0]]) + 1);
    text(0x6f2d40, std::to_string(signedValue(a[1])));
    text(0x6f3044, text(0x6f3044) + text(0x6f2d40));
    w(destination, 0);
    if (property(0x5ec178)) {
      trim(';');
      trim(' ');
      auto font = upper(text(0x6f2d40));
      U p = font == "LARGE" ? 0x6b0400 : font == "TINY" ? 0x6df1c0 : 0x656680;
      w(destination, p);
      call(0x4a6970, p);
    }
    if (property(0x5ec160)) {
      Tokens values{text(0x6f2d40)};
      for (U i = 0; i < 4; ++i)
        w(destination + 0x14 + 4 * i, required(values));
    }
    if (property(0x5ec158)) {
      Tokens values{text(0x6f2d40)};
      for (U i = 0; i < 4; ++i)
        w(destination + 4 + 4 * i, required(values));
    } else {
      std::string value;
      for (U i = 0; i < 4; ++i) {
        const U v = U(std::int64_t(static_cast<long double>(signedValue(
                                       r(destination + 0x14 + 4 * i))) *
                                   double(.8)));
        w(destination + 4 + 4 * i, v);
        if (i)
          value += ',';
        value += std::to_string(signedValue(v));
      }
      text(0x6f2d40, value);
      call(0x5c50b0, 0, {0x6f3044, 0x5ec158, 0x6f2d40, 0x6f326c});
    }
    if (property(0x5ec144)) {
      trim(';');
      if (m.get8(0x6f2d40) == '"') {
        copy(0x6f2d40, 0x6f2d41, length(0x6f2d40) + 1);
        trim('"');
      }
      U source = 0x6f2d40;
      if (m.get8(source) >= '0' && m.get8(source) <= '9')
        source = m.fastcall(0x58a6b0, atoi(text(source)), 0x5da4f0, {});
      copy(destination + 0x24, source, length(source) + 1);
    }
    if (property(0x5ec134)) {
      Tokens values{text(0x6f2d40)};
      for (U i = 0; i < 3; ++i)
        if (auto token = values.take())
          w(destination + 0x124 + 4 * i, atoi(*token));
    }
    if (property(0x5ec128))
      w(destination + 0x130, atoi(text(0x6f2d40)));
    if (property(0x5ec11c))
      w(destination + 0x134, atoi(text(0x6f2d40)));
    if (property(0x5ec110)) {
      trim(';');
      trim(' ');
      const auto flags = upper(text(0x6f2d40));
      call(0x4a5a90, 0x656680,
           {destination + 0x138, flags == "LEFT"    ? 1u
                                 : flags == "RIGHT" ? 2u
                                                    : 4u});
    }
    if (property(0x5ec108)) {
      Tokens values{text(0x6f2d40)};
      const U count = required(values), out = call(0x597850, 0, {count * 12});
      w(destination + 0x139, out);
      if (out && signedValue(count) > 0)
        for (U i = 0; i < count; ++i)
          for (U channel = 0; channel < 3; ++channel)
            w(r(destination + 0x139) + i * 12 + channel * 4, required(values));
    }
    if (property(0x5ec100))
      w(destination + 0x13d, atoi(text(0x6f2d40)));
    return 1;
  }
};
} // namespace mnm::reconstruction
