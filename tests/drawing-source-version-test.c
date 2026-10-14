#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
__declspec(dllimport) HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
__declspec(dllimport) u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 mode,original_blits,original_workers,packet[25],getter_calls;
static u8* pixels;
static HANDLE forwards;
static u8 expected_fp[512] __attribute__((aligned(16))),result_fp[512] __attribute__((aligned(16)));
static void check(int value){if(!value)ExitProcess(210);}
static void check_fp(const u8* expected){u8 actual[512] __attribute__((aligned(16)))={0};snapshot_fp(actual);check(equal(actual,expected,512));}
static void* __attribute__((cdecl)) get_resource(void* texture){check(texture==(void*)2||texture==(void*)4);++getter_calls;SetLastError(999);__asm__ volatile("fninit; fldpi":::"memory");return (void*)3;}
static u32 __attribute__((cdecl)) get_desc(void* resource,u32 sub,u32* out){check(resource==(void*)3&&!sub);memset(out,0,40);out[0]=116;out[5]=13;out[6]=out[7]=40;out[8]=1;out[9]=3200;return 0;}
static u32 __attribute__((cdecl)) map_resource(void* resource,u32 sub,u32* out,const u32* box,u32 flags){check(resource==(void*)3&&!sub&&!box);out[0]=80;out[1]=3200;out[2]=(u32)pixels;(void)flags;return 0;}
static u32 __attribute__((cdecl)) unmap_resource(void* resource,u32 sub){check(resource==(void*)3&&!sub);if(mode==9){EnterCriticalSection(journal_lock);wine_source_version_write(resource,sub,4,1);wine_source_version_write(resource,sub,4,2);LeaveCriticalSection(journal_lock);}return 0;}
static u32 __attribute__((cdecl)) rebind_original(void* texture,u32 sub,void* memory,u32 pitch){check(texture==(void*)4&&!sub&&memory==pixels&&pitch==80);check_fp(expected_fp);check(GetLastError()==0x1234);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);return 0x8876086cu;}
static u32 WIN alias_write(void* unused){
 (void)unused;void* resource=wine_resource_get((void*)4);u32 mapped[3]={0};
 check(wine_resource_map(resource,0,mapped,0,0x40000000u)==0);pixels[0]^=1;pixels[0]^=1;
 if(mode!=2)check(wine_resource_unmap(resource,0)==0);return 0;
}
static void __attribute__((cdecl)) worker_original(void* context,const void* data){
 check(context==(void*)9&&data==packet);check_fp(expected_fp);check(GetLastError()==0x1234);++original_workers;check(write(forwards,&original_workers,4));
 char name[600];path(name,"fixture-consumed-byte.bin");HANDLE byte_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(byte_file!=(HANDLE)-1);check(write(byte_file,pixels,1));check(CloseHandle(byte_file));
 if(mode==4){HANDLE thread=CreateThread(0,0,&alias_write,0,0,0);check(thread!=0);check(WaitForSingleObject(thread,5000)==0);CloseHandle(thread);}
 __asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);
}
static u32 WIN execute_worker(void* unused){
 (void)unused;Sleep(10);SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
 wine_blit_worker((void*)9,packet);check(GetLastError()==0x4321);check_fp(result_fp);return 0;
}
static u32 __attribute__((cdecl)) blit_original(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 (void)context;(void)dst;(void)dstsub;(void)dstrect;(void)src;(void)srcsub;(void)srcrect;(void)flags;(void)fx;(void)filter;++original_blits;
 memset(packet,0,100);packet[0]=42;packet[1]=mode==7?3:1;packet[9]=3;
 packet[5]=packet[6]=packet[13]=packet[14]=40;packet[8]=packet[16]=1;packet[24]=1;packet[10]=mode==6?1:0;
 u32 registers[9]={0};registers[7]=(u32)packet;wine_blit_packet_observe(registers);
 if(mode==1||mode==2){HANDLE writer=CreateThread(0,0,&alias_write,0,0,0);check(writer!=0);check(WaitForSingleObject(writer,5000)==0);CloseHandle(writer);}
 if(mode==3){for(u32 i=0;i<2048;++i)if(wine_lifetime_objects[i].pointer==(void*)2)wine_lifetime_objects[i].pointer=0;wine_lifetime_create((void*)2,1);}
 if(mode==5){SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);check(wine_source_version_rebind((void*)4,0,pixels,80)==0x8876086cu);check(GetLastError()==0x4321);check_fp(result_fp);}
 if(mode==8){EnterCriticalSection(journal_lock);wine_source_version_write((void*)1,0,2,1);wine_source_version_write((void*)1,0,2,2);LeaveCriticalSection(journal_lock);}
 if(mode==10)pixels[0]^=1; /* Deliberately bypass every observed Wine write path. */
 HANDLE worker=CreateThread(0,0,&execute_worker,0,0,0);check(worker!=0);check(WaitForSingleObject(worker,5000)==0);CloseHandle(worker);return 0;
}
void start(void){
 check(initialize());char value[2];if(GetEnvironmentVariableA("MNM_TEST_VERSION",value,2)==1)mode=value[0]=='a'?10:value[0]=='b'?11:value[0]-'0';
 pixels=VirtualAlloc(0,4096,0x3000,4);check(pixels!=0);for(u32 i=0;i<3200;++i)pixels[i]=(u8)(i*7+3);
 char name[600];path(name,"fixture-worker-forwards.bin");forwards=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(forwards!=(HANDLE)-1);
 wine_resource_original[0]=(void*)&get_resource;wine_resource_original[1]=(void*)&map_resource;wine_resource_original[2]=(void*)&unmap_resource;
 wine_source_descriptor_get=&get_desc;wine_submission_original[0]=(void*)&blit_original;wine_blit_worker_original=(void*)&worker_original;wine_source_version_rebind_original=(void*)&rebind_original;
 wine_lifetime_create((void*)2,1);wine_lifetime_create((void*)4,1);
 check(wine_submission_blt((void*)9,(void*)1,0,0,(void*)2,0,0,0,0,1)==0);check(original_blits==1&&original_workers==1);
 if(mode==11){u32 out[3]={0};check(wine_resource_map((void*)3,0,out,0,0x40000000u)==0);}
 wine_submission_close(1);u32 calls=getter_calls;SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
 check(wine_source_version_rebind((void*)4,0,pixels,80)==0x8876086cu);check(GetLastError()==0x4321);check_fp(result_fp);check(getter_calls==calls);
 check(CloseHandle(forwards));ExitProcess(0);
}
