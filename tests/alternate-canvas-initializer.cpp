// Full unchanged 4e40d0 with owned mode/CreateSurface boundaries. Original
// 58bc10 clear and 58d540 pixel-format wrappers execute against owned COM calls.
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <algorithm>
using U = std::uint32_t;
static U ptr(const void* p) { return reinterpret_cast<U>(p); }
static U get(U p) { return *reinterpret_cast<U*>(p); }
static void require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
static void replace(U at, const Bytes& expected, void* target) {
  require(expected.size() >= 5 && !std::memcmp(reinterpret_cast<void*>(at), expected.data(), expected.size()), "Adapter signature");
  auto* p = reinterpret_cast<unsigned char*>(at); p[0] = 0xe9;
  const U delta = ptr(target) - at - 5; std::memcpy(p + 1, &delta, 4);
}
static U caller, video, setupCount, windowCount, clearCount, formatCount;
static std::vector<std::array<U, 27>> descriptors;
static std::vector<U> events;
static std::array<U, 40> table{};
static U iface;
static void __attribute__((fastcall)) setup(void* self, U width, U height, U depth) {
  require(ptr(self) == caller && width == (video ? 800u : 640u) &&
      height == (video ? 600u : 480u) && depth == 16, "Mode setup arguments");
  ++setupCount; events.push_back(1);
}
static void __attribute__((thiscall)) window(void* self) {
  require(ptr(self) == caller, "Window receiver"); ++windowCount; events.push_back(2);
}
static void __attribute__((thiscall)) create(void* self, U* desc, U file, U line) {
  require((ptr(self) == 0x6a4fe0 || ptr(self) == 0x6a49e0) && file == 0x5e3328 &&
      line == (descriptors.empty() ? 0x171u : 0x184u), "Create boundary identity");
  std::array<U,27> copy{}; std::copy(desc, desc + 27, copy.begin());
  descriptors.push_back(copy); global(ptr(self) + 8, ptr(&iface));
  events.push_back(ptr(self));
}
static U __attribute__((stdcall)) blt(void* self, void* dst, void* src, void* rect, U flags, U* fx) {
  require(ptr(self) == ptr(&iface) && !dst && !src && !rect && flags == 0x1000400, "Clear COM arguments");
  require(fx[0] == 100, "Clear FX size");
  for (unsigned i = 1; i < 25; ++i) require(fx[i] == 0, "Clear FX must be zero");
  ++clearCount; events.push_back(3); return 0;
}
static U __attribute__((stdcall)) pixelFormat(void* self, U* format) {
  require(ptr(self) == ptr(&iface) && format[0] == 32, "Format COM arguments");
  format[2] = 0xf800; format[3] = 0x07e0; format[4] = 0x001f;
  ++formatCount; events.push_back(4); return 0;
}
static void __attribute__((thiscall)) diagnostic(void*, U result) { require(result == 0, "Unexpected original error"); }
#ifndef MNM_ALTERNATE_INITIALIZER_ENTRY
#define MNM_ALTERNATE_INITIALIZER_ENTRY main
#endif
int MNM_ALTERNATE_INITIALIZER_ENTRY(int argc, char** argv) try {
  if (argc != 2) throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  const auto snapshot = [](U at, unsigned n) { auto* p = reinterpret_cast<unsigned char*>(at); return Bytes(p, p+n); };
  const auto initBody = snapshot(0x4e40d0, 0x193);
  const auto clearBody = snapshot(0x58bc10, 0x140);
  const auto formatBody = snapshot(0x58d540, 0x70);
  replace(0x58a9b0, snapshot(0x58a9b0, 8), reinterpret_cast<void*>(setup));
  // The runner supplies exact pinned prefixes for these independently owned boundaries.
  replace(0x58aa90, snapshot(0x58aa90, 8), reinterpret_cast<void*>(window));
  replace(0x58ad90, snapshot(0x58ad90, 8), reinterpret_cast<void*>(create));
  replace(0x484c00, snapshot(0x484c00, 8), reinterpret_cast<void*>(diagnostic));
  table[5] = ptr(reinterpret_cast<void*>(blt)); table[21] = ptr(reinterpret_cast<void*>(pixelFormat));
  iface = ptr(table.data());
  std::array<U,64> owner{}; caller = ptr(owner.data());
  std::array<U,256> alternate{};
  unsigned cases = 0;
  for (U v : {0u,1u}) for (U mode : {0u,1u}) for (U config : {0u,1u,0x80000000u})
    for (U xp : {0u,7u,96u}) for (U yp : {0u,5u})
      for (U enable : {0u,1u}) for (U source : {0u,ptr(alternate.data())}) {
        video=v; setupCount=windowCount=clearCount=formatCount=0;
        descriptors.clear(); events.clear(); alternate.fill(0xa57b1359);
        global(0x6a2920,mode); global(0x6de6d5,v); global(0x5e13fc,16);
        global(0x6e1f78,config); global(0x6e1f70,xp); global(0x6e1f74,yp);
        global(0x6e1f68,enable); global(0x6e1f6c,source);
        using Init = U(__attribute__((thiscall)) *)(void*);
        require(reinterpret_cast<Init>(0x4e40d0)(owner.data()) == 1, "Init return");
        require(setupCount == !mode && windowCount == mode && clearCount == 1 && formatCount == 1, "Initialization call counts");
        require(events == std::vector<U>{mode ? 2u : 1u,0x6a4fe0,0x6a49e0,3,4}, "Initialization event order");
        const U actualX = config ? 96u : xp, actualY = config ? 0u : yp;
        std::array<U,27> primary{}, offscreen{};
        primary[0]=offscreen[0]=108; primary[1]=1; primary[26]=0x200;
        offscreen[1]=7; offscreen[2]=(v?600u:480u)+actualY;
        offscreen[3]=(v?800u:640u)+actualX; offscreen[26]=0x840;
        require(descriptors == std::vector<std::array<U,27>>{primary,offscreen}, "Complete descriptors");
        require(get(0x6e1f70)==actualX && get(0x6e1f74)==actualY && get(0x6e1f78)==config, "Padding state");
        require(get(0x6e1f68)==enable && get(0x6e1f6c)==source, "Initializer changed alternate publication");
        require(std::all_of(alternate.begin(),alternate.end(),[](U x){return x==0xa57b1359;}), "Alternate storage unexpectedly written");
        ++cases;
      }
  require(snapshot(0x4e40d0,0x193)==initBody && snapshot(0x58bc10,0x140)==clearBody &&
      snapshot(0x58d540,0x70)==formatBody, "Original initializer/clear/format body changed");
  require(cases==288, "Initializer denominator");
  std::cout << "{\"cases\":" << cases << ",\"complete_descriptors_compared\":" << cases*2
    << ",\"original_initializer_clear_format_unchanged\":true,\"alternate_storage_unchanged\":true,"
       "\"real_surface_allocation_or_clear_pixels_claimed\":false}\n";
  return 0;
} catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
