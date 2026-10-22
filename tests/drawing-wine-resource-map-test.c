#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls,fail,close_inside,pending_stop;
static u8 expected_fp[512] __attribute__((aligned(16)));
static void original_fp(void){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);}
static void check_fp(void){u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);if(!equal(actual,expected_fp,512))ExitProcess(221);}
static void control(void){u32 args[6]={0};struct Context c=begin((void*)1,3,args,0,0x400100,5);if(!c.observed)ExitProcess(229);end(&c,(void*)1,0);}
static void* __attribute__((cdecl)) getter(void* texture){if(texture!=(void*)1)ExitProcess(211);++calls;
 if(close_inside){if(wine_resource_pending!=1)ExitProcess(227);EnterCriticalSection(journal_lock);wine_resource_close(1);LeaveCriticalSection(journal_lock);}
 if(pending_stop){if(wine_resource_pending!=1)ExitProcess(228);char name[600];path(name,"stop.flag");HANDLE stop=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(stop==(HANDLE)-1||!CloseHandle(stop))ExitProcess(230);control();if(!journal||!enabled||wine_resource_pending!=1)ExitProcess(231);pending_stop=0;}
 SetLastError(0xcafe);original_fp();return (void*)2;}
static u32 __attribute__((cdecl)) mapper(void* resource,u32 sub,u32* descriptor,const u32* box,u32 flags){if(resource!=(void*)2||sub||(box&&!fail)||flags!=7)ExitProcess(212);++calls;SetLastError(0xabcd);original_fp();if(fail)return 0x8876086cu;descriptor[0]=1600;descriptor[1]=960000;descriptor[2]=3;return 0;}
static u32 __attribute__((cdecl)) unmapper(void* resource,u32 sub){if((resource!=(void*)2&&resource)||sub)ExitProcess(213);++calls;SetLastError(0x1234);original_fp();return fail?0x8876086cu:0;}
void start(void){
 if(!initialize())ExitProcess(210);wine_resource_original[0]=(void*)&getter;wine_resource_original[1]=(void*)&mapper;wine_resource_original[2]=(void*)&unmapper;u32 descriptor[3]={0};
 char mode[16];u32 n=GetEnvironmentVariableA("MNM_RESOURCE_FIXTURE_CASE",mode,sizeof(mode));
 if(n&&mode[0]=='n'){
  if(wine_resource_get((void*)1)!=(void*)2||wine_resource_map((void*)2,0,descriptor,0,7)||wine_resource_unmap((void*)2,0))ExitProcess(239);
  if(wine_resource_unmap(0,0)||wine_resource_pending||calls!=4)ExitProcess(240);
  wine_resource_close(1);ExitProcess(0);
 }
 if(n&&mode[0]=='m'){close_inside=1;if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(232);check_fp();if(calls!=1||wine_resource_pending||wine_resource_enabled||wine_resource_file)ExitProcess(233);ExitProcess(0);}
 if(n&&mode[0]=='p'){pending_stop=1;if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(234);check_fp();if(wine_resource_pending||!journal)ExitProcess(235);control();if(journal||enabled||wine_resource_file||wine_resource_enabled)ExitProcess(236);if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(237);check_fp();if(calls!=2)ExitProcess(238);ExitProcess(0);}
 SetLastError(0xbeef);if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(214);check_fp();
 if(wine_resource_map((void*)2,0,descriptor,0,7)||GetLastError()!=0xabcd||descriptor[0]!=1600)ExitProcess(215);check_fp();
 fail=1;if(wine_resource_unmap((void*)2,0)!=0x8876086cu||GetLastError()!=0x1234)ExitProcess(216);check_fp();fail=0;if(wine_resource_unmap((void*)2,0)||GetLastError()!=0x1234)ExitProcess(217);check_fp();
 if(wine_resource_unmap((void*)2,0))ExitProcess(218);check_fp();fail=1;if(wine_resource_map((void*)2,0,0,0,7)!=0x8876086cu)ExitProcess(219);check_fp();if(wine_resource_map((void*)2,0,0,(void*)1,7)!=0x8876086cu)ExitProcess(222);check_fp();if(calls!=7)ExitProcess(220);EnterCriticalSection(journal_lock);wine_resource_close(1);LeaveCriticalSection(journal_lock);
 /* Hooks remain installed during original teardown. Closing the journal must
  * leave all three callbacks as once-only forwarding with preserved state. */
 fail=0;if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(223);check_fp();
 if(wine_resource_map((void*)2,0,descriptor,0,7)||GetLastError()!=0xabcd||descriptor[0]!=1600)ExitProcess(224);check_fp();
 if(wine_resource_unmap((void*)2,0)||GetLastError()!=0x1234)ExitProcess(225);check_fp();
 if(calls!=10||wine_resource_file||wine_resource_enabled)ExitProcess(226);ExitProcess(0);
}
