#pragma push_macro("main")
#undef main
#define main unused_contribution_fixture_main
#include "creature-resource-contribution-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/contribution_owned.hpp"
namespace {
struct OwnedFixture {
  ContributionFixture base;
  ContributionOwnership ownership;
};
OwnedFixture *owned;
bool runningOriginal = false;
void ownedDisplay(unsigned id, unsigned quantity, bool second) {
  record(second ? 149 : 148, id, quantity);
  if (fixture->number % 7 == 0) {
#ifdef MNM_EXECUTION_REFERENCE
    if (runningOriginal) {
      auto *p = actors.data() + id * 0xe4b;
      write32(p, 0xe8, read32(p, 0xe8) + (second ? 128 : 256));
      return;
    }
#endif
    owned->base.state.resource.at(id) += second ? 128 : 256;
  }
}
OwnedFixture generateOwned(unsigned n) {
  OwnedFixture f;
  f.base = generateContribution(n);
  auto &world = f.base.lifecycle.base.world;
  auto count = world.creatures.size();
  f.ownership.maximum.resize(count);
  constexpr unsigned kinds[] = {0, 1, 8, 20, 24, 25, 26, 27};
  for (unsigned i = 0; i < 28; ++i) {
    auto &p = f.ownership.profiles[i];
    p.kind = kinds[(n / 13 + i) % 8];
    p.capacity = 1 + (n + i) % 1000;
    p.flag58 = (n + i) % 3;
    for (unsigned j = 0; j < 28; ++j) {
      p.first[j] = (n + i + j) % 77;
      p.second[j] = (n / 3 + i + j) % 91;
    }
  }
  for (unsigned i = 0; i < 8; ++i) {
    f.base.state.controllers[i] = (n / 8 + i) % 4 ? int(i % count) : -1;
    f.ownership.ownerEnabled[i] = 1;
  }
  for (unsigned i = 0; i < count; ++i) {
    auto &a = world.creatures[i];
    a.ai.type = (n + i) % 28;
    a.ai.kind = f.ownership.profiles[a.ai.type].kind;
    f.base.state.resource[i] = (n * 17 + i) % 100000;
    f.ownership.maximum[i] = 1 + (n + i) % 2048;
    for (unsigned j = 0; j < 42; ++j)
      a.inventory[j] = {(n + j) % 3 ? 19u : 18u, (n + i + j) % 20, 0,
                        (n + j) % 4};
  }
  return f;
}
void nativeOwned(OwnedFixture &f) {
  owned = &f;
  contribution = &f.base;
  life = &f.base.lifecycle;
  fixture = &f.base.lifecycle.base;
  runningOriginal = false;
  auto s = maintenanceServices();
  s.upkeep = [&](auto &a) {
    contributeCreatureResourceOwned(
        a, fixture->world, fixture->execution, f.base.state, f.ownership,
        f.base.grant, maintenanceEnvironment().localOwner,
        {[](auto &actor, unsigned quantity) {
           ownedDisplay(actor.ai.slot, quantity, false);
         },
         [](auto &actor, unsigned quantity) {
           ownedDisplay(actor.ai.slot, quantity, true);
         }});
  };
  maintainCreature(fixture->world.creatures[0], fixture->execution[0],
                   life->maintenance[0], fixture->world,
                   maintenanceEnvironment(), s);
}
#ifdef MNM_EXECUTION_REFERENCE
std::vector<std::pair<unsigned, Bytes>> ownedBodies;
extern "C" void __attribute__((thiscall)) ownedPrimaryStub(void *p,
                                                           unsigned q) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("owned primary ABI");
  int owner = maintenanceEnvironment().localOwner;
  ownedDisplay(unsigned(owned->base.state.controllers[unsigned(owner)]), q,
               false);
}
extern "C" void __attribute__((thiscall)) ownedSecondaryStub(void *p,
                                                             unsigned q) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6c4e40)
    throw std::runtime_error("owned secondary ABI");
  int owner = maintenanceEnvironment().localOwner;
  ownedDisplay(unsigned(owned->base.state.controllers[unsigned(owner)]), q,
               true);
}
void compareOwned(const OwnedFixture &input, const OwnedFixture &expected,
                  const std::vector<Event> &trace) {
  auto original = input;
  owned = &original;
  contribution = &original.base;
  runningOriginal = true;
  auto &state = input.base.state;
  contributionNodes.assign(state.ancestors.size() * 64, 0x3d);
  contributionRecipients.assign(state.totals.size() * 64, 0x2d);
  for (unsigned i = 0; i < state.ancestors.size(); ++i) {
    const auto &node = state.ancestors[i];
    auto *p = contributionNodes.data() + i * 64;
    write32(p, 0x39,
            node.parent ? reinterpret_cast<std::uintptr_t>(
                              contributionNodes.data() + *node.parent * 64)
                        : 0);
    write32(p, 5,
            node.recipient
                ? reinterpret_cast<std::uintptr_t>(
                      contributionRecipients.data() + *node.recipient * 64)
                : 0);
  }
  for (unsigned i = 0; i < state.totals.size(); ++i)
    write32(contributionRecipients.data() + i * 64, 0x2f, state.totals[i]);
  auto nodesBefore = contributionNodes,
       recipientsExpected = contributionRecipients;
  for (unsigned i = 0; i < state.totals.size(); ++i)
    write32(recipientsExpected.data() + i * 64, 0x2f,
            expected.base.state.totals[i]);
  Bytes profileBefore, ownerBefore, configBefore;
  LifeCompareExtension ext;
  ext.install = [&] {
    // The inherited lifecycle fixture installs a death-account stub. Restore
    // every compared callee before executing this owned contribution graph.
    for (const auto &body : ownedBodies)
      std::memcpy(reinterpret_cast<void *>(body.first), body.second.data(),
                  body.second.size());
    redirect(0x4d6f30, reinterpret_cast<void *>(ownedPrimaryStub));
    redirect(0x4d3a10, reinterpret_cast<void *>(ownedSecondaryStub));
    for (unsigned i = 0; i < 8; ++i) {
      global(0x643a80 + i * 340, input.ownership.ownerEnabled[i]);
      global(0x643a80 + i * 340 + 12, unsigned(state.controllers[i]));
    }
    profileBefore.assign(28 * 0x5c9, 0xa7);
    for (unsigned i = 0; i < 28; ++i) {
      const auto &v = input.ownership.profiles[i];
      auto *p = profileBefore.data() + i * 0x5c9;
      write32(p, 0, v.kind);
      write32(p, 0x18, v.capacity);
      write32(p, 0x58, v.flag58);
      for (unsigned j = 0; j < 28; ++j) {
        write32(p, 0x280 + j * 4, v.first[j]);
        write32(p, 0x1a0 + j * 4, v.second[j]);
      }
    }
    std::memcpy(reinterpret_cast<void *>(0x6a5f80), profileBefore.data(),
                profileBefore.size());
    global(0x5e1908, input.base.grant);
    ownerBefore = Bytes(reinterpret_cast<unsigned char *>(0x643a80),
                        reinterpret_cast<unsigned char *>(0x643a80 + 8 * 340));
    configBefore = configs;
  };
  ext.pack = [&](Bytes &b, bool expectedState) {
    packContribution(b, expectedState ? expected.base : input.base);
    for (unsigned i = 0; i < input.ownership.maximum.size(); ++i)
      write32(b.data() + i * 0xe4b, 0xec, input.ownership.maximum[i]);
  };
  ext.verify = [&] {
    if (contributionNodes != nodesBefore ||
        contributionRecipients != recipientsExpected ||
        configs != configBefore ||
        std::memcmp(reinterpret_cast<void *>(0x643a80), ownerBefore.data(),
                    ownerBefore.size()) ||
        std::memcmp(reinterpret_cast<void *>(0x6a5f80), profileBefore.data(),
                    profileBefore.size()))
      throw std::runtime_error("owned contribution buffers guard");
    for (const auto &b : ownedBodies)
      if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                      b.second.size()))
        throw std::runtime_error("owned contribution callee modified");
  };
  compareLife(input.base.lifecycle, expected.base.lifecycle, 0, false, trace,
              &ext);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
  for (auto r : {std::pair<unsigned, unsigned>{0x466870, 0x4668b0},
                 {0x507f00, 0x507f9c},
                 {0x416d80, 0x416ea0},
                 {0x505bf0, 0x505c49},
                 {0x505c50, 0x505c95},
                 {0x59bee0, 0x59bf07}})
    ownedBodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
#else
  (void)argc;
  (void)argv;
#endif
  unsigned short before, cw;
  asm volatile("fnstcw %0" : "=m"(before));
  cw = (before & ~0x0f00u) | 0x0300u;
  asm volatile("fldcw %0" ::"m"(cw));
  unsigned displays = 0, credits = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generateOwned(n), expected = input;
    events.clear();
    nativeOwned(expected);
    auto trace = events;
    for (const auto &e : trace) {
      mix(e.kind);
      mix(e.slot);
      mix(e.argument);
      displays += e.kind == 148;
    }
    for (auto v : expected.base.state.resource)
      mix(v);
    for (auto v : expected.base.state.totals)
      mix(v);
    credits += expected.base.state.totals != input.base.state.totals;
#ifdef MNM_EXECUTION_REFERENCE
    compareOwned(input, expected, trace);
#endif
  }
  asm volatile("fldcw %0" ::"m"(before));
  if (!displays || !credits)
    throw std::runtime_error("empty owned contribution branch");
  std::cout
      << "{\"success\":true,\"cases\":8192,\"functions\":7,\"outcome_digest\":"
      << digest << ",\"display_cases\":" << displays
      << ",\"credited_cases\":" << credits << ",\"owned_graph_redirects\":0}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
