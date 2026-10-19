// Execute selected pinned original branches in a private i386 mapping.
// Only explicitly checked external/window boundaries are substituted.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../reconstruction/rendering/startup_canvas.hpp"
#include "../reconstruction/rendering/palette_shading.hpp"
#include "../reconstruction/rendering/generic_producer_caller.h"
#include "../assets/sprite_run_clipping.hpp"
#include "rendering-wave-fixture.hpp"
#include <algorithm>
#include <array>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>

using U=std::uint32_t;
static std::ofstream waveOutput;
static U ptr(const void* p){return reinterpret_cast<U>(p);}
static U get(U p){return *reinterpret_cast<U*>(p);}
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void replace(U at,const char* expected,unsigned n,void* target){
  check(n>=5&&!std::memcmp(reinterpret_cast<void*>(at),expected,n),"Adapter signature changed");
  auto* p=reinterpret_cast<unsigned char*>(at);p[0]=0xe9;const U delta=ptr(target)-at-5;std::memcpy(p+1,&delta,4);
}
static Bytes indexed(unsigned width,unsigned height){
  Bytes b(40+height*8);auto put=[&](unsigned at,U v){std::memcpy(b.data()+at,&v,4);};
  put(4,width);put(8,height);
  for(unsigned y=0;y<height;++y){put(40+y*8,b.size());for(unsigned remaining=width;remaining;){const auto run=std::min(remaining,255u);b.push_back(0);b.push_back(run);remaining-=run;}}
  for(unsigned y=0;y<height;++y){put(44+y*8,b.size());for(unsigned x=0;x<width;++x)b.push_back((x+y*17)&255);}
  put(0,b.size());return b;
}
static void bindCanvas(std::vector<std::uint16_t>& p,unsigned stride,unsigned height){
  global(0x658174,ptr(p.data()));global(0x6a2dc8,stride);global(0x6def50,height);
  global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,stride);global(0x656618,height);
  global(0x6e1f68,0);global(0x6e1f88,0);
}
static void wave(){
  check(!std::memcmp(reinterpret_cast<void*>(0x5806f0),"\x83\xec\x20\x8b\x44",5),"Wave entry");
  check(!std::memcmp(reinterpret_cast<void*>(0x59bee0),"\x55\x8b\xec\x83\xc4\xf4",6),"Original truncation helper");
  const double angle=*reinterpret_cast<double*>(0x5c7858);
  std::cerr<<"Original wave angle "<<std::hexfloat<<angle<<'\n';
  check(angle==0x1.921fb52713c02p-2,"Wave angle constant");
  using Wave=unsigned(__attribute__((fastcall)) *)(void*,int,int,int);
  std::array<U,11> empty{};empty[0]=44;
  global(0x5e15f8,0);global(0x5f14d0,0x7d00);
  reinterpret_cast<Wave>(0x5806f0)(empty.data(),0,0,0);
  check(get(0x5f14d0)==0x7d00,"Zero amplitude/default must skip initializer");
  unsigned cases=1;
  std::array<std::int32_t,256> originalTable{};
  const auto expected=mnm::reconstruction::startupWaveTable();
  for(unsigned amplitude:{1u,2u,3u,4u,5u,6u,7u,8u})for(unsigned implicit:{0u,1u}){
    global(0x5f14d0,0x7d00);global(0x5e15f8,amplitude);
    reinterpret_cast<Wave>(0x5806f0)(empty.data(),0,0,implicit?0:amplitude);
    if(amplitude==1&&!implicit)for(unsigned i=0;i<256;++i)originalTable[i]=std::int32_t(get(0x5f14d0+i*4));
    for(unsigned i=0;i<256;++i)if(std::int32_t(get(0x5f14d0+i*4))!=expected[i]){
      std::cerr<<"wave index "<<i<<" expected "<<expected[i]<<" original "<<std::int32_t(get(0x5f14d0+i*4))<<'\n';throw std::runtime_error("Original initializer table differs");}
    // A warm table is retained even when its values are deliberately unusual.
    global(0x5f14d0,123);global(0x5f14d4,456);
    reinterpret_cast<Wave>(0x5806f0)(empty.data(),0,0,amplitude);
    check(get(0x5f14d0)==123&&get(0x5f14d4)==456,"Warm initializer overwrite");cases+=2;
  }
  unsigned pixelCases=0;
  auto visible=wave_fixture::frame();std::vector<std::uint16_t> pixels(64*32);std::vector<U> hashes;
  mnm::assets::SpriteFrame coverage;coverage.width=7;coverage.height=9;coverage.opaqueMask.assign(63,1);
  for(unsigned y=0;y<9;++y)coverage.opaqueRuns.push_back({y,0,7});
  for(unsigned amplitude=1;amplitude<=8;++amplitude)for(unsigned implicit:{0u,1u})
    for(unsigned phase:{0u,3u,15u,31u})for(int x:{0,10,37})for(int y:{-2,4})for(unsigned split:{0u,1u}){
      visible=wave_fixture::frame(split);coverage.opaqueRuns.clear();
      for(unsigned row=0;row<9;++row){if(split)coverage.opaqueRuns.push_back({row,0,2});coverage.opaqueRuns.push_back({row,split?2u:0u,7});}
      for(unsigned i=0;i<pixels.size();++i)pixels[i]=std::uint16_t(i*17+31);
      auto wanted=pixels;bindCanvas(pixels,64,32);
      global(0x6e0008,2);global(0x6cbb6c,2);global(0x6a49b8,40);global(0x656618,25);
      global(0x6c4830,phase);global(0x5e15f8,amplitude);
      for(unsigned i=0;i<256;++i)global(0x5f14d0+i*4,U(expected[i]));
      const auto offsets=mnm::reconstruction::startupWaveOffsets(expected,amplitude,phase);
      const bool clipped=x<2||x+7>40;
      auto mask=coverage.opaqueMask;
      if(clipped){mask.assign(63,0);for(const auto& run:mnm::assets::clipWholeOpaqueRuns(coverage,2-x,40-x))
        std::fill(mask.begin()+run.row*7+run.left,mask.begin()+run.row*7+run.right,1);}
      for(int row=std::max(2,y);row<std::min(25,y+9);++row)
        for(int col=std::max(2,x);col<std::min(40,x+7);++col){
          if(!mask[(row-y)*7+col-x])continue;
          const unsigned shift=clipped?unsigned(std::abs(8-int((row+phase)&15))):unsigned(offsets[unsigned(row)%(2*amplitude)]);
          wanted[row*64+col]=pixels[row*64+col+shift];
        }
      reinterpret_cast<Wave>(0x5806f0)(visible.data(),x,y,implicit?0:amplitude);
      if(pixels!=wanted){auto at=std::mismatch(pixels.begin(),pixels.end(),wanted.begin()).first-pixels.begin();std::cerr<<"wave amplitude "<<amplitude<<" implicit "<<implicit<<" phase "<<phase<<" xy "<<x<<','<<y<<" split "<<split<<" at "<<at<<" original "<<pixels[at]<<" native "<<wanted[at]<<'\n';}
      check(pixels==wanted,"Original effective wave phase/clipping differs");++pixelCases;
      hashes.push_back(wave_fixture::hash(pixels));
      if(waveOutput.is_open())wave_fixture::write(waveOutput,pixels);
    }
  std::cout<<"{\"case\":\"wave-initializer\",\"cases\":"<<cases<<",\"table_words_compared\":4096,\"effective_pixel_cases\":"<<pixelCases<<",\"effective_words_compared\":"<<pixelCases*64*32<<",\"original_ftol_executed\":true,\"initialized_table\":[";
  for(unsigned i=0;i<256;++i){if(i)std::cout<<',';std::cout<<originalTable[i];}std::cout<<"],\"effective_hashes\":[";
  for(unsigned i=0;i<hashes.size();++i){if(i)std::cout<<',';std::cout<<hashes[i];}std::cout<<"]}\n";
}
static void palette(){
  using Draw=unsigned(*)(void*,int,int,int,int);
  auto frame=indexed(256,1);std::vector<std::uint16_t> pixels(260*3,0x9abc);
  unsigned cases=0;std::uint64_t words=0;
  for(unsigned shift:{1u,2u,3u,4u,5u,6u,7u}){
    const unsigned count=256u>>shift,neutral=(count-1)/2;
    std::array<std::vector<U>,3> nodes;
    for(unsigned n=0;n<3;++n){nodes[n].resize(3+(count-1)*128);nodes[n][1]=shift;nodes[n][2]=neutral;
      auto* colours=reinterpret_cast<std::uint16_t*>(nodes[n].data()+3);
      for(unsigned t=0;t<count-1;++t)for(unsigned i=0;i<256;++i)colours[t*256+i]=std::uint16_t(n*8192+t*256+i);
    }
    nodes[0][0]=ptr(nodes[1].data());nodes[1][0]=ptr(nodes[2].data());U pointer=ptr(nodes[0].data());std::memcpy(frame.data()+28,&pointer,4);
    mnm::reconstruction::ShadedPalette model;model.shift=shift;model.neutral=neutral;model.tables.resize(count-1);
    for(int shade=-127;shade<=127;++shade)for(unsigned ordinal:{0u,1u,2u}){
      const auto table=model.tableIndex(shade);bindCanvas(pixels,260,3);std::fill(pixels.begin(),pixels.end(),0x9abc);
      reinterpret_cast<Draw>(0x57de00)(frame.data(),2,1,shade,ordinal);
      for(unsigned i=0;i<256;++i)check(pixels[260+2+i]==std::uint16_t(ordinal*8192+table*256+i),"Palette shade/ordinal table selection");
      ++cases;words+=256;
    }
  }
  // Invalid native selector shapes are refused before any original unsafe read.
  unsigned refused=0;for(unsigned shift:{8u,32u}){mnm::reconstruction::ShadedPalette p;p.shift=shift;p.tables.resize(1);try{p.tableIndex(0);}catch(const std::exception&){++refused;}}
  std::cout<<"{\"case\":\"palette-selection\",\"cases\":"<<cases<<",\"palette_words_compared\":"<<words<<",\"invalid_native_shapes_refused\":"<<refused<<"}\n";
}

