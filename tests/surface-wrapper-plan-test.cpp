#include "../reconstruction/rendering/surface_wrappers.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static SurfaceCopyWrapper wrapper(unsigned entry) {
    switch(entry) {
    case 0x58c360: return SurfaceCopyWrapper::opaqueGuarded;
    case 0x58c4a0: return SurfaceCopyWrapper::opaqueRecovering;
    case 0x58c8a0: return SurfaceCopyWrapper::keyedRecovering;
    case 0x58ca90: return SurfaceCopyWrapper::keyedRectangle;
    case 0x58cbc0: return SurfaceCopyWrapper::keyedExplicit;
    default: throw std::runtime_error("Unknown original wrapper input");
    }
}
static void print(SurfaceRectangle32 r) {
    std::cout<<'['<<r.left<<','<<r.top<<','<<r.right<<','<<r.bottom<<']';
}
int main(int argc,char** argv) try {
    if(argc!=2) throw std::runtime_error("Expected original argument corpus input");
    std::ifstream input(argv[1]);
    if(!input) throw std::runtime_error("Cannot open input cases");
    unsigned id,entry,fast,noWait,window,missing;
    std::int32_t x,y;
    SurfaceRectangle32 source,destination;
    bool first=true;
    std::cout<<'[';
    while(input>>id>>entry>>fast>>noWait>>window>>missing
               >>source.left>>source.top>>source.right>>source.bottom>>x>>y
               >>destination.left>>destination.top>>destination.right>>destination.bottom) {
        const auto plan=planSurfaceWrapperCopy(wrapper(entry),source,x,y,destination,
            {bool(fast),bool(noWait),bool(window),window==1 ? -19 : 0,window==1 ? 33 : 0},
            !(missing&1),!(missing&2));
        if(!first) std::cout<<',';
        first=false;
        std::cout<<"{\"id\":"<<id<<",\"calls\":"<<plan.called
                 <<",\"operation\":"<<(plan.called ? plan.fast ? 2 : 1 : 0)
                 <<",\"flags\":"<<plan.flags<<",\"source\":";
        print(plan.source); std::cout<<",\"destination\":"; print(plan.destination);
        std::cout<<",\"source_after\":"; print(source);
        std::cout<<",\"destination_after\":"; print(destination);
        std::cout<<",\"window_calls\":"<<plan.windowCall<<'}';
    }
    if(!input.eof()) throw std::runtime_error("Invalid argument corpus");
    std::cout<<"]\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n'; return 1;}
