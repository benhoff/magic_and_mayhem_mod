#include "creature_occupancy.hpp"
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Creature footprint assertion failed");}
int main(){
    CreatureOccupancy value;value.height=2;
    for(unsigned row=0;row<16;++row)for(unsigned col=0;col<16;++col){
        value.rows.fill(0);value.rows[row]=std::uint16_t(0x8000u>>col);
        // Independent rectangular fine-coordinate oracle for each authored bit.
        for(int y=0;y<64;++y)for(int x=0;x<64;++x){
            const bool inside=x>=int(col*4) && x<int(col*4+4) && y>=int(row*4) && y<int(row*4+4);
            require(value.occupied(x,y,0)==inside && value.occupied(x,y,31)==inside);
            require(!value.occupied(x,y,-1) && !value.occupied(x,y,32));
        }
    }
    value.rows.fill(0xffff);
    for(int n:{-1,64,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()})
        require(!value.occupied(n,0,0) && !value.occupied(0,n,0));
    value.height=0;require(!value.occupied(0,0,0));
    value.height=0x10000000u;require(!value.occupied(0,0,0));
    value.height=0x10000001u;require(value.occupied(0,0,15) && !value.occupied(0,0,16));
    value.height=0xffffffffu;require(value.occupied(63,63,std::numeric_limits<std::int32_t>::max()));
    require(!value.occupied(63,63,std::numeric_limits<std::int32_t>::min()));
}
