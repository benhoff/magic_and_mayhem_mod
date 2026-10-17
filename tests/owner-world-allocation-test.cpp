#define main unused_world_allocation_brain_fixture
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_brain_driver.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
std::vector<unsigned> allocatedBrains;
void __attribute__((thiscall)) allocationRecord(void *brain, int n) {
  allocatedBrains.push_back(address(brain));
  allocatedBrains.push_back(unsigned(n));
}
void allocationEnd() {}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  redirect(0x415c40, reinterpret_cast<void *>(&allocationRecord));
  redirect(0x46b5e8, reinterpret_cast<void *>(&allocationEnd));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "allocation RX");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 2048; ++n) {
    unsigned total = n < 1024 ? n : (n * 7919) % 1000001;
    const auto share = m::ownerWorldBrainAllocation(int(total));
#ifdef MNM_BRAIN_REFERENCE
    allocatedBrains.clear();
    global(0x5e15f4, total);
    reinterpret_cast<void (*)()>(0x46b5be)();
    need(allocatedBrains.size() == 4, "two original brain calls");
    record(0x690600, allocatedBrains[0], "first brain");
    record(unsigned(share), allocatedBrains[1], "first allocation");
    record(0x644550, allocatedBrains[2], "second brain");
    record(unsigned(share), allocatedBrains[3], "second allocation");
#else
    for (unsigned v : {0x690600u, unsigned(share), 0x644550u, unsigned(share)})
      record(v, v, "allocation");
#endif
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "allocation unchanged text");
#endif
  std::cout << "{\"success\":true,\"cases\":2048,\"slices\":2048,\"checks\":"
            << checks << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
