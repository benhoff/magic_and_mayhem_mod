// Original global map storage, watched at CPU write-access granularity.
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

using U = std::uint32_t;
constexpr U Map = 0x6c5490, Byte = Map + 0x6380, Collection = 0x689498;
static volatile U phase = 0;
static std::array<std::array<std::string, 3>, 89> fields;
static std::array<Bytes, 5> buffers;
static std::vector<U> releases;
static unsigned requests;
static void setWord(U p, U value) { std::memcpy(reinterpret_cast<void*>(p), &value, 4); }
static U word(std::istream& in) { U n; in.read(reinterpret_cast<char*>(&n), 4); if (!in) throw std::runtime_error("fixture extent"); return n; }
static U __attribute__((stdcall, force_align_arg_pointer)) directory(U capacity, char* out) {
    if (capacity != 256) throw std::runtime_error("directory extent");
    std::memcpy(out, "C:\\game", 8); return 7;
}
static U __attribute__((stdcall, force_align_arg_pointer)) profile(const char* section, const char* key,
        const char* fallback, char* out, U capacity, const char* path) {
    static const char* keys[] = {"LightSourceDiameter", "LightSourceAffected", "ClippedToHeight"};
    const U type = requests / 3, field = requests % 3;
    if (type >= 89 || section != "FXA_" + std::to_string(type) ||
        std::strcmp(key, keys[field]) || *fallback || capacity != 256 ||
        std::strcmp(path, "C:\\game\\cfg\\effects.cfg")) throw std::runtime_error("profile request");
    const auto& value = fields[type][field];
    std::memcpy(out, value.c_str(), value.size() + 1); ++requests; return value.size();
}
extern "C" void __attribute__((force_align_arg_pointer)) release_endpoint(void* p) {
    const U address = U(reinterpret_cast<std::uintptr_t>(p));
    bool admitted = false;
    for (const auto& b : buffers) admitted |= address == U(reinterpret_cast<std::uintptr_t>(b.data() + 16));
    if (!admitted) throw std::runtime_error("unexpected released handle");
    releases.push_back(address); // Supplied delete boundary; vectors keep ownership.
}
static void configure() {
    constexpr U w = 19, h = 21, z = 5, n = w * h * 3;
    setWord(Map + 4, w); setWord(Map + 8, h); setWord(Map + 12, z); setWord(Map + 16, w * h);
    setWord(Map + 0x7e0, n);
    for (U i = 0; i < 5; ++i) {
        buffers[i].assign(n + 32, 0xa5);
        setWord(Map + 0x7cc + i * 4, U(reinterpret_cast<std::uintptr_t>(buffers[i].data() + 16)));
    }
    for (U y = 0; y < h; ++y) setWord(Map + 0x64b2 + y * 4, y * w);
    for (U layer = 0; layer < 3; ++layer) setWord(Map + 0x6432 + layer * 4, layer * w * h);
}
static void protect(const Bytes& image) {
    const U head = u32(image, 0x3c), opt = head + 24;
    const U count = image[head + 6] | image[head + 7] << 8;
    const U length = image[head + 20] | image[head + 21] << 8;
    for (U i = 0; i < count; ++i) {
        const U s = opt + length + i * 40;
        if (u32(image, s + 36) & 0x20000000) {
            const U size = (std::max(u32(image, s + 8), u32(image, s + 16)) + 4095) & ~4095u;
            if (mprotect(reinterpret_cast<void*>(0x400000 + u32(image, s + 12)), size, PROT_READ | PROT_EXEC))
                throw std::runtime_error("RX protection");
        }
    }
}
static long checked_ptrace(enum __ptrace_request request, pid_t child, void* address, void* data) {
    errno = 0; const long result = ptrace(request, child, address, data);
    if (result == -1 && errno) throw std::runtime_error("ptrace request " + std::to_string(request) + ": " + std::strerror(errno));
    return result;
}
int main(int argc, char** argv) try {
    if (argc != 3) throw std::runtime_error("expected pinned PE and decoded installed FX fields");
    const auto image = read(argv[1]); map_image(image);
    if (*reinterpret_cast<unsigned char*>(Byte) != 0) throw std::runtime_error("global image byte not zero");
    std::ifstream input(argv[2], std::ios::binary);
    for (auto& row : fields) for (auto& value : row) {
        const U n = word(input); if (n > 255) throw std::runtime_error("profile length");
        value.resize(n); input.read(value.data(), n); if (!input) throw std::runtime_error("profile extent");
    }
    setWord(0x5c50bc, U(reinterpret_cast<std::uintptr_t>(directory)));
    setWord(0x5c507c, U(reinterpret_cast<std::uintptr_t>(profile)));
    using Reload = U(__attribute__((thiscall)) *)(void*);
    using Select = U(__attribute__((thiscall)) *)(void*, U);
    const auto reload = reinterpret_cast<Reload>(0x49c2e0);
    const auto select = reinterpret_cast<Select>(0x49cf60);
    if (reload(reinterpret_cast<void*>(Collection)) != 1 || requests != 267) throw std::runtime_error("FX loader");
    std::vector<U> baseline;
    for (U type = 0; type < 89; ++type) baseline.push_back(select(reinterpret_cast<void*>(Collection), type));
    if (std::find(baseline.begin(), baseline.end(), 1) != baseline.end()) throw std::runtime_error("installed index-one assumption");
    // Verify before adapting only the opaque delete endpoint, never a selected body.
    const unsigned char expected[] = {0x8b, 0x44, 0x24, 0x04, 0x85};
    auto* bridge = reinterpret_cast<unsigned char*>(0x597870);
    if (std::memcmp(bridge, expected, sizeof(expected))) throw std::runtime_error("delete endpoint bytes");
    bridge[0] = 0xe9; setWord(0x597871, U(reinterpret_cast<std::uintptr_t>(release_endpoint)) - 0x597875);
    protect(image);
    U cases = 0, controls = 0, selectedWrites = 0, releaseCalls = 0;
    std::vector<U> controlPCs;
    for (int trial = -1; trial < 256; ++trial) {
        const pid_t child = fork(); if (child < 0) throw std::runtime_error("fork");
        if (!child) {
            try {
                if (trial >= 0) *reinterpret_cast<volatile unsigned char*>(Byte) = unsigned(trial);
                const unsigned char inherited = *reinterpret_cast<unsigned char*>(Byte);
                configure(); setWord(0x5e41a8, 1); releases.clear();
                if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr)) _exit(81);
                raise(SIGSTOP);
                phase = 1; asm volatile("movb %0,(%1)" :: "q"(inherited), "r"(Byte) : "memory");
                U overlapping; std::memcpy(&overlapping, reinterpret_cast<void*>(Byte - 1), 4);
                phase = 2; asm volatile("movl %0,(%1)" :: "r"(overlapping), "r"(Byte - 1) : "memory");
                using Call = void(__attribute__((thiscall)) *)(void*);
                const auto init = reinterpret_cast<Call>(0x4ecdd0), kernel = reinterpret_cast<Call>(0x4f1890), clear = reinterpret_cast<Call>(0x4ecbe0);
                phase = 3; init(reinterpret_cast<void*>(Map));
                phase = 4; kernel(reinterpret_cast<void*>(Map));
                phase = 5; clear(reinterpret_cast<void*>(Map));
                if (releases.size() != 5) _exit(82);
                phase = 6; configure(); init(reinterpret_cast<void*>(Map));
                phase = 7; clear(reinterpret_cast<void*>(Map));
                if (releases.size() != 10 || *reinterpret_cast<unsigned char*>(Byte) != inherited) _exit(83);
                phase = 8; fields[0][0] = "1"; requests = 0;
                if (reload(reinterpret_cast<void*>(Collection)) != 1 || requests != 267 || select(reinterpret_cast<void*>(Collection), 0) != 1) _exit(84);
                _exit(0);
            } catch (...) { _exit(85); }
        }
        int status; if (waitpid(child, &status, 0) != child || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) throw std::runtime_error("child startup");
        checked_ptrace(PTRACE_SETOPTIONS, child, nullptr, reinterpret_cast<void*>(PTRACE_O_EXITKILL));
        const auto debug = [&](U number, U value) {
            checked_ptrace(PTRACE_POKEUSER, child, reinterpret_cast<void*>(offsetof(struct user, u_debugreg) + number * sizeof(long)), reinterpret_cast<void*>(value));
        };
        debug(0, Byte); debug(6, 0); debug(7, 1 | (1 << 16)); // Local DR0, write, one byte.
        std::array<U, 9> hits{};
        for (;;) {
            checked_ptrace(PTRACE_CONT, child, nullptr, nullptr);
            if (waitpid(child, &status, 0) != child) throw std::runtime_error("wait");
            if (WIFEXITED(status)) { if (WEXITSTATUS(status)) throw std::runtime_error("child failure " + std::to_string(WEXITSTATUS(status))); break; }
            if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) { kill(child, SIGKILL); waitpid(child, &status, 0); throw std::runtime_error("unexpected child stop " + std::to_string(status)); }
            const U current = U(checked_ptrace(PTRACE_PEEKDATA, child, const_cast<U*>(&phase), nullptr));
            if (current >= hits.size()) throw std::runtime_error("phase");
            ++hits[current]; struct user_regs_struct registers{};
            checked_ptrace(PTRACE_GETREGS, child, nullptr, &registers);
            if (current < 3 && std::find(controlPCs.begin(), controlPCs.end(), registers.eip) == controlPCs.end()) controlPCs.push_back(registers.eip);
            debug(6, 0);
        }
        if (hits[1] != 1 || hits[2] != 1) throw std::runtime_error("positive write controls");
        for (U stage = 3; stage < hits.size(); ++stage) selectedWrites += hits[stage];
        controls += hits[1] + hits[2]; ++cases; releaseCalls += 10;
    }
    if (selectedWrites) throw std::runtime_error("selected path writes inherited byte");
    std::cout << "{\"success\":true,\"global_image_initial_byte\":0,\"cases\":" << cases
        << ",\"positive_same_value_and_overlapping_writes\":" << controls << ",\"selected_byte_writes\":" << selectedWrites
        << ",\"initializations\":" << cases * 2 << ",\"direct_kernel_rebuilds\":" << cases
        << ",\"clears\":" << cases * 2 << ",\"supplied_delete_calls\":" << releaseCalls
        << ",\"installed_selectors\":89,\"authored_index_one_selectors\":" << cases << ",\"installed_indices\":[";
    for (U i = 0; i < baseline.size(); ++i) std::cout << (i ? "," : "") << baseline[i];
    std::cout << "],\"control_post_write_pcs\":[";
    for (U i = 0; i < controlPCs.size(); ++i) std::cout << (i ? "," : "") << controlPCs[i];
    std::cout << "]}\n";
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
