#include "../reconstruction/rendering/original_random_streams.hpp"
#include <cstring>
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
static U clockValue, clockCalls;
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
static const U tokens[]{0x5fceb0, 0x6e1ac8, 0x6a29d8, 0x6574c8,
                        0x6e8968, 0x6f3db0, 0x100,    0x200};
static const U routes[]{0x4bec0a, 0x4e4484, 0x55780e};
#ifdef MNM_ORIGINAL_REFERENCE
static U __attribute__((force_align_arg_pointer)) ownedClock() {
  ++clockCalls;
  return clockValue;
}
static void *callerSlice(U start, U end, U call) {
  const U size = end - start;
  const auto *source = reinterpret_cast<const unsigned char *>(start);
  if (call < start || call + 5 != end || source[call - start] != 0xe8)
    throw std::runtime_error("Original reseed slice boundary");
  U relative;
  std::memcpy(&relative, source + call - start + 1, 4);
  if (call + 5 + relative != 0x54e170)
    throw std::runtime_error("Original reseed target");
  auto *body = static_cast<unsigned char *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (body == MAP_FAILED)
    throw std::runtime_error("Reseed slice allocation");
  std::memcpy(body, source, size);
  relative =
      0x54e170 - U(reinterpret_cast<std::uintptr_t>(body) + call - start + 5);
  std::memcpy(body + call - start + 1, &relative, 4);
  body[size] = 0xc3;
  // All bytes except the one relative CALL operand are copied exactly.
  if (std::memcmp(body, source, call - start + 1))
    throw std::runtime_error("Reseed prefix relocation changed");
  return body;
}
#endif
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  const auto *core = reinterpret_cast<const unsigned char *>(0x54e170);
  const Bytes coreBefore(core, core + 0xde);
  void *slices[]{callerSlice(0x4bec00, 0x4bec0f, 0x4bec0a),
                 callerSlice(0x4e4478, 0x4e4489, 0x4e4484),
                 callerSlice(0x557802, 0x557813, 0x55780e)};
  global(0x5c52e8, U(reinterpret_cast<std::uintptr_t>(&ownedClock)));
  std::array<OriginalRandomSnapshot, 2> locals{};
  auto pointer = [&](U token) -> void * {
    return token >= 0x400000 ? reinterpret_cast<void *>(token)
                             : &locals[token == 0x100 ? 0 : 1];
  };
  auto seedStream = [&](U token, U seed) {
    using Seed = void(__attribute__((thiscall)) *)(void *, U);
    reinterpret_cast<Seed>(0x54e170)(pointer(token), seed);
  };
  auto nextStream = [&](U token) {
    using Next = U(__attribute__((thiscall)) *)(void *);
    return reinterpret_cast<Next>(0x54e200)(pointer(token));
  };
  auto snapshot = [&](U token) {
    OriginalRandomSnapshot state;
    std::memcpy(&state, pointer(token), 1008);
    return state;
  };
  auto apply = [&](U route) {
    using Slice = void (*)();
    reinterpret_cast<Slice>(slices[route])();
  };
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native inputs");
  OriginalRandomStreams streams;
  auto seedStream = [&](U token, U value) { streams.seed(token, value); };
  auto nextStream = [&](U token) { return streams.next(token); };
  auto snapshot = [&](U token) { return streams.snapshot(token); };
  auto apply = [&](U route) {
    streams.seedParticleRoute(routes[route], [&] {
      ++clockCalls;
      return clockValue;
    });
  };
#endif
  static_assert(sizeof(OriginalRandomSnapshot) == 1008);
  const U seeds[]{0, 1, 0x12345678, 0xffffffff, 0x80000000, 0xdeadbeef};
  U id = 0;
  for (U seed : seeds)
    for (U route = 0; route < 3; ++route)
      for (U pattern = 0; pattern < 8; ++pattern)
        for (U advance : {0u, 1u, 249u, 251u}) {
          for (U index = 0; index < 8; ++index)
            seedStream(tokens[index], seed ^ (index * 7919));
          clockCalls = 0;
          clockValue = seed;
          U last = 0;
          const U foreign = tokens[(pattern % 7) + (pattern % 7 >= 3)];
          for (U stage = 0; stage < 8; ++stage) {
            switch (stage) {
            case 0:
              break;
            case 1:
              apply(route);
              break;
            case 2:
              for (U n = 0; n < advance; ++n)
                last = nextStream(tokens[pattern]);
              break;
            case 3:
              seedStream(foreign, seed ^ 0x89abcdef);
              break;
            case 4:
              last = nextStream(OriginalRandomStreams::particle);
              break;
            case 5:
              last = nextStream(foreign);
              last = nextStream(foreign);
              break;
            case 6:
              clockValue += 17;
              apply((route + 1) % 3);
              break;
            case 7:
              for (U token : tokens)
                last = nextStream(token);
              break;
            }
            const U header[]{id,      stage,   seed, route,
                             pattern, advance, last, clockCalls};
            output(header, sizeof(header));
#ifdef MNM_ORIGINAL_REFERENCE
            const auto lcg = *reinterpret_cast<U *>(0x6f2a9c);
#else
            const auto lcg = streams.sharedLcg();
#endif
            output(&lcg, 4);
            for (U token : tokens) {
              const auto state = snapshot(token);
              output(&state, sizeof(state));
            }
          }
          ++id;
        }
  if (id != 576)
    throw std::runtime_error("R250 context denominator");
#ifdef MNM_ORIGINAL_REFERENCE
  if (std::memcmp(core, coreBefore.data(), coreBefore.size()))
    throw std::runtime_error("Original R250 bodies changed");
#endif
  return 0;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
