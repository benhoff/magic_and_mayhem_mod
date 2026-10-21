// Whole unchanged PE32 cell-position and optional actor-follow helpers.
#define main camera_previous_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
static void positions(const void* p,const mnm::reconstruction::TerrainCamera& c){
 for(const auto f:std::array<std::pair<unsigned,int>,6>{{{0x39,c.column},{0x3d,c.row},{0x41,c.f41},{0x45,c.f45},{0x49,c.f49},{0x4d,c.f4d}}})
  if(get(p,f.first)!=unsigned(f.second))throw std::runtime_error("Follow position mismatch at "+std::to_string(f.first));
}
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));
 const unsigned char cellBytes[]={0x8b,0x44,0x24,0x0c,0x8b,0x54,0x24,0x08};
 const unsigned char followBytes[]={0x57,0x8b,0x7c,0x24,0x08,0x85,0xff};
 require(!std::memcmp(reinterpret_cast<void*>(0x4f7a00),cellBytes,sizeof(cellBytes)));
 require(!std::memcmp(reinterpret_cast<void*>(0x4f7ae0),followBytes,sizeof(followBytes)));
 using Cell=void(__attribute__((thiscall)) *)(void*,int,int,int);
 using Follow=void(__attribute__((thiscall)) *)(void*,void*);
 unsigned follows=0,cells=0,nulls=0;
 std::mt19937 rng(0x4f7ae0);
 const std::array<int,13> boundaries{{std::numeric_limits<int>::min(),std::numeric_limits<int>::min()+1,-8193,-4096,-32,-1,0,1,31,32,4096,std::numeric_limits<int>::max()-1,std::numeric_limits<int>::max()}};
 for(unsigned trial=0;trial<4096;++trial){
  const unsigned w=trial<3?std::array<unsigned,3>{{1,17,128}}[trial]:1+rng()%128,h=1+rng()%128,l=1+rng()%32;
  std::array<unsigned char,0x260> receiver;receiver.fill(0xa5);std::array<unsigned,5> map{{0,w,h,l,w*h}};
  put(receiver.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));
  put(reinterpret_cast<void*>(0x6c5494),0,w);put(reinterpret_cast<void*>(0x6c5498),0,h);
  const auto before=receiver;mnm::reconstruction::TerrainCamera native;
  const int x=trial<boundaries.size()?boundaries[trial]:int(rng()),y=trial<boundaries.size()?boundaries[boundaries.size()-1-trial]:int(rng()),z=int(rng());
  std::array<unsigned char,0x20> actor{};put(actor.data(),0x14,unsigned(x));put(actor.data(),0x18,unsigned(y));put(actor.data(),0x1c,unsigned(z));
  reinterpret_cast<Follow>(0x4f7ae0)(receiver.data(),actor.data());
  mnm::reconstruction::followTerrainCamera(native,w,h,l,mnm::reconstruction::TerrainCameraFocus{x,y,z});positions(receiver.data(),native);++follows;
  for(unsigned i=0;i<receiver.size();++i)if(!(i>=0x39&&i<0x51))require(receiver[i]==before[i]);
  const auto focused=receiver;reinterpret_cast<Follow>(0x4f7ae0)(receiver.data(),nullptr);require(receiver==focused);
  const auto saved=native;mnm::reconstruction::followTerrainCamera(native,0,0,0,{});require(std::memcmp(&saved,&native,sizeof(native))==0);++nulls;
 }
 // Bound original repeated-subtraction cost for raw high-bit cell words.
 for(auto x:boundaries)for(auto y:boundaries)for(int z:{-1,0,1,33,std::numeric_limits<int>::max()}){
  std::array<unsigned char,0x260> receiver;receiver.fill(0xa5);std::array<unsigned,5> map{{0,128,128,9,128*128}};put(receiver.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));const auto before=receiver;
  mnm::reconstruction::TerrainCamera native;reinterpret_cast<Cell>(0x4f7a00)(receiver.data(),x,y,z);mnm::reconstruction::setTerrainCameraCellPosition(native,128,128,9,x,y,z);positions(receiver.data(),native);
  for(unsigned i=0;i<receiver.size();++i)if(!(i>=0x39&&i<0x51))require(receiver[i]==before[i]);++cells;
 }
 std::cout<<"{\"success\":true,\"follow_cases\":"<<follows<<",\"null_cases\":"<<nulls<<",\"cell_cases\":"<<cells<<",\"entry_bytes\":{\"004f7a00\":\"8b44240c8b542408\",\"004f7ae0\":\"578b7c240885ff\"}}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
