#pragma once
#include <array>
#include <cstdint>
#include <cstring>

namespace mnm::reconstruction {
struct OriginalCanvasStartup {
    std::uint32_t window{}, video{}, depth{};
    // Original 6e1f68..6e1f8b, including alternate enable/source, padding,
    // three format WORDs and neighboring bytes that must remain untouched.
    std::array<unsigned char,36> globals{};
    std::uint32_t get(unsigned at)const{std::uint32_t v;std::memcpy(&v,globals.data()+at,4);return v;}
    void put(unsigned at,std::uint32_t v){std::memcpy(globals.data()+at,&v,4);}
    void word(unsigned at,std::uint16_t v){std::memcpy(globals.data()+at,&v,2);}
};
template<class Provider>
std::uint32_t initializeOriginalCanvas(OriginalCanvasStartup& state,std::uint32_t hwnd,Provider& provider){
    const std::uint32_t width=state.video?800:640,height=state.video?600:480;
    if(state.window)provider.window(hwnd);
    else provider.mode(hwnd,width,height,state.depth);
    std::array<std::uint32_t,27> desc{};
    desc[0]=108;desc[1]=1;desc[26]=0x200;
    provider.create(0x6a4fe0,desc,0x5e3328,0x171);
    if(state.get(16)){state.put(8,96);state.put(12,0);}
    desc={};desc[0]=108;desc[1]=7;desc[2]=height+state.get(12);
    desc[3]=width+state.get(8);desc[26]=0x840;
    provider.create(0x6a49e0,desc,0x5e3328,0x184);
    provider.fill(0x6a49e0,0);
    const auto format=provider.format(0x6a4fe0);
    state.word(20,std::uint16_t(format[4]));state.word(22,std::uint16_t(format[5]));
    state.word(24,std::uint16_t(format[6]));
    state.put(32,(~(format[4]|format[5]|format[6])>>15)&1);
    return 1;
}
} // namespace mnm::reconstruction
