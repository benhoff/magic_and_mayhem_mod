/* Call executable cached export addresses, rather than wrapper C functions. */
#define MNM_RESERVATION_FIXTURE_ENTRY reservation_fixture_start
#include "drawing-source-reservations-test.c"
#undef MNM_RESERVATION_FIXTURE_ENTRY
static u8* entry_module;
static const u32 entry_rvas[4]={0x204490,0x204bc0,0x1df0d0,0x221460};
static u32 entry_case,entry_copies;
static u32 __attribute__((cdecl)) entry_copy(void* context,void* dst,u32 sub,u32 x,u32 y,u32 z,void* src,u32 srcsub,const u32* box,u32 flags){
 check(context==(void*)9&&dst==(void*)3&&!sub&&!x&&!y&&!z&&src==(void*)1&&!srcsub&&!box&&!flags);
 ++entry_copies;check_fp(expected_fp);check(GetLastError()==0x1234);
 __asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);return 0x8876086cu;
}
static void entry_stub(u32 i,const u8* code,u32 size,void* target){
 u8* site=entry_module+entry_rvas[i];copy(site,code,size);site[size]=0xe9;
 u32 delta=(u32)target-(u32)site-size-5;copy(site+size+1,&delta,4);
 check(FlushInstructionCache(GetCurrentProcess(),site,size+5));
}
static void entry_prepare(void){
 entry_module=VirtualAlloc(0,0x222000,0x3000,0x40);check(entry_module!=0);
 /* Complete pinned prefixes, then undo their stack effect and tail-call an
  * owned public-ABI body. This exercises each real trampoline under Wine. */
 const u8 map[]={0x55,0x89,0xe5,0x53,0x83,0xec,0x24,0x83,0xc4,0x24,0x5b,0x5d};
 const u8 unmap[]={0x55,0x89,0xe5,0x83,0xec,0x18,0x83,0xc4,0x18,0x5d};
 const u8 copy_body[]={0x55,0x89,0xe5,0x57,0x56,0x53,0x5b,0x5e,0x5f,0x5d};
 const u8 rebind[]={0x8d,0x4c,0x24,0x04,0x83,0xe4,0xf8,0x8d,0x61,0xfc};
 entry_stub(0,map,sizeof(map),(void*)&map_resource);entry_stub(1,unmap,sizeof(unmap),(void*)&unmap_resource);
 entry_stub(2,copy_body,sizeof(copy_body),(void*)&entry_copy);entry_stub(3,rebind,sizeof(rebind),(void*)&rebind_original);
 wine_resource_original[1]=entry_module+entry_rvas[0];wine_resource_original[2]=entry_module+entry_rvas[1];
 wine_copy_original=entry_module+entry_rvas[2];wine_source_version_rebind_original=entry_module+entry_rvas[3];
}
static void entry_map_call(void){
 u32 out[3]={0};check(((u32(__attribute__((cdecl))*)(void*,u32,u32*,const u32*,u32))(entry_module+entry_rvas[0]))((void*)3,0,out,0,0x40000000u)==0);
 check(out[0]==80&&out[1]==3200&&out[2]==(u32)pixels);
 check(((u32(__attribute__((cdecl))*)(void*,u32))(entry_module+entry_rvas[1]))((void*)3,0)==0);
}
static void entry_copy_call(void){
 SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
 check(((u32(__attribute__((cdecl))*)(void*,void*,u32,u32,u32,u32,void*,u32,const u32*,u32))(entry_module+entry_rvas[2]))((void*)9,(void*)3,0,0,0,0,(void*)1,0,0,0)==0x8876086cu);
 check(GetLastError()==0x4321);check_fp(result_fp);
}
static void entry_rebind_call(void){
 SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
 check(((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))(entry_module+entry_rvas[3]))((void*)4,0,pixels,80)==0x8876086cu);
 check(GetLastError()==0x4321);check_fp(result_fp);
}
static u32 __attribute__((cdecl)) entry_blit(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 if(entry_case==1)entry_map_call();
 if(entry_case==2)entry_copy_call();
 if(entry_case==3)entry_rebind_call();
 return reserve_blit(context,dst,dstsub,dstrect,src,srcsub,srcrect,flags,fx,filter);
}
void start(void){
 check(initialize());char value[2];if(GetEnvironmentVariableA("MNM_TEST_ENTRY",value,2)==1)entry_case=value[0]-'0';
 pixels=VirtualAlloc(0,4096,0x3000,4);check(pixels!=0);for(u32 i=0;i<3200;++i)pixels[i]=(u8)(i*7+3);
 char name[600];path(name,"fixture-worker-forwards.bin");forwards=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(forwards!=(HANDLE)-1);
 wine_resource_original[0]=(void*)&get_resource;wine_source_descriptor_get=&get_desc;
 wine_submission_original[0]=(void*)&entry_blit;wine_blit_worker_original=(void*)&worker_original;
 wine_gpu_current=&reserve_context;wine_gpu_proc=&reserve_proc;
 wine_lifetime_create((void*)2,1);wine_lifetime_create((void*)4,1);entry_prepare();
 if(entry_case>=4&&entry_case<=7){
  u32 which=entry_case-4;entry_module[entry_rvas[which]]^=1;
  check(!wine_mutator_entry_patch(entry_module));
  for(u32 i=0;i<4;++i)check(entry_module[entry_rvas[i]]!=(u8)0xe9);
  check(wine_resource_original[1]==entry_module+entry_rvas[0]&&wine_resource_original[2]==entry_module+entry_rvas[1]&&wine_copy_original==entry_module+entry_rvas[2]&&wine_source_version_rebind_original==entry_module+entry_rvas[3]);
  wine_submission_close(1);check(CloseHandle(forwards));ExitProcess(0);
 }
 check(wine_mutator_entry_patch(entry_module));
 if(wine_reserve_enabled)check(!wine_mutator_entry_patch(entry_module));
 entry_map_call();entry_copy_call();entry_rebind_call();check(entry_copies==1);
 check(wine_submission_blt((void*)9,(void*)1,0,0,(void*)2,0,0,0,0,1)==0);check(original_blits==1&&original_workers==1);
 wine_submission_close(1);
 /* Patched entries remain callable after recorder close, forwarding once. */
 entry_map_call();entry_copy_call();entry_rebind_call();check(entry_copies==2);
 check(CloseHandle(forwards));ExitProcess(0);
}
