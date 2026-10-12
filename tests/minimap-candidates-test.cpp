#include "../renderer/canvas_sequence.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
namespace {
void check(bool ok){if(!ok)throw std::runtime_error("Minimap candidate fixture");}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
}
int main() try {
 constexpr int width=800,height=600,stride=803;
 unsigned cases=0,refusals=0;std::size_t uniqueWords=0;
 Image base{width,height,{}};for(unsigned i=0;i<unsigned(width*height);++i)base.pixels.push_back((i*97+13)&65535);
 for(unsigned orientation=0;orientation<4;++orientation)for(unsigned kind=0;kind<4;++kind){
  MinimapOverlayView view{80,60,40,30,80,60,400,150,orientation,false};
  MinimapPlane history{width,height,stride,std::vector<std::uint16_t>(stride*height,0xeca8)};
  MinimapTerrain terrain{80,60,40,30,400,150,orientation,{},{}};
  for(unsigned i=0;i<80*60;++i){terrain.colours.push_back(i&1?0x1357:0xeca8);terrain.hidden.push_back(i%7==0);}
  auto draw=[&](MinimapPlane& p)->std::size_t {
   if(kind==0){drawMinimapTerrain(p,terrain,&history);return 0;}
   if(kind==1){std::array<std::uint16_t,9> palette{};palette[0]=0x1357;return drawMinimapCellMarker(p,view,{3,2,0,true},palette,true);}
   if(kind==2){const auto result=drawMinimapCreatureMarkers(p,view,{{3,2,false},{0,0,true},{3,2,false}},true);return result.drawn;}
   MinimapCameraOutline outline{800,600,{{{-1,-1},{1,-1},{-1,1},{1,1}}},true};
   const auto result=drawMinimapCameraOutline(p,view,outline);return result.originX+result.originY*stride;
  };
  // Independently enumerate changed destination words on a third background,
  // including padding, to check that every service commit emits a receipt.
  std::vector<std::size_t> writes;
  MinimapPlane probe{width,height,stride,std::vector<std::uint16_t>(stride*height,0x2468),&writes};draw(probe);
  std::sort(writes.begin(),writes.end());writes.erase(std::unique(writes.begin(),writes.end()),writes.end());
  std::vector<std::size_t> changed;for(std::size_t i=0;i<probe.words.size();++i)if(probe.words[i]!=0x2468)changed.push_back(i);
  check(writes==changed);uniqueWords+=writes.size();
  CanvasSequence reference,traced;reference.create(1,width,height);traced.create(1,width,height);
  reference.update(1,0,0,base);traced.update(1,0,0,base);
  check(reference.minimap(1,stride,draw)==traced.minimap(1,stride,draw,true));
  check(reference.read(1).pixels==traced.read(1).pixels);++cases;
 }
 // Repeated mixed extents share scratch storage. Compare against fresh generic
 // dual-background evaluation, including sentinels, duplicate and padding writes.
 CanvasSequence reusable, fresh;
 reusable.create(1,width,height); fresh.create(1,width,height);
 reusable.update(1,0,0,base); fresh.update(1,0,0,base);
 const Image small{8,6,std::vector<std::uint32_t>(48,0x2468)};
 reusable.create(2,8,6); fresh.create(2,8,6);
 reusable.update(2,0,0,small); fresh.update(2,0,0,small);
 unsigned recycled=0;
 for(unsigned cycle=0;cycle<40;++cycle) {
  const unsigned id=cycle%2?2:1;const int w=id==1?width:8,h=id==1?height:6,pitch=w+int(cycle%3)+1;
  const auto draw=[&](MinimapPlane& p)->std::size_t {
   const auto sentinel=p.words.front();
   check(sentinel==0x1357 || sentinel==0xeca8);
   check(std::all_of(p.words.begin(),p.words.end(),[sentinel](auto v){return v==sentinel;}));
   const auto visible=std::size_t(h-2)*pitch+(cycle%unsigned(w));
   const auto padding=std::size_t(h-1)*pitch+w;
   const auto colour=std::uint16_t(cycle%3==0?0x1357:cycle%3==1?0xeca8:cycle*19);
   for(auto at:{visible,visible,padding}) {
    if(p.writeCandidates)p.writeCandidates->push_back(at);
    p.words[at]=colour;
   }
   return visible;
  };
  check(reusable.minimap(id,pitch,draw,true)==fresh.minimap(id,pitch,draw));
  check(reusable.read(id).pixels==fresh.read(id).pixels);++recycled;
 }
 const auto prior=reusable.read(1).pixels;
 refuses([&]{reusable.minimap(1,stride,[](MinimapPlane& p)->std::size_t{
  p.writeCandidates->push_back(3);p.words[3]=0x7777;throw std::runtime_error("after admitted scratch write");
 },true);});++refusals;
 check(reusable.read(1).pixels==prior);
 reusable.minimap(1,stride,[](MinimapPlane& p){
  check(std::all_of(p.words.begin(),p.words.end(),[s=p.words.front()](auto v){return v==s;}));return 0;
 },true);
 CanvasSequence canvas;canvas.create(1,width,height);canvas.update(1,0,0,base);
 const auto before=canvas.read(1);
 // Untraced arbitrary callbacks continue to use the generic complete scan.
 canvas.minimap(1,stride,[](MinimapPlane& p){p.words[17*stride+5]=0x1357;return 0;});
 auto expected=base;expected.pixels[17*width+5]=0x1357;check(canvas.read(1).pixels==expected.pixels);
 refuses([&]{canvas.minimap(1,stride,[](MinimapPlane& p){p.words[3]=0x1357;p.writeCandidates->push_back(3);p.writeCandidates->push_back(p.words.size());return 0;},true);});++refusals;
 check(canvas.read(1).pixels==expected.pixels);
 refuses([&]{canvas.minimap(1,stride,[](MinimapPlane& p){return p.words[0];},true);});++refusals;
 check(canvas.read(1).pixels==expected.pixels);
 CanvasSequence undefined;undefined.create(1,width,height);
 undefined.minimap(1,stride,[](MinimapPlane& p){return drawMinimapCellMarker(p,{80,60,40,30,80,60,400,150,0,false},{3,2,0,false},{},false);},true);
 refuses([&]{undefined.read(1);});++refusals;
 std::vector<std::size_t> writes;MinimapPlane plane{width,height,stride,std::vector<std::uint16_t>(stride*height,0x2468),&writes};
 const auto unchanged=plane.words;MinimapCameraOutline invalid{800,600,{{{-1,-1},{1,-1},{-1,1},{0,1}}},true};
 refuses([&]{drawMinimapCameraOutline(plane,{80,60,40,30,80,60,400,150,0,false},invalid);});++refusals;
 check(plane.words==unchanged && writes.empty());
 check(uniqueWords<unsigned(width*height));check(before.pixels==base.pixels);
 std::cout<<"{\"cases\":"<<cases<<",\"compared_words\":"<<cases*std::size_t(width)*height<<",\"unique_service_words\":"<<uniqueWords<<",\"recycled_cases\":"<<recycled<<",\"throw_recovery\":true,\"atomic_and_undefined_refusals\":"<<refusals<<",\"generic_callback_preserved\":true,\"sentinel_equal_writes\":true}"<<std::endl;
 return 0;
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
