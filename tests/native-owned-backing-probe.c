/* Actual PE32 facade creation: strict mode, section-backed storage and leases. */
#include "../runtime/shadow/win32_min.h"
#include "../runtime/native-draw/image_copy_original.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
__declspec(allocate(".own")) static u8 owner_code[0x140000] __attribute__((used));
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
static void** table(void* object){return *(void***)object;}
static u32 release(void* object){return ((u32(WIN*)(void*))table(object)[2])(object);}
void start(void){
 char name[768];u32 n=GetEnvironmentVariableA("MNM_OWNER_DRAW_DLL",name,sizeof(name));require(n&&n<sizeof(name),170);
 void* module=LoadLibraryA(name);require(module!=0,171);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 Transfer transfer=(Transfer)GetProcAddress(module,"NativeDrawCpuLeaseTransferV1");
 require(create&&history&&transfer,172);
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
  if(test==0)require(pixels[3]==0x4589,197);
  if(test==1){pixels[3]^=0x1234;require(pixels[3]==(0x4589^0x1234),198);}
  if(test==2){
   const u8* input=source;u8* output=(u8*)pixels+32;u32 count=sizeof(source);
   __asm__ volatile("rep movsb":"+S"(input),"+D"(output),"+c"(count)::"memory");
   for(u32 i=0;i<sizeof(source);++i)require(((const u8*)pixels)[32+i]==source[i],199);
  }
  if(test==3){
   u32 expected[4]={0x12345678,0xabcdef01,0x87654321,0x10fedcba},observed[4];
   __asm__ volatile("movdqu (%0),%%xmm2;movdqu (%1),%%xmm0;movdqu %%xmm2,(%2)"::"r"(expected),"r"(pixels),"r"(observed):"memory");
   for(u32 i=0;i<4;++i)require(observed[i]==expected[i],200);
  }
  if(test==4){__asm__ volatile("lock addw $3,(%0)"::"r"(pixels+3):"memory","cc");require(pixels[3]==0x458c,201);}
  require(GetLastError()==0x4321 && reader_calls==test+2,202);
 }
 require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),184);
 require(!release(surface)&&!release(draw),185);
 u32 result[8]={0x4d4e4f57,1,13,9,0x40000,1,1,1},written=0;
 HANDLE output=CreateFileA("backing-owner-result.bin",0x40000000,0,0,1,0x80,0);
 require(output!=(HANDLE)-1&&WriteFile(output,result,sizeof(result),&written,0)&&written==sizeof(result)&&CloseHandle(output),186);
 ExitProcess(0);
}
