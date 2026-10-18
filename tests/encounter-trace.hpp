#pragma once
#include "../apps/native-encounter/encounter.hpp"
#include <fstream>
#include <stdexcept>
namespace mnm::encounter::test {
class Trace {
  std::ifstream in;
  unsigned remaining = 0;
  unsigned word() {
    unsigned v = 0;
    for (unsigned i = 0; i < 4; ++i) {
      auto b = in.get();
      if (b < 0)
        throw std::invalid_argument("trace extent");
      v |= unsigned(b) << (8 * i);
    }
    return v;
  }
  game::Bytes blob(unsigned limit) {
    auto n = word();
    if (n > limit)
      throw std::invalid_argument("trace blob limit");
    game::Bytes b(n);
    in.read(reinterpret_cast<char *>(b.data()), std::streamsize(n));
    if (!in)
      throw std::invalid_argument("trace read");
    return b;
  }

public:
  Profile profile;
  explicit Trace(const std::filesystem::path &path)
      : in(path, std::ios::binary) {
    if (std::filesystem::file_size(path) > 256 * 1024 * 1024)
      throw std::invalid_argument("trace file limit");
    char magic[8];
    in.read(magic, 8);
    if (std::string(magic, 8) != "MNMETR01")
      throw std::invalid_argument("trace version");
    profile = decodeInputs(blob(20 * 1024 * 1024));
    remaining = word();
    if (remaining > 1024)
      throw std::invalid_argument("trace frame count");
  }
  bool next(cs::Continuation &before, cs::Continuation &after) {
    if (!remaining) {
      if (in.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("trace trailing data");
      return false;
    }
    before = cs::decode(blob(1024 * 1024));
    after = cs::decode(blob(1024 * 1024));
    --remaining;
    if (before.world.creatures.size() != 2 ||
        after.world.creatures.size() != 2 ||
        after.native.tick != before.native.tick + 1)
      throw std::invalid_argument("trace frame shape/order");
    return true;
  }
};
} // namespace mnm::encounter::test
