#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static u32 calls,last;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static u32 THIS fake_action(void* p,u32 action){require(p==(void*)0x6e0088,10);++calls;last=action;return action;}
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
 u8* p=(u8*)0x6e0088;zero(p,0x1a40);put(p,0x5c6b70);put(p+0x4a,1);
 put(p+0x16,0x680100);put(p+0x1a,0x682000);put(p+0xa00,0x683000);put(p+0x1a3a,0x683020);
 put((void*)0x683000,0x5c6b8c);put((void*)0x683004,0x4c77c0);put((void*)0x683008,(u32)p);
 for(u32 i=0;i<5;++i){u8* e=(u8*)(0x683100+i*32);put((void*)(0x68010c+i*4),(u32)e);put(e,i?0x683040:0);put(e+4,i);put(e+12,i?0x683040:0);}
 put((void*)0x680308,0x683500);put((void*)0x683500,0x683040);put((void*)0x683504,1);put((void*)0x68350c,0x683040);
 put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34b4,(u32)p);put((void*)0x6f34b0,0x6578c0);put((void*)0x6f34ac,0x659408);put((void*)0x6f34a8,0x657ce0);
 put((void*)0x6578c0,0x5c667c);put((void*)0x6578c4,18);put((void*)0x6578c8,0);put((void*)0x65793c,4);put((void*)0x65792f,1);
 put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);
 put((void*)0x689920,5);put((void*)0x6e1aba,1);put((void*)0x6e1abe,1);
 *(u8*)0x4c77c0=0xe9;put((void*)0x4c77c1,(u32)&fake_action-0x4c77c0-5);
 u8 out[32];require(grimoire_snapshot(p,out)&&get(out+12)==63&&get(out+16)==18,20);
 for(u32 i=0;i<5;++i){require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_SELECT,i)==MNM_MENU_OK&&calls==i+1&&last==i,21);}
 require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_CLOSE,0)==MNM_MENU_OK&&calls==6&&last==3999,22);
 require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_SELECT,5)==MNM_MENU_OK&&calls==7&&last==125,61);
 const u32 bad[]={6,124,991,1999,2999,3999,0xffffffffu};
 for(u32 i=0;i<7;++i)require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_SELECT,bad[i])==MNM_MENU_INVALID&&calls==7,23);
 require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_CLOSE,1)==MNM_MENU_INVALID&&calls==7,24);
 u32* words[]={ (u32*)(p+0x1e),(u32*)(p+0x3a),(u32*)(p+0x3e),(u32*)(p+0x1a2e)};
 for(u32 i=0;i<4;++i){put(words[i],1);require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_CLOSE,0)==MNM_MENU_UNAVAILABLE&&calls==7,25);put(words[i],0);}
 p[0xc]=1;require(grimoire_dispatch(p,MNM_MENU_GRIMOIRE_CLOSE,0)==MNM_MENU_UNAVAILABLE&&calls==7,26);p[0xc]=0;
 u32* guards[]={(u32*)p,(u32*)(p+4),(u32*)(p+8),(u32*)(p+0x42),(u32*)(p+0x4a),(u32*)(p+0xa00),(u32*)(p+0x16),(u32*)(p+0x1a),(u32*)(p+0x1a3a),(u32*)0x6f349c,(u32*)0x6f34e0,(u32*)0x6f34b0,(u32*)0x6f34ac,(u32*)0x6f34a8,(u32*)0x689920,(u32*)0x6e1aba,(u32*)0x6e1abe,(u32*)0x683004,(u32*)0x683008};
 for(u32 i=0;i<19;++i){u32 saved=get(guards[i]);put(guards[i],saved?0:1);require(!grimoire_snapshot(p,out)&&grimoire_dispatch(p,MNM_MENU_GRIMOIRE_CLOSE,0)==MNM_MENU_UNAVAILABLE&&calls==7,30+i);put(guards[i],saved);}
 put((void*)0x683124,0);require(!grimoire_snapshot(p,out),50);put((void*)0x683124,1);
 put((void*)0x68318c,0);require(!grimoire_snapshot(p,out),51);put((void*)0x68318c,0x683040);
 put((void*)0x68010c,0);require(!grimoire_snapshot(p,out),52);
 require(calls==7,53);
 u8* region=(u8*)0x6578c0,*button=(u8*)0x684000;put(region+0x4f,(u32)button);put(region+0x5b,0x684100);put(button,0x5c6bd0);put(button+8,1);put(button+0x2d,2);put(button+0x25,0x684100);
 require(region_grimoire_available(region),54);
 u32* auxguards[]={(u32*)button,(u32*)(button+8),(u32*)(button+0x2d),(u32*)(button+0x25)};
 for(u32 i=0;i<4;++i){u32 saved=get(auxguards[i]);put(auxguards[i],0);require(!region_grimoire_available(region),55+i);put(auxguards[i],saved);}
 put(button+0x3d,2);require(!region_grimoire_available(region),59);put(button+0x3d,3);require(!region_grimoire_available(region),60);

 const char result[]="{\"success\":true,\"original_action_stubbed\":true,\"accepted_actions\":7,\"invalid_arguments\":8,\"identity_guards\":19,\"transition_guards\":5,\"catalog_guards\":3}\n";
 u32 wrote;WriteFile(GetStdHandle(0xfffffff5),result,sizeof(result)-1,&wrote,0);ExitProcess(0);
}
