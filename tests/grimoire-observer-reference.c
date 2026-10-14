#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "grimoire_observer_bytes.h"
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static u32 calls;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static u32 THIS fake_tick(void* p){
    require(p==(void*)0x6e0088,10);++calls;
    if(get((u8*)p+0x1e)){put((void*)0x6f34e0,0x6578c0);put((void*)0x6f349c,4);}
    SetLastError(0x12345678);return 0xabcdef01;
}
void start(void){
    fixture_space[0]=1;u32 old;
    require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
    copy((void*)0x4c7890,book_tick_prefix,sizeof(book_tick_prefix));
    put((void*)0x5c6b70,0x4ca300);put((void*)0x5c6b74,0x4ca320);
    put((void*)0x5c6b80,0x4c7890);put((void*)0x5c6b84,0x4c7920);
    put((void*)0x5c6b70,0);require(!install_grimoire_observe()&&!grimoire_log&&get((void*)0x5c6b80)==0x4c7890,20);put((void*)0x5c6b70,0x4ca300);
    put((void*)0x5c6b80,0);require(!install_grimoire_observe()&&!grimoire_log,21);put((void*)0x5c6b80,0x4c7890);
    *(u8*)0x4c7890=0x90;require(!install_grimoire_observe()&&!grimoire_log,22);copy((void*)0x4c7890,book_tick_prefix,sizeof(book_tick_prefix));
    require(install_grimoire_observe()&&grimoire_log&&get((void*)0x5c6b80)==(u32)&grimoire_tick,23);
    // Private synthetic forwarding target replaces the verified original prefix
    // only after installation; this does not execute the actual original tick.
    require(equal((void*)0x4c7890,book_tick_prefix,sizeof(book_tick_prefix)),24);
    *(u8*)0x4c7890=0xe9;put((void*)0x4c7891,(u32)&fake_tick-0x4c7890-5);
    u8* p=(u8*)0x6e0088;zero(p,0x1a40);put(p,0x5c6b70);
    put(p+0x16,0x680100);put(p+0x1a,0x680200);put(p+0xa00,0x680300);
    put((void*)0x680300,0x5c6b8c);put((void*)0x680304,0x4c77c0);put((void*)0x680308,(u32)p);
    put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34b0,0x6578c0);
    put((void*)0x6578c4,18);put((void*)0x689920,5);put((void*)0x6e1ab6,1);put((void*)0x6e1aba,0);put((void*)0x6e1abe,1);
    TickFn tick=(TickFn)get((void*)0x5c6b80);
    SetLastError(99);require(tick(p)==0xabcdef01&&GetLastError()==0x12345678&&calls==1&&grimoire_sequence==2,25);
    SetLastError(77);require(tick(p)==0xabcdef01&&GetLastError()==0x12345678&&calls==2&&grimoire_sequence==2,26);
    put(p,0);require(tick(p)==0xabcdef01&&calls==3&&grimoire_sequence==2,27);put(p,0x5c6b70);
    put(p+0x1e,1);require(tick(p)==0xabcdef01&&calls==4&&grimoire_sequence==4&&get((void*)0x6f34e0)==0x6578c0,28);
    for(u32 i=0;i<300;++i){put(p+0x46,i);tick(p);}require(grimoire_sequence==256&&calls==304,29);
    CloseHandle(grimoire_log);grimoire_log=0;
    const char result[]="{\"success\":true,\"original_tick_stubbed\":true,\"cases\":10,\"forwarded_calls\":304,\"bounded_records\":256}\n";
    u32 wrote;WriteFile(GetStdHandle(0xfffffff5),result,sizeof(result)-1,&wrote,0);ExitProcess(0);
}