static std::array<U,40> factoryTable{},surfaceTable{},factory{},surface1{},surface2{};
static U createResult,queryResult,earlyResult,restored,created,queried,reported;
static U __attribute__((stdcall)) createSurface(void* self,U* desc,U* output,void* outer){check(ptr(self)==ptr(factory.data())&&!outer&&desc[0]==108,"CreateSurface arguments");*output=ptr(surface1.data());++created;return createResult;}
static U __attribute__((stdcall)) querySurface(void* self,void* iid,U* output){check(ptr(self)==ptr(surface1.data())&&ptr(iid)==0x5c7990,"QI identity");*output=ptr(surface2.data());++queried;if(queryResult)global(0x6f98c8,0);return queryResult;}
static U __attribute__((stdcall)) surfaceDescription(void*,U*){return 0;}
static U __attribute__((stdcall)) early(void*,U){if(earlyResult)global(0x6f98c8,0);return earlyResult;}
static U __attribute__((stdcall)) restore(void*){return restored;}
static void __attribute__((thiscall)) report(void*,U){++reported;}
static U __attribute__((stdcall)) terminateWindow(void*){_exit(73);}
static void creation(){
  // The runner pins the complete original image before this checked boundary.
  const Bytes reportPrefix(reinterpret_cast<unsigned char*>(0x484c00),reinterpret_cast<unsigned char*>(0x484c00)+5);
  replace(0x484c00,reinterpret_cast<const char*>(reportPrefix.data()),5,reinterpret_cast<void*>(report));
  global(0x5c5208,ptr(reinterpret_cast<void*>(terminateWindow)));
  factoryTable[6]=ptr(reinterpret_cast<void*>(createSurface));factory[0]=ptr(factoryTable.data());
  surfaceTable[0]=ptr(reinterpret_cast<void*>(querySurface));surfaceTable[22]=ptr(reinterpret_cast<void*>(surfaceDescription));
  surfaceTable[37]=ptr(reinterpret_cast<void*>(early));surfaceTable[27]=ptr(reinterpret_cast<void*>(restore));
  surface1[0]=surface2[0]=ptr(surfaceTable.data());using Create=void(__attribute__((thiscall)) *)(void*,void*,U,U);
  std::array<U,27> desc{};desc[0]=108;desc[1]=7;unsigned cases=0;
  for(U width:{1u,7u,800u})for(U height:{1u,11u,600u})for(U caps:{0x840u,0x4840u})for(U result:{0u,0x80004005u}){
    std::array<unsigned char,64> object{};desc[2]=height;desc[3]=width;desc[26]=caps;
    createResult=result;queryResult=earlyResult=0;created=queried=reported=0;
    global(0x6f8154,0);global(0x6f98c8,ptr(factory.data()));global(0x6e1f1c,0);
    reinterpret_cast<Create>(0x58ad90)(object.data(),desc.data(),0,0);
    check(created==1&&queried==1&&reported==unsigned(result!=0),"Create/QI/report schedule");
    check(get(ptr(object.data())+4)==ptr(surface1.data())&&get(ptr(object.data())+8)==ptr(surface2.data()),"Distinct created interfaces");
    check(get(ptr(object.data())+16)==width&&get(ptr(object.data())+20)==height,"Created dimensions");
    check(get(ptr(object.data())+28)==0&&get(0x6f8154)==1&&get(0x6f6950)==ptr(object.data()),"Registry generation bookkeeping");
    check(get(ptr(object.data()))==unsigned((caps&0x4000)!=0),"Recovered surface flag");++cases;
  }
  std::array<unsigned char,64> object{};object.fill(0x5a);global(0x6f8154,128);const auto before=object;
  reinterpret_cast<Create>(0x58ad90)(object.data(),desc.data(),0,0);check(object==before,"Registry ceiling must leave wrapper untouched");++cases;
  for(unsigned mode:{0u,1u,2u}){const auto pid=fork();check(pid>=0,"fork");if(!pid){
      object.fill(0);
      global(0x6f8154,0);global(0x6f98c8,ptr(factory.data()));global(0x6f6950,0);global(0x6f98c4,ptr(factory.data()));global(0x6f98cc,ptr(factory.data()));
      // Stop exactly at the fatal UI boundary, avoiding real display destruction.
      queryResult=mode==0?0x80004005u:0;earlyResult=mode?0x887601c2u:0;restored=mode==1?0:0x80004005u;
      global(0x6e1f1c,mode?1:0);global(0x6f98c8,ptr(factory.data()));
      // Failure teardown is outside this selected creation window; an empty
      // factory global makes the original skip its global interface teardown.
      if(mode==0)queryResult=0x80004005u;
      reinterpret_cast<Create>(0x58ad90)(object.data(),desc.data(),0,0);_exit(74);
    }int status=0;waitpid(pid,&status,0);if(!WIFEXITED(status)||WEXITSTATUS(status)!=73)std::cerr<<"fatal mode "<<mode<<" status "<<status<<'\n';check(WIFEXITED(status)&&WEXITSTATUS(status)==73,"Fatal child must reach teardown boundary");++cases;}
  std::cout<<"{\"case\":\"create-wrapper\",\"cases\":"<<cases<<",\"scripted_interfaces\":true,\"physical_allocation_owned\":false}\n";
}

