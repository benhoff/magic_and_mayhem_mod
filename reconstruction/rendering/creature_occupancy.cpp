#include "creature_occupancy.hpp"
namespace mnm::reconstruction {
bool CreatureOccupancy::occupied(std::int32_t x,std::int32_t y,std::int32_t z) const noexcept {
    if(x<0 || x>=64 || y<0 || y>=64 || z<0 || std::uint32_t(z)>=(height<<4))return false;
    return (rows[unsigned(y)>>2] & (0x8000u>>(unsigned(x)>>2)))!=0;
}
}
