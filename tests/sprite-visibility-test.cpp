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
    // A malformed earlier record is discovered after later covering records in
    // reverse order. Native admission must precede grid/kind/owner mutation.
    for(bool expanded:{false,true}){
        SpriteVisibilityGrid isolated(expanded);SpriteVisibilityEntry valid{shape,100,100,0,1,0};
        std::vector<SpriteVisibilityEntry> entries(4,valid);entries[1].shape->coverRows.clear();
        std::vector<VisibilityOwner> flags{{1,2,3,0xffff,0xffff}};const auto before=isolated.data();
        caught=false;try{applySpriteVisibility(entries,isolated,flags);}catch(const std::invalid_argument&){caught=true;}
        require(caught && isolated.data()==before && flags[0].flags8==0xffff && flags[0].flags10==0xffff);
        for(const auto& entry:entries)require(entry.kind==0);
        entries[1]=valid;entries[1].shape->testRows.resize(255);entries[1].shape->coverRows.resize(255);
        caught=false;try{applySpriteVisibility(entries,isolated,flags);}catch(const std::out_of_range&){caught=true;}
        require(caught && isolated.data()==before && flags[0].flags10==0xffff);
        for(const auto& entry:entries)require(entry.kind==0);
        entries[1]=valid;entries[0].shape->coverRows.clear(); // entry zero is deliberately skipped
        applySpriteVisibility(entries,isolated,flags);require(entries[0].kind==0 && entries[1].kind==-2);
    }
}