static U drawResults[2],drawAt,restoreAt,clearRestore,clearWord;
static std::vector<std::uint16_t> clearPixels;
static U __attribute__((stdcall)) clearBlt(void*,void* dest,void* source,void* rect,U flags,U* effects){
  check(!dest&&!source&&!rect&&flags==0x1000400&&effects[0]==100&&effects[20]==clearWord,"Full-fill arguments/low16 colour");
  check(drawAt<2,"Full-fill draw budget");const U result=drawResults[drawAt++];
  if(!result)for(unsigned y=0;y<5;++y)for(unsigned x=0;x<7;++x)clearPixels[y*11+x]=effects[20];
  return result;
}
static U __attribute__((stdcall)) clearRestoreApi(void*){++restoreAt;return clearRestore;}
static void clear(){
  const Bytes prefix(reinterpret_cast<unsigned char*>(0x484c00),reinterpret_cast<unsigned char*>(0x484c00)+5);
  replace(0x484c00,reinterpret_cast<const char*>(prefix.data()),5,reinterpret_cast<void*>(report));
  std::array<U,33> table{};table[5]=ptr(reinterpret_cast<void*>(clearBlt));table[27]=ptr(reinterpret_cast<void*>(clearRestoreApi));
  U interface=ptr(table.data());std::array<U,16> object{};object[2]=ptr(&interface);
  using Fill=void(__attribute__((thiscall)) *)(void*,U);global(0x6de6dd,0);unsigned cases=0;
  for(U colour:{0u,0x12345678u})for(U initial:{0u,0x887601c2u,0x80004005u})for(U retry:{0u,0x80004005u})for(U restoreResult:{0u,0x80004005u}){
    clearPixels.assign(11*5+8,0x1234);drawResults[0]=initial;drawResults[1]=retry;drawAt=restoreAt=reported=0;clearRestore=restoreResult;
    clearWord=colour&65535;reinterpret_cast<Fill>(0x58bc10)(object.data(),colour);
    const bool lost=initial==0x887601c2u,written=!initial||(lost&&!retry);
    check(drawAt==(lost?2u:1u)&&restoreAt==unsigned(lost)&&reported==(lost?3u:1u),"Original full-fill retry/report schedule");
    for(unsigned i=0;i<clearPixels.size();++i)check(clearPixels[i]==(written&&i<55&&i%11<7?clearWord:0x1234),"Lease-scoped clear/guard words");
    ++cases;
  }
  object[2]=0;drawAt=reported=0;reinterpret_cast<Fill>(0x58bc10)(object.data(),0);check(!drawAt&&!reported,"Absent full-fill interface");++cases;
  std::cout<<"{\"case\":\"full-fill-completion\",\"cases\":"<<cases<<",\"visible_words_per_case\":35,\"padding_guarded\":true,\"driver_completion_scripted\":true}\n";
}

