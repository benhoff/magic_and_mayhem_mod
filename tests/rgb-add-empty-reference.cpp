// Exact original horizontal-empty RGB-add wrapper versus the production splice.
// Both runs use inaccessible pixel backing; some source descriptors expose only
// the two original preparation DWORDs. Empty work must not request CPU history.
static void* empty_getproc(void*, const char*);
#define MNM_PIXEL_TEST_GETPROC empty_getproc
#define MNM_PIXEL_TEST_UTILITIES
#define GetEnvironmentVariableA empty_fixture_getenv
#include "pixel-splice-reference.cpp"
#undef GetEnvironmentVariableA
static int empty_selection = 1;
extern "C" u32 WIN GetEnvironmentVariableA(const char* name, char* out, u32 size) {
    if (!std::strcmp(name, "MNM_PIXEL_EMPTY_RGB_NATIVE")) {
        if (empty_selection < 0) { lastError = 203; return 0; }
        if (!empty_selection) {
            lastError = 203;
            if (size >= 2) { out[0] = '0'; out[1] = 0; }
            return 1;
        }
    }
    return empty_fixture_getenv(name, out, size);
}
static unsigned history_requests, history_bad_error;
static int WIN empty_history(const void*, u32 operation, u32, u32, u32, u32) {
    if (operation == 0) { ++history_requests; if (lastError != 1234) ++history_bad_error; }
    return 0;
}
static void* empty_getproc(void*, const char* name) {
    if (!std::strcmp(name, "NativeDrawCpuHistoryV1")) return reinterpret_cast<void*>(empty_history);
    if (!std::strcmp(name, "NativeDrawProducerCountV1")) return reinterpret_cast<void*>(pixelCountProducer);
    return reinterpret_cast<void*>(nativeDib);
}

static void equal_state(const State& a, const State& b) {
    if (a.gpr != b.gpr || ((a.flags ^ b.flags) & 0xcd5) || a.unlocks != b.unlocks ||
        a.args != b.args || a.unlockGpr != b.unlockGpr || a.error != b.error || a.fx != b.fx)
        throw std::runtime_error("Empty RGB-add changed original caller state");
}

