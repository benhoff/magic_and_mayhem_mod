#include "../apps/command-session/native_geometry.hpp"
#include "../apps/command-session/native_route.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "paired-encounter-original.hpp"
#else
#include "paired-encounter-state.hpp"
#include <iostream>
#endif
int main(int argc, char **argv) try {
  using namespace mnm;
  namespace m = command_session::model;
#ifdef MNM_PAIRED_REFERENCE
  if (argc != 3)
    throw std::runtime_error("PE INSTALL");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  sealText(pe);
  initializeSearch();
  const char *install = argv[2];
#else
  if (argc != 2)
    throw std::runtime_error("INSTALL");
  const char *install = argv[1];
#endif
  unsigned cases = 0;
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  for (unsigned scenario : {0u, 1u, 2u}) {
    auto profile = encounter::decodeInputs(
        encounter::loadInputs(install, {0, 0, 12, 12}, scenario));
    for (unsigned i = 0; i < profile.initial.world.creatures.size(); ++i) {
      auto c = profile.initial;
      command_session::NativeCreatureGeometry geometry(c, profile.native);
#ifdef MNM_PAIRED_REFERENCE
      Original o(profile);
      setup(o, c, 17);
#endif
      for (unsigned j = 0; j < c.world.creatures.size(); ++j) {
        if (i == j)
          continue;
        auto p = c.world.creatures[i].ai.position;
        auto ok = geometry.reach(c.world.creatures[i], c.world.creatures[j], p);
#ifdef MNM_PAIRED_REFERENCE
        using Reach = unsigned(__attribute__((thiscall)) *)(void *, void *, int,
                                                            int, int);
        auto actual = reinterpret_cast<Reach>(0x50bd60)(
            o.actors.data() + i * 0xe4b, o.actors.data() + j * 0xe4b, p.x, p.y,
            p.z);
        if (bool(actual) != ok)
          throw std::runtime_error(
              "reach scenario=" + std::to_string(scenario) +
              " i=" + std::to_string(i) + " j=" + std::to_string(j) +
              " native=" + std::to_string(ok) +
              " original=" + std::to_string(actual));
#endif
        ++cases;
        mix(ok);
      }
      m::AiGeometry g;
      g.reach = [&](const auto &a, const auto &b, auto p) {
        return geometry.reach(a, b, p);
      };
      auto random = m::seedRandom(17);
      auto result = m::immediateOpponent(c.world.creatures[i], c.world, g,
                                         [&] { return m::nextRandom(random); });
#ifdef MNM_PAIRED_REFERENCE
      using Query = unsigned(__attribute__((thiscall)) *)(void *);
      auto got = reinterpret_cast<Query>(0x516220)(o.actors.data() + i * 0xe4b);
      if (bool(got) != result || get(o.actors.data() + i * 0xe4b, 0x64c) !=
                                     unsigned(c.world.creatures[i].ai.target))
        throw std::runtime_error(
            "selection scenario=" + std::to_string(scenario) +
            " i=" + std::to_string(i) + " native=" +
            std::to_string(c.world.creatures[i].ai.target) + " original=" +
            std::to_string(get(o.actors.data() + i * 0xe4b, 0x64c)));
      if (get(reinterpret_cast<void *>(0x6e1ac8)) != random.left ||
          std::memcmp(reinterpret_cast<void *>(0x6e1ad0), random.words.data(),
                      1000))
        throw std::runtime_error("selection RNG differs");
#endif
      ++cases;
      mix(result);
      mix(unsigned(c.world.creatures[i].ai.target));
      for (unsigned q = 0; q < 24; ++q) {
        auto route = profile.initial;
        auto &actor = route.world.creatures[i];
        const m::Cell target{int((q * 7 + i) % 12), int((q * 5 + i * 3) % 12),
                             int(2 + q % 4)};
        const auto mode = q % 2;
#ifdef MNM_PAIRED_REFERENCE
        setup(o, route, 17);
        using Request = unsigned(__attribute__((thiscall)) *)(void *, int, int,
                                                              int, unsigned);
        const auto got = reinterpret_cast<Request>(0x512800)(
            o.actors.data() + i * 0xe4b, target.x, target.y, target.z, mode);
#endif
        const auto result = command_session::immediateNativeRoute(
            route, profile.native, actor, target, mode);
#ifdef MNM_PAIRED_REFERENCE
        if (bool(got) != result)
          throw std::runtime_error("immediate route result differs");
        const auto *raw = o.actors.data() + i * 0xe4b;
        for (const auto &[at, f] : encounter::paired::fields(route, i))
          if (get(raw, at, f.size) != f.value)
            throw std::runtime_error(std::string("immediate route field ") +
                                     f.name);
        if (get(raw, 0xd03) !=
            unsigned(route.actions.segments[i].regionPending))
          throw std::runtime_error("immediate route region flag differs");
        for (unsigned n = 0; n < 16; ++n) {
          const auto &point = route.actions.segments[i].route[n];
          const unsigned words[]{
              unsigned(point.position.x),   unsigned(point.position.y),
              unsigned(point.position.z),   unsigned(point.remainder[0]),
              unsigned(point.remainder[1]), unsigned(point.remainder[2]),
              unsigned(point.remainder[3])};
          for (unsigned k = 0; k < 7; ++k)
            if (get(raw, 0x97f + n * 28 + k * 4) != words[k])
              throw std::runtime_error(
                  "immediate route transfer scenario=" +
                  std::to_string(scenario) + " actor=" + std::to_string(i) +
                  " query=" + std::to_string(q) +
                  " point=" + std::to_string(n) + " word=" + std::to_string(k) +
                  " native=" + std::to_string(words[k]) + " original=" +
                  std::to_string(get(raw, 0x97f + n * 28 + k * 4)));
        }
#endif
        ++cases;
        mix(result);
        mix(route.actions.segments[i].count);
        for (const auto &point : route.actions.segments[i].route) {
          mix(unsigned(point.position.x));
          mix(unsigned(point.position.y));
          mix(unsigned(point.position.z));
          for (auto v : point.remainder)
            mix(unsigned(v));
        }
      }
    }
  }
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  destroySearch();
#endif
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
