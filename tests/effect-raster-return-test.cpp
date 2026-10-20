#include "../reconstruction/rendering/effect_raster_return.hpp"
#include <stdexcept>
int main(){
    for(unsigned backend:{7u,13u,14u,15u,16u})
        if(mnm::reconstruction::effectRasterReturn(backend))throw std::runtime_error("Effect AX");
    for(unsigned backend:{0u,6u,8u,12u,17u,~0u}){
        bool refused=false;try{mnm::reconstruction::effectRasterReturn(backend);}catch(const std::invalid_argument&){refused=true;}
        if(!refused)throw std::runtime_error("Unknown return domain admitted");
    }
}
