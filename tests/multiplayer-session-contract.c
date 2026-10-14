#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "multiplayer_fixture_bytes.h"
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
u32 saved_esp,callback_result,abi_failure;
u32 invoke_action(u32,void*,u32);
static u32 fades,selections,cases,rejections;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
void* memset(void* p,int v,unsigned n){u8* b=p;while(n--)*b++=(u8)v;return p;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static void THIS fade(void* p){require(p==(void*)0x6de838,10);++fades;}
static u32 THIS selection(void* p,u32 row){require(p==(void*)0x680400,11);++selections;put((u8*)p+0x5b,row);return 0;}
static void stub(u32 at,void* target){((u8*)at)[0]=0xe9;put((void*)(at+1),(u32)target-at-5);}
static void setup(void){
 u8* p=(u8*)0x6de838;zero(p,0x70);zero((void*)0x680000,0x3000);put(p,0x5c65d4);put(p+4,21);put(p+8,1);p[0x5f]=1;
 put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34b4,(u32)p);put((void*)0x6f34b0,0x6a5018);put((void*)0x6f34ac,0x6e0020);put((void*)0x6f34a8,0x657ce0);
 put((void*)0x6a5018,0x5c659c);put((void*)0x6a501c,16);put((void*)0x6a5020,0);put((void*)0x6e0020,0x5c641c);put((void*)0x6e0024,22);put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x68991c,1);put((void*)0x689920,2);put((void*)0x6de6b0,2);put((void*)0x6e1f0c,0);
 put(p+0x4b,0x680000);put(p+0x4f,0x680020);put(p+0x53,0x680400);
 for(u32 i=0;i<2;++i){u8* cb=(u8*)(0x680100+16*i),*b=(u8*)(0x680200+0x60*i);put((void*)(0x680020+4*i),(u32)cb);put(cb,0x5c6604);put(cb+4,i?0x4b0a30:0x4b0a00);put(cb+8,(u32)p);put((void*)(0x680000+4*i),(u32)b);put(b,0x5c6c6c);put(b+8,1);put(b+0x2d,i);put(b+0x25,0x680100);}
 put((void*)0x680400,0x5c6db8);put((void*)0x680408,1);put((void*)0x680425,0x680110);put((void*)0x680445,0x681000);put((void*)0x680449,2);put((void*)0x68045b,0xffffffffu);copy((void*)0x681000,"first",6);copy((void*)0x681101,"second",7);fades=selections=0;
}
void start(void){fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);copy((void*)0x4b0a00,session_bytes,sizeof(session_bytes));stub(0x557510,(void*)&fade);stub(0x4d1060,(void*)&selection);u8* p=(u8*)0x6de838;
 const u32 args[]={0,1,2,0xffffffffu};for(u32 mode=0;mode<3;++mode)for(u32 i=0;i<4;++i){setup();p[0x5f]=(u8)mode;require(invoke_action(0x4b0a00,p,args[i])==0&&!abi_failure,20);require(fades==(i==1)&&get(p+0x37)==(i==1?0x6e0020u:0)&&p[0x64]==(i==0)&&!get(p+0x43),21);++cases;}
 for(u32 row=1;row<=2;++row){setup();require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,row)==MNM_MENU_OK&&selections==2&&get((void*)0x68045b)==row-1&&p[0x64]==1&&!fades,22);++cases;}
 setup();static u8 out[MNM_MENU_V16_SESSIONS_SIZE];require(sessions_snapshot(p,out)&&get(out+32)==2&&equal(out+40,"first",6)&&equal(out+168,"second",7),23);
 u32* guards[]={(u32*)p,(u32*)(p+4),(u32*)(p+8),(u32*)0x6f349c,(u32*)0x6f34e0,(u32*)0x6f34b4,(u32*)0x6f34b0,(u32*)0x6f34ac,(u32*)0x6f34a8,(u32*)0x6a5018,(u32*)0x6a501c,(u32*)0x6a5020,(u32*)0x68991c,(u32*)0x689920,(u32*)0x6de6b0,(u32*)0x6e1f0c,(u32*)0x680104,(u32*)0x680108,(u32*)0x680400,(u32*)0x680425};
 for(u32 i=0;i<20;++i){u32 saved=get(guards[i]);put(guards[i],saved?0:1);require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,1)==MNM_MENU_UNAVAILABLE&&!selections&&!fades&&!p[0x64],30+i);put(guards[i],saved);++rejections;}
 const u32 words[]={0x33,0x37,0x3b,0x43};for(u32 i=0;i<4;++i){put(p+words[i],1);require(sessions_dispatch(p,MNM_MENU_SESSION_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,50);put(p+words[i],0);++rejections;}
 const u32 bytes[]={0xc,0x12,0x64,0x65};for(u32 i=0;i<4;++i){p[bytes[i]]=1;require(sessions_dispatch(p,MNM_MENU_SESSION_CANCEL,0)==MNM_MENU_UNAVAILABLE&&!fades,51);p[bytes[i]]=0;++rejections;}
 const u32 invalid[]={0,3,0xffffffffu};for(u32 i=0;i<3;++i){require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,invalid[i])==MNM_MENU_INVALID&&!selections,52);++rejections;}
 put((void*)0x680449,101);require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,1)==MNM_MENU_UNAVAILABLE&&!selections,53);put((void*)0x680449,2);++rejections;
 put((void*)0x68045b,2);require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,1)==MNM_MENU_UNAVAILABLE&&!selections,54);put((void*)0x68045b,0xffffffffu);++rejections;
 p[0x5f]=0;require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,1)==MNM_MENU_UNAVAILABLE&&!selections,55);p[0x5f]=1;++rejections;
 put((void*)0x680208,0);require(sessions_dispatch(p,MNM_MENU_SESSION_JOIN,1)==MNM_MENU_UNAVAILABLE&&!selections,56);put((void*)0x680208,1);++rejections;
 require(sessions_dispatch(p,MNM_MENU_SESSION_CANCEL,0)==MNM_MENU_OK&&fades==1&&get(p+0x37)==0x6e0020&&!p[0x64],57);++cases;
 require(cases==15&&rejections==35,58);
 const char report[]="{\"success\":true,\"cases\":15,\"guard_rejections\":35,\"original_callback_executed\":true,\"callback_abi_verified\":true,\"selection_setter_stubbed\":true,\"fade_stubbed\":true,\"networking_executed\":false}\n";u32 wrote;WriteFile(GetStdHandle(0xfffffff5),report,sizeof(report)-1,&wrote,0);ExitProcess(0);}
