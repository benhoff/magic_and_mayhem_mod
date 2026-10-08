/* Actual PE32 hook/trampoline calls against synthetic compatible prologues.
   This tests observer forwarding, not original renderer equivalence. */
#include "../runtime/shadow/win32_min.h"
extern int canvas_startup_install(void);
extern void canvas_startup_world(u32*);
extern void canvas_test_call(u32,u32,u32,u32,void*);
static const u32 addresses[]={0x58ad90,0x58b3e0,0x58b660,0x57dc20,0x57dc60,0x58bc10,0x58bac0,0x58bf40,0x58c140,0x58d320,0x581ec0,0x57e1b0};
static const u32 pops[]={12,0,0,4,8,4,8,12,12,4,16,12};
static const u8 lengths[]={5,5,5,10,10,6,6,5,5,9,7,5};
static const u8 prefixes[][10]={
    {0xa1,0x54,0x81,0x6f,0}, {0x53,0x56,0x8b,0xf1,0x57}, {0x83,0xec,8,0x33,0xc0},
    {0x8b,0x44,0x24,4,0x89,0x0d,0x74,0x81,0x65,0}, {0x8b,0x44,0x24,4,0x89,0x0d,0x6c,0xbb,0x6c,0},
    {0x83,0xec,0x6c,0x56,0x8b,0xf1}, {0x83,0xec,0x6c,0x56,0x8b,0xf1},
    {0x83,0xec,0x30,0x53,0x55}, {0x83,0xec,0x30,0x53,0x55},
    {0x56,0x57,0x8b,0xf1,0xe8,0x37,0xe3,0xff,0xff},
    {0x83,0xec,0x28,0x8b,0x44,0x24,0x30}, {0x83,0xec,0x14,0x53,0x55},
};
static const u8 tails[][32]={
    {0xc2,12,0}, {0x5f,0x5e,0x5b,0xc3},
    {0x83,0xc4,8,0xb8,0,0,0x70,0,0xc3},
    {0x89,0x15,0xc8,0x2d,0x6a,0,0xa3,0x50,0xef,0x6d,0,0xc2,4,0},
    {0x8b,0x4c,0x24,8,0x89,0x15,8,0,0x6e,0,0xa3,0xb8,0x49,0x6a,0,0x89,0x0d,0x18,0x66,0x65,0,0xc2,8,0},
    {0x5e,0x83,0xc4,0x6c,0xc2,4,0}, {0x5e,0x83,0xc4,0x6c,0xc2,8,0},
    {0x5d,0x5b,0x83,0xc4,0x30,0xc2,12,0}, {0x5d,0x5b,0x83,0xc4,0x30,0xc2,12,0},
    {0x5f,0x5e,0xc2,4,0}, {0x83,0xc4,0x28,0xc2,16,0}, {0x5d,0x5b,0x83,0xc4,0x14,0xc2,12,0},
};
static u8 before[12][560] __attribute__((aligned(16)));
static u8 after[560] __attribute__((aligned(16)));
/* Reserve the pinned fixture addresses in this executable's own BSS. Wine
   can reserve low free ranges, so anonymous fixed allocations are unsuitable. */
static volatile u8 fixture_reserve[0x310000];
void start(void){
    fixture_reserve[sizeof(fixture_reserve)-1]=1;
    u32 old;if((u32)GetModuleHandleA(0)!=0x400000||!VirtualProtect((void*)0x570000,0x20000,0x40,&old))ExitProcess(1);
    for(u32 i=0;i<12;++i){u8* p=(u8*)addresses[i];for(u32 j=0;j<lengths[i];++j)p[j]=prefixes[i][j];for(u32 j=0;j<32;++j)p[lengths[i]+j]=tails[i][j];}
    u32* object=(u32*)0x700100;object[4]=4;object[5]=4;object[6]=8;
    ((u16*)0x700000)[0]=32;
    for(u32 i=0;i<12;++i){SetLastError(1234);canvas_test_call(addresses[i],i==3?0x700000:0x700100,4,pops[i],before[i]);if(GetLastError()!=1234)ExitProcess(3);}
    /* All-entry byte validation must refuse before mutating any earlier site. */
    ((u8*)addresses[11])[0]^=1;
    if(canvas_startup_install()||((u8*)addresses[0])[0]!=prefixes[0][0])ExitProcess(4);
    ((u8*)addresses[11])[0]^=1;
    if(!canvas_startup_install())ExitProcess(5);
    for(u32 i=0;i<12;++i){SetLastError(1234);canvas_test_call(addresses[i],i==3?0x700000:0x700100,4,pops[i],after);
        if(GetLastError()!=1234)ExitProcess(6);
        /* EBP/ESP record the same helper stack; registers, EFLAGS, x87/SSE all compare. */
        for(u32 j=0;j<560;++j)if(after[j]!=before[i][j]){
            HANDLE file=CreateFileA("forwarding-difference.bin",0x40000000,0,0,1,0x80,0);u32 wrote;
            WriteFile(file,before[i],560,&wrote,0);WriteFile(file,after,560,&wrote,0);CloseHandle(file);ExitProcess(20+i);}
    }
    *(u32*)0x658174=0x700000;*(u32*)0x6a2dc8=4;*(u32*)0x6a49b8=4;*(u32*)0x656618=4;
    u32 r[10]={0};canvas_startup_world(r);
    /* Calls continue forwarding after the diagnostic limit/boundary closes. */
    for(u32 i=0;i<12;++i)canvas_test_call(addresses[i],i==3?0x700000:0x700100,4,pops[i],after);
    ExitProcess(0);
}
