/* Exercise the real wrappers and the existing two-thread version fixture. */
#define start version_fixture_start
#include "drawing-source-version-test.c"
#undef start
static u32 reserve_case;
static void* WIN reserve_context(void){return reserve_case==1?0:(void*)20;}
static void* WIN reserve_fence(u32 condition,u32 flags){check(condition==0x9117&&!flags);return (void*)21;}
static u32 WIN reserve_wait(void* sync,u32 flags,unsigned long long timeout){check(sync==(void*)21&&flags==1&&timeout==1000000000ULL);if(reserve_case==9){EnterCriticalSection(journal_lock);wine_source_version_write((void*)3,0,2,1);LeaveCriticalSection(journal_lock);}return 0x911c;}
static void WIN reserve_delete(void* sync){check(sync==(void*)21);}
static void* WIN reserve_proc(const char* name){
 if(equal(name,"glFenceSync",12))return (void*)&reserve_fence;
 if(equal(name,"glClientWaitSync",17))return (void*)&reserve_wait;
 if(equal(name,"glDeleteSync",13))return (void*)&reserve_delete;
 return 0;
}
static u32 __attribute__((cdecl)) reserve_blit(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 if(reserve_case==2){wine_lifetime_texture_decref((void*)4);ExitProcess(211);}
 if(reserve_case==3){EnterCriticalSection(journal_lock);wine_source_version_write((void*)3,0,2,1);LeaveCriticalSection(journal_lock);ExitProcess(211);}
 if(reserve_case==4){EnterCriticalSection(journal_lock);wine_reserve_close(1);LeaveCriticalSection(journal_lock);ExitProcess(211);}
 if(reserve_case==5){u32 sample[64]={0};sample[2]=100;sample[3]=99;sample[6]=2;sample[7]=wine_lifetime_find((void*)2,1);wine_reserve_begin(sample,(void*)2,(void*)3);}
 u32 result=blit_original(context,dst,dstsub,dstrect,src,srcsub,srcrect,flags,fx,filter);
 if(reserve_case==5){EnterCriticalSection(journal_lock);wine_source_version_write((void*)3,0,2,1);LeaveCriticalSection(journal_lock);ExitProcess(211);}
 if(reserve_case==6){EnterCriticalSection(journal_lock);wine_source_version_write((void*)3,0,2,1);wine_source_version_write((void*)3,0,2,2);LeaveCriticalSection(journal_lock);}
 return result;
}
static u32 __attribute__((cdecl)) reserve_decref_original(void* texture){
 check(texture==(void*)4);check_fp(expected_fp);check(GetLastError()==0x1234);
 if(reserve_case==10){u32 sample[64]={0};sample[2]=100;sample[6]=2;sample[7]=wine_lifetime_find((void*)2,1);wine_reserve_begin(sample,(void*)2,(void*)3);ExitProcess(211);}
 __asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);return 1;
}
#ifndef MNM_RESERVATION_FIXTURE_ENTRY
#define MNM_RESERVATION_FIXTURE_ENTRY start
#endif
void MNM_RESERVATION_FIXTURE_ENTRY(void){
 check(initialize());char value[2];if(GetEnvironmentVariableA("MNM_TEST_VERSION",value,2)==1)mode=value[0]=='a'?10:value[0]-'0';
 if(GetEnvironmentVariableA("MNM_TEST_RESERVATION",value,2)==1)reserve_case=value[0]=='a'?10:value[0]=='b'?11:value[0]-'0';
 pixels=VirtualAlloc(0,4096,0x3000,4);check(pixels!=0);for(u32 i=0;i<3200;++i)pixels[i]=(u8)(i*7+3);
 char name[600];path(name,"fixture-worker-forwards.bin");forwards=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(forwards!=(HANDLE)-1);
 wine_resource_original[0]=(void*)&get_resource;wine_resource_original[1]=(void*)&map_resource;wine_resource_original[2]=(void*)&unmap_resource;
 wine_source_descriptor_get=&get_desc;wine_submission_original[0]=(void*)&reserve_blit;wine_blit_worker_original=(void*)&worker_original;wine_source_version_rebind_original=(void*)&rebind_original;
 wine_gpu_current=&reserve_context;wine_gpu_proc=&reserve_proc;
 wine_lifetime_create((void*)2,1);wine_lifetime_create((void*)4,1);
 if(reserve_case>=10){
  wine_lifetime_original[1]=(void*)&reserve_decref_original;
  SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
  check(wine_lifetime_texture_decref((void*)4)==1);check(GetLastError()==0x4321);check_fp(result_fp);
 }

 check(wine_submission_blt((void*)9,(void*)1,0,0,(void*)2,0,0,0,0,1)==0);check(original_blits==1&&original_workers==1);
 wine_submission_close(1);check(CloseHandle(forwards));ExitProcess(0);
}
