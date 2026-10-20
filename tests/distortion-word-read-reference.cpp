// Derived from the retained 2026-10-12 standalone boundary fixture. The PE
// remains unmodified; only caller-owned pixels, frame and engine globals vary.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <csignal>
#include <ucontext.h>
#include <unistd.h>

static void fault(int, siginfo_t* info, void* raw) {
    const auto* context = static_cast<ucontext_t*>(raw);
    const auto& regs = context->uc_mcontext.gregs;
    const std::uint32_t values[]{std::uint32_t(std::uintptr_t(info->si_addr)),
        std::uint32_t(regs[REG_EIP]), std::uint32_t(regs[REG_EDI]) + std::uint32_t(regs[REG_EAX]) * 2};
    char message[] = "fault=00000000 eip=00000000 source=00000000\n";
    constexpr unsigned positions[]{6, 19, 35};
    for (unsigned field = 0; field < 3; ++field)
        for (unsigned digit = 0; digit < 8; ++digit)
            message[positions[field] + digit] = "0123456789abcdef"[(values[field] >> ((7 - digit) * 4)) & 15];
    const auto ignored = write(STDOUT_FILENO, message, sizeof(message) - 1);
    (void)ignored;
    _exit(42);
}

int main(int argc, char** argv) try {
    if (argc != 6) throw std::runtime_error("expected PE width height tail-bytes distance-from-last");
    const auto width = std::stoul(argv[2]), height = std::stoul(argv[3]);
    const auto tail = std::stoul(argv[4]), distance = std::stoul(argv[5]);
    if (!((width == 640 && height == 480) || (width == 800 && height == 640)) || tail > 2 || distance > 2)
        throw std::runtime_error("outside selected fixture domain");
    map_image(read(argv[1]));
    const unsigned char anchor[]{0x8b, 0x14, 0x47, 0x66, 0x89, 0x17};
    if (std::memcmp(reinterpret_cast<void*>(0x5965f7), anchor, sizeof(anchor)))
        throw std::runtime_error("DWORD read / WORD store signature mismatch");
    const auto length = width * height * 2;
    const auto page = static_cast<unsigned>(sysconf(_SC_PAGESIZE));
    if (page != 4096 || length % page) throw std::runtime_error("fixture requires 4096-byte pages");
    auto* allocation = static_cast<unsigned char*>(mmap(nullptr, length + 2 * page,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (allocation == MAP_FAILED) throw std::runtime_error("allocation");
    auto* guard = allocation + length + page;
    auto* pixels = guard - length - tail;
    if (mprotect(guard, page, PROT_NONE)) throw std::runtime_error("guard");
    struct sigaction action{};
    action.sa_sigaction = fault; action.sa_flags = SA_SIGINFO; sigemptyset(&action.sa_mask);
    if (sigaction(SIGSEGV, &action, nullptr)) throw std::runtime_error("signal handler");
    for (unsigned i = 0; i < length / 2; ++i) {
        const auto word = std::uint16_t(i * 97 + 13);
        std::memcpy(pixels + i * 2, &word, 2);
    }
    for (unsigned i = 0; i < tail; ++i) pixels[length + i] = 0xa5;
    global(0x658174, std::uintptr_t(pixels)); global(0x6a2dc8, width);
    global(0x6e0008, 0); global(0x6cbb6c, 0); global(0x6a49b8, width); global(0x656618, height); global(0x6c4830, 0);
    for (unsigned i = 0; i < 16; ++i) global(0x656640 + i * 4, 1);
    Bytes frame(50, 0);
    auto field = [&](unsigned at, std::uint32_t value) { std::memcpy(frame.data() + at, &value, 4); };
    field(0, 50); field(4, 1); field(8, 1); field(40, 48); field(44, 50); frame[49] = 1;
    const auto sourceIndex = width * height - 1 - distance;
    const auto expected = std::uint16_t(sourceIndex * 97 + 13);
    std::cout << "base=" << std::hex << std::uintptr_t(pixels) << " guard=" << std::uintptr_t(guard) << std::dec << '\n' << std::flush;
    using Draw = unsigned (*)(void*, int, int);
    const auto result = reinterpret_cast<Draw>(0x596490)(frame.data(), width - 2 - distance, height - 1);
    std::uint16_t actual;
    std::memcpy(&actual, pixels + (sourceIndex - 1) * 2, 2);
    std::cout << "{\"returned\":" << result << ",\"stored_word\":" << actual << ",\"pixel_matches\":" << (actual == expected ? "true" : "false") << "}\n";
    return result || actual != expected ? 1 : 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
