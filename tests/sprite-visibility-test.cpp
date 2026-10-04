#include "sprite_visibility.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
void require(bool ok){if(!ok)throw std::runtime_error("Visibility expectation");}
int main(){
    SpriteVisibilityShape shape{0,0,{0x80000000},{0x80000000}};
    SpriteVisibilityEntry draw{shape,0,0,0,1,{}};SpriteVisibilityGrid grid;
    require(!grid.testAndCover(draw));require(grid.testAndCover(draw));
    grid.clear();draw.kind=2;require(!grid.testAndCover(draw));require(!grid.testAndCover(draw));
    draw.kind=1;require(!grid.testAndCover(draw));draw.kind=0;
    std::vector<SpriteVisibilityEntry> queue(3,draw);std::vector<VisibilityOwner> owners;
    applySpriteVisibility(queue,grid,owners);require(queue[0].kind==0 && queue[1].kind==-2 && queue[2].kind==0);
    bool caught=false;try{decodeSpriteVisibility({32,2,0,0},{0,0,0,0},0,0);}catch(const std::invalid_argument&){caught=true;}require(caught);
    draw.shape->testRows.resize(256);caught=false;try{grid.testAndCover(draw);}catch(const std::invalid_argument&){caught=true;}require(caught);
}
