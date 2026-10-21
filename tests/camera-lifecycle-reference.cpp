// Whole unchanged camera init/registry/standalone turns plus unchanged post-frame
// rotation tail. Byte-checked private consumer adapters record dispatch only.
#define main previous_camera_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
#include "camera_lifecycle.hpp"
using namespace mnm::reconstruction;
static std::vector<CameraRefresh> events;
static std::uintptr_t creatureBase=0,effectBase=0;
static void record(CameraRefresh::Kind kind,void* receiver){
 require(get(reinterpret_cast<void*>(0x689b80),0x8d)==0xa5a5a5a5);
 const auto base=kind==CameraRefresh::Kind::creature?creatureBase:effectBase;
 const auto stride=kind==CameraRefresh::Kind::creature?0xe4b:0x22e;
 const auto view=get(reinterpret_cast<void*>(kind==CameraRefresh::Kind::creature?0x6e9850:0x5fed78),0);
 events.push_back({kind,(reinterpret_cast<std::uintptr_t>(receiver)-base)/stride,view});
}
static void __attribute__((thiscall)) creatureRefresh(void* p){record(CameraRefresh::Kind::creature,p);}
static void __attribute__((thiscall)) effectRefresh(void* p){record(CameraRefresh::Kind::effect,p);}
static void adapter(std::uintptr_t address,const std::vector<unsigned char>& expected,void* callback){
 require(expected.size()>=6&&!std::memcmp(reinterpret_cast<void*>(address),expected.data(),expected.size()));
 unsigned char bytes[6]={0x68,0,0,0,0,0xc3};const auto target=reinterpret_cast<std::uintptr_t>(callback);std::memcpy(bytes+1,&target,4);std::memcpy(reinterpret_cast<void*>(address),bytes,sizeof(bytes));
}
// Recreate only the stack/register preconditions at004fd604. All selected
// instructions and relative calls stay in their original mapped locations.
static void __attribute__((naked)) pendingTail(void*){
 asm volatile("push %ebx\n push %ebp\n push %esi\n push %edi\n mov 20(%esp),%esi\n xor %ebx,%ebx\n mov $0x4fd604,%eax\n jmp *%eax\n");
}
static void fields(void* receiver,const TerrainCameraLifecycle& state){
 const auto& c=state.camera;
 for(const auto field:std::vector<std::pair<unsigned,unsigned>>{{0x31,c.view},{0x23d,state.requestedView},{0x35,state.rendered},{0x99,state.redrawFrames},{0x221,state.clockwise},{0x225,state.counterclockwise},{0x39,unsigned(c.column)},{0x3d,unsigned(c.row)},{0x41,unsigned(c.f41)},{0x45,unsigned(c.f45)},{0x49,unsigned(c.f49)},{0x4d,unsigned(c.f4d)},{0x11,unsigned(c.f11)},{0x15,unsigned(c.f15)},{0x51,unsigned(c.f51)},{0x55,unsigned(c.f55)},{0x59,c.span},{0x5d,c.diagonal},{0x61,c.cutLevel},{0x6d,c.mode},{0x29,state.parameter29},{0x2d,state.parameter2d}})put(receiver,field.first,field.second);
}
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));
 const unsigned char cw[]={0x8b,0x41,0x31,0x56,0x40,0x57},ccw[]={0x8b,0x41,0x31,0x48,0x79,0x03};
 require(!std::memcmp(reinterpret_cast<void*>(0x4fd780),cw,sizeof(cw)));require(!std::memcmp(reinterpret_cast<void*>(0x4fd840),ccw,sizeof(ccw)));
 const unsigned char tail[]={0x39,0x9e,0x21,0x02,0,0};require(!std::memcmp(reinterpret_cast<void*>(0x4fd604),tail,sizeof(tail)));
 adapter(0x508a10,{0xa1,0x50,0x98,0x6e,0,0x56},reinterpret_cast<void*>(&creatureRefresh));adapter(0x4945f0,{0x83,0xec,0x38,0x56,0x8b,0xf1},reinterpret_cast<void*>(&effectRefresh));events.reserve(128);
 using Init=unsigned(__attribute__((thiscall)) *)(void*,void*,unsigned,unsigned,void*,unsigned);
 using Register=unsigned(__attribute__((thiscall)) *)(void*,void*,unsigned);
 using Remove=void(__attribute__((fastcall)) *)(unsigned);
 using Turn=void(__attribute__((thiscall)) *)(void*);
 std::mt19937 rng(0x4fd780);unsigned initializations=0,turns=0,queued=0,registryChecks=0,consumerCalls=0;
 for(unsigned trial=0;trial<4096;++trial){
  std::array<unsigned char,0x260> receiver;receiver.fill(0xa5);TerrainCameraLifecycle state;state.camera.column=int(rng());state.camera.row=int(rng());state.camera.f45=int(rng());state.camera.f49=int(rng());state.camera.f4d=int(rng());state.camera.f41=int(rng());state.camera.view=rng()%4;state.requestedView=rng()%4;state.rendered=rng();state.redrawFrames=rng();state.clockwise=rng()%2;state.counterclockwise=rng()%2;state.parameter29=rng();state.parameter2d=rng();fields(receiver.data(),state);auto expected=receiver;
  const unsigned w=1+rng()%128,h=1+rng()%128,l=1+rng()%32,slot=trial%401,a=rng(),b=rng(),flags=rng();std::array<unsigned,5> map{{0,w,h,l,w*h}};std::array<int,4> rectangle{{int(rng()%201)-100,int(rng()%201)-100,512,256}};put(reinterpret_cast<void*>(0x642c68),0,slot);
  const auto result=reinterpret_cast<Init>(0x4f7860)(receiver.data(),map.data(),a,b,rectangle.data(),flags);require(result==unsigned(initializeTerrainCamera(state,w,h,l,{rectangle[0],rectangle[1],rectangle[2],rectangle[3]},a,b,slot<400)));fields(expected.data(),state);
  if(result){put(expected.data(),4,slot);put(expected.data(),8,flags);put(expected.data(),0x19,rectangle[2]);put(expected.data(),0x1d,rectangle[3]);put(expected.data(),0x21,reinterpret_cast<std::uintptr_t>(map.data()));put(expected.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));require(get(reinterpret_cast<void*>(0x642034),slot*4)==reinterpret_cast<std::uintptr_t>(receiver.data()));}
  require(receiver==expected&&get(reinterpret_cast<void*>(0x642c68),0)==slot+result);++initializations;
 }
 for(unsigned trial=0;trial<8192;++trial){
  std::array<unsigned char,0x260> local;local.fill(0xa5);const bool primary=trial&1;void* receiver=primary?reinterpret_cast<void*>(0x689930):local.data();TerrainCameraLifecycle state;state.camera.view=trial%4;state.requestedView=rng()%4;state.redrawFrames=rng();state.clockwise=(trial>>2)&1;state.counterclockwise=(trial>>3)&1;state.rendered=1;fields(receiver,state);std::array<unsigned char,0x260> expected;std::memcpy(expected.data(),receiver,expected.size());
  const unsigned otherView=rng()%4;std::optional<unsigned> distinct=primary?std::nullopt:std::optional<unsigned>(otherView);if(!primary)put(reinterpret_cast<void*>(0x689930),0x31,otherView);
  const unsigned nc=rng()%17,ne=rng()%17;std::vector<std::array<unsigned char,0xe4b>> creatures(nc);std::vector<std::array<unsigned char,0x22e>> effects(ne);std::vector<bool> activeCreatures(nc),activeEffects(ne);
  for(unsigned i=0;i<nc;++i){activeCreatures[i]=rng()%2;put(creatures[i].data(),4,activeCreatures[i]?123:0);}for(unsigned i=0;i<ne;++i){activeEffects[i]=rng()%2;put(effects[i].data(),4,activeEffects[i]?321:0);}
  creatureBase=reinterpret_cast<std::uintptr_t>(creatures.data());effectBase=reinterpret_cast<std::uintptr_t>(effects.data());put(reinterpret_cast<void*>(0x6def58),0,creatureBase);put(reinterpret_cast<void*>(0x6df180),0,nc);put(reinterpret_cast<void*>(0x689498),0,effectBase);put(reinterpret_cast<void*>(0x6898dc),0,ne);put(reinterpret_cast<void*>(0x689b80),0x8d,0xa5a5a5a5);put(reinterpret_cast<void*>(0x689b80),0xc5,99);events.clear();std::vector<CameraRefresh> requested;
  if(trial<4096){const bool clockwise=trial&2;reinterpret_cast<Turn>(clockwise?0x4fd780:0x4fd840)(receiver);requested=rotateTerrainCamera(state,clockwise?CameraTurn::clockwise:CameraTurn::counterclockwise,activeCreatures,activeEffects,distinct);++turns;}
  else{pendingTail(receiver);requested=finishTerrainCameraFrame(state,activeCreatures,activeEffects,distinct);++queued;}
  if(!requested.empty()){const auto view=distinct.value_or(state.camera.view);require(get(reinterpret_cast<void*>(0x689b80),0x8d)==0&&get(reinterpret_cast<void*>(0x689b80),0xc5)==view);events.push_back({CameraRefresh::Kind::minimap,0,view});require(get(reinterpret_cast<void*>(0x6e9850),0)==view&&get(reinterpret_cast<void*>(0x5fed78),0)==view);}
  else require(get(reinterpret_cast<void*>(0x689b80),0xc5)==99);
  require(events.size()==requested.size());for(unsigned i=0;i<events.size();++i)require(events[i].kind==requested[i].kind&&events[i].ordinal==requested[i].ordinal&&events[i].view==requested[i].view);consumerCalls+=events.size();fields(expected.data(),state);require(!std::memcmp(expected.data(),receiver,expected.size()));
 }
 for(unsigned session=0;session<8;++session){
  TerrainViewportRegistry registry;std::array<std::array<unsigned char,64>,400> receivers{};put(reinterpret_cast<void*>(0x642c68),0,0);put(reinterpret_cast<void*>(0x5e04cc),0,0xffffffffu);
  for(unsigned i=0;i<400;++i){std::array<int,4> rect{{int(i),int(i),int(i+20),int(i+10)}};require(reinterpret_cast<Register>(0x4a4e20)(receivers[i].data(),rect.data(),i+1)==1);require(registry.append({i+1,{rect[0],rect[1],rect[2],rect[3]},i+1}));++registryChecks;}
  std::array<int,4> rect{};require(!reinterpret_cast<Register>(0x4a4e20)(receivers[0].data(),rect.data(),999));require(!registry.append({999,{},1}));++registryChecks;
  while(!registry.entries().empty()){
   const auto selected=rng()%(registry.entries().size()+1);const auto chosen=selected==registry.entries().size()?std::nullopt:std::optional<std::size_t>(selected);registry.select(chosen);put(reinterpret_cast<void*>(0x5e04cc),0,chosen?unsigned(*chosen):0xffffffffu);const auto index=rng()%registry.entries().size();reinterpret_cast<Remove>(0x4a4f70)(unsigned(index));registry.remove(index);
   require(get(reinterpret_cast<void*>(0x642c68),0)==registry.entries().size());require(get(reinterpret_cast<void*>(0x5e04cc),0)==(registry.selected()?unsigned(*registry.selected()):0xffffffffu));
   for(unsigned i=0;i<registry.entries().size();++i){const auto identity=registry.entries()[i].identity;require(get(reinterpret_cast<void*>(0x642034),i*4)==reinterpret_cast<std::uintptr_t>(receivers[identity-1].data()));require(get(receivers[identity-1].data(),4)==i);}
   ++registryChecks;
  }
  reinterpret_cast<Remove>(0x4a4f70)(0xffffffffu);registry.remove({});require(get(reinterpret_cast<void*>(0x642c68),0)==0);++registryChecks;
 }
 std::cout<<"{\"success\":true,\"initializations\":"<<initializations<<",\"standalone_rotations\":"<<turns<<",\"queued_rotations\":"<<queued<<",\"registry_checks\":"<<registryChecks<<",\"consumer_requests\":"<<consumerCalls<<",\"entry_bytes\":{\"004fd780\":\"8b4131564057\",\"004fd840\":\"8b4131487903\",\"004fd604\":\"399e21020000\"}}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
