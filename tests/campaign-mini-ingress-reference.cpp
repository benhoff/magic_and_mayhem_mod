// Execute the pinned original key handler with only OS key translation and
// screen-push dependencies supplied. No live process or source file is patched.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main

static unsigned pushes, receiver, translations;
static unsigned __attribute__((stdcall)) translate(unsigned key, unsigned kind) {
    if (key != 27 || kind != 0) throw std::runtime_error("Unexpected key translation");
    ++translations;
    return 1; // Windows MapVirtualKeyA(VK_ESCAPE, MAPVK_VK_TO_VSC).
}
static unsigned __attribute__((thiscall)) push(void* object) {
    ++pushes; receiver = reinterpret_cast<std::uintptr_t>(object); return 0;
}
int main(int argc, char** argv) try {
    if (argc != 2) throw std::runtime_error("Expected hash-pinned PE");
    map_image(read(argv[1]));
    global(0x5c528c, reinterpret_cast<std::uintptr_t>(&translate));
    // Only replace the external screen-stack dependency in this private image.
    auto* target = reinterpret_cast<unsigned char*>(0x557040);
    target[0] = 0xe9;
    global(0x557041, reinterpret_cast<std::uintptr_t>(&push) - 0x557045);
    using Input = unsigned (__attribute__((thiscall)) *)(void*, unsigned, unsigned, unsigned, unsigned);
    unsigned cases = 0;
    for (unsigned context : {0u, 5u}) for (unsigned locked : {0u, 1u, 255u}) {
        global(0x68991c, 0); global(0x689920, context);
        *reinterpret_cast<unsigned char*>(0x6c4859) = locked;
        global(0x6a50db, 0xfeed1234); pushes = receiver = translations = 0;
        const auto result = reinterpret_cast<Input>(0x46c070)(reinterpret_cast<void*>(0x6cbb78), 0, 0x100, 27, 0);
        if (result || translations != 1 || pushes != unsigned(!locked) ||
            (locked ? (*reinterpret_cast<unsigned*>(0x6a50db) != 0xfeed1234 || receiver != 0)
                    : (*reinterpret_cast<unsigned*>(0x6a50db) != 2 || receiver != 0x6a5088)))
            throw std::runtime_error("Original tutorial/Mini branch mismatch");
        ++cases;
    }
    std::cout << "{\"success\":true,\"cases\":" << cases
              << ",\"scope\":\"Original Escape keydown, layout0, contexts0/5, tutorial byte0/1/255; key translation and screen push supplied\"}\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
