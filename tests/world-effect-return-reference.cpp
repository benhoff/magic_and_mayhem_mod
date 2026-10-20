// Instrument returned values outside the unchanged original raster bodies.
#include "../reconstruction/rendering/effect_raster_return.hpp"
#include <array>
#include <iostream>
static std::array<unsigned,17> counts{};
static unsigned result(unsigned op,unsigned value){
    const unsigned backend=op==8?13:op==9?14:op==10?15:op==11?16:7;
    if((value&0xffff)!=mnm::reconstruction::effectRasterReturn(backend))
        throw std::runtime_error("Original effect raster low16 return differs");
    ++counts[backend];return value;
}
#define MNM_WORLD_RASTER_RESULT(op,call) result(op,call)
#define MNM_WORLD_REFERENCE_MAIN effect_reference_main
#include "world-frame-reference.cpp"

int main(int argc,char** argv){
    const auto code=effect_reference_main(argc,argv);if(code)return code;
    std::cout<<"{\"success\":true,\"low16_returns_match\":true,\"backend_calls\":{";
    bool first=true;for(unsigned i=0;i<counts.size();++i)if(counts[i]){
        if(!first)std::cout<<',';
        first=false;std::cout<<'"'<<i<<"\":"<<counts[i];
    }
    std::cout<<"}}\n";return 0;
}
