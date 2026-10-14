/* Original callbacks/setters in a private PE32 fixture; networking is not run. */
#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "multiplayer_fixture_bytes.h"
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
u32 saved_esp,callback_result,abi_failure;
u32 invoke_action(u32,void*,u32);
static u32 fades,focus,focused,cases,rejections;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
void* memset(void* d,int v,unsigned n){u8* b=d;while(n--)*b++=(u8)v;return d;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static void THIS fade(void* p){require(p==(void*)0x6de750||p==(void*)0x6a5018,10);++fades;}
static u32 THIS set_focus(void* p,u32 edit){require(p==(void*)(0x6de750+0x1f)||p==(void*)(0x6a5018+0x1f),11);++focus;focused=edit;return 0;}
static void stub(u32 at,void* target){((u8*)at)[0]=0xe9;put((void*)(at+1),(u32)target-at-5);}
static void setup(int create){
 u8* p=(u8*)(create?0x6de750:0x6a5018);zero(p,0x70);zero((void*)0x680000,0x3000);
 put(p,create?0x5c660c:0x5c659c);put(p+4,create?15:16);put(p+8,1);
 put((void*)0x6f349c,4);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34b0,(u32)p);put((void*)0x6f34ac,0x6e0020);put((void*)0x6f34a8,0x657ce0);
 put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x6e0020,0x5c641c);put((void*)0x6e0024,22);
 put((void*)0x68991c,1);put((void*)0x689920,2);put((void*)0x6e1f0c,0);
 put(p+0x4b,0x680000);put(p+(create?0x5b:0x57),0x680020);put(p+(create?0x57:0x53),0x680040);put(p+0x5f,0x680060);put(p+0x67,3);put(p+0x6b,3);
 for(u32 i=0;i<2;++i){u8* cb=(u8*)(0x680100+16*i),*b=(u8*)(0x680200+0x60*i);put((void*)(0x680020+4*i),(u32)cb);put(cb,create?0x5c663c:0x5c65cc);put(cb+4,create?(i?0x4b1920:0x4b1840):(i?0x4af9a0:0x4af910));put(cb+8,(u32)p);put((void*)(0x680000+4*i),(u32)b);put(b,0x5c6c6c);put(b+8,1);put(b+0x2d,i);put(b+0x25,0x680100);}
 for(u32 i=0;i<3;++i){u8* r=(u8*)(0x680400+0x80*i);put((void*)(0x680040+4*i),(u32)r);put((void*)(0x680060+4*i),(u32)r);put(r,0x5c6d40);put(r+8,1);put(r+0x2d,i);put(r+0x39,i==0);}
 u8* game=(u8*)0x681000,*user=(u8*)0x681100;put(game,0x5c6ca0);put(game+8,1);put(game+0x60,256);put(game+0x25,0x680110);put(game+0x45,0x682000);put(user,0x5c6ca0);put(user+8,1);put(user+0x60,15);put(user+0x25,0x680110);put(user+0x45,0x682100);
 if(create){put(p+0x53,0x680080);put((void*)0x680080,(u32)game);put((void*)0x680084,(u32)user);}else put(p+0x5b,(u32)user);
 fades=focus=focused=0;
}
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
 copy((void*)0x4af910,join_bytes,sizeof(join_bytes));copy((void*)0x4b1840,create_bytes,sizeof(create_bytes));copy((void*)0x4cd0e0,edit_bytes,sizeof(edit_bytes));copy((void*)0x4ce730,radio_bytes,sizeof(radio_bytes));
 *(u8*)0x5faf28=0;stub(0x557510,(void*)&fade);stub(0x4caf00,(void*)&set_focus);
 for(u32 create=0;create<2;++create){u8* p=(u8*)(create?0x6de750:0x6a5018);u32 callback=create?0x4b1840:0x4af910;
  for(u32 mode=0;mode<6;++mode){setup(create);if(mode>=1)*(u8*)0x682000='g';if(mode>=2)*(u8*)0x682100='u';u32 action=mode==4?1:mode==5?99:0;
   require(invoke_action(callback,p,action)==0&&!abi_failure,20);
   u32 valid=create?mode>=2:mode>=2;
   require(fades==(mode==4||(mode<4&&valid)),21);require(get(p+0x43)==(mode==4||(create&&mode<4&&valid)),22);require(p[0x6f]==(mode<4&&valid),23);
   require(focus==(mode<2)&&(!focus||focused==(create&&mode==0?0x681000:0x681100)),24);++cases;
  }
  for(u32 transport=0;transport<3;++transport){setup(create);u8 names[80]={0};if(create)copy(names," game ",7);copy(names+64," user ",7);require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,transport,names)==MNM_MENU_OK,30);require(fades==1&&p[0x6f]==1&&get(p+0x43)==create&&get(p+0x63)==transport,31);require(equal((void*)0x682100,names+64,7)&&(!create||equal((void*)0x682000,names,7)),32);++cases;}
  setup(create);u8 out[128],names[80]={0};if(create)copy(names,"game",5);copy(names+64,"user",5);require(multiplayer_snapshot(p,out),40);
  u32* guards[]={(u32*)p,(u32*)(p+4),(u32*)(p+8),(u32*)0x6f349c,(u32*)0x6f34e0,(u32*)0x6f34b0,(u32*)0x6f34ac,(u32*)0x6f34a8,(u32*)0x68991c,(u32*)0x689920,(u32*)0x6e1f0c,(u32*)0x680104,(u32*)0x680108,(u32*)0x681160,(u32*)0x681125,(u32*)(p+0x67),(u32*)(p+0x6b),(u32*)0x68042d,(u32*)0x680068};
  for(u32 i=0;i<19;++i){u32 saved=get(guards[i]);put(guards[i],saved?0:1);require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,0,names)==MNM_MENU_UNAVAILABLE&&!fades&&*(u8*)0x682100==0,50+i);put(guards[i],saved);++rejections;}
  require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,3,names)==MNM_MENU_INVALID&&!fades,70);++rejections;
  names[64]=0;require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,0,names)==MNM_MENU_INVALID&&!fades,71);++rejections;names[64]='u';
  for(u32 i=0;i<16;++i)names[64+i]='u';require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,0,names)==MNM_MENU_INVALID&&!fades,72);++rejections;zero(names+64,16);copy(names+64,"user",5);
  put((void*)0x6804bd,2);require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_SUBMIT,1,names)==MNM_MENU_UNAVAILABLE&&!fades,73);++rejections;put((void*)0x6804bd,0);
  p[0xc]=1;require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_CANCEL,0,names)==MNM_MENU_UNAVAILABLE&&!fades,74);++rejections;p[0xc]=0;
  require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_CANCEL,1,names)==MNM_MENU_INVALID&&!fades,75);++rejections;
  require(multiplayer_dispatch(p,MNM_MENU_MULTIPLAYER_CANCEL,0,names)==MNM_MENU_OK&&fades==1&&get(p+0x43)==1&&!p[0x6f],76);++cases;
 }
 require(cases==20&&rejections==50,90);
 const char report[]="{\"success\":true,\"cases\":20,\"guard_rejections\":50,\"original_callbacks_setters_executed\":true,\"callback_abi_verified\":true,\"fade_focus_stubbed\":true,\"networking_executed\":false}\n";u32 wrote;WriteFile(GetStdHandle(0xfffffff5),report,sizeof(report)-1,&wrote,0);ExitProcess(0);
}