static std::vector<std::array<U,7>> consumed;
static std::vector<std::uint16_t> mainPixels;
static bool nullLock;
static U __attribute__((thiscall)) bindingLock(void*){return nullLock?0:ptr(mainPixels.data());}
static U __attribute__((stdcall)) bindingUnlock(void*,void*){return 0;}
static void __attribute__((thiscall)) bindingConsumer(void*){
  consumed.push_back({get(0x658174),get(0x6a2dc8),get(0x6def50),get(0x6e0008),get(0x6cbb6c),get(0x6a49b8),get(0x656618)});
}
static void bindingPost(){}
extern "C" __attribute__((naked,noinline,noipa)) void bindingWindow(void*,void*){
  asm volatile("push %ebp;push %ebx;push %esi;push %edi;mov 20(%esp),%esi;mov 24(%esp),%ebp;xor %ebx,%ebx;mov $0x4fd438,%eax;call *%eax;pop %edi;pop %esi;pop %ebx;pop %ebp;ret");
}
static void binding(){
  // Execute precisely the original bind/consume/rebind window, rather than
  // pretending that the rest of the scene-production dispatcher is exercised.
  replace(0x58b660,"\x83\xec\x08\x33\xc0",5,reinterpret_cast<void*>(bindingLock));
  const Bytes queuePrefix(reinterpret_cast<unsigned char*>(0x5002a0),reinterpret_cast<unsigned char*>(0x5002a0)+5);
  replace(0x5002a0,reinterpret_cast<const char*>(queuePrefix.data()),5,reinterpret_cast<void*>(bindingConsumer));
  const Bytes postPrefix(reinterpret_cast<unsigned char*>(0x594508),reinterpret_cast<unsigned char*>(0x594508)+5);
  replace(0x594508,reinterpret_cast<const char*>(postPrefix.data()),5,reinterpret_cast<void*>(bindingPost));
  check(!std::memcmp(reinterpret_cast<void*>(0x4fd5af),"\x8b\x96\x29\x02\x00\x00",6),"Binding window end");
  *reinterpret_cast<unsigned char*>(0x4fd5af)=0xc3;
  const Bytes window(reinterpret_cast<unsigned char*>(0x4fd438),reinterpret_cast<unsigned char*>(0x4fd5ad));
  std::array<unsigned char,0x240> object{};std::array<U,6> queue{};std::array<U,4> clip{2,3,11,17};
  U table[33]={};table[32]=ptr(reinterpret_cast<void*>(bindingUnlock));U interface=ptr(table);
  mainPixels.assign(31*23,0x1234);std::vector<std::uint16_t> padded(1000*800,0x5678);unsigned cases=0;
  for(unsigned video:{0u,1u})for(unsigned xp:{0u,3u,11u})for(unsigned yp:{0u,5u})for(unsigned alternate:{0u,1u})for(unsigned absent:{0u,1u}){
    consumed.clear();nullLock=absent;global(0x6de6d5,video);global(0x6e1f68,alternate?ptr(padded.data()):0);global(0x6e1f70,xp);global(0x6e1f74,yp);global(0x6c482c,ptr(clip.data()));
    global(0x6a49e4,ptr(&interface));global(0x6a49f4,23);global(0x6a49f8,62);
    global(0x6e0008,clip[0]);global(0x6cbb6c,clip[1]);global(0x6a49b8,clip[2]);global(0x656618,clip[3]);
    bindingWindow(object.data(),queue.data());
    check(consumed.size()==(alternate||!absent?1u:0u),"Selected World consumer count");
    if(alternate){const auto expected=mnm::reconstruction::paddedStartupBinding(video,xp,yp);const auto& r=consumed[0];
      check(r==std::array<U,7>{ptr(padded.data()),U(expected.stride),U(expected.height),clip[0],clip[1],clip[2]+2*xp,clip[3]+2*yp},"Padded binding/independent clips");}
    else if(!absent)check(consumed[0]==std::array<U,7>{ptr(mainPixels.data()),31,23,clip[0],clip[1],clip[2],clip[3]},"Ordinary binding");
    if(!absent)check(get(0x658174)==ptr(mainPixels.data())&&get(0x6a2dc8)==31&&get(0x6def50)==23&&get(0x656618)==17,"Ordinary rebind height/stride/clip remain distinct");
    check(window==Bytes(reinterpret_cast<unsigned char*>(0x4fd438),reinterpret_cast<unsigned char*>(0x4fd5ad)),"Binding window text unchanged");++cases;
  }
  std::cout<<"{\"case\":\"padded-bind-rebind\",\"cases\":"<<cases<<",\"original_setters_executed\":true,\"consumer_and_lock_scripted\":true}\n";
}

