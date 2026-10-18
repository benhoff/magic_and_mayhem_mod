#define main unused_admission_brain_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_task_lifecycle.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
bool special = false, distressed = false;
unsigned cacheAddress = 0;
unsigned __attribute__((thiscall)) specialCell(void *) { return special; }
unsigned __attribute__((thiscall)) ownerDistress(void *) { return distressed; }
unsigned __attribute__((thiscall)) groups(void *, void *, int, unsigned stale) {
  need(stale == 1, "preparation include stale");
  return cacheAddress;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  redirect(0x522690, reinterpret_cast<void *>(&specialCell));
  redirect(0x52a3a0, reinterpret_cast<void *>(&ownerDistress));
  redirect(0x413330, reinterpret_cast<void *>(&groups));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "RX text");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned admitted = 0, rejected = 0;
  for (unsigned n = 0; n < 2048; ++n) {
    m::OwnerTask6Admission a;
    a.enabled = !(n & 1);
    a.brain = (n >> 1) & 1;
    a.parent = n & 4;
    a.allAssigned = !(n & 8);
    a.allOffSpecialCells = !(n & 16);
    a.singletonWizardReady = n & 32;
    a.ownerAvailable = !(n & 64);
    a.ownerDistressed = n & 128;
    const bool nonempty = !(n & 256);
    const unsigned owner = n >> 9;
    const int clock = 100 + int(owner) * 37;
    const int enabledValue = std::array<int, 4>{100, 1, -100, 37}[owner];
    const bool ready = m::admitOwnerTask6(a) && nonempty;
    admitted += ready;
    rejected += !ready;
#ifdef MNM_BRAIN_REFERENCE
    Bytes actor(0xe4b), cfg(0x5c9), source(0x8d), child(0x8d), cache(0x8d),
        object(0x200), memories(8 * 48);
    unsigned children[1] = {address(child.data())};
    put(actor.data(), 0, 7);
    put(actor.data(), 0x174, owner);
    put(actor.data(), 0xac, address(cfg.data()));
    put(actor.data(), 0xde7, a.allAssigned ? owner : unsigned(-1));
    put(actor.data(), 0x8f5, a.singletonWizardReady ? 1 : 0);
    put(actor.data(), 0x936, a.enabled ? unsigned(enabledValue) : 0);
    put(cfg.data(), 0, a.singletonWizardReady ? 0 : 1);
    put(source.data(), 0, a.brain);
    put(source.data(), 0x35, address(actor.data()));
    put(source.data(), 0x39, a.parent ? address(child.data()) : 0);
    put(source.data(), 0x89, 3);
    for (unsigned i = 0; i < 8; ++i)
      source[0x41 + i] = 1;
    put(cache.data(), 0x29, address(children));
    put(cache.data(), 0x2d, address(children + unsigned(nonempty)));
    put(child.data(), 0x35, address(actor.data()));
    put(child.data(), 0x89, 3);
    put(object.data(), 4, 6);
    put(object.data(), 0x27, owner);
    object[0x149] = 1;
    put(object.data(), 0x16, 0x12345678);
    global(0x643a80 + owner * 0x154, a.ownerAvailable);
    global(0x643a80 + owner * 0x154 + 12, 0);
    global(0x6def58, address(actor.data()));
    global(0x6def5c, 1);
    global(0x5d854c, 3);
    special = !a.allOffSpecialCells;
    distressed = a.ownerDistressed;
    cacheAddress = address(cache.data());
    using Prepare =
        void(__attribute__((thiscall)) *)(void *, void *, void *, int);
    reinterpret_cast<Prepare>(0x431be0)(object.data(), source.data(),
                                        memories.data(), clock);
    record(ready, object[0x149], "task6 eligibility");
    record(ready ? unsigned(clock + 22) : 0x12345678,
           *reinterpret_cast<unsigned *>(object.data() + 0x16),
           "deadline writes");
    record(ready, object[0x18b], "prepared cursor");
    record(ready && a.singletonWizardReady, object[0x152], "wizard presence");
#else
    (void)enabledValue;
    record(ready, ready, "task6 eligibility");
    record(ready ? unsigned(clock + 22) : 0x12345678,
           ready ? unsigned(clock + 22) : 0x12345678, "deadline writes");
    record(ready, ready, "prepared cursor");
    record(ready && a.singletonWizardReady, ready && a.singletonWizardReady,
           "wizard presence");
#endif
  }
  need(admitted && rejected, "admission witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "unchanged RX text");
#endif
  std::cout << "{\"success\":true,\"cases\":2048,\"checks\":" << checks
            << ",\"admitted\":" << admitted << ",\"rejected\":" << rejected
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
