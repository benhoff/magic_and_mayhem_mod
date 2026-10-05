#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "light_fixture.hpp"
#include <algorithm>
#include <sstream>
using namespace mnm::reconstruction;
static void put(Bytes& b,unsigned offset,unsigned value){std::memcpy(b.data()+offset,&value,4);}
int main(int argc,char** argv)try{
 if(argc!=9)throw std::runtime_error("Expected PE width height layers ambient ramp fixture prefix");map_image(read(argv[1]));
 const unsigned char expected[]={0x56,0x8b,0xf1,0xe8};if(std::memcmp(reinterpret_cast<void*>(0x4f27a0),expected,sizeof(expected)))throw std::runtime_error("Outer updater entry mismatch");
 const unsigned w=std::stoul(argv[2]),h=std::stoul(argv[3]),layers=std::stoul(argv[4]);TerrainLightingConfig config{std::stoi(argv[5]),std::stoi(argv[6])};
 const auto text=read(argv[7]);if(text.size()>1024*1024)throw std::runtime_error("Fixture too large");std::istringstream input(std::string(text.begin(),text.end()));const auto snapshots=mnm::preview::readLightingFixture(input);
 TerrainLightingCycle native(w,h,layers,config);const unsigned address=0x6c5490;const auto count=native.field().buffers()[0].size();Bytes object(0x66c0);std::array<Bytes,5> buffers;
 put(object,4,w);put(object,8,h);put(object,12,layers);put(object,16,w*h);put(object,0x7e0,count);
 for(unsigned i=0;i<5;++i){buffers[i].assign(count+32,0xa5);put(object,0x7cc+4*i,reinterpret_cast<std::uintptr_t>(buffers[i].data()+16));}
 for(unsigned y=0;y<h;++y)put(object,0x64b2+4*y,y*w);
 for(unsigned z=0;z<(layers+1)/2;++z)put(object,0x6432+4*z,z*w*h);
 std::memcpy(reinterpret_cast<void*>(address),object.data(),object.size());global(0x5e18e8,config.ambient);global(0x5e18ec,config.ramp);
 using Call=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Call>(0x4ecdd0)(reinterpret_cast<void*>(address));
 for(unsigned a:{0x6e8d90u,0x6e8d94u,0x6e8d98u,0x6e8d9cu,0x6e8da0u,0x6e8da4u,0x6e8da8u,0x6e8dacu})global(a,0);
 for(unsigned kind=0;kind<33;++kind)global(0x689498+12+kind*12,kind==32?0:kind+2);
 std::cout<<"{\"trace\":[";
 for(unsigned tick=0;tick<snapshots.size();++tick){const auto& s=snapshots[tick];native.step(s);
  Bytes creatures(s.creatures.size()*0xe4b+32,0xa5),objects(s.objects.size()*0x22e + 32,0xa5);
  for(unsigned i=0;i<s.creatures.size();++i){const auto& c=s.creatures[i];const unsigned at=16+i*0xe4b;put(creatures,at+4,c.active);put(creatures,at+0xe4,c.lightEnabled);put(creatures,at+0x174,c.owner);put(creatures,at+8,c.column);put(creatures,at+12,c.row);put(creatures,at+16,c.layer);}
  for(unsigned i=0;i<s.objects.size();++i){const auto& o=s.objects[i];const unsigned at=16+i*0x22e;put(objects,at+4,o.active);put(objects,at+0x28,o.lightIndex?o.lightIndex-2:32);put(objects,at+8,o.column);put(objects,at+12,o.row);put(objects,at+16,o.layer);put(objects,at+0x1c2,o.admissionColumn);put(objects,at+0x1c6,o.admissionRow);}
  global(0x6def58,reinterpret_cast<std::uintptr_t>(creatures.data()+16));global(0x6def5c,s.creatures.size());global(0x6df180,s.creatureScan);global(0x644520,s.player);
  global(0x689498,reinterpret_cast<std::uintptr_t>(objects.data()+16));global(0x68949c,s.objects.size());global(0x6898dc,s.objectScan);
  for(unsigned owner=0;owner<8;++owner)for(unsigned player=0;player<8;++player){*reinterpret_cast<unsigned char*>(0x643a80+owner*0x154+0x18+player)=s.relations.first[owner][player];*reinterpret_cast<unsigned char*>(0x643a80+owner*0x154+0x20+player)=s.relations.second[owner][player];}
  global(address+0x808,s.creaturesEnabled);global(address+0x80c,s.objectsEnabled);global(0x5e41a8,s.changed);
  global(0x689930+0x39,s.view.column);global(0x689930+0x3d,s.view.row);global(0x689930+0x59,s.view.extent);
  reinterpret_cast<Call>(0x4f27a0)(reinterpret_cast<void*>(address));
  const auto& c=native.creatures();const auto& o=native.objects();
  const std::array<unsigned,11> values{c.phase,c.remaining,c.quota,c.cursor,c.published,o.phase,o.remaining,o.quota,o.cursor,o.active,o.requested};
  const std::array<unsigned,11> fields{0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,address+0x642e,0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,address+0x642a,address+0x6426};
  const std::array<const char*,11> names{"creature_phase","creature_remaining","creature_quota","creature_cursor","published_flag","object_phase","object_remaining","object_quota","object_cursor","object_active","object_requested"};
  if(tick)std::cout<<',';std::cout<<"{\"tick\":"<<tick<<",\"changed_input\":"<<s.changed<<",\"changed_after\":"<<*reinterpret_cast<unsigned*>(0x5e41a8);
  if(*reinterpret_cast<unsigned*>(0x5e41a8))throw std::runtime_error("Changed flag not cleared");
  for(unsigned i=0;i<11;++i){if(values[i]!=*reinterpret_cast<unsigned*>(fields[i]))throw std::runtime_error("Lighting state differs tick/field "+std::to_string(tick)+"/"+std::to_string(i));std::cout<<",\""<<names[i]<<"\":"<<values[i];}std::cout<<'}';
  const auto prefix=std::string(argv[8])+"-"+std::to_string(tick);std::ofstream all(prefix+".buffers",std::ios::binary),published(prefix+".field",std::ios::binary);
  for(unsigned i=0;i<5;++i){if(std::memcmp(buffers[i].data()+16,native.field().buffers()[i].data(),count))throw std::runtime_error("Lighting bytes differ tick/buffer "+std::to_string(tick)+"/"+std::to_string(i));for(unsigned j=0;j<16;++j)if(buffers[i][j]!=0xa5 || buffers[i][count+16+j]!=0xa5)throw std::runtime_error("Light buffer guard changed");all.write(reinterpret_cast<const char*>(buffers[i].data()+16),count);}
  for(const auto* b:{&creatures,&objects})for(unsigned j=0;j<16;++j)if((*b)[j]!=0xa5 || (*b)[b->size()-16+j]!=0xa5)throw std::runtime_error("Source record guard changed");
  published.write(reinterpret_cast<const char*>(buffers[0].data()+16),count);if(!all || !published)throw std::runtime_error("Field output failed");
 }
 std::cout<<"],\"ticks\":"<<snapshots.size()<<",\"field_bytes\":"<<count<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
