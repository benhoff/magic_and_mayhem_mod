// Execute the unmodified original initializer with per-thread CPUID trapping.
// Only the two original CPUID instructions are serviced; no game code or
// dispatch pointer is patched, and the original computes every selected target.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <csignal>
#include <sys/syscall.h>
#include <ucontext.h>
#include <unistd.h>

static volatile sig_atomic_t cpuFamily, cpuMmx, cpuCalls;

static void cpuid(int, siginfo_t*, void* context) {
    auto& r = static_cast<ucontext_t*>(context)->uc_mcontext.gregs;
    const auto ip = static_cast<unsigned>(r[REG_EIP]);
    if ((ip != 0x4844d8 && ip != 0x484602) ||
        *reinterpret_cast<const unsigned short*>(ip) != 0xa20f)
        _exit(70);
    const auto leaf = static_cast<unsigned>(r[REG_EAX]);
    if (ip == 0x4844d8 && leaf == 0) {
        r[REG_EAX] = 1;
        r[REG_EBX] = 0x756e6547; // GenuineIntel, in the CPUID register order.
        r[REG_EDX] = 0x49656e69;
        r[REG_ECX] = 0x6c65746e;
    } else if (ip == 0x484602 && leaf == 1) {
        r[REG_EAX] = static_cast<unsigned>(cpuFamily) << 8;
        r[REG_EBX] = r[REG_ECX] = 0;
        r[REG_EDX] = static_cast<unsigned>(cpuMmx) << 23;
    } else {
        _exit(71);
    }
    ++cpuCalls;
    r[REG_EIP] += 2;
}

int main(int argc, char** argv) try {
    if (argc != 3) throw std::runtime_error("Expected original PE and result");
    map_image(read(argv[1]));
    const auto* begin = reinterpret_cast<const unsigned char*>(0x484470);
    const Bytes original(begin, begin + 0x231);
    struct sigaction action{};
    action.sa_sigaction = cpuid;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGSEGV, &action, nullptr)) throw std::runtime_error("CPUID signal setup failed");
    using Initialize = void (__attribute__((thiscall)) *)(void*);
    const auto initialize = reinterpret_cast<Initialize>(0x484470);
    const unsigned slots[]{0x6c4e3c, 0x689b7c, 0x689b78, 0x6903dc};
    const unsigned overrides[]{0, 1, 2, UINT32_MAX};
    std::ofstream out(argv[2]);
    out << "{\"success\":true,\"original_code_unchanged\":true,\"cases\":[";
    unsigned cases = 0;
    for (unsigned family = 0; family < 16; ++family)
        for (unsigned mmx = 0; mmx < 2; ++mmx)
            for (auto overrideValue : overrides) {
                std::array<unsigned, 32> object;
                object.fill(0xa5a5a5a5);
                for (auto slot : slots) global(slot, 0xdeadbeef);
                global(0x6e1f38, overrideValue);
                cpuFamily = family; cpuMmx = mmx; cpuCalls = 0;
                if (syscall(SYS_arch_prctl, 0x1012, 0))
                    throw std::runtime_error("Kernel per-thread CPUID trapping unavailable");
                initialize(object.data());
                if (syscall(SYS_arch_prctl, 0x1012, 1)) _exit(72);
                if (cpuCalls != 2 || object[0x2c / 4] != family || object[0] != mmx)
                    throw std::runtime_error("Original CPU preparation differs");
                const std::array<unsigned, 4> expected{
                    overrideValue == 1 ? 0x57de00u : family == 6 ? 0x59521au : 0x5947b2u,
                    family == 6 ? 0x59603eu : 0x595b47u,
                    family == 6 ? 0x596490u : 0x5968a4u,
                    family == 6 ? 0x596cb8u : 0x597086u};
                std::array<unsigned, 4> actual{};
                for (unsigned i = 0; i < 4; ++i) actual[i] = *reinterpret_cast<unsigned*>(slots[i]);
                if (actual != expected) throw std::runtime_error("Original raster selection differs");
                for (unsigned i = 15; i < object.size(); ++i)
                    if (object[i] != 0xa5a5a5a5) throw std::runtime_error("CPU object guard changed");
                if (std::memcmp(original.data(), begin, original.size()))
                    throw std::runtime_error("Original initializer/helper code changed");
                if (cases++) out << ',';
                out << "{\"base_family\":" << family << ",\"mmx\":" << mmx
                    << ",\"override\":" << overrideValue << ",\"targets\":[";
                for (unsigned i = 0; i < 4; ++i) { if (i) out << ','; out << actual[i]; }
                out << "]}";
            }
    out << "],\"case_count\":" << cases << ",\"live_game\":false}\n";
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
