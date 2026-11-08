/* Actual production DLL versus unchanged relocated NoCD image-copy bytes.
 * Private fixture only: no game entrypoint, original file modification, or
 * original output as a native input. Differences are diagnostic, never admission. */
#include "../runtime/shadow/win32_min.h"
#include "../runtime/native-draw/image_copy_original.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
__declspec(allocate(".own")) static u8 address_space[0x200000] __attribute__((used));
u32 image_state_saved_sp,image_state_args[6],image_state_object,image_state_function;
u32 image_state_seed_flags,image_state_after[15];
u32 image_state_stack_padding;
u8 image_state_host_fx[512] __attribute__((aligned(16)));
u8 image_state_seed_fx[512] __attribute__((aligned(16)));
u8 image_state_after_fx[512] __attribute__((aligned(16)));
void mnm_image_state_call(void);
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void write_all(HANDLE f,const void* p,u32 n){u32 got;require(WriteFile(f,p,n,&got,0)&&got==n,180);}
static void copy_bytes(void* out,const void* in,u32 n){for(u32 i=0;i<n;++i)((u8*)out)[i]=((const u8*)in)[i];}
void start(void){
 char library[1024],output[1024];
 require(GetEnvironmentVariableA("MNM_TEST_LIBRARY",library,sizeof(library))>0,181);
 require(GetEnvironmentVariableA("MNM_STATE_OUTPUT",output,sizeof(output))>0,182);
 void* module=LoadLibraryA(library);require(module!=0,183);
 u32 old;require(VirtualProtect((void*)0x533000,4096,0x40,&old),184);
 copy_bytes((void*)0x533a00,ns_image_copy_original,sizeof(ns_image_copy_original));
 copy_bytes((void*)0x533d00,ns_image_copy_original,sizeof(ns_image_copy_original));
 require(VirtualProtect((void*)0x5c5000,4096,4,&old),185);
 void* user=LoadLibraryA("user32.dll");require(user!=0,186);
 *(void**)0x5c5294=GetProcAddress(user,"IntersectRect");require(*(void**)0x5c5294!=0,187);
 i32(WIN* create)(void*,void**,void*)=GetProcAddress(module,"NativeDrawCreate");
 void* draw=0;require(create&&!create(0,&draw,0),188);
 for(u32 i=0;i<sizeof(ns_image_copy_original);++i)require(((u8*)0x533d00)[i]==ns_image_copy_original[i],192);
 /* A handled full native call may execute none of the retired original body. */
 for(u32 i=8;i<sizeof(ns_image_copy_original);++i)((u8*)0x533a00)[i]=0xcc;
 static u8 source[4096],expected[4096],actual[4096],original_fx[512];
 for(u32 i=0;i<4096;++i)source[i]=(u8)(i*17+3);
 char direction[2];
 if(GetEnvironmentVariableA("MNM_IMAGE_DIRECTION_REFUSAL",direction,sizeof(direction))==1&&direction[0]=='1'){
  u32 image[5]={(u32)(source+1024),0,1,1,1};i32 clip[4]={0,0,1,1};
  __asm__ volatile("fninit;fxsave %0":"=m"(image_state_seed_fx));
  image_state_object=(u32)image;image_state_function=0x533a00;image_state_seed_flags=0x602;
  image_state_args[0]=(u32)(actual+512);image_state_args[1]=64;image_state_args[2]=0;
  image_state_args[3]=0;image_state_args[4]=0;image_state_args[5]=(u32)clip;
  mnm_image_state_call();ExitProcess(194); /* Expected native terminal refusal124. */
 }
 HANDLE file=CreateFileA(output,0x40000000,0,0,1,0x80,0);require(file!=(HANDLE)-1,189);
 const u32 header[4]={0x494d4e4d,0x31305343,1,164}; /* MNMIS C01, diagnostic v1 */
 write_all(file,header,sizeof(header));
 u32 ordinal=0;const u32 flag_seeds[4]={0x202,0xad7,0x297,0xa42};
 for(u32 format=0;format<2;++format)for(i32 x=-4;x<7;++x)for(i32 y=-3;y<5;++y)
 for(u32 clip_mode=0;clip_mode<9;++clip_mode)for(u32 seed=0;seed<12;++seed){
  for(u32 i=0;i<4096;++i)expected[i]=actual[i]=0xa5;
  u32 image[5]={(u32)(source+1024),0,1+((u32)(x+4)*7+format)%31,9,format};
  i32 clip[4]={clip_mode==1?3:0,clip_mode==2?2:0,clip_mode==1?11:26,clip_mode==2?7:9};
  if(clip_mode==3){clip[0]=100;clip[1]=100;clip[2]=110;clip[3]=110;}
  if(clip_mode==4)clip[2]=clip[0];
  if(clip_mode==5)image[2]=0;
  if(clip_mode==6)image[3]=0;
  if(clip_mode==7){image[2]=0;image[3]=0;}
  __asm__ volatile("fninit;fld1;fldpi;fldl2e;fxsave %0":"=m"(image_state_seed_fx));
  const u32 precision=(seed/4)==0?0:(seed/4)==1?2:3;
  image_state_seed_fx[1]=(u8)((image_state_seed_fx[1]&0xf0u)|precision|((seed%4)<<2));
  /* Deterministic nonzero SIMD payload and masked SSE status/rounding inputs. */
  *(u32*)(image_state_seed_fx+24)=0x1f80u|((seed%4)<<13)|(seed&7u);
  for(u32 i=160;i<288;++i)image_state_seed_fx[i]=(u8)(i*37+seed*19);
  for(u32 i=3;i<8;++i)for(u32 j=0;j<16;++j)image_state_seed_fx[32+i*16+j]=0;
  image_state_object=(u32)image;image_state_seed_flags=flag_seeds[seed%4];
  image_state_stack_padding=(seed%4)*4;
  u32 destination_offset=clip_mode==8?2048:512;
  image_state_args[0]=(u32)(expected+destination_offset);image_state_args[1]=clip_mode==8?(u32)-64:64;image_state_args[2]=0;
  image_state_args[3]=(u32)x;image_state_args[4]=(u32)y;image_state_args[5]=(u32)clip;
  u32 row[41];row[0]=++ordinal;row[1]=format;row[2]=(u32)x;row[3]=(u32)y;
  row[4]=clip_mode;row[5]=seed;row[6]=(u32)(expected+destination_offset);row[7]=(u32)(actual+destination_offset);
  image_state_function=0x533d00;SetLastError(0x71357);mnm_image_state_call();
  copy_bytes(row+8,image_state_after,60);row[23]=GetLastError();copy_bytes(original_fx,image_state_after_fx,512);
  image_state_args[0]=(u32)(actual+destination_offset);image_state_function=0x533a00;
  SetLastError(0x71357);mnm_image_state_call();copy_bytes(row+24,image_state_after,60);row[39]=GetLastError();
  u32 checks=0;int pixels=1,source_unchanged=1,fx=1;
  for(u32 i=0;i<4096;++i){if(actual[i]!=expected[i])pixels=0;if(source[i]!=(u8)(i*17+3))source_unchanged=0;}
  for(u32 i=0;i<512;++i)if(!(i>=6&&i<24)&&original_fx[i]!=image_state_after_fx[i])fx=0;
  if(pixels)checks|=1;if(source_unchanged)checks|=2;if(fx)checks|=4;
  row[40]=checks;write_all(file,row,sizeof(row));
 }
 for(u32 i=0;i<sizeof(ns_image_copy_original);++i)require(((u8*)0x533d00)[i]==ns_image_copy_original[i],193);
 require(CloseHandle(file),190);
 u32(WIN* release)(void*)=(*(void***)draw)[2];require(!release(draw),191);
 ExitProcess(0);
}
