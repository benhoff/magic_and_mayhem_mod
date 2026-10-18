/* Execute raw PE32 loads/stores, not a model of exception dispatch. */
#define start retained_resource_probe
#include "native-draw-resource-probe.c"
#undef start
#include "../protocols/include/mnm/native_gpu_copy_plan_v1.h"
API void* WIN AddVectoredExceptionHandler(u32,void*);
API i32 WIN FreeLibrary(void*);
typedef i32 (WIN *History)(const void*,u32,u32,u32,u32,u32);
typedef i32 (WIN *Transfer)(void*,u32*,const u16*,u32);
typedef i32 (WIN *LeaseSnapshot)(void*,const u32*,u16*,u32);
typedef i32 (WIN *WorldGuard)(const void*);
typedef i32 (WIN *Gpu)(void*,const i32*,void*,const i32*,u32,u16*,u32);
typedef i32 (WIN *Plan)(void*,const i32*,void*,const i32*,u32,struct MnmGpuCopyPlan*,u32);
static void save(const char* name,const void* bytes,u32 n){
 u32 done=0;HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 require(f!=(HANDLE)-1 && WriteFile(f,bytes,n,&done,0) && done==n,201);CloseHandle(f);
}
static u32 protection(void* p){u32 m[7];require(VirtualQuery(p,m,28)==28,202);return m[5];}
static i32 WIN unloaded_fault(void* p){(void)p;ExitProcess(0);return 0;}
static i32 WIN foreign_fault(void* p){(void)p;ExitProcess(123);return 0;}
void start(void){
 char mode[4];require(GetEnvironmentVariableA("MNM_CPU_GUARD_CASE",mode,4)==1,101);
 void* module=LoadLibraryA("MnmDraw.dll");require(module!=0,102);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 Transfer transfer=(Transfer)GetProcAddress(module,"NativeDrawCpuTransferV1");
 LeaseSnapshot lease_snapshot=(LeaseSnapshot)GetProcAddress(module,"NativeDrawCpuLeaseSnapshotV1");
 WorldGuard world_guard=(WorldGuard)GetProcAddress(module,"NativeDrawCpuGuardWorldV1");
 Gpu gpu=(Gpu)GetProcAddress(module,"NativeDrawGpuCopyV1");
 Plan plan=(Plan)GetProcAddress(module,"NativeDrawGpuCopyPlanV1");
 Count surfaces=(Count)GetProcAddress(module,"NativeDrawSurfaces");
 void *draw=0,*s=0,*other=0;u32 d[31];
 require(create && history && transfer && lease_snapshot && world_guard && gpu && plan && surfaces && !create(0,&draw,0),103);
 desc(d,2);require(!((Surface)table(draw)[6])(draw,d,&s,0),104);
 desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),105);volatile u16* pixels=(u16*)d[9];u32 pitch=d[4];
 pixels[0]=0x1234;require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)pixels),106);
 require(protection((void*)pixels)==4,107);
 if(mode[0]=='0' || mode[0]=='u')goto success;
 void* dc=0;
 if(mode[0]=='h'){
  require(!((i32 (WIN *)(void*,void**))table(s)[17])(s,&dc),158);
  require(history((void*)pixels,1,2,0,5,6)<0 && !history((void*)pixels,2,0,0,0,0),159);
  require(!((i32 (WIN *)(void*,void*))table(s)[26])(s,dc),160);
 }
 require(!history((void*)pixels,1,2,0,5,6),108);
 if(mode[0]=='d'){require(protection((void*)pixels)==4 && pixels[0]==0x1234,109);goto success;}
 require(protection((void*)pixels)==1 && history((void*)pixels,2,0,0,0,0)==(i32)0x887601aeu,110);
 if(mode[0]=='e'){
  u32 old;require(VirtualProtect((void*)pixels,4096,2,&old) && old==1,161);
  desc(d,2);((Lock)table(s)[25])(s,0,d,1,0);ExitProcess(162);
 }
 if(mode[0]=='K' || mode[0]=='W' || mode[0]=='U'){
  u32 layout[4]={(u32)pixels,13,9,pitch};u16 private_bytes[117];
  require(lease_snapshot(s,layout,private_bytes,sizeof(private_bytes))<0 && protection((void*)pixels)==1,174);
  desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),175);
  if(mode[0]=='K'){
   for(u32 i=0;i<4;++i){layout[i]^=1;require(lease_snapshot(s,layout,private_bytes,sizeof(private_bytes))<0 && protection((void*)pixels)==1,176);layout[i]^=1;}
   require(lease_snapshot(s,layout,(u16*)pixels,sizeof(private_bytes))<0 && protection((void*)pixels)==1,177);
   require(lease_snapshot(s,layout,private_bytes,sizeof(private_bytes)-2)<0 && protection((void*)pixels)==1,178);
   require(!lease_snapshot(s,layout,private_bytes,sizeof(private_bytes)) && private_bytes[0]==0x1234 && protection((void*)pixels)==1,179);
   pixels[1]=0xabcd;
   require(!lease_snapshot(s,layout,private_bytes,sizeof(private_bytes)) && private_bytes[1]==0xabcd && history((void*)pixels,2,0,0,0,0)<0 && protection((void*)pixels)==1,180);
   require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)pixels),181);
   require(lease_snapshot(s,layout,private_bytes,sizeof(private_bytes))<0 && protection((void*)pixels)==1,182);
   u32 identity[8];for(u32 i=0;i<117;++i)private_bytes[i]=0x2222;
   require(!transfer(s,identity,0,0) && !transfer(s,identity,private_bytes,sizeof(private_bytes)),183);
   goto success;
  }
  require(world_guard((const u8*)pixels+2)<0 && protection((void*)pixels)==1,184);
  require(world_guard((void*)pixels)==1 && protection((void*)pixels)==4 && world_guard((void*)pixels)<0,185);
  if(mode[0]=='U'){((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)pixels);ExitProcess(186);}
  u32 old;require(VirtualProtect((void*)pixels,4096,1,&old) && old==4,187);
  require(VirtualProtect((void*)pixels,4096,4,&old) && old==1,188);
  require(!lease_snapshot(s,layout,private_bytes,sizeof(private_bytes)) && private_bytes[0]==0x1234,189);
  require(!history((void*)pixels,1,3,0,5,6) && protection((void*)pixels)==1,190);
  u32 witness[5]={(u32)pixels,pitch,13,9,(u32)mode[0]};save("cpu-guard-input.bin",witness,sizeof(witness));
  volatile u16 got=pixels[0];(void)got;ExitProcess(191);
 }
 if(mode[0]=='A' || mode[0]=='S' || mode[0]=='R' || mode[0]=='P' || mode[0]=='B'){
  desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),163);
  require(protection((void*)pixels)==1,164);
  require(history((void*)pixels,3,pitch*9,0,0,0)==1 && history((const u8*)pixels+pitch*9-1,3,2,0,0,0)==0 && history((void*)pixels,3,0,0,0,0)==0,192);
  u32 witness[5]={(u32)pixels,pitch,13,9,(u32)mode[0]};save("cpu-guard-input.bin",witness,sizeof(witness));
  if(mode[0]=='R'){__asm__ volatile("addw $1, (%0)"::"r"(pixels):"memory","cc");ExitProcess(165);}
  if(mode[0]=='P'){u16* to=(u16*)pixels;u32 count=2;__asm__ volatile("rep stosw":"+D"(to),"+c"(count):"a"(0xabcd):"memory");ExitProcess(166);}
  /* Exercise register/immediate stores at byte, WORD and DWORD widths. */
  __asm__ volatile("movb $0x56, (%0)\n\tmovw $0x789a, 2(%0)\n\tmovl %1, 4(%0)"::"r"(pixels),"r"(0xabcdef01u):"memory");
  require(protection((void*)pixels)==1,167);
  u32 old=0;require(VirtualProtect((void*)pixels,4096,4,&old)&&old==1,168);
  require(((volatile u8*)pixels)[0]==0x56 && pixels[1]==0x789a && ((volatile u32*)pixels)[1]==0xabcdef01u,169);
  require(VirtualProtect((void*)pixels,4096,1,&old)&&old==4,170);
  if(mode[0]=='B')require(!history((void*)pixels,1,3,0,5,6),171);
  if(mode[0]=='S')require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)pixels),172);
  volatile u16 got=pixels[0];(void)got;ExitProcess(173);
 }
 if(mode[0]=='l' || mode[0]=='a'){
  desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),111);
  require(protection((void*)pixels)==4 && pixels[0]==0x1234,112);
  pixels[1]=0xabcd;require(history((void*)pixels,2,0,0,0,0)<0,113);
  require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)pixels),114);
  require(protection((void*)pixels)==1,115);
  if(mode[0]=='a'){require(!release(s) && !release(draw) && !surfaces(),116);ExitProcess(0);}
 }
 if(mode[0]=='t' || mode[0]=='n'){
  u32 identity[8];u16 reply[117];for(u32 i=0;i<117;++i)reply[i]=0x2222;
  require(!transfer(s,identity,0,0),117);
  identity[7]^=1;require(transfer(s,identity,reply,sizeof(reply))<0 && protection((void*)pixels)==1,118);identity[7]^=1;
  require(transfer(s,identity,reply,sizeof(reply)-2)<0 && protection((void*)pixels)==1,119);
  if(mode[0]=='t'){
   require(!transfer(s,identity,reply,sizeof(reply)),120);
   require(protection((void*)pixels)==4 && pixels[0]==0x2222 && pixels[8*(pitch/2)+12]==0x2222,121);
   require(transfer(s,identity,0,0)==1 && !history((void*)pixels,2,0,0,0,0),122);goto success;
  }
 }
 if(mode[0]=='f' || mode[0]=='p'){
  u32 fx[25]={100};fx[20]=0x7fff;i32 rect[4]={0,0,1,1};
  require(!((i32 (WIN *)(void*,const i32*,void*,void*,u32,u32*))table(s)[5])(s,mode[0]=='p'?rect:0,0,0,0x400,fx),124);
  if(mode[0]=='f'){require(protection((void*)pixels)==4 && pixels[0]==0x7fff && !history((void*)pixels,2,0,0,0,0),125);goto success;}
  require(protection((void*)pixels)==1 && history((void*)pixels,2,0,0,0,0)<0,126);
 }
 if(mode[0]=='b' || mode[0]=='c'){
  desc(d,2);require(!((Surface)table(draw)[6])(draw,d,&other,0),127);
  i32 rect[4]={0,0,13,9};u16 baseline[117];
  if(mode[0]=='b'){
   desc(d,2);require(!((Lock)table(other)[25])(other,0,d,1,0),128);u16* target=(u16*)d[9];target[0]=0x5678;
   require(!((i32 (WIN *)(void*,void*))table(other)[32])(other,target),129);
   require(!gpu(other,rect,s,rect,0,baseline,sizeof(baseline)) && baseline[0]==0x5678,130);
   require(protection(target)==1 && protection((void*)pixels)==1,131);pixels=target;
  }else{
   require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(s)[5])(s,0,other,0,0,0),132);
   require(protection((void*)pixels)==4 && pixels[0]==0 && !history((void*)pixels,2,0,0,0,0),133);goto success;
  }
 }
 if(mode[0]=='s'){
  struct MnmGpuCopyPlan* p=VirtualAlloc(0,sizeof(*p),0x3000,4);i32 rect[4]={0,0,13,9};
  require(p && !plan(s,rect,s,rect,0,p,sizeof(*p)) && p->self && p->count==1 && protection((void*)pixels)==1,134);
 }
 if(mode[0]=='o'){
  desc(d,2);((Lock)table(s)[25])(s,0,d,0x11,0);ExitProcess(135);
 }
 if(mode[0]=='v'){
  require(!release(s),136);s=0;
  require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),0x11),137);
  require(!((i32 (WIN *)(void*,u32,u32,u32))table(draw)[21])(draw,13,9,16),138);
  desc(d,2);d[1]=0x21;d[5]=1;d[26]=0x218;
  require(!((Surface)table(draw)[6])(draw,d,&s,0),139);u32 caps=4;
  require(!((i32 (WIN *)(void*,u32*,void**))table(s)[12])(s,&caps,&other),140);
  desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),141);pixels=(u16*)d[9];pixels[0]=0x4444;
  require(!history((void*)pixels,1,2,0,5,6) && protection((void*)pixels)==4,142);
  require(!((i32 (WIN *)(void*,void*,u32))table(s)[11])(s,0,0),143);
  require(!((i32 (WIN *)(void*,void*))table(other)[32])(other,(void*)pixels),144);
  require(protection((void*)pixels)==1,145);
  u32 identity[8];require(!transfer(other,identity,0,0) && identity[0]==(u32)pixels && identity[4]==2,146);
 }
 if(mode[0]=='q'){
  require(AddVectoredExceptionHandler(0,(void*)foreign_fault)!=0,147);
  volatile u32* foreign=VirtualAlloc(0,4096,0x3000,1);require(foreign!=0,148);*foreign=1;ExitProcess(149);
 }
 {
  u32 witness[5]={(u32)pixels,pitch,13,9,(u32)mode[0]};save("cpu-guard-input.bin",witness,sizeof(witness));
  volatile u16* access=mode[0]=='i'?(volatile u16*)((u8*)pixels+pitch*9-2):pixels;
  if(mode[0]=='w')*access=0x9999;else {volatile u16 got=*access;(void)got;}
  ExitProcess(150); /* A guard case must terminate before reaching this point. */
 }
success:
 require(pixels[0]==((mode[0]=='t'||mode[0]=='K')?0x2222:mode[0]=='f'?0x7fff:mode[0]=='c'?0:0x1234),151);
 if(other)require(!release(other),152);require(!release(s) && !release(draw) && !surfaces(),153);
 save("cpu-guard-success.bin",&checks,4);
 if(mode[0]=='u'){
  require(FreeLibrary(module) && !GetModuleHandleA("MnmDraw.dll"),154);
  require(AddVectoredExceptionHandler(0,(void*)unloaded_fault)!=0,155);
  volatile u32* foreign=VirtualAlloc(0,4096,0x3000,1);require(foreign!=0,156);*foreign=1;ExitProcess(157);
 }
 ExitProcess(0);
}
