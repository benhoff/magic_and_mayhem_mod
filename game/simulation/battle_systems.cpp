#include "battle_systems.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
namespace mnm::game {
namespace {
constexpr std::array<unsigned char, 8> tag{'M', 'N', 'M', 'B',
                                           'A', 'T', '0', '1'};
constexpr unsigned limit = 4 * 1024 * 1024;
void require(bool value) {
  if (!value)
    throw std::invalid_argument("invalid native battle sections");
}
void validate(const BattleSystems &s) {
  require(s.combat.size() >= 8 && s.combat.size() <= 8192 &&
          !isBattleSystems(s.combat) && s.effects.data.size() <= limit &&
          s.effects.entities.size() <= 32);
  require(s.effects.identity ||
          (s.effects.data.empty() && s.effects.entities.empty()));
  for (unsigned i = 0; i < s.effects.entities.size(); ++i) {
    require(s.effects.entities[i].generation != 0);
    for (unsigned j = 0; j < i; ++j)
      require(!(s.effects.entities[i] == s.effects.entities[j]));
  }
}
} // namespace
bool isBattleSystems(const Bytes &b) {
  return b.size() >= tag.size() &&
         std::equal(tag.begin(), tag.end(), b.begin());
}
Bytes encodeBattleSystems(const BattleSystems &s) {
  validate(s);
  if (!s.effects.identity)
    return s.combat;
  Bytes b(tag.begin(), tag.end());
  const auto word = [&](unsigned n) {
    for (unsigned i = 0; i < 4; ++i)
      b.push_back(std::uint8_t(n >> (8 * i)));
  };
  const auto blob = [&](const Bytes &v) {
    word(unsigned(v.size()));
    b.insert(b.end(), v.begin(), v.end());
  };
  word(unsigned(s.effects.identity));
  word(unsigned(s.effects.identity >> 32));
  blob(s.combat);
  word(unsigned(s.effects.entities.size()));
  for (auto h : s.effects.entities) {
    word(h.slot);
    word(h.generation);
  }
  blob(s.effects.data);
  return b;
}
BattleSystems decodeBattleSystems(const Bytes &b) {
  if (!isBattleSystems(b)) {
    BattleSystems s{b, {}};
    validate(s);
    return s;
  }
  require(b.size() <= limit + 16384);
  std::size_t at = 8;
  const auto word = [&]() {
    require(at <= b.size() && b.size() - at >= 4);
    unsigned n = 0;
    for (unsigned i = 0; i < 4; ++i)
      n |= unsigned(b[at++]) << (8 * i);
    return n;
  };
  const auto blob = [&](unsigned maximum) {
    const auto n = word();
    require(n <= maximum && n <= b.size() - at);
    Bytes v(b.begin() + at, b.begin() + at + n);
    at += n;
    return v;
  };
  BattleSystems s;
  const auto low = word(), high = word();
  s.effects.identity = std::uint64_t(high) << 32 | low;
  require(s.effects.identity != 0);
  s.combat = blob(8192);
  const auto count = word();
  require(count <= 32 && std::uint64_t(count) * 8 <= b.size() - at);
  for (unsigned i = 0; i < count; ++i)
    s.effects.entities.push_back({word(), word()});
  s.effects.data = blob(limit);
  require(at == b.size());
  validate(s);
  return s;
}
} // namespace mnm::game
