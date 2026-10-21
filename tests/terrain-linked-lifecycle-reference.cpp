// Whole unchanged frame/local geometry helpers and actual owned XY occupancy.
#define main previous_camera_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
#include "terrain_linked_lifecycle.hpp"
#include <cstdlib>
#include <optional>
using namespace mnm;
static std::vector<std::array<unsigned,3>> requests;
static mnm::assets::MapCell* receivers=nullptr;
static unsigned receiverCount=0,freeCalls=0;
static void __attribute__((thiscall)) frameRequest(void* receiver,unsigned frame){
 auto* c=static_cast<mnm::assets::MapCell*>(receiver);const auto i=std::uintptr_t(c)-std::uintptr_t(receivers);
 if(i%12||i/12>=receiverCount)std::abort();requests.push_back({unsigned(i/12),frame,c->flags10});
}
static void freeNode(void* node){if(get(node,8))std::abort();++freeCalls;}
static void adapter(unsigned address,std::uintptr_t callback){unsigned char bytes[]={0x68,0,0,0,0,0xc3};std::memcpy(bytes+1,&callback,4);std::memcpy(reinterpret_cast<void*>(address),bytes,6);}
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));static_assert(sizeof(assets::MapCell)==12);
 const unsigned char writer[]={0xa1,0x08,0x66,0x65,0};require(!std::memcmp(reinterpret_cast<void*>(0x4eb820),writer,sizeof(writer)));
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
   else{const auto result=reconstruction::setTerrainAnimationFrame(native,index,catalog,requested,occupied);dirty=result.redrawDirty;accepted+=result.accepted;refused+=!result.accepted;reinterpret_cast<Frame>(0x4eb820)(raw.data()+16+index*12,requested);++frameChecks;}
   put(expectedHeader.data(),0x66ca,wasDirty||dirty);require(!std::memcmp(expectedHeader.data(),receiver,expectedHeader.size()));require(!std::memcmp(native.cells.data(),raw.data()+16,native.cells.size()*12));require(std::equal(raw.begin(),raw.begin()+16,guards.begin())&&std::equal(raw.end()-16,raw.end(),guards.end()-16));cellsChecked+=native.cells.size();
  }
 }
 using Terminal=int(__attribute__((thiscall)) *)(void*);
 unsigned terminals=0,tickCalls=0,dispatches=0,retirements=0;
 for(unsigned trial=0;trial<512;++trial){
  std::vector<assets::AnimationRecord> records;for(unsigned i=0;i<16;++i)records.push_back({i%4?0u:1u,int(rng()),{}});records.push_back({6,0,{}});
  unsigned offsets[]={0,unsigned(records.size())};unsigned asset[]={0,unsigned(reinterpret_cast<std::uintptr_t>(offsets)),unsigned(reinterpret_cast<std::uintptr_t>(records.data()))};
  for(unsigned reverse=0;reverse<2;++reverse)for(unsigned pc=reverse?0:1;pc<(reverse?records.size()-1:records.size()+1);++pc){
   reconstruction::AnimationControllerState state;state.reverse=reverse;state.forward.pc=pc;unsigned raw[12]={};raw[0]=0;raw[6]=unsigned(reinterpret_cast<std::uintptr_t>(records.data()+pc));raw[7]=unsigned(reinterpret_cast<std::uintptr_t>(asset));raw[9]=reverse;
   require(reinterpret_cast<Terminal>(0x4650d0)(raw)==reconstruction::terrainLinkedTerminalFrame(records,state));++terminals;
  }
 }
 const unsigned char freeEntry[]={0x8b,0x44,0x24,0x04,0x85,0xc0};require(!std::memcmp(reinterpret_cast<void*>(0x597870),freeEntry,sizeof(freeEntry)));require(!std::memcmp(reinterpret_cast<void*>(0x4eb820),writer,sizeof(writer)));adapter(0x4eb820,reinterpret_cast<std::uintptr_t>(&frameRequest));adapter(0x597870,reinterpret_cast<std::uintptr_t>(&freeNode));
 using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);using Tick=void(__attribute__((thiscall)) *)(void*);
 const unsigned char loopEntry[]={0x56,0x57,0x8b,0xf9};require(!std::memcmp(reinterpret_cast<void*>(0x4f2e20),loopEntry,sizeof(loopEntry)));
 for(unsigned trial=0;trial<256;++trial){
  std::vector<assets::AnimationRecord> records={{1,int(trial%4),{}}};for(unsigned i=0;i<1+trial%8;++i){records.push_back({0,int(i),{}});if(i==2&&trial%5==0)records.push_back({5,1,{}});}if(trial%4==0)records.push_back({4,-int(records.size()),{}});records.push_back({6,0,{}});
  unsigned offsets[]={0,unsigned(records.size())};unsigned asset[]={0,unsigned(reinterpret_cast<std::uintptr_t>(offsets)),unsigned(reinterpret_cast<std::uintptr_t>(records.data()))};
  const unsigned count=trial%7;std::array<std::array<unsigned,15>,6> nodes{};std::array<unsigned,2> head{};std::array<assets::MapCell,6> cells{};std::vector<std::optional<reconstruction::NoCdAnimationController>> native;std::vector<std::uint16_t> flags;
  const auto pointer=[](const void* p){return unsigned(reinterpret_cast<std::uintptr_t>(p));};head[0]=count?pointer(nodes[0].data()):pointer(head.data());head[1]=count?pointer(nodes[count-1].data()):pointer(head.data());
  for(unsigned i=0;i<count;++i){auto& node=nodes[i];node[0]=i+1<count?pointer(nodes[i+1].data()):pointer(head.data());node[1]=i?pointer(nodes[i-1].data()):pointer(head.data());node[2]=pointer(&cells[i]);node[10]=pointer(asset);cells[i].flags10=std::uint16_t(rng()|8);flags.push_back(cells[i].flags10);reinterpret_cast<Start>(0x464cb0)(node.data()+3,0,0);native.emplace_back(std::in_place,records);native.back()->start(false);}
  std::array<unsigned char,0x7f0> receiver{};put(receiver.data(),0x7e8,pointer(head.data()));put(receiver.data(),0x7ec,count);receivers=cells.data();receiverCount=count;freeCalls=0;unsigned expectedFree=0;
  for(unsigned tick=0;tick<64;++tick){std::vector<std::array<unsigned,3>> expected;
   for(unsigned i=0;i<count;++i)if(native[i]){const auto event=native[i]->tick();const auto frame=event==1?unsigned(reconstruction::terrainLinkedTerminalFrame(records,native[i]->state())):native[i]->sprite().value();if(event==1){flags[i]&=0xfff7;native[i].reset();++expectedFree;}expected.push_back({i,frame,flags[i]});}
   requests.clear();reinterpret_cast<Tick>(0x4f2e20)(receiver.data());require(requests==expected&&freeCalls==expectedFree&&get(receiver.data(),0x7ec)==count-expectedFree);for(unsigned i=0;i<count;++i)require(cells[i].flags10==flags[i]);++tickCalls;dispatches+=requests.size();
  }
  retirements+=expectedFree;
 }
 std::cout<<"{\"success\":true,\"local_refreshes\":"<<localChecks<<",\"fast_frame_writes\":"<<frameChecks<<",\"accepted\":"<<accepted<<",\"ignored\":"<<refused<<",\"cells_compared\":"<<cellsChecked<<",\"terminal_frame_scans\":"<<terminals<<",\"linked_tick_calls\":"<<tickCalls<<",\"frame_requests\":"<<dispatches<<",\"retired_nodes\":"<<retirements<<",\"original_controller_executed\":true,\"private_frame_and_free_adapters\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
