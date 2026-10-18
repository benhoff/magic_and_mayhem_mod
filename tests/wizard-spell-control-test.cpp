#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#define main unused_wizard_brain_fixture
#include "owner-brain-inference-test.cpp"
#undef main
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/wizard_spell_control.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
unsigned __attribute__((thiscall)) eligibleProducer(void *actor) {
  unsigned value;
  std::memcpy(&value, static_cast<char *>(actor) + 0x900, 4);
  return value;
}
#endif
unsigned sample(unsigned &seed) {
  seed = seed * 1664525u + 1013904223u;
  return seed;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "original PE input");
  map_image(load(argv[1]));
  redirect(0x412aa0, reinterpret_cast<void *>(&eligibleProducer));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "wizard RX text");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned seed = 173, ready = 0, finite = 0, impossible = 0;
  for (unsigned n = 0; n < 32768; ++n) {
    m::WizardMana a;
    a.kind =
        std::array<unsigned, 8>{0, 24, 25, 26, 17, 1, 27, UINT32_MAX}[n % 8];
    a.owner = (n / 8) % 8;
    unsigned cost = sample(seed) % 201, count = (n / 64) % 6;
    a.maximum = sample(seed) % 240;
    a.value = sample(seed) % 65536;
    if (n % 7 == 0)
      a.value = sample(seed);
    if (n % 11 == 0) {
      cost = sample(seed);
      a.maximum = sample(seed);
      count = sample(seed);
    }
    for (unsigned i = 0; i < a.inventory.size(); ++i) {
      auto &v = a.inventory[i];
      v.kind = sample(seed) % 3 ? 19u : 18u;
      v.magnitude = sample(seed) % 6;
      v.count = sample(seed) % 3;
      v.available = (sample(seed) >> 24) % 4;
      if (n % 13 == 0)
        v.magnitude = sample(seed);
    }
    int rate = int(sample(seed) % 9) - 2, period = int(sample(seed) % 120) + 1;
    if (n % 17 == 0)
      rate = int(sample(seed) % 8388608);
    std::vector<m::WizardManaProducer> producers;
    for (unsigned i = 0; i < n % 9; ++i) {
      const unsigned v = sample(seed);
      producers.push_back(
          {(v >> 4) % 8, bool(v & 1), bool(v & 2), bool(v & 4)});
    }
    const auto expected =
        m::wizardSpellWait(a, cost, count, producers, rate, period);
    ready += expected == 0;
    impossible += expected == INT_MAX;
    finite += expected != 0 && expected != INT_MAX;
#ifdef MNM_BRAIN_REFERENCE
    Bytes actor(0xe4b), definition(0x5c9), pool(1 + producers.size() * 0xe4b);
    put(actor.data(), 0xac, address(definition.data()));
    put(definition.data(), 0, a.kind);
    put(actor.data(), 0xe8, a.value);
    put(actor.data(), 0xec, a.maximum);
    put(actor.data(), 0x174, a.owner);
    for (unsigned i = 0; i < 42; ++i) {
      put(actor.data(), 0x17c + i * 24, a.inventory[i].kind);
      put(actor.data(), 0x180 + i * 24, a.inventory[i].magnitude);
      put(actor.data(), 0x188 + i * 24, a.inventory[i].count);
      put(actor.data(), 0x190 + i * 24, a.inventory[i].available);
    }
    for (unsigned i = 0; i < producers.size(); ++i) {
      const auto &p = producers[i];
      auto *row = pool.data() + i * 0xe4b;
      put(row, 4, p.active);
      put(row, 0xfc, p.producing);
      put(row, 0x174, p.owner);
      put(row, 0x900, p.eligible);
    }
    global(0x6b1270 + 7 * 0x2d1, cost);
    global(0x5e1908, unsigned(rate));
    global(0x5e1904, unsigned(period));
    global(0x6def58, address(pool.data()));
    global(0x6df180, producers.size());
    const auto before = actor;
    using Wait = int(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
    const auto actual =
        reinterpret_cast<Wait>(0x51e530)(actor.data(), 7, count);
    record(unsigned(expected), unsigned(actual), "wizard spell wait");
    need(before == actor, "wizard spell wait mutated actor");
#else
    record(unsigned(expected), unsigned(expected), "wizard spell wait");
#endif
  }
  need(ready && finite && impossible, "wizard wait branch coverage");
  need(!m::wizardSpellAvailability(false, 0, {{true, 0}}), "absent source");
  need(!m::wizardSpellAvailability(true, 17, {{true, 0}}), "ordinary source");
  need(!m::wizardSpellAvailability(true, 0, {{false, 0}}), "unknown spell");
  need(m::wizardSpellAvailability(true, 24, {{true, INT_MAX}}) == INT_MAX,
       "known infinite wait");
  need(m::wizardSpellAvailability(true, 25,
                                  {{true, 7}, {true, 3}, {false, 0}}) == 3,
       "peer minimum");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "wizard original text changed");
#endif
  std::cout << "{\"success\":true,\"cases\":32768,\"checks\":" << checks
            << ",\"ready\":" << ready << ",\"finite\":" << finite
            << ",\"impossible\":" << impossible << ",\"digest\":\"" << digest
            << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
