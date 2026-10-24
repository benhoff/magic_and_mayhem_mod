#include "sprite_queue.hpp"
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <numeric>
#include <random>
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
    // Admission failure must preserve every payload and key.
    std::vector<SpriteQueueEntry> oversized(65537,{19,73});
    caught=false;try{sortSpriteQueue(oversized);}catch(const std::length_error&){caught=true;}
    require(caught && std::all_of(oversized.begin(),oversized.end(),[](auto e){return e.key==19 && e.payload==73;}));
    // Maximum admitted queue: independent sorted-key and payload-permutation oracles.
    std::mt19937 random(1803);std::vector<SpriteQueueEntry> large;std::vector<std::int32_t> expected;
    for(unsigned i=0;i<65536;++i){auto key=std::int32_t(random());large.push_back({key,i});expected.push_back(key);}
    const auto original=large;std::sort(expected.begin(),expected.end());sortSpriteQueue(large);
    std::vector<unsigned> payloads;
    for(unsigned i=0;i<large.size();++i){require(large[i].key==expected[i] && large[i].key==original[large[i].payload].key);payloads.push_back(large[i].payload);}
    std::sort(payloads.begin(),payloads.end());for(unsigned i=0;i<payloads.size();++i)require(payloads[i]==i);
}
