#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "multiplayer_fixture_bytes.h"
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
u32 saved_esp,callback_result,abi_failure;
u32 invoke_action(u32,void*,u32);
static u32 fades,disconnects,cases,rejections;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
void* memset(void* p,int v,unsigned n){u8* b=p;while(n--)*b++=(u8)v;return p;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static void THIS fade(void* p){require(p==(void*)0x6a4b1c,10);++fades;}
static u32 THIS disconnect(void* p){require(p==(void*)0x6903e0,11);++disconnects;return 0;}
static void stub(u32 at,void* target){((u8*)at)[0]=0xe9;put((void*)(at+1),(u32)target-at-5);}
static void setup(void){
 u8* p=(u8*)0x6a4b1c;zero(p-4,0x4c4);zero((void*)0x680000,0x4000);put(p-4,0x5c68cc);put(p,0x5c689c);put(p+4,24);put(p+8,1);put(p+0x8b,0x6903e0);put(p+0x9b,15);put((void*)0x6903e0,0x5c6000);put((void*)0x6903e4,(u32)(p-4));put((void*)0x5c6044,0x4b5a00);
 put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34b4,(u32)p);put((void*)0x6f34b0,0x6de750);put((void*)0x6f34ac,0x6e0020);put((void*)0x6f34a8,0x657ce0);put((void*)0x6de750,0x5c660c);put((void*)0x6de754,15);put((void*)0x6de758,0);put((void*)0x6e0020,0x5c641c);put((void*)0x6e0024,22);put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x68991c,1);put((void*)0x689920,2);put((void*)0x6de6ac,1);put((void*)0x6de6b0,2);put((void*)0x6de6b4,1);put((void*)0x6e1f0c,0);
 put(p+0x4b,0x680000);put((void*)0x680000,0x680100);put((void*)0x680100,0x5c67f0);put((void*)0x680104,0x4b7bf0);put((void*)0x680108,0);put((void*)0x68010c,(u32)(p-4));put(p+0x57,0x680020);put((void*)0x680020,0x680200);put((void*)0x680200,0x5c6c6c);put((void*)0x680208,1);put((void*)0x680225,0x680100);put(p+0x5b,0x680400);put((void*)(0x680400+13*4),0x680800);put((void*)0x680808,0x682100);copy((void*)0x682100,"No Map",7);
 copy((void*)0x6ead98,"No Player",10);zero((void*)0x658f10,4*0x133);copy((void*)0x658f10,"NativeHost",11);for(u32 i=1;i<4;++i)copy((void*)(0x658f10+i*0x133),"No Player",10);copy((void*)0x6de59c,"NativeRoom",11);zero((void*)0x6c3758,56);put((void*)0x6c375c,200);put((void*)0x6c3760,400);fades=disconnects=0;
}
void start(void){fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);copy((void*)0x4b7bf0,lobby_bytes,sizeof(lobby_bytes));stub(0x557510,(void*)&fade);stub(0x4b5a00,(void*)&disconnect);u8* p=(u8*)0x6a4b1c;const u32 args[]={0,1,0xffffffffu};
 for(u32 service=0;service<2;++service)for(u32 i=0;i<3;++i){setup();if(!service)put(p+0x8b,0);require(invoke_action(0x4b7bf0,p-4,args[i])==0&&!abi_failure,20);require(fades==(i==0)&&disconnects==(service&&i==0)&&get(p+0x37)==(i==0?0x6e0020u:0)&&!get(p+0x43),21);++cases;}
 setup();u8 out[544];require(lobby_snapshot(p,out)&&get(out)==15&&get(out+84)==1&&equal(out+100,"NativeHost",11)&&equal(out+276,"NativeRoom",11),22);
 u32* guards[]={(u32*)p,(u32*)(p-4),(u32*)(p+4),(u32*)(p+8),(u32*)0x6f349c,(u32*)0x6f34e0,(u32*)0x6f34b4,(u32*)0x6f34b0,(u32*)0x6de750,(u32*)0x6de758,(u32*)0x68991c,(u32*)0x689920,(u32*)0x6de6ac,(u32*)0x6de6b4,(u32*)0x6de6b0,(u32*)0x6e1f0c,(u32*)(p+0x8b),(u32*)(p+0x9b),(u32*)(p+0x8f),(u32*)0x6903e4,(u32*)0x680100,(u32*)0x680104,(u32*)0x680108,(u32*)0x68010c,(u32*)0x680200,(u32*)0x680225,(u32*)0x5c6044};
 for(u32 i=0;i<27;++i){u32 saved=get(guards[i]);put(guards[i],saved?0:1);require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades&&!disconnects&&!get(p+0x37),30+i);put(guards[i],saved);++rejections;}
 const u32 words[]={0x33,0x37,0x3b,0x43};for(u32 i=0;i<4;++i){put(p+words[i],1);require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,60);put(p+words[i],0);++rejections;}
 p[0xc]=1;require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,61);p[0xc]=0;++rejections;
 copy((void*)(0x658f10+0x133),"Peer",5);require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,62);copy((void*)(0x658f10+0x133),"No Player",10);++rejections;
 require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,1)==MNM_MENU_INVALID&&!fades,63);++rejections;put((void*)0x680208,0);require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,64);put((void*)0x680208,1);++rejections;
 require(lobby_dispatch(p,MNM_MENU_LOBBY_CANCEL,0)==MNM_MENU_OK&&fades==1&&disconnects==1&&get(p+0x37)==0x6e0020,65);++cases;require(cases==7&&rejections==35,66);
 const char report[]="{\"success\":true,\"cases\":7,\"guard_rejections\":35,\"original_callback_executed\":true,\"callback_abi_verified\":true,\"network_disconnect_stubbed\":true,\"fade_stubbed\":true,\"networking_executed\":false}\n";u32 wrote;WriteFile(GetStdHandle(0xfffffff5),report,sizeof(report)-1,&wrote,0);ExitProcess(0);}
