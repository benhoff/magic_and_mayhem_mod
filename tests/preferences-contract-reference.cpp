// Offline oracle: execute pinned original callbacks in a private PE32 mapping.
// Checked stubs isolate device, filesystem, formatting and screen presentation.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "preferences_fixture_bytes.h"
#include <cstdio>
#include <cstdarg>
#include <map>
#include <array>

static std::string leaveOrder;
static unsigned musicCalls, soundCalls, sampleCalls, closeCalls, initCalls, starts;
static int music, sound; static unsigned frame;
static std::map<std::string,std::string> writes;
static unsigned word(unsigned at){unsigned v;std::memcpy(&v,reinterpret_cast<void*>(at),4);return v;}
static void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
static void __attribute__((thiscall)) release_controls(void* self){check(self==(void*)0x6a4948,"release receiver");global(0x6a4950,0);leaveOrder+='D';}
static void display_rebuild(){leaveOrder+='B';}
static void __attribute__((thiscall)) game_release(void* self,int value){check(self==(void*)0x6cbb78&&!value,"game release receiver");leaveOrder+='S';}
static void __attribute__((thiscall)) game_restore(void* self){check(self==(void*)0x6cbb78,"game restore receiver");leaveOrder+='R';}
static void __attribute__((thiscall)) context_reset(void* self){check(self==(void*)0x6c4e40,"context reset receiver");leaveOrder+='C';}
static void __attribute__((thiscall)) context_refresh(void* self){check(self==(void*)0x6c4e40,"context refresh receiver");leaveOrder+='F';}
static int __attribute__((thiscall)) music_set(void* self,int value){check(self==(void*)0x657948,"music receiver");music=value;++musicCalls;return 0;}
static int __attribute__((thiscall)) sound_set(void* self,int value){check(self==(void*)0x6b0198,"sound receiver");sound=value;++soundCalls;return 0;}
static int __attribute__((thiscall)) music_get(void*,int* value){*value=music;return 0;}
static int __attribute__((thiscall)) sound_get(void*,int* value){*value=sound;return 0;}
static int __attribute__((thiscall)) start_music(void*,int value){check(value==1,"music restart argument");++starts;return 0;}
static int __attribute__((thiscall)) sample(void*,int id,int value,int a,int b,int c,int d,int e){check(id==509&&value==sound&&!a&&!b&&!c&&d==-1&&e==-1,"preview sample arguments");++sampleCalls;return 0;}
static int __attribute__((thiscall)) frame_set(void*,unsigned value){frame=value;return 0;}
static int __attribute__((thiscall)) close_screen(void* self){check(self==(void*)0x6a4948,"close receiver");++closeCalls;return 0;}
static int __attribute__((thiscall)) initialize(void* self){check(self==(void*)0x6a4948,"init receiver");global(0x6a4950,1);++initCalls;return 0;}
static unsigned mciStatus, mciDevice=0x212; static int profileResult=1;
static unsigned __attribute__((stdcall)) mci(unsigned,unsigned message,unsigned flags,void*){check(message==0x814&&flags==0x102,"MCI arguments");global(0x657cc4,mciDevice);return mciStatus;}
static int __attribute__((stdcall)) attributes(const char*,unsigned flags){check(flags==0x80,"attributes");return 1;}
static unsigned __attribute__((stdcall)) directory(unsigned size,char* out){check(size==256,"cwd size");std::strcpy(out,"fixture");return 7;}
static int __attribute__((stdcall)) profile(const char* section,const char* key,const char* value,const char* path){check(std::string(path)=="fixture\\CFG\\prefs.cfg","preferences path");writes[std::string(section)+"/"+key]=value;return profileResult;}
static int formatting(char* out,const char* format,...){va_list ap;va_start(ap,format);int n=std::vsprintf(out,format,ap);va_end(ap);return n;}
static char* decimal(int value,char* out,int base){check(base==10,"itoa base");std::sprintf(out,"%d",value);return out;}
static void stub(unsigned at,const unsigned char* expected,unsigned length,void* target){check(length>=5&&!std::memcmp((void*)at,expected,length),"stub original bytes");auto* p=(unsigned char*)at;p[0]=0xe9;global(at+1,unsigned((uintptr_t)target-at-5));}
using Callback=int (__attribute__((thiscall)) *)(void*,int);
using Entry=void (__attribute__((thiscall)) *)(void*);
static unsigned groups=0x6a4a00,sliders=0x6a4b00,radio=0x6a4d00;
static void reset(){
 std::memset((void*)0x6a4948,0,0x70);global(0x6a4948,0x5c648c);global(0x6a494c,10);global(0x6a499b,sliders);global(0x6a49ab,groups);
 for(unsigned i=0;i<5;++i){unsigned list=0x6a4c20+i*4;global(groups+i*16,list);global(groups+i*16+4,0);global(groups+i*16+8,1);global(list,radio+i*0x40);}
 global(0x657c74,1);global(0x6b01b4,1);global(0x6de6c8,0x6a4f00);std::strcpy((char*)0x6a4f00,"CFG\\prefs.cfg");
 global(0x6de6d5,1);global(0x6de6d9,1);global(0x6de6e5,256);global(0x6de6f1,0);global(0x6de6f5,1);global(0x6de6fd,17);
 musicCalls=soundCalls=sampleCalls=closeCalls=initCalls=starts=0;writes.clear();mciStatus=0;mciDevice=0x212;profileResult=1;
}
int main(int argc,char** argv)try{
 check(argc==2,"PE required");map_image(read(argv[1]));
#define STUB(name,fn) stub(name##_address,name##_bytes,sizeof(name##_bytes),(void*)&fn)
 STUB(music_set,music_set);STUB(sound_set,sound_set);STUB(music_get,music_get);STUB(sound_get,sound_get);STUB(start_music,start_music);STUB(sample,sample);STUB(frame_set,frame_set);STUB(close_screen,close_screen);STUB(formatting,formatting);STUB(decimal,decimal);STUB(display_rebuild,display_rebuild);STUB(game_release,game_release);STUB(game_restore,game_restore);STUB(context_reset,context_reset);STUB(context_refresh,context_refresh);
 global(0x5c52dc,(unsigned)&mci);global(0x5c5090,(unsigned)&attributes);global(0x5c50bc,(unsigned)&directory);global(0x5c50b0,(unsigned)&profile);global(0x5c64b0,(unsigned)&initialize);
 auto callback=reinterpret_cast<Callback>(0x4a9840);auto slider=reinterpret_cast<Callback>(0x4a9700);auto enter=reinterpret_cast<Entry>(0x4a8af0);
 unsigned applied=0;
 for(int res=0;res<2;++res)for(int anim=2;anim<=3;++anim)for(int dialog=4;dialog<=6;++dialog)for(int speed=7;speed<=9;++speed)for(int border=10;border<=11;++border)for(int level:{2500,3750,5000}){
  reset();music=4;sound=-250;enter((void*)0x6a4948);check(word(0x6a49b0)==unsigned(-250)&&word(0x6a49b4)==4&&initCalls==1,"entry snapshots");music=9;sound=-900;enter((void*)0x6a4948);check(initCalls==1&&word(0x6a49b4)==4,"entry idempotence");
  const int ids[]={res,anim,dialog,speed,border};for(int i=0;i<5;++i)global(radio+i*0x40+0x2d,ids[i]);global(sliders+0x61,15);global(sliders+0xe7,level);
  check(callback((void*)0x6a4948,0)==0,"OK return");
  check(word(0x6de6d5)==unsigned(res==0)&&word(0x6de6f1)==unsigned(anim==3)&&word(0x6de6f5)==unsigned(dialog-4)&&word(0x6de6d9)==unsigned(border==10),"radio mapping");
  const unsigned fps=speed==7?20:speed==8?17:14;check(word(0x6de6fd)==fps&&word(0x6dbe99)==fps&&word(0x6de6f9)==1000/fps&&word(0x6dbe95)==1000/fps,"timing mapping");
  check(frame==unsigned((res==0?0x5e1720:0x5e1700)+(border==10?16:0))&&word(0x6c482c)==frame,"border pointer");
  check(music==15&&sound==level-5000&&musicCalls==1&&soundCalls==1&&closeCalls==1&&word(0x6a498b)==1,"OK effects");
  check(writes.at("SOUND/MusicVolume")=="15"&&writes.at("SOUND/SFXVolume")==std::to_string(level-5000)&&writes.at("VIDEO/MaxFramesPerSec")==std::to_string(fps)&&writes.at("VIDEO/DialogSpeed")==std::to_string(dialog-4)&&writes.at("VIDEO/TerrainLightLevels")=="256","persisted values");
  check(writes.at("VIDEO/IsHighRes")== (res==0?"TRUE":"FALSE")&&writes.at("VIDEO/CutDownAnims")== (anim==3?"TRUE":"FALSE")&&writes.at("VIDEO/WindowSize")==std::to_string(border==10),"persisted radios");++applied;
 }
 unsigned cancelled=0;
 for(int enabled=0;enabled<4;++enabled)for(int level:{2500,3750,5000}){reset();music=4;sound=-250;enter((void*)0x6a4948);global(sliders+0x61,15);global(sliders+0xe7,level);slider((void*)0x6a4948,0);slider((void*)0x6a4948,1);check(music==15&&sound==level-5000&&sampleCalls==1,"live slider preview");global(0x657c74,enabled&1);global(0x6b01b4,enabled&2);callback((void*)0x6a4948,1);check(music==4&&sound==-250&&writes.empty()&&closeCalls==1&&word(0x6a498b)==1&&word(0x6de6fd)==17,"Cancel rollback");++cancelled;}
 // Device availability gates final OK audio application, not slider callbacks.
 for(int enabled=0;enabled<4;++enabled){reset();global(0x657c74,enabled&1);global(0x6b01b4,enabled&2);for(int i=0;i<5;++i)global(radio+i*0x40+0x2d,std::array<int,5>{0,2,4,7,10}[i]);global(sliders+0x61,0);global(sliders+0xe7,5000);callback((void*)0x6a4948,0);check(musicCalls==unsigned(bool(enabled&1))&&soundCalls==unsigned(bool(enabled&2)),"device gates");}
 for(unsigned status:{0u,1u})for(unsigned device:{0x212u,0x20du,0x20cu,0u}){reset();mciStatus=status;mciDevice=device;global(sliders+0x61,9);slider((void*)0x6a4948,0);check(starts==unsigned(!status&&device!=0),"MCI restart branch");}
 reset();profileResult=0;for(int i=0;i<5;++i)global(radio+i*0x40+0x2d,std::array<int,5>{0,2,4,7,10}[i]);callback((void*)0x6a4948,0);check(!writes.empty()&&closeCalls==1&&word(0x6a498b)==1,"original ignores profile failure");
 reset();global(sliders+0x61,0);slider((void*)0x6a4948,0);check(music==0&&musicCalls==1&&!starts,"music zero skips MCI restart");
 reset();callback((void*)0x6a4948,2);slider((void*)0x6a4948,2);check(writes.empty()&&!closeCalls&&!soundCalls&&!musicCalls,"unknown indices inert");
 unsigned leaveCases=0;global(0x5c64ac,(unsigned)&release_controls);auto leave=reinterpret_cast<Entry>(0x4a8b60);
 for(unsigned game:{0u,1u})for(unsigned changed:{0u,1u}){
  reset();global(0x6cbb80,game);global(0x6a4950,1);global(0x6a497b,0x12345678);*(unsigned char*)0x6a49af=changed;leaveOrder.clear();
  leave((void*)0x6a4948);const std::string expected=game?(changed?"DSBRCF":"DSRCF"):(changed?"DBC":"D");
  check(leaveOrder==expected,"leave dependency ordering");check(word(0x6a4950)==0&&word(0x6a497b)==0&&!*(unsigned char*)0x6a49af,"leave clears initialized, next and rebuild flag");++leaveCases;
  if(changed){leaveOrder.clear();leave((void*)0x6a4948);check(leaveOrder==(game?"DSRCF":"D"),"repeat leave must not rebuild again");++leaveCases;}
 }
 std::cout<<"{\"ok_cases\":"<<applied<<",\"leave_cases\":"<<leaveCases<<",\"cancel_cases\":"<<cancelled<<",\"device_cases\":4,\"mci_cases\":8,\"unknown_case\":1,\"profile_failure_case\":1,\"music_zero_case\":1,\"success\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
