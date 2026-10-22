#include "../reconstruction/rendering/particle_classes.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include <csetjmp>
#include <csignal>
#include <ucontext.h>
#include <unistd.h>
static sigjmp_buf propertyFault;
static volatile sig_atomic_t propertyActive;
static void propertySignal(int signal, siginfo_t *info, void *context) {
  const auto pc = static_cast<unsigned>(
      static_cast<ucontext_t *>(context)->uc_mcontext.gregs[REG_EIP]);
  if (signal != SIGFPE || !propertyActive || pc < 0x54aa80 || pc >= 0x54aaa0 ||
      (info->si_code != FPE_INTDIV && info->si_code != FPE_INTOVF))
    _exit(70);
  propertyActive = 0;
  siglongjmp(propertyFault, 1);
}
static std::pair<unsigned, bool> callOriginalProperty(unsigned method,
                                                   const void *object) {
  if (sigsetjmp(propertyFault, 1))
    return {0, true};
  propertyActive = 1;
  using Property = unsigned(__attribute__((thiscall)) *)(const void *);
  const auto value = reinterpret_cast<Property>(method)(object);
  propertyActive = 0;
  return {value, false};
}
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  struct sigaction action{};
  action.sa_sigaction = propertySignal;
  action.sa_flags = SA_SIGINFO;
  sigemptyset(&action.sa_mask);
  if (sigaction(SIGFPE, &action, nullptr))
    throw std::runtime_error("Original priority fault setup");
  for (const auto &type : originalParticleClasses)
    if (std::memcmp(reinterpret_cast<const void *>(type.table),
                    type.slots.data(), 24))
      throw std::runtime_error("Original virtual table differs");
  const auto *code = reinterpret_cast<const unsigned char *>(0x54a9e0);
  const Bytes original(code, code + 0x350);
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native argument");
#endif
  const int values[]{0, 1, -1, 15, 1024, INT32_MAX, INT32_MIN, 12345678};
  U id = 0;
  for (U classIndex = 0; classIndex < 16; ++classIndex)
    for (U valueIndex = 0; valueIndex < 8; ++valueIndex)
      for (bool framePresent : {false, true})
        for (int duration : {-1, 0, 100})
          for (bool format : {false, true}) {
            const auto &type = originalParticleClasses[classIndex];
            ParticleObject object;
            object.bytes.fill(0xa5);
            object.put(0, type.table);
            for (U offset : {0x10u, 0x14u, 0x18u, 0x20u, 0x24u, 0x28u, 0x30u,
                             0x34u, 0x38u, 0x56u, 0x5eu})
              object.put(offset, values[valueIndex]);
            object.put(0x2c, values[(valueIndex + 3) % 8]);
            object.put(0x5a, framePresent ? 0x4000 : 0);
            if (valueIndex == 6)
              object.put(0x2c, -1); // INT_MIN/-1 original IDIV fault.
            const std::array<int, 2> frameSize{values[(valueIndex + 1) % 8],
                                               values[(valueIndex + 2) % 8]};
            const bool ownerEmitter = classIndex == 7 || classIndex == 8;
            if (ownerEmitter)
              object.put(0x44, 0x1010);
            if (classIndex == 9 || classIndex == 10) {
              object.put(0x2e, 0);
              object.put(0x46, valueIndex % 2 ? -127 : 127);
            }
            std::array<U, 3> results{};
            U faultMask = 0, drawResult = 0xffff, ownerResult = 0;
#ifdef MNM_ORIGINAL_REFERENCE
            const auto ptr = [](const void *p) {
              return U(reinterpret_cast<std::uintptr_t>(p));
            };
            U frame[10]{};
            frame[1] = U(frameSize[0]);
            frame[2] = U(frameSize[1]);
            U owner[512]{};
            owner[0x798 / 4] = U(values[valueIndex]);
            owner[0x110 / 4] = U(values[(valueIndex + 1) % 8]);
            if (framePresent)
              object.put(0x5a, particleSigned(ptr(frame)));
            if (ownerEmitter)
              object.put(0x44, particleSigned(ptr(owner)));
            global(0x5e1428, U(duration));
            global(0x6e1f88, format);
            for (U slot = 0; slot < 3; ++slot) {
              const auto value = callOriginalProperty(
                  type.slots[slot == 2 ? 5 : slot + 1], object.bytes.data());
              if (value.second)
                faultMask |= 1u << slot;
              results[slot] = value.first;
            }
#else
            for (U slot = 0; slot < 3; ++slot)
              try {
                results[slot] = U(originalParticleProperty(
                    type.slots[slot == 2 ? 5 : slot + 1], object, duration,
                    frameSize));
              } catch (const std::invalid_argument &) {
                faultMask |= 1u << slot;
                results[slot] = 0;
              }
#endif
            if (type.slots[3] == 0x54ac30 || type.slots[3] == 0x54ac40 ||
                type.slots[3] == 0x54ac00) {
#ifdef MNM_ORIGINAL_REFERENCE
              using Draw = U(__attribute__((thiscall)) *)(void *, int, int);
              drawResult = std::uint16_t(reinterpret_cast<Draw>(type.slots[3])(
                  object.bytes.data(), 7, 11));
#else
              drawResult = drawNonRasterParticle(type.slots[3], object);
#endif
            }
            if (ownerEmitter) {
              const std::array<int, 6> parameters{
                  values[valueIndex],           values[(valueIndex + 1) % 8],
                  values[(valueIndex + 2) % 8], values[(valueIndex + 3) % 8],
                  values[(valueIndex + 4) % 8], values[(valueIndex + 5) % 8]};
#ifdef MNM_ORIGINAL_REFERENCE
              using Set = void(__attribute__((thiscall)) *)(void *, int, int,
                                                            int, int, int, int);
              const auto slot = *reinterpret_cast<U *>(type.table + 24);
              reinterpret_cast<Set>(slot)(
                  object.bytes.data(), parameters[0], parameters[1],
                  parameters[2], parameters[3], parameters[4], parameters[5]);
              using Alive = U(__attribute__((thiscall)) *)(const void *);
              ownerResult = reinterpret_cast<Alive>(
                  *reinterpret_cast<U *>(type.table + 28))(object.bytes.data());
#else
              setOriginalParticleParameters(object, parameters);
              ownerResult = U(values[(valueIndex + (classIndex == 8)) % 8]);
#endif
            } else if (classIndex == 9 || classIndex == 10) {
              std::array<int, 3> rgb{values[valueIndex],
                                     values[(valueIndex + 1) % 8],
                                     values[(valueIndex + 2) % 8]};
#ifdef MNM_ORIGINAL_REFERENCE
              using Set =
                  void(__attribute__((thiscall)) *)(void *, int, int, int);
              reinterpret_cast<Set>(*reinterpret_cast<U *>(type.table + 24))(
                  object.bytes.data(), rgb[0], rgb[1], rgb[2]);
#else
              std::vector<std::uint16_t> plane;
              colourEmittedParticle(classIndex == 9
                                        ? ParticleChildKind::colour
                                        : ParticleChildKind::terrainColour,
                                    object, plane, rgb, format);
#endif
            }
#ifdef MNM_ORIGINAL_REFERENCE
            if (framePresent)
              object.put(0x5a, 0x4000);
            if (ownerEmitter)
              object.put(0x44, 0x1010);
#endif
            const U header[]{
                id++,        classIndex, valueIndex, U(framePresent),
                U(duration), U(format),  results[0], results[1],
                results[2],  faultMask,  drawResult, ownerResult};
            output(header, sizeof(header));
            output(object.bytes.data(), object.bytes.size());
          }
  if (id != 1536)
    throw std::runtime_error("Class property denominator");
#ifdef MNM_ORIGINAL_REFERENCE
  if (std::memcmp(code, original.data(), original.size()))
    throw std::runtime_error("Original particle virtual methods changed");
#endif
  return 0;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
