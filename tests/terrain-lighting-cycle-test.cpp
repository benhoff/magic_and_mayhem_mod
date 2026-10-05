#include "light_fixture.hpp"
#include <sstream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Lighting cycle assertion failed");}
static std::string header(){std::string s="MNM_LIGHTING 1 relations ";for(unsigned i=0;i<128;++i)s+="0 ";return s;}
int main(){
 const auto decode=[](std::string text){std::istringstream in(text);return mnm::preview::readLightingFixture(in);};
 auto snapshots=decode(header()+"tick 0 1 1 1 8 8 16 1 1 1 1 creature 1 1 0 8 8 0 object 1 16 8 8 0 8 8 end");
 require(snapshots.size()==1 && snapshots[0].objects.size()==1 && snapshots[0].creatures.size()==1);
 TerrainLightingCycle cycle(32,32,3,{-50,7});cycle.step(snapshots[0]);require(cycle.ticks()==1 && cycle.creatures().phase==0 && cycle.objects().phase==0 && cycle.field().at(8,8,0)==0);
 const auto old=cycle.field().buffers();const auto state=cycle.creatures();const auto staticState=cycle.objects();
 auto invalid=snapshots[0];invalid.objects[0].layer=3;bool refused=false;
 try{cycle.step(invalid);}catch(const std::invalid_argument&){refused=true;}
 require(refused && cycle.ticks()==1 && cycle.field().buffers()==old && cycle.creatures().cursor==state.cursor && cycle.objects().cursor==staticState.cursor);
 auto copy=cycle;copy.step(snapshots[0]);require(copy.ticks()==2 && cycle.ticks()==1 && cycle.field().buffers()==old);
 for(const auto& body:{"end trailing", "tick 0 0 1 1 0 0 4 0 1 0 0 end", "tick 0 0 1 1 0 0 4 0 0 0 1 object 1 1 0 0 0 0 0 end", "tick 0 0 1 1 0 0 4 1 0 1 0 creature 1 1 -1 0 0 0 end", "tick 0 0 2 1 0 0 4 0 0 0 0 end", "tick 0 0 1 1 0 0 4 0 0 0 0", "tick 0 0 1 1 0 0 4 0 0 0 0 end extra", "tick 0 0 1 1 0 0 4.0 0 0 0 0 end"}){
  refused=false;try{decode(header()+body);}catch(const std::invalid_argument&){refused=true;}require(refused);
 }
 snapshots=decode(header()+"tick 0 1 1 0 8 8 16 1 0 1 0 creature 1 1 0 8 8 0 tick 0 0 0 1 8 8 16 0 0 0 0 tick 0 0 0 1 8 8 16 0 0 0 0 end");
 TerrainLightingCycle staged(32,32,3,{-50,7});staged.step(snapshots[0]);require(staged.field().at(8,8,0)==-127 && !staged.creatures().published);
 staged.step(snapshots[1]);require(staged.field().at(8,8,0)==-127);staged.step(snapshots[2]);require(staged.field().at(8,8,0)==-8);
}
