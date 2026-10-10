#include "../compat/legacy/canvas_producers.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm;
namespace {
void check(bool ok) { if (!ok) throw std::runtime_error("Owned minimap producer assertion"); }
void put(std::vector<std::uint8_t> &b, unsigned i, std::uint32_t v) {
  for (unsigned k=0;k<4;++k) b.at(i+k)=std::uint8_t(v>>(8*k));
}
template<class F> void refuses(F f) { bool failed=false;try {f();}catch(const std::exception &){failed=true;}check(failed); }
legacy::CanvasProducer packet(unsigned view,bool rgb555,unsigned kind) {
  legacy::CanvasProducer c{};c.fields[2]=24;c.fields[3]=1;c.fields[5]=80;c.fields[6]=80;c.fields[7]=83;c.payload.resize(148);
  std::uint32_t m[37]={kind,8,6,4,3,16,24,30,12,view,unsigned(rgb555),1,1,3,2,4,1,0,128,64};
  for(unsigned i=0;i<4;++i){m[20+i*2]=(i&1)?1:unsigned(-1);m[21+i*2]=(i&2)?1:unsigned(-1);}
  for(unsigned i=0;i<9;++i)m[28+i]=0x1000+i*351;
  for(unsigned i=0;i<37;++i)put(c.payload,i*4,m[i]);
  c.fields[17]=kind==2?view:0;
  c.fields[18]=c.payload.size();c.fields[0]=96+c.fields[18];return c;
}
}
int main() try {
 unsigned cases=0,refusals=0;
 for(unsigned view=0;view<4;++view)for(bool rgb555:{false,true})for(unsigned kind=0;kind<3;++kind){
  legacy::CanvasProducerReplay replay([](const std::string&)->std::vector<std::uint8_t>{throw std::runtime_error("Unexpected source read");});
  legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=1;create.fields[5]=80;create.fields[6]=80;replay.apply(create);
  legacy::CanvasProducer fill{};fill.fields[2]=5;fill.fields[3]=1;fill.fields[12]=80;fill.fields[13]=80;fill.fields[14]=0x1357;replay.apply(fill);
  auto c=packet(view,rgb555,kind);if(kind==2)put(c.payload,36,(4-view)%4);
  render::MinimapPlane expected{80,80,83,std::vector<std::uint16_t>(83*80,0x1357)};
  render::MinimapOverlayView v{8,6,4,3,16,24,30,12,view,rgb555};
  if(kind==0){std::array<std::uint16_t,9> palette{};for(unsigned i=0;i<9;++i)palette[i]=0x1000+i*351;c.fields[21]=render::drawMinimapCellMarker(expected,v,{3,2,4,true},palette,true);}
  if(kind==1){put(c.payload,68,3);c.payload.resize(184);const std::vector<render::MinimapCreatureMarker> markers={{3,2,false},{0,0,true},{3,2,false}};for(unsigned i=0;i<3;++i){put(c.payload,148+i*12,markers[i].x);put(c.payload,152+i*12,markers[i].y);put(c.payload,156+i*12,markers[i].hidden);}c.fields[18]=184;c.fields[0]=280;const auto result=render::drawMinimapCreatureMarkers(expected,v,markers,true);c.fields[19]=result.gridWidth;c.fields[20]=result.gridHeight;}
  if(kind==2){render::MinimapCameraOutline o{128,64,{},true};for(unsigned i=0;i<4;++i)o.corners[i]={(i&1)?1:-1,(i&2)?1:-1};const auto result=render::drawMinimapCameraOutline(expected,v,o);c.fields[19]=result.originX;c.fields[20]=result.originY;}
  // Result mismatches must refuse before committing native canvas writes.
  auto bad=c;if(kind==0)++bad.fields[21];else ++bad.fields[19];const auto before=replay.read(1);refuses([&]{replay.apply(bad);});check(replay.read(1).pixels==before.pixels);++refusals;
  replay.apply(c);const auto actual=replay.read(1);for(unsigned y=0;y<80;++y)for(unsigned x=0;x<80;++x)check(actual.pixels[y*80+x]==expected.words[y*83+x]);++cases;
 }
 render::CanvasSequence undefined;undefined.create(1,80,80);refuses([&]{undefined.read(1);});++refusals;
 undefined.minimap(1,83,[](render::MinimapPlane &p){return render::drawMinimapCellMarker(p,{8,6,4,3,16,24,30,12,0,false},{3,2,0,false},{},false);});refuses([&]{undefined.read(1);});++refusals;
 std::vector<std::uint8_t> wire(64);std::copy_n("MNMPRO02",8,wire.begin());put(wire,8,2);put(wire,12,64);put(wire,16,0x40209ca7);put(wire,20,65536);put(wire,24,1);
 auto c=packet(0,false,0);c.fields[1]=1;for(unsigned k=0;k<24;++k){wire.resize(64+96);put(wire,64+k*4,c.fields[k]);}wire.insert(wire.end(),c.payload.begin(),c.payload.end());check(legacy::decodeCanvasProducers(wire,false).operations.size()==1);
 legacy::CanvasProducerStream incremental{};legacy::appendCanvasProducers(incremental,wire);auto changedVersion=std::vector<std::uint8_t>(wire.begin(),wire.begin()+64);std::copy_n("MNMPRO01",8,changedVersion.begin());put(changedVersion,8,1);refuses([&]{legacy::appendCanvasProducers(incremental,changedVersion);});++refusals;
 auto larger=wire;for(unsigned budget:{131072u,262144u}){put(larger,20,budget);check(legacy::decodeCanvasProducers(larger,false).operations.size()==1);}auto headerOnly=std::vector<std::uint8_t>(larger.begin(),larger.begin()+64);std::copy_n("MNMPRO01",8,headerOnly.begin());put(headerOnly,8,1);refuses([&]{legacy::decodeCanvasProducers(headerOnly,false);});++refusals;
 auto invalidOutline=wire;put(invalidOutline,64+17*4,4);put(invalidOutline,160,2);refuses([&]{legacy::decodeCanvasProducers(invalidOutline,false);});++refusals;
 auto old=wire;std::copy_n("MNMPRO01",8,old.begin());put(old,8,1);refuses([&]{legacy::decodeCanvasProducers(old,false);});++refusals;
 for(unsigned off:{64u+17u*4u,64u+18u*4u,160u,160u+9u*4u,160u+10u*4u,160u+17u*4u}){auto bad=wire;put(bad,off,0xffffffff);refuses([&]{legacy::decodeCanvasProducers(bad,false);});++refusals;}
 std::cout<<cases<<" owned four-view adapter cases, "<<refusals<<" result/undefined/wire refusals passed\n";return 0;
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
