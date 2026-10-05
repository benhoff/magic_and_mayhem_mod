#include "creature_footprint.hpp"
#include <cstdlib>
using namespace mnm::reconstruction;
int main() {
    for (auto selector : {0u, 1u, 2u, 0x80000000u, 0xffffffffu}) {
        auto value = initializeCreatureFootprint(2, selector);
        // Independent geometric oracle: centered 12x12 square or eight-row diamond.
        const unsigned widths[] = {2,4,6,8,8,6,4,2};
        for (int y = 0; y < 64; ++y) for (int x = 0; x < 64; ++x) {
            unsigned bx = unsigned(x)/4, by = unsigned(y)/4;
            bool inside = selector ? bx>=2 && bx<14 && by>=2 && by<14
                : by<8 && bx >= (8-widths[by])/2 && bx < (8+widths[by])/2;
            if (value.occupied(x,y,0)!=inside || value.occupied(x,y,31)!=inside ||
                value.occupied(x,y,32) || value.occupied(x,y,-1)) std::abort();
        }
    }
    for (auto height : {0u, 1u, 0x10000000u, 0xffffffffu})
        if (initializeCreatureFootprint(height,0).height!=height) std::abort();
}
