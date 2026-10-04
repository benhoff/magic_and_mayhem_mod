#include "sprite_queue.hpp"
#include <stdexcept>
#include <limits>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Queue expectation failed");}
int main(){
    require(spriteDepthKey({10,20,499,6},0)==445);
    require(spriteDepthKey({10,20,499,6},1)==425);
    require(spriteDepthKey({10,20,500,6},1)==446);
    require(spriteDepthKey({10,20,499,6},2)==385);
    require(spriteDepthKey({10,20,499,6},3)==405);
    require(spriteDepthKey({std::numeric_limits<std::int32_t>::max(),1,0,0},0)==std::numeric_limits<std::int32_t>::min());
    std::vector<SpriteQueueEntry> a{{0,0},{0,1},{0,2},{0,3}};sortSpriteQueue(a);
    require(a[0].payload==1 && a[1].payload==0 && a[2].payload==2 && a[3].payload==3);
    for(auto bad: {SpriteDepth{0,0,-1,0}}){bool caught=false;try{spriteDepthKey(bad,0);}catch(const std::invalid_argument&){caught=true;}require(caught);}
    bool caught=false;try{spriteDepthKey({},4);}catch(const std::invalid_argument&){caught=true;}require(caught);
}
