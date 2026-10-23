/* Actual PE32 facade creation: strict mode, section-backed storage and leases. */
#include "../runtime/shadow/win32_min.h"
#include "../runtime/native-draw/image_copy_original.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
__declspec(allocate(".own")) static u8 owner_code[0x140000] __attribute__((used));
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
typedef i32(WIN* DirectStore)(void*,u32,u32);
static DirectStore direct_store;static void* foreign_target;static i32 foreign_result;
static u32 WIN foreign_store(void* unused){(void)unused;SetLastError(0x7337);foreign_result=direct_store(foreign_target,0xeeee,2);if(GetLastError()!=0x7337)ExitProcess(231);return 0;}
typedef i32(WIN* Create)(void*,void**,void*);
typedef i32(WIN* History)(const void*,u32,u32,u32,u32,u32);
typedef i32(WIN* Transfer)(void*,u32*,const u16*,u32);
static Transfer restore;static void* owned;static u32 reader_calls;
static i32(WIN* snapshot)(void*,const u32*,u16*,u32);
static i32(WIN* mask)(void*,const u32*,u8*,u32,u32);
static i32 WIN reader(void* object,const void* base,u32 family,u32 route,u32 caller){
 (void)base;if(object!=owned || family || route || !caller)return 0;
 u32 identity[12];u16 image[117],pending[117];u8 dirty[234];for(u32 i=0;i<117;++i)image[i]=0x4567;
 if(restore(object,identity,0,0))return 0;
 u32 layout[4]={identity[0],identity[1],identity[2],identity[3]};
 if(snapshot(object,layout,pending,sizeof(pending)) || mask(object,layout,dirty,sizeof(dirty),0)!=1)return 0;
 for(u32 i=0;i<sizeof(dirty);++i)if(dirty[i])((u8*)image)[i]=((u8*)pending)[i];
 if(restore(object,identity,image,sizeof(image)))return 0;
 ++reader_calls;return 1;
}
__attribute__((naked)) static void image_copy(const void* source __attribute__((unused)),void* destination __attribute__((unused)),u32 bytes __attribute__((unused))){
 __asm__ volatile("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi;movl 20(%esp),%eax;movl 24(%esp),%edx;movl 28(%esp),%ebp;xorl %ebx,%ebx;movl $0x533aec,%ecx;calll *%ecx;popl %edi;popl %esi;popl %ebp;popl %ebx;retl");
}
static void require(int value,u32 code){if(!value)ExitProcess(code);}
#include "native-continuation-state.h"
static void** table(void* object){return *(void***)object;}
static u32 release(void* object){return ((u32(WIN*)(void*))table(object)[2])(object);}
void start(void){
 char name[768];u32 n=GetEnvironmentVariableA("MNM_OWNER_DRAW_DLL",name,sizeof(name));require(n&&n<sizeof(name),170);
 void* module=LoadLibraryA(name);require(module!=0,171);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 Transfer transfer=(Transfer)GetProcAddress(module,"NativeDrawCpuLeaseTransferV1");
 require(create&&history&&transfer,172);
 direct_store=(DirectStore)GetProcAddress(module,"NativeDrawCpuStoreV1");require(direct_store!=0,230);
 u8* stub=(u8*)0x533000;u32 page[7],protection;
 require(VirtualQuery(stub,page,28)==28 && page[1]==0x400000 && page[4]==0x1000 && VirtualProtect(stub,4096,0x40,&protection),187);
 for(u32 i=0;i<sizeof(ns_image_copy_original);++i)stub[0xa00+i]=ns_image_copy_original[i];
 char mismatch[2];if(GetEnvironmentVariableA("MNM_OWNER_BAD_COPY_PREFIX",mismatch,2)==1)stub[0xaec]=0x90;
 void* draw=0;require(!create(0,&draw,0)&&draw,173);
 char simple[2];if(GetEnvironmentVariableA("MNM_OWNER_PROBE_ONLY_CREATE",simple,2)==1){require(!release(draw),174);ExitProcess(0);}
 /* Any return to the old REP body now traps, so success proves replacement. */
 for(u32 i=8;i<20;++i)stub[0xaec+i]=0xcc;
 stub[0xb00]=0xc3; /* Direct row ABI control returns after the replaced body. */
 u32 descriptor[27]={0};descriptor[0]=108;descriptor[1]=7;descriptor[2]=9;descriptor[3]=13;descriptor[26]=0x840;
 void* surface=0;require(!((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,descriptor,&surface,0)&&surface,175);
 descriptor[0]=108;require(!((i32(WIN*)(void*,const void*,u32*,u32,void*))table(surface)[25])(surface,0,descriptor,1,0),176);
 volatile u16* pixels=(volatile u16*)descriptor[9];u32 memory[7];
 require(VirtualQuery((void*)pixels,memory,28)==28&&memory[6]==0x40000&&memory[5]==4,177);
 require(!history((void*)pixels,1,1,0,17,34),178);
 require(VirtualQuery((void*)pixels,memory,28)==28&&memory[5]==1,179);
 pixels[0]=0x1234; /* The real mode3 handler executes this via the private view. */
 require(VirtualQuery((void*)pixels,memory,28)==28&&memory[5]==1,180);
 SetLastError(0x6776);require(!direct_store((void*)(pixels+1),0xabcd,2)&&GetLastError()==0x6776,232);
 require(!direct_store((void*)(pixels+1),0xabcd,2),233);
 require(!direct_store((u8*)pixels+4,0x78,1),234);
 u32 layout[4]={(u32)pixels,13,9,descriptor[4]};
 snapshot=GetProcAddress(module,"NativeDrawCpuLeaseSnapshotV1");mask=GetProcAddress(module,"NativeDrawCpuLeaseMaskV1");require(snapshot&&mask,235);
 require(mask(surface,layout,0,0,2)==3,236);
 require(!direct_store((u8*)pixels+5,0x56,1)&&mask(surface,layout,0,0,2)==1,237);
 u16 observed[117];u8 dirty_flags[234];require(!snapshot(surface,layout,observed,sizeof(observed)),238);
 require(observed[0]==0x1234&&observed[1]==0xabcd&&observed[2]==0x5678,239);
 require(mask(surface,layout,dirty_flags,sizeof(dirty_flags),0)==1,240);
 for(u32 i=0;i<234;++i)require(dirty_flags[i]==(i<6),241);
 require(VirtualQuery((void*)pixels,memory,28)==28&&memory[5]==1,242);
 require(direct_store((u8*)pixels+memory[3]-1,1,2)<0&&direct_store((void*)pixels,1,3)<0,243);
 foreign_target=(void*)pixels;HANDLE thread=CreateThread(0,0,foreign_store,0,0,0);require(thread&&WaitForSingleObject(thread,2000)==0&&CloseHandle(thread)&&foreign_result==(i32)0x887601aeu,244);

 u32 identity[12];require(!transfer(surface,identity,0,0),181);
 u16 image[117];for(u32 i=0;i<117;++i)image[i]=0x4567;
 require(!transfer(surface,identity,image,sizeof(image)),182);
 require(VirtualQuery((void*)pixels,memory,28)==28&&memory[5]==4&&pixels[0]==0x4567,183);
 restore=transfer;owned=surface;
 snapshot=GetProcAddress(module,"NativeDrawCpuLeaseSnapshotV1");mask=GetProcAddress(module,"NativeDrawCpuLeaseMaskV1");require(snapshot&&mask,195);
 i32(WIN* register_reader)(void*)=(void*)GetProcAddress(module,"NativeDrawCpuReaderV1");
 require(register_reader && !register_reader((void*)reader),188);
 require(!history((void*)pixels,1,2,0,17,34),189);
 const u8 source[7]={1,2,3,4,5,6,7};
 char bad_span[2];if(GetEnvironmentVariableA("MNM_OWNER_BAD_COPY_SPAN",bad_span,2)==1)image_copy(source,(u8*)pixels+descriptor[4]*9-2,sizeof(source));
 image_copy(source,(u8*)pixels+2,sizeof(source));
 require(reader_calls==1 && pixels[0]==0x4567,190);
 for(u32 i=0;i<sizeof(source);++i)require(((const u8*)pixels)[2+i]==source[i],191);
 for(u32 test=0;test<5;++test){
  require(!history((void*)pixels,1,3+test,0,17,34),196);
  ((volatile u8*)pixels)[6]=0x89;SetLastError(0x4321);
  t1_state_check(test,pixels,source);
  if(test==0)require(pixels[3]==0x4589,197);
  if(test==1)require(pixels[3]==(0x4589^0x1234),198);
  if(test==2){
   for(u32 i=0;i<sizeof(source);++i)require(((const u8*)pixels)[32+i]==source[i],199);
  }
  if(test==3){
   u32 expected[4]={0x12345678,0xabcdef01,0x87654321,0x10fedcba},observed[4];
   __asm__ volatile("movdqu (%0),%%xmm2;movdqu (%1),%%xmm0;movdqu %%xmm2,(%2)"::"r"(expected),"r"(pixels),"r"(observed):"memory");
   for(u32 i=0;i<4;++i)require(observed[i]==expected[i],200);
  }
  if(test==4)require(pixels[3]==0x458c,201);
  require(GetLastError()==0x4321 && reader_calls==test+2,202);
 }
 require(!history((void*)pixels,1,8,0,17,34),245);
 i32(WIN* world)(const void*,u32)=GetProcAddress(module,"NativeDrawCpuGuardWorldV2");require(world&&world((void*)pixels,1)==1,246);
 require(direct_store((void*)pixels,1,2)==(i32)0x887601aeu&&world((void*)pixels,0)==0,247);
 require(!history((void*)pixels,1,9,0,17,34),248);
 require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),184);
 require(!release(surface)&&!release(draw),185);
 u32 direct_result[9]={0x52545344,1,4,6,6,1,1,1,1},direct_written=0;
 HANDLE direct_file=CreateFileA("direct-store-result.bin",0x40000000,0,0,1,0x80,0);
 require(direct_file!=(HANDLE)-1&&WriteFile(direct_file,direct_result,sizeof(direct_result),&direct_written,0)&&direct_written==sizeof(direct_result)&&CloseHandle(direct_file),249);
 u32 states[3]={5,45,t1_fx_checks},state_written=0;
 HANDLE state_file=CreateFileA("continuation-state.bin",0x40000000,0,0,1,0x80,0);
 require(state_file!=(HANDLE)-1&&WriteFile(state_file,states,sizeof(states),&state_written,0)&&state_written==sizeof(states)&&CloseHandle(state_file),221);
 u32 result[8]={0x4d4e4f57,1,13,9,0x40000,1,1,1},written=0;
 HANDLE output=CreateFileA("backing-owner-result.bin",0x40000000,0,0,1,0x80,0);
 require(output!=(HANDLE)-1&&WriteFile(output,result,sizeof(result),&written,0)&&written==sizeof(result)&&CloseHandle(output),186);
 ExitProcess(0);
}