static void uniform(){
  using Draw=unsigned(__attribute__((thiscall)) *)(void*,int,int);
  std::vector<std::uint16_t> pixels(21*17);unsigned cases=0;std::uint64_t compared=0;
  for(int x:{-8,-1,0,3,12,21})for(int y:{-4,0,2,13,17})for(unsigned w:{1u,4u,11u})for(unsigned h:{1u,5u}){
    bindCanvas(pixels,21,17);global(0x6e0008,2);global(0x6cbb6c,1);global(0x6a49b8,19);global(0x656618,16);
    std::fill(pixels.begin(),pixels.end(),0x1234);unsigned char shape[22]={};U mode=0,plane=0;std::memcpy(shape,&mode,4);std::memcpy(shape+4,&w,4);std::memcpy(shape+8,&h,4);
    const std::uint16_t rgb[3]={23,45,17};std::memcpy(shape+12,rgb,6);std::memcpy(shape+18,&plane,4);
    reinterpret_cast<Draw>(0x547d80)(shape,x,y);const auto packed=std::uint16_t((23<<11)|(45<<5)|17);
    for(int row=0;row<17;++row)for(int col=0;col<21;++col){const bool covered=col>=2&&col<19&&row>=1&&row<16&&col>=x&&col<std::int64_t(x)+w&&row>=y&&row<std::int64_t(y)+h;
      check(pixels[row*21+col]==(covered?packed:0x1234),"Uniform original clipping/pack");++compared;}++cases;
  }
  std::cout<<"{\"case\":\"uniform-original\",\"cases\":"<<cases<<",\"words_compared\":"<<compared<<",\"destination_seed_is_fixture_only\":true}\n";
}
static void caller(){
  using Draw=unsigned(*)(void*,int,int,int,int);
  unsigned cases=0,guarded=0;std::uint64_t words=0;
  for(unsigned caller:{0x500512u,0x500a4eu,0x57e940u})for(unsigned tag:{13u,14u}){
    check(mnm_generic_producer_caller(caller,tag,reinterpret_cast<unsigned char*>(caller)),"Pinned original caller continuation differs");++guarded;
  }
  for(U entry:{0x5947b2u,0x59521au})for(unsigned w:{1u,3u,17u})for(unsigned h:{1u,4u})for(unsigned x:{0u,2u,8u}){
    auto frame=indexed(w,h);std::array<U,131> palette{};auto* colours=reinterpret_cast<std::uint16_t*>(palette.data()+3);
    for(unsigned i=0;i<256;++i)colours[i]=std::uint16_t(0x4000+i);
    const auto address=ptr(palette.data());std::memcpy(frame.data()+28,&address,4);
    std::vector<std::uint16_t> pixels(64*12,0x1234);bindCanvas(pixels,64,12);
    const U result=reinterpret_cast<Draw>(entry)(frame.data(),x,2,0,0);
    check(result==0,"Selected generic original return must be zero");
    for(unsigned row=0;row<12;++row)for(unsigned col=0;col<64;++col){const bool hit=row>=2&&row<2+h&&col>=x&&col<x+w;check(pixels[row*64+col]==(hit?0x4000+((col-x+(row-2)*17)&255):0x1234),"Selected generic native words");++words;}
    ++cases;
  }
  std::cout<<"{\"case\":\"generic-caller\",\"cases\":"<<cases<<",\"caller_tag_pairs\":"<<guarded<<",\"consumed_eax_zero\":true,\"pixels_compared\":"<<words<<",\"flags_overwritten_by_continuation\":true}\n";
}

int main(int argc,char** argv)try{
  if(argc!=3 && !(argc==4 && std::string(argv[2])=="wave"))throw std::runtime_error("Expected pinned PE and case");
  if(argc==4){waveOutput.open(argv[3],std::ios::binary);check(bool(waveOutput),"Wave pixel output");}
  map_image(read(argv[1]));
  const std::string mode=argv[2];if(mode=="wave")wave();else if(mode=="palette")palette();else if(mode=="create")creation();else if(mode=="clear")clear();else if(mode=="bind")binding();else if(mode=="uniform")uniform();else if(mode=="caller")caller();else throw std::runtime_error("Unknown reference case");return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
