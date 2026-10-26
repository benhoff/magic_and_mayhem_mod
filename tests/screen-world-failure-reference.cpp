#ifdef MNM_ORIGINAL_REFERENCE
#define MNM_WORLD_ORIGINAL
#undef MNM_ORIGINAL_REFERENCE
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#define main unused_acquisition_main
#include "screen-acquisition-reference.cpp"
#undef main
#pragma GCC diagnostic pop
#ifdef MNM_WORLD_ORIGINAL
#define MNM_ORIGINAL_REFERENCE
#define main unused_world_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include <csetjmp>
#include <csignal>
#ifdef MNM_SCREEN_PROTECTED
static void check(bool value, const char *message) { need(value, message); }
#include "screen-provider-protected.hpp"
#else
static void guardBodies() {}
#endif
#ifdef MNM_ORIGINAL_REFERENCE
static sigjmp_buf faultJump;
static void divided(int signal) { siglongjmp(faultJump, signal); }
#endif
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  need(argc == 2, "pinned executable");
  map_image(read(argv[1]));
  need(mmap((void *)Base, Size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
            0) != MAP_FAILED,
       "World fault storage");
  std::signal(SIGFPE, divided);
#else
  (void)argc;
  (void)argv;
#endif
  profile = 0;
  mutation = 0;
  auto native = input(0);
  native.set(0x6de6fd, 0);
  reset(native);
  bool failed = false;
  try {
    Services services;
    mnm::reconstruction::enterOriginalWorldScreen(native, services, World);
  } catch (const std::domain_error &) {
    failed = true;
  }
  need(failed, "native zero-rate fault missing");
  need(native.get(0x6dbe99) == 0, "pre-fault rate store missing");
  need(calls.empty(), "post-fault provider executed");
#ifdef MNM_ORIGINAL_REFERENCE
  auto original = input(0);
  original.set(0x6de6fd, 0);
  original.original = true;
  std::memcpy((void *)Base, original.bytes.data() + GlobalSize, Size);
  for (U address : Globals)
    std::memcpy((void *)address, original.bytes.data() + address - GlobalBase,
                4);
  reset(original);
  const int signal = sigsetjmp(faultJump, 1);
  if (!signal) {
    using Fn = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Fn>(0x46a460)((void *)World);
    need(false, "original did not fault");
  }
  need(signal == SIGFPE, "original failure signal");
  std::memcpy(original.bytes.data() + GlobalSize, (void *)Base, Size);
  for (U address : Globals)
    std::memcpy(original.bytes.data() + address - GlobalBase, (void *)address,
                4);
  need(original.snapshot() == native.snapshot(),
       "World divide partial state differs");
#endif
  guardBodies();
  std::cout << "{\"original_zero_rate_divide\":true,\"partial_state_compared\":"
               "true,\"native_undefined_division\":false}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
