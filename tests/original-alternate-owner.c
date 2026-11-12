/* Authored lifecycle/providers around unchanged original bind/copy bodies.
 * This fixture is not an independently acquired original game session. */
#include "../runtime/shadow/win32_min.h"
#include "alternate_owner_fixture.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API char* WIN GetCommandLineA(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
void* memset(void* p,int v,unsigned n){u8* d=p;while(n--)*d++=(u8)v;return p;}
#define THIS __attribute__((thiscall))
#define FAST __attribute__((fastcall))
static void copy(void* p,const void* q,u32 n){u8* d=p;const u8* s=q;while(n--)*d++=*s++;}
static int has(const char* s,const char* word){for(;*s;++s){const char* a=s;const char* b=word;while(*b&&*a==*b){++a;++b;}if(!*b)return 1;}return 0;}
static u32 checkpoint=40;
static void require(int v){if(!v)ExitProcess(checkpoint);}
static void put(u32 p,u32 v){*(u32*)p=v;}
static u32 get(u32 p){return *(u32*)p;}
static void jump(u32 p,const void* to){*(u8*)p=0xe9;put(p+1,(u32)to-p-5);}
static u32 THIS create(void* self,u32* d,u32 file,u32 line){
 (void)file;(void)line;require((u32)self==0x6a49e0);put(0x6a49f0,d[3]);put(0x6a49f4,d[2]);put(0x6a49f8,d[3]*2);return 1;
}
static void initialize_provider(void){
 u32 w=get(0x6de6d5)?800:640,h=get(0x6de6d5)?600:480;
 if(get(0x6e1f78)){put(0x6e1f70,96);put(0x6e1f74,0);}
 *(u16*)0x6e1f7c=0xf800;*(u16*)0x6e1f7e=0x7e0;*(u16*)0x6e1f80=0x1f;
 u32 d[27]={108,7};d[2]=h+get(0x6e1f74);d[3]=w+get(0x6e1f70);d[26]=0x840;
 ((u32 (THIS*)(void*,u32*,u32,u32))0x58ad90)((void*)0x6a49e0,d,0,0);
}
static void init(void){require(((u32 (FAST*)(void*,void*))0x4e40d0)(0,0)==1);}
static void exercise(u32 salt){
 u32 w=get(0x6de6d5)?800:640,h=get(0x6de6d5)?600:480,base=get(0x6e1f68),pitch=w+192;
 require(base&&get(0x6e1f6c)==base+192&&get(0x6a49f0)==w&&get(0x6a49f4)==h);
 u16* from=(u16*)base;for(u32 y=0;y<h;++y)for(u32 x=0;x<pitch;++x)from[y*pitch+x]=(u16)(x*13+y*31+salt);
 u16* to=VirtualAlloc(0,w*h*2,0x3000,4);require(to!=0);
 ((void (FAST*)(void*,u32,u32))0x57dc20)(to,w,h);
 ((void(*)(void))0x594508)();
 for(u32 y=0;y<h;++y)for(u32 x=0;x<w;++x)require(to[y*w+x]==from[y*pitch+x+96]);
 API int WIN VirtualFree(void*,u32,u32);
 require(VirtualFree(to,0,0x8000));
}
static u32 WIN foreign_world(void* unused){
 (void)unused;((void(FAST*)(void*,u32,u32))0x4fd477)((void*)get(0x6e1f68),832,480);ExitProcess(91);return 0;
}
void mainCRTStartup(void){
 u32 old;void* mapped=GetModuleHandleA(0);
 if(mapped!=(void*)0x400000||!VirtualProtect((void*)0x400000,0x310000,0x40,&old)){
  u32 data[256]={0x464f4e4d,(u32)mapped,GetLastError(),(u32)GetModuleHandleA(0)},at=0x400000,n=4;
  while(at<0x710000&&n+8<256){u32 q[7];if(VirtualQuery((void*)at,q,28)!=28)break;for(u32 i=0;i<7;++i)data[n++]=q[i];u32 next=q[0]+q[3];if(next<=at)break;at=next;}
  HANDLE f=CreateFileA("fixture-address-space.bin",0x40000000,1,0,1,0x80,0);u32 done;if(f!=(HANDLE)-1){WriteFile(f,data,n*4,&done,0);CloseHandle(f);}ExitProcess(40);
 }
 for(u32 i=0;i<6;++i)copy((void*)fixture_hooks[i].address,fixture_hooks[i].bytes,fixture_hooks[i].length);
 /* Lifecycle providers are authored stubs. Bind and copy below are original. */
 const u8 init_tail[]={0xe8,0,0,0,0,0x5e,0x83,0xc4,0x6c,0xb8,1,0,0,0,0xc3};
 copy((void*)0x4e40d6,init_tail,sizeof(init_tail));put(0x4e40d7,(u32)&initialize_provider-0x4e40db);
 jump(0x58ad95,(void*)&create);
 const u8 retire_tail[]={0x5f,0x5e,0x5b,0xc3};copy((void*)0x58b3e5,retire_tail,sizeof(retire_tail));*(u8*)0x58acf5=0xc3;
 copy((void*)0x57dc20,fixture_bind,sizeof(fixture_bind));copy((void*)0x594508,fixture_copy,sizeof(fixture_copy));
 const u8 bind_site[]={0xff,0x74,0x24,4,0xe8,0,0,0,0,0xc2,4,0};copy((void*)0x4fd477,bind_site,sizeof(bind_site));put(0x4fd47c,0x57dc20-0x4fd480);
 const char* command=GetCommandLineA();int preexisting=has(command,"preexisting-owner");if(preexisting)put(0x6e1f68,0x123456);int corrupt=has(command,"bad-entry");if(corrupt)*(u8*)0x594508=0x90;
 checkpoint=41;void* dll=LoadLibraryA("MnmAlt.dll");require(dll!=0);checkpoint=42;int(WIN* install)(void)=(void*)GetProcAddress(dll,"AlternateOwnerInstallForTest");require(install!=0);
 if(corrupt||preexisting){require(!install());if(preexisting)require(get(0x6e1f68)==0x123456);ExitProcess(0);}checkpoint=43;require(install());checkpoint=44;
 if(has(command,"saturated-log")){void(WIN* saturate)(void)=(void*)GetProcAddress(dll,"AlternateOwnerSaturateLogForTest");require(saturate!=0);saturate();}
 put(0x6e1f78,0);init();require(!get(0x6e1f68)&&!get(0x6e1f6c));
 put(0x6e1f78,1);init();
 if(has(command,"guard")){((u8*)get(0x6e1f68))[-1]=0;exercise(1);ExitProcess(91);}
 if(has(command,"foreign-thread")){HANDLE t=CreateThread(0,0,foreign_world,0,0,0);require(t!=0);WaitForSingleObject(t,10000);ExitProcess(91);}
 if(has(command,"foreign")){put(0x6e1f6c,0x123456);((void(*)(void))0x594508)();ExitProcess(91);}
 for(u32 n=0;n<2;++n)exercise(n);
 init();exercise(3);exercise(4);
 put(0x6de6d5,1);init();exercise(5);exercise(6);
 put(0x6e1f78,0);init();require(!get(0x6e1f68)&&!get(0x6e1f6c)&&!get(0x6e1f70));
 put(0x6e1f78,1);init();exercise(7);exercise(8);
 /* Retirement must invalidate an active binding into the owned generation. */
 ((void (FAST*)(void*,u32,u32))0x57dc20)((void*)get(0x6e1f68),992,600);
 ((void(THIS*)(void*))0x58b3e0)((void*)0x6a49e0);require(!get(0x658174)&&!get(0x6e1f68)&&!get(0x6e1f6c));
 init();exercise(9);exercise(10);((void(*)(void))0x58acf0)();require(!get(0x6e1f68)&&!get(0x6e1f6c));
 ExitProcess(0);
}
