// Execute unchanged original settled Realm tick; stub only external dependencies.
#pragma push_macro("main")
#undef main
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#include "realm_selection_fixture_bytes.h"
#include "../reconstruction/campaign/realm_selection.hpp"
#include <array>
#include <cstdlib>

namespace {
using namespace mnm::recovered::campaign;
constexpr unsigned realm = 0x659408;
static unsigned cueCount, selectionSamples, refusalSamples, pushes;
static void check(bool value, const char* message) {
    // Original PE frames carry no ELF exception-unwind metadata.
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
static unsigned word(unsigned address) {
    unsigned result; std::memcpy(&result, reinterpret_cast<void*>(address), 4); return result;
}
static void __attribute__((thiscall)) no_stack(void*) {}
static void __attribute__((thiscall)) one_stack(void*, unsigned) {}
static void __attribute__((thiscall)) render(void*, unsigned, unsigned, unsigned) {}
static int __attribute__((thiscall)) kind(void* self) {
    check(self == reinterpret_cast<void*>(realm + 0xa49), "realm-kind receiver"); return 1;
}
static int __attribute__((thiscall)) wizard(void*) { return -1; }
static void __attribute__((thiscall)) cue(unsigned id) {
    check(id == 1, "hover cue"); ++cueCount;
}
static void __attribute__((thiscall)) sample(void* self, int id, int a, int b, int c, int d, int e, int f) {
    check(self == reinterpret_cast<void*>(0x6b0198) && !a && !b && !c && !d && e == -1 && f == -1,
          "sample arguments");
    if (id == 801) ++selectionSamples;
    else { check(id == 99999, "capacity sample ID"); ++refusalSamples; }
}
static void __attribute__((thiscall)) push(void* self) {
    const auto p = reinterpret_cast<std::uintptr_t>(self);
    check(p == 0x6f2aa0 || p == 0x6c5148 || p == 0x6e0088 || p == 0x6a5088,
          "auxiliary target"); ++pushes;
}
static void __attribute__((thiscall)) tutorial(void* self) {
    check(self == reinterpret_cast<void*>(0x6e8000), "tutorial receiver");
}
static void free_tutorial(void* self) {
    check(self == reinterpret_cast<void*>(0x6e8000), "tutorial free");
}
static std::array<unsigned char, 800 * 600> hitMap{};
static void reset(unsigned player, unsigned region, const RegionSelectionInput& in, unsigned auxiliary, bool high) {
    std::memset(reinterpret_cast<void*>(realm), 0, 0x1e25);
    std::memset(reinterpret_cast<void*>(0x641ab8), 0, 0x28);
    global(0x641fbc, 0);
    global(0x641ab8, high ? 10 : 8); global(0x641abc, high ? 10 : 8);
    global(0x641ac4, in.buttonDown);
    global(0x6de6d5, high); global(0x6c54dc, 0); global(0x5e04c8, 0);
    global(0x6e0080, in.alternateMode); global(0x689918, unsigned(-1));
    global(realm + 0x12, realm + 0xa49); std::strcpy(reinterpret_cast<char*>(realm + 0xa49), "Celtic");
    global(realm + 0x59, reinterpret_cast<std::uintptr_t>(hitMap.data()));
    hitMap.fill(0); hitMap[10 * 800 + 10] = static_cast<unsigned char>(region);
    global(realm + 0x823, 9); global(0x659f51, player); global(0x659f55, 9);
    for (unsigned i = 0; i < 9; ++i) {
        global(0x65ab9d + i * 4, i < in.eligibleOccupants);
        global(0x65ae1d + i * 4, region);
        global(0x65af5d + i * 4, 0x11223344);
        global(0x65b1d9 + i * 4, player + 1);
    }
    // Avoid the separate automatic-battle ingress before settled-map processing.
    global(0x65a065 + player * 20, 1);
    global(0x65b1d9 + region * 4, in.playerOwns ? player : player + 1);
    global(0x65b1dd, player);
    global(realm + 0x1df + region * 4, in.adjacentOwned);
    for (unsigned at : {0x6f2d28u, 0x6f2d34u, 0x6f2d2cu, 0x6f2d30u}) global(at, 0);
    if (auxiliary) global(std::array<unsigned,4>{0x6f2d28,0x6f2d34,0x6f2d2c,0x6f2d30}[auxiliary-1], 1);
    global(0x6f2d38, region); *reinterpret_cast<unsigned char*>(0x6f2d3d) = in.releaseLatched;
    *reinterpret_cast<unsigned char*>(0x6f2d3c) = 0;
    global(realm + 0xa44, in.tutorialActive ? 0x6e8000 : 0);
    cueCount = selectionSamples = refusalSamples = pushes = 0;
}
static void verify(unsigned player, unsigned region, const RegionSelectionInput& in, unsigned auxiliary) {
    const auto expected = regionSelection(in);
    using Tick = void (__attribute__((thiscall)) *)(void*);
    reinterpret_cast<Tick>(0x5517a0)(reinterpret_cast<void*>(realm));
    check(*reinterpret_cast<unsigned char*>(0x6f2d3d) == expected.releaseLatched, "release latch mismatch");
    check(cueCount == unsigned(expected.hoverCue), "hover admission mismatch");
    check(selectionSamples == unsigned(expected.selectionCue), "selection cue mismatch");
    check(refusalSamples == unsigned(expected.effect == RegionSelectionEffect::CapacityRefusal), "capacity refusal mismatch");
    check(word(0x65af5d + player * 4) == (expected.effect == RegionSelectionEffect::Destination ? region : 0x11223344),
          "destination mutation mismatch");
    check(word(realm + 0xa2b) == unsigned(expected.effect == RegionSelectionEffect::RegionEntry), "entry request mismatch");
    if (expected.effect == RegionSelectionEffect::RegionEntry)
        check(word(realm + 0xa33) == region && word(realm + 0xa2f) == 1, "entry identity mismatch");
    check(pushes == unsigned(auxiliary != 0), "auxiliary priority mismatch");
}
}

int main(int argc, char** argv) try {
    check(argc == 2, "Pinned PE required"); map_image(read(argv[1]));
    const auto* tickStart = reinterpret_cast<const unsigned char*>(0x5517a0);
    const Bytes tickBytes(tickStart, reinterpret_cast<const unsigned char*>(0x552209));
    const auto* countStart = reinterpret_cast<const unsigned char*>(0x54e690);
    const Bytes countBytes(countStart, reinterpret_cast<const unsigned char*>(0x54e6d0));
    auto stub = [](unsigned at, const unsigned char* bytes, unsigned n, void* target) {
        check(n >= 5 && !std::memcmp(reinterpret_cast<void*>(at), bytes, n), "dependency original bytes");
        *reinterpret_cast<unsigned char*>(at) = 0xe9;
        global(at + 1, unsigned(reinterpret_cast<std::uintptr_t>(target) - at - 5));
    };
#define STUB(name, target) stub(name##_address, name##_bytes, sizeof(name##_bytes), reinterpret_cast<void*>(&target))
    STUB(input_poll, no_stack); STUB(hover_clear, no_stack); STUB(hover_text, one_stack);
    STUB(cue, cue); STUB(sample, sample); STUB(kind, kind); STUB(movement, no_stack);
    STUB(draw, one_stack); STUB(draw_prepare, no_stack); STUB(render, render); STUB(draw_finish, no_stack);
    STUB(push, push); STUB(wizard, wizard); STUB(tutorial, tutorial); STUB(free_tutorial, free_tutorial);
    unsigned cases = 0;
    for (unsigned player : {0u, 2u}) for (unsigned region : {0u, 3u})
    for (bool alternate : {false,true}) for (bool owns : {false,true}) for (bool adjacent : {false,true})
    for (unsigned occupants : {0u,1u,3u,4u,5u,9u}) for (bool down : {false,true})
    for (bool latched : {false,true}) for (unsigned auxiliary = 0; auxiliary <= 4; ++auxiliary)
    for (bool overlay : {false,true}) for (bool high : {false,true}) {
        RegionSelectionInput in{region,owns,adjacent,alternate,occupants,overlay,auxiliary!=0,down,latched};
        reset(player,region,in,auxiliary,high);
        verify(player,region,in,auxiliary); ++cases;
    }
    unsigned adjacencyCases = 0;
    for (unsigned source : {1u,4u,9u}) for (unsigned visible = 1; visible <= 9; ++visible)
    for (bool connected : {false,true}) {
        RegionSelectionInput in{3,false,connected && source <= visible,false,0,false,false,false,true};
        reset(0,3,in,0,true);
        global(0x65b1dd, 1); // Replace the default source1 ownership.
        global(realm + 0x1df + 3 * 4, 0);
        global(0x65b1d9 + source * 4, 0);
        global(realm + 0x1df + 3 * 4 + (source-1) * 0x50, connected);
        global(realm + 0x823, visible);
        verify(0,3,in,0); ++adjacencyCases;
    }
    unsigned coordinateCases = 0;
    for (bool high : {false,true}) {
        const int width = high ? 800 : 640, height = high ? 600 : 480;
        for (const auto& point : std::array<std::array<int,2>,7>{{
            {0,0},{width-1,height-1},{width,height-1},{width-1,height},{-1,0},{0,-1},{10,10}}}) {
            const int x = point[0]*800/width, y = point[1]*600/height;
            const bool inside = x >= 0 && x < 800 && y >= 0 && y < 600;
            const unsigned selected = inside ? 3 : 0;
            RegionSelectionInput in{selected,true,false,false,0,false,false,false,true};
            reset(0,selected,in,0,high);
            hitMap.fill(0);
            if (inside) hitMap[y*800+x] = 3;
            global(0x641ab8, unsigned(point[0])); global(0x641abc, unsigned(point[1]));
            verify(0,selected,in,0); ++coordinateCases;
        }
    }
    check(!std::memcmp(tickStart,tickBytes.data(),tickBytes.size()), "original tick mutated");
    check(!std::memcmp(countStart,countBytes.data(),countBytes.size()), "original occupancy counter mutated");
    std::cout << "{\"success\":true,\"cases\":" << cases
              << ",\"adjacency_cases\":" << adjacencyCases << ",\"coordinate_cases\":" << coordinateCases
              << ",\"original_tick_unchanged\":true,\"original_occupancy_unchanged\":true}\n";
    return 0;
} catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
