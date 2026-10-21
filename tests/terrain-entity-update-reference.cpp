// Whole unchanged frame/local geometry helpers and actual owned XY occupancy.
#define main previous_camera_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
#include "terrain_entity_updates.hpp"
using namespace mnm;
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));static_assert(sizeof(assets::MapCell)==12);
 const unsigned char writer[]={0xa1,0x08,0x66,0x65,0};require(!std::memcmp(reinterpret_cast<void*>(0x4eb710),writer,sizeof(writer)));
 using Refresh=void(__attribute__((thiscall)) *)(void*,void*);using Frame=void(__attribute__((thiscall)) *)(void*,unsigned);
 std::mt19937 rng(0x4ed440);unsigned localChecks=0,frameChecks=0,accepted=0,refused=0;std::uint64_t cellsChecked=0;
 for(unsigned trial=0;trial<4096;++trial){
  assets::MapAsset source;source.width=1+rng()%9;source.height=1+rng()%9;source.layers=1+rng()%4;source.cells.resize(source.width*source.height*source.layers);
  for(auto& cell:source.cells){cell.definition=rng()%8;for(auto& reference:cell.references)reference=rng()%3?0xffff:std::uint16_t(rng()%100);cell.flags8=std::uint16_t(rng());cell.flags10=std::uint16_t(rng());if(trial%8==0){cell.definition=0;cell.references={0xffff,0xffff,0xffff};cell.flags10&=0xdfff;}}
  assets::TerrainCatalog catalog;catalog.records.resize(8);for(auto& record:catalog.records){record[0xa8]=std::uint8_t(rng());record[0xb0]=std::uint8_t(rng());put(record.data(),0x94,std::array<unsigned,3>{{0,7,16}}[rng()%3]);}
  std::array<unsigned,4> header{{0,0,0,8}};put(reinterpret_cast<void*>(0x656608),0,reinterpret_cast<std::uintptr_t>(header.data()));put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(catalog.records.data()));
  std::vector<unsigned char> occupancy(source.width*source.height*6);std::vector<std::uint8_t> occupied(source.cells.size());
  for(unsigned xy=0;xy<source.width*source.height;++xy){const unsigned count=rng()%3,a=rng()%source.layers,b=rng()%source.layers;occupancy[xy*6]=count;occupancy[xy*6+2]=a;occupancy[xy*6+3]=b;for(unsigned z=0;z<source.layers;++z)occupied[z*source.width*source.height+xy]=count&&(z==a||z==b);}
  put(reinterpret_cast<void*>(0x6a49c0),0,reinterpret_cast<std::uintptr_t>(occupancy.data()));const auto index=rng()%source.cells.size();
  for(unsigned operation=0;operation<13;++operation){
   // Operation0 is standalone local geometry. The rest cover every valid
   // frame plus successful out-of-count no-ops; one-past stays native-only.
   const auto requested=operation?operation-1:0;if(operation&&requested==8)continue;
   auto native=source;std::vector<unsigned char> raw(source.cells.size()*12+32,0xa5);std::memcpy(raw.data()+16,source.cells.data(),source.cells.size()*12);const auto guards=raw;auto* receiver=reinterpret_cast<void*>(0x6c5490);std::memset(receiver,0xa5,0x66d0);put(receiver,4,source.width);put(receiver,8,source.height);put(receiver,12,source.layers);put(receiver,16,source.width*source.height);put(receiver,0x4c,reinterpret_cast<std::uintptr_t>(raw.data()+16));
   for(unsigned z=0;z<source.layers;++z)put(receiver,0x6432+z*4,z*source.width*source.height);for(unsigned y=0;y<source.height;++y)put(receiver,0x64b2+y*4,y*source.width);const unsigned wasDirty=(trial+operation)%2;put(receiver,0x66ca,wasDirty);std::array<unsigned char,0x66d0> expectedHeader;std::memcpy(expectedHeader.data(),receiver,expectedHeader.size());bool dirty=false;
   if(!operation){dirty=reconstruction::refreshTerrainCellNeighborhood(native,index,catalog,occupied);reinterpret_cast<Refresh>(0x4ed440)(receiver,raw.data()+16+index*12);++localChecks;}
   else{const auto result=reconstruction::setTerrainLinkedFrame(native,index,catalog,requested,occupied);dirty=result.redrawDirty;accepted+=result.accepted;refused+=!result.accepted;reinterpret_cast<Frame>(0x4eb710)(raw.data()+16+index*12,requested);++frameChecks;}
   put(expectedHeader.data(),0x66ca,wasDirty||dirty);require(!std::memcmp(expectedHeader.data(),receiver,expectedHeader.size()));require(!std::memcmp(native.cells.data(),raw.data()+16,native.cells.size()*12));require(std::equal(raw.begin(),raw.begin()+16,guards.begin())&&std::equal(raw.end()-16,raw.end(),guards.end()-16));cellsChecked+=native.cells.size();
  }
 }
 std::cout<<"{\"success\":true,\"local_refreshes\":"<<localChecks<<",\"frame_writes\":"<<frameChecks<<",\"accepted\":"<<accepted<<",\"ignored\":"<<refused<<",\"cells_compared\":"<<cellsChecked<<",\"original_occupancy_helper_executed\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
