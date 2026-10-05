#include "../runtime/shadow/win32_min.h"
API int WIN MenuCampaignInstallForTest(void);
API int WIN MenuCampaignNavigationForTest(void);
#define THIS __attribute__((thiscall))
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static void put(void* p,u32 v){u8* b=p;for(u32 i=0;i<4;++i)b[i]=v>>(i*8);}
static u32 get(void* p){u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
void start(void){
    fixture_space[0]=1;u32 old;
    require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
    u8* p=(u8*)0x659408;for(u32 i=0;i<0x1e25;++i)p[i]=0;
    put(p,0x5c6a60);put(p+4,4);put(p+0xb4d,9);
    put((void*)0x6f34e0,(u32)p);put((void*)0x6f349c,1);put((void*)0x6f34a0,0x6ddd58);put((void*)0x6f34a4,(u32)p);put((void*)0x689920,5);
    // Synthetic original tick: expected entry, stack restoration, return zero.
    const u8 tick[]={0x81,0xec,0x08,0x01,0,0,0x33,0xc0,0x81,0xc4,0x08,0x01,0,0,0xc3};
    for(u32 i=0;i<sizeof(tick);++i)((u8*)0x5517a0)[i]=tick[i];
    put((void*)0x5c6a70,0x5517a1);SetLastError(0xabc123);
    require(!MenuCampaignInstallForTest()&&get((void*)0x5c6a70)==0x5517a1&&GetLastError()==0xabc123,2);
    put((void*)0x5c6a70,0x5517a0);*(u8*)0x5517a0=0x90;
    require(!MenuCampaignInstallForTest()&&get((void*)0x5c6a70)==0x5517a0&&GetLastError()==0xabc123,3);
    *(u8*)0x5517a0=tick[0];require(MenuCampaignInstallForTest()&&GetLastError()==0xabc123,4);
    require(get((void*)0x5c6a70)!=0x5517a0,5);
    u32 (THIS *forward)(void*)=(void*)get((void*)0x5c6a70);
    for(u32 i=0;i<300;++i)require(forward(p)==0&&GetLastError()==0xabc123,6);
    put(p+0x823,8);require(forward(p)==0&&GetLastError()==0xabc123,7);
    require(forward(p+4)==0&&GetLastError()==0xabc123,8);
    require(!MenuCampaignInstallForTest()&&GetLastError()==0xabc123,9);
    const u8 common[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57,0x5f,0x5e,0x83,0xc4,0x1c,0xb8,0x00,0x0f,0,0,0xc3};
    const u8 resume[]={0x8b,0x81,0x37,0x0a,0,0,0x85,0xc0,0xc7,0x81,0x37,0x0a,0,0,0,0,0,0,0xc3};
    for(u32 i=0;i<sizeof(common);++i)((u8*)0x5595d0)[i]=common[i];
    for(u32 i=0;i<sizeof(resume);++i)((u8*)0x552210)[i]=resume[i];
    put((void*)0x5c668c,0x5595d1);put((void*)0x5c6a60,0x552210);
    require(!MenuCampaignNavigationForTest()&&get((void*)0x5c6a60)==0x552210&&GetLastError()==0xabc123,10);
    put((void*)0x5c668c,0x5595d0);*(u8*)0x552210=0x90;
    require(!MenuCampaignNavigationForTest()&&get((void*)0x5c668c)==0x5595d0&&GetLastError()==0xabc123,11);
    *(u8*)0x552210=resume[0];require(MenuCampaignNavigationForTest()&&GetLastError()==0xabc123,12);
    u8* entry=(u8*)0x6578c0;for(u32 i=0;i<0x81;++i)entry[i]=0;
    put(entry,0x5c667c);put(entry+4,18);
    forward=(void*)get((void*)0x5c668c);require(forward(entry)==0xf00&&GetLastError()==0xabc123,13);
    put(p+0xa37,1);((void (THIS *)(void*))get((void*)0x5c6a60))(p);
    require(!get(p+0xa37)&&GetLastError()==0xabc123,14);
    require(!MenuCampaignNavigationForTest()&&GetLastError()==0xabc123,15);
    ExitProcess(0);
}
