#include "../runtime/scene/world_colour.h"
#include <array>
#include <iostream>
#include <stdexcept>
int main(){
    std::array<uint8_t,24> pixels{};
    for(unsigned i:{1u,3u,6u,9u,10u}){pixels[i*2]=0xff;pixels[i*2+1]=0x45;}
    const auto before=pixels;
    auto result=mnm_colour_scan(pixels.data(),4,2,6,0x45ff);
    if(result.count!=4||result.left!=0||result.top!=0||result.right!=4||result.bottom!=2||pixels!=before)
        throw std::runtime_error("Colour scan count/padding/bounds or read-only invariant");
    // Independent bytewise position hash over visible linear positions1,3,4,7.
    uint32_t expected=2166136261u;
    for(uint32_t position:{1u,3u,4u,7u})for(unsigned shift:{0u,8u,16u,24u}){expected^=(position>>shift)&255;expected*=16777619u;}
    if(result.positions!=expected)throw std::runtime_error("Colour position hash");
    result=mnm_colour_scan(pixels.data(),4,2,6,0xffff);
    if(result.count||result.left||result.top||result.right||result.bottom||result.positions!=2166136261u)
        throw std::runtime_error("Empty colour checkpoint");
    std::cout<<"Read-only colour count, padding, bounds and position hash pass\n";
}