int main(int argc, char** argv) try {
    if (argc != 3) throw std::runtime_error("Expected original image and result");
    map_image(read(argv[1]));
    constexpr unsigned site = 0x545ce9;
    std::array<unsigned char, 6> original{}, patched{};
    std::memcpy(original.data(), reinterpret_cast<void*>(site), original.size());
    if (!PixelWritersInstallForTest()) throw std::runtime_error("Production installer refused original");
    std::memcpy(patched.data(), reinterpret_cast<void*>(site), patched.size());
    auto* backing = static_cast<unsigned short*>(mmap(nullptr, 4096, 3, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    auto* descriptor = static_cast<unsigned*>(mmap(nullptr, 8192, 3, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (backing == MAP_FAILED || descriptor == MAP_FAILED || mprotect(reinterpret_cast<char*>(descriptor) + 4096, 4096, PROT_NONE))
        throw std::runtime_error("Fixture mapping failed");
    asm volatile("fninit; fxsave %0" : "=m"(pixel_seed_fx));
    unsigned cases = 0, words = 0, guarded_descriptors = 0, disabled_cases = 0, default_cases = 0, disabled_history = 0;
    for (unsigned selection = 0; selection < 2; ++selection) {
    empty_selection = selection ? -1 : 1;
    for (unsigned format = 0; format < 2; ++format)
        for (unsigned df = 0; df < 2; ++df)
            for (unsigned edge = 0; edge < 4; ++edge)
                for (unsigned variant = 0; variant < 16; ++variant) {
                    const unsigned width = 9 + variant, height = 9 + variant % 7, stride = width + variant % 4;
                    const unsigned shape_width = 1 + variant % 4, shape_height = 3 + variant % 4;
                    const int x = edge == 0 ? -int(shape_width) : edge == 1 ? int(width) :
                                  edge == 2 ? int(width + 1 + variant) : -int(shape_width + 1 + variant);
                    const int y = int(variant % 4) - 2;
                    std::array<unsigned short, 2048> seed{};
                    for (unsigned i = 0; i < seed.size(); ++i) seed[i] = i * 391 + variant * 17;
                    unsigned shape[5] = {shape_width, shape_height, 2, 3, 4};
                    unsigned* input = shape;
                    if (variant & 1) { input = descriptor + 1022; input[0] = shape_width; input[1] = shape_height; ++guarded_descriptors; }
                    global(0x6e1f88, format); global(0x658174, reinterpret_cast<unsigned>(backing));
                    global(0x6a2dc8, stride); global(0x6e0008, 0); global(0x6cbb6c, 0);
                    global(0x6a49b8, width); global(0x656618, height);
                    pixel_function = 0x545c10; pixel_object = reinterpret_cast<unsigned>(input);
                    pixel_argc = 2; pixel_args[0] = x; pixel_args[1] = y;
                    pixel_seed_flags = 0x202 | (df ? 0x400 : 0); threadId = 1;
                    std::memcpy(backing, seed.data(), 4096);
                    if (mprotect(backing, 4096, PROT_NONE)) throw std::runtime_error("Protect original pixels failed");
                    std::memcpy(reinterpret_cast<void*>(site), original.data(), original.size());
                    const auto before = call();
                    const auto admitted = mnm_pixel_native[MNM_RGB_ADD], forwarded = mnm_pixel_forwarded[MNM_RGB_ADD];
                    history_requests = 0;
                    std::memcpy(reinterpret_cast<void*>(site), patched.data(), patched.size());
                    const auto after = call();
                    if (mprotect(backing, 4096, PROT_READ | PROT_WRITE)) throw std::runtime_error("Unprotect pixels failed");
                    equal_state(before, after);
                    if (std::memcmp(backing, seed.data(), 4096)) throw std::runtime_error("Empty RGB-add changed physical pixels");
                    if (mnm_pixel_native[MNM_RGB_ADD] != admitted + 1 || mnm_pixel_forwarded[MNM_RGB_ADD] != forwarded || history_requests)
                        throw std::runtime_error("Empty RGB-add was forwarded or queried history");
                    ++cases; default_cases += selection; words += 2048;
                }
    }
    empty_selection = 0;
    for (unsigned format = 0; format < 2; ++format) for (unsigned edge = 0; edge < 4; ++edge) {
        std::array<unsigned short, 2048> seed{};
        for (unsigned i = 0; i < seed.size(); ++i) seed[i] = i * 17 + edge;
        unsigned shape[5] = {2, 3, 1, 2, 3};
        global(0x6e1f88, format); global(0x658174, reinterpret_cast<unsigned>(backing));
        global(0x6a2dc8, 19); global(0x6e0008, 0); global(0x6cbb6c, 0);
        global(0x6a49b8, 16); global(0x656618, 12);
        pixel_object = reinterpret_cast<unsigned>(shape); pixel_args[0] = edge == 0 ? unsigned(-2) : edge == 1 ? 16 : edge == 2 ? 17 : unsigned(-3);
        pixel_args[1] = 0; pixel_seed_flags = 0x202;
        std::memcpy(backing, seed.data(), 4096);
        if (mprotect(backing, 4096, PROT_NONE)) throw std::runtime_error("Protect disabled pixels failed");
        std::memcpy(reinterpret_cast<void*>(site), original.data(), original.size());
        const auto before = call();
        const auto admitted = mnm_pixel_native[MNM_RGB_ADD], forwarded = mnm_pixel_forwarded[MNM_RGB_ADD];
        history_requests = history_bad_error = 0;
        std::memcpy(reinterpret_cast<void*>(site), patched.data(), patched.size());
        const auto after = call();
        if (mprotect(backing, 4096, PROT_READ | PROT_WRITE)) throw std::runtime_error("Unprotect disabled pixels failed");
        equal_state(before, after);
        if (std::memcmp(backing, seed.data(), 4096) || mnm_pixel_native[MNM_RGB_ADD] != admitted ||
            mnm_pixel_forwarded[MNM_RGB_ADD] != forwarded + 1 || history_requests != 1 || history_bad_error)
            throw std::runtime_error("Disabled empty admission changed fallback/history/LastError");
        ++disabled_cases; disabled_history += history_requests; words += 2048;
    }
    if (munmap(backing, 4096) || munmap(descriptor, 8192)) throw std::runtime_error("Fixture unmap failed");
    std::ofstream out(argv[2]);
    out << "{\"success\":true,\"empty_native_cases\":" << cases << ",\"physical_words\":" << words
        << ",\"guarded_source_descriptors\":" << guarded_descriptors
        << ",\"disabled_cases\":" << disabled_cases
        << ",\"default_native_cases\":" << default_cases
        << ",\"explicit_native_cases\":" << cases-default_cases
        << ",\"disabled_history_requests\":" << disabled_history
        << ",\"formats\":2,\"direction_flags\":2,\"cpu_history_requests\":0,\"live_game\":false}\n";
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
