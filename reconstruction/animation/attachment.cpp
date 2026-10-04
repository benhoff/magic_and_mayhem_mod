#include "attachment.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
ModeOneSelection modeOneSelection(std::uint32_t base,std::uint32_t assetIndex,std::uint32_t facing){
    if(facing>7 || base>std::numeric_limits<std::uint32_t>::max()-facing)
        throw std::runtime_error("Mode-one attachment sequence outside native bounds");
    return {assetIndex,base+facing};
}
bool modeOneVisible(std::int32_t health,std::uint32_t mode){return health!=0 && mode==1;}
bool modeOneTicks(std::uint32_t mode){return mode==1;}
}
