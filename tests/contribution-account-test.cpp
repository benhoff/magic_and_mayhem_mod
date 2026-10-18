#include "../reconstruction/creature-ai/contribution_account.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_ACCOUNT_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  ContributionProfiles profiles;
  unsigned type = 0, target = 0;
  int level = 0, quantity = 0;
  float amount = 0, delta = 0, extra = 0;
};
Fixture generateAccount(unsigned n) {
  Fixture f;
  f.type = n % 28;
  f.target = n / 28 % 28;
  constexpr unsigned kinds[] = {0, 1, 8, 20, 23, 24, 25, 26};
  constexpr unsigned words[] = {0,          1,          2,          3,
                                0x7fffffff, 0x80000000, 0xffffff00, 0xffffffff};
  for (unsigned i = 0; i < 28; ++i) {
    auto &p = f.profiles[i];
    p.kind = kinds[(n / 13 + i) % 8];
    p.capacity = 1 + (n / 17 + i) % 1000;
    p.flag58 = (n + i) % 3;
    for (unsigned j = 0; j < 28; ++j) {
      p.first[j] = words[(n / 7 + i + j) % 8];
      p.second[j] = words[(n / 11 + i + j) % 8];
    }
  }
  f.level = int(n % 127);
  f.quantity = int(n / 7 % 9) - 4;
  if (f.level - f.quantity + 1 == 0)
    ++f.level;
  f.amount = float(int(n / 3 % 150) - 20) + float(n % 4) / 4;
  f.delta = float(int(n / 5 % 9) - 4) / 4;
  f.extra = float(int(n / 19 % 5) - 2) / 8;
  return f;
}
unsigned refusals() {
  unsigned count = 0;
  for (unsigned op = 0; op < 5; ++op) {
    auto f = generateAccount(1);
    f.profiles[f.type].kind = 1;
    bool refused = false;
    if (op == 0)
      f.type = 28;
    if (op == 1)
      f.profiles[f.type].capacity = 0;
    if (op == 2)
      f.amount = -30;
    if (op == 3)
      f.extra = std::nanf("");
    if (op == 4) {
      f.profiles[f.type].kind = 0;
      f.level = -1;
    }
    try {
      contributionAccount(f.profiles, f.type, f.level, f.quantity, f.amount,
                          f.delta, f.extra);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("account refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_ACCOUNT_REFERENCE
std::vector<std::pair<unsigned, Bytes>> bodies;
void put(void *p, unsigned off, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &n, 4);
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x416d80, 0x416ea0},
                 {0x505bf0, 0x505c49},
                 {0x505c50, 0x505c95},
                 {0x59bee0, 0x59bf07}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  // No redirects: the whole original account executes both whole profile
  // accessors and its actual x87 truncate helper against owned profile data.
}
void compare(const Fixture &f, unsigned op, unsigned result) {
  Bytes profiles(28 * 0x5c9, 0xa7);
  for (unsigned i = 0; i < 28; ++i) {
    auto *p = profiles.data() + i * 0x5c9;
    const auto &v = f.profiles[i];
    put(p, 0, v.kind);
    put(p, 0x18, v.capacity);
    put(p, 0x58, v.flag58);
    for (unsigned j = 0; j < 28; ++j) {
      put(p, 0x280 + j * 4, v.first[j]);
      put(p, 0x1a0 + j * 4, v.second[j]);
    }
  }
  std::memcpy(reinterpret_cast<void *>(0x6a5f80), profiles.data(),
              profiles.size());
  unsigned short cwBefore, cwAfter, statusBefore, statusAfter;
  asm volatile("fnstcw %0; fnstsw %1" : "=m"(cwBefore), "=m"(statusBefore));
  unsigned got;
  if (op == 0) {
    using F = unsigned(__attribute__((fastcall)) *)(unsigned, int, int, float,
                                                    float, float);
    got = reinterpret_cast<F>(0x416d80)(f.type, f.level, f.quantity, f.amount,
                                        f.delta, f.extra);
  } else {
    using F = unsigned(__attribute__((thiscall)) *)(void *, unsigned);
    got = reinterpret_cast<F>(op == 1 ? 0x505bf0 : 0x505c50)(
        reinterpret_cast<void *>(0x6a5f80 + f.type * 0x5c9), f.target);
  }
  asm volatile("fnstcw %0; fnstsw %1" : "=m"(cwAfter), "=m"(statusAfter));
  if (got != result)
    throw std::runtime_error("account return op=" + std::to_string(op) +
                             " type=" + std::to_string(f.type) +
                             " got=" + std::to_string(got) +
                             " expected=" + std::to_string(result));
  if (cwBefore != cwAfter || ((statusBefore ^ statusAfter) & 0x3800))
    throw std::runtime_error("account FPU ownership");
  if (std::memcmp(reinterpret_cast<void *>(0x6a5f80), profiles.data(),
                  profiles.size()))
    throw std::runtime_error("account profile guard");
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("account body guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_ACCOUNT_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  // Match the isolated original execution environment: x87 extended precision,
  // round to nearest; this is a fixture contract, not a live Windows
  // assumption.
  unsigned short before, cw;
  asm volatile("fnstcw %0" : "=m"(before));
  cw = (before & ~0x0f00u) | 0x0300u;
  asm volatile("fldcw %0" ::"m"(cw));
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 3; ++op) {
      auto f = generateAccount(n);
      unsigned result =
          op == 0 ? contributionAccount(f.profiles, f.type, f.level, f.quantity,
                                        f.amount, f.delta, f.extra)
          : op == 1 ? contributionProfileFirst(f.profiles, f.type, f.target)
                    : contributionProfileSecond(f.profiles, f.type, f.target);
      digest = (digest ^ result) * 1099511628211ull;
#ifdef MNM_ACCOUNT_REFERENCE
      try {
        compare(f, op, result);
      } catch (const std::exception &e) {
        throw std::runtime_error("case=" + std::to_string(n) + " " + e.what());
      }
#endif
    }
  unsigned refused = refusals();
  asm volatile("fldcw %0" ::"m"(before));
  std::cout << "{\"success\":true,\"cases\":24576,\"functions\":4,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"helper_redirects\":0}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
