#include "win32_min.h"
API int WIN ShadowInstallForTest(u32);
extern u8 fake_start[],fake_end[];
extern u32 invoke_checked(u32 fn,u32 current,u32* output,const void* descriptor,const void* prior,i32* budget);
void* memset(void* pointer,int value,u32 size) {volatile u8* p=pointer;while(size--)*p++=(u8)value;return pointer;}
static const u32 fake_base=0x20000000;
static void put(u32 address,u32 value) { if(address>=0x400000 && address<0x700000)address+=fake_base-0x400000;*(u32*)address=value; }
static void fill(u32 address,u8 value,u32 length) { u8* p=(u8*)address;while(length--)*p++=value; }
void fake_body(u32 current,u32* output,const u8* descriptor,const u8* prior,i32* budget) {
    (void)descriptor;(void)prior;
    if(GetLastError()!=0x77)ExitProcess(10);
    if(*budget>0) {
        static const i32 xy[8][2]={{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
        for(u32 i=0;i<8;++i) {
            u32* candidate=(u32*)output[2];
            candidate[0]=(i&1)?48:32;candidate[1]=current+(xy[i][0]+xy[i][1]*6)*12;
            candidate[2]=0;candidate[3]=i;candidate[4]=0;candidate[5]=720;
            candidate[6]=(i&1)?0x4099999a:0x404ccccd;candidate[7]=0;candidate[8]=0;
            output[2]+=36;
        }
        --*budget;
    }
    SetLastError(0x88);
}
static void fail(u32 code) {
    u32 details[10]={code,GetLastError(),(u32)GetModuleHandleA(0)};
    VirtualQuery((void*)0x400000,details+3,28);
    HANDLE file=CreateFileA("selftest-failure.bin",0x40000000,0,0,2,0x80,0);
    u32 written;WriteFile(file,details,sizeof(details),&written,0);CloseHandle(file);ExitProcess(code);
}
void start(void) {
    if(VirtualAlloc((void*)fake_base,0x300000,0x3000,0x40)!=(void*)fake_base)fail(1);
    if(VirtualAlloc((void*)0x1000000,0x500000,0x3000,4)!=(void*)0x1000000)fail(2);
    for(u32 i=0;i<(u32)(fake_end-fake_start);++i)((u8*)(fake_base+0xeaae0))[i]=fake_start[i];
    put(0x6c5494,6);put(0x6c5498,6);put(0x5e1780,3);put(0x6c54a0,36);
    put(0x6c54dc,0x1000000);put(0x65660c,0x1100000);put(0x6c5c80,1);
    put(0x6c007d,720);put(0x5e15f0,0x3f781062);
    for(u32 i=0;i<6;++i)put(0x6cb942+i*4,i*6);
    for(u32 i=0;i<3;++i)put(0x6cb8c2+i*4,i*36);
    for(u32 i=0;i<108;++i) { *(u16*)(0x1000000+i*12+4)=0xffff;if(i<36)*(u16*)(0x1000000+i*12)=1; }
    put(0x1100000+0x164+0x94,16);*(u8*)(0x1100000+0x164+0xb0)=8;
    put(0x1200008,1);put(0x120000c,1);put(0x1200010,1);put(0x12000ac,0x1300000);
    put(0x1300008,1);put(0x130000c,1);put(0x1300010,500);
    for(u32 i=0;i<48;++i)put(0x1300000+0xd8+i*4,(i/12)%2?40:60);
    put(0x6a5f80+0x589,720);
    if(!ShadowInstallForTest(fake_base))ExitProcess(3);
    u32 output[4]={0,0x1400000,0x1400000+36,0x1400000+36*27};
    fill(0x1400000,0x55,36);
    u8 descriptor[70]={0};*(u32*)(descriptor+4)=0x1200000;
    *(u32*)(descriptor+0x2e)=3;*(u32*)(descriptor+0x32)=1;*(u32*)(descriptor+0x36)=1;
    u32 prior[7]={5,0,0,0,0,0,0};i32 budget=2;
    typedef u32 (__attribute__((thiscall)) *Fn)(void*,u32*,const void*,const void*,i32*);
    /* Unsupported mode must forward without consuming the capture slot. */
    *(u32*)descriptor=1;SetLastError(0x77);
    if(((Fn)(fake_base+0xeaae0))((void*)(0x1000000+43*12),output,descriptor,prior,&budget)!=0x12345678)ExitProcess(8);
    *(u32*)descriptor=0;budget=2;output[2]=0x1400000+36;
    u32 floating[2],sse,mxcsr;
    SetLastError(0x77);
    __asm__ volatile("fld1");
    u32 result=invoke_checked(fake_base+0xeaae0,0x1000000+43*12,output,descriptor,prior,&budget);
    __asm__ volatile("movd %%xmm0, %0; stmxcsr %1":"=r"(sse),"=m"(mxcsr));
    __asm__ volatile("fstpl %0":"=m"(floating));
    if(sse!=0x76543210||mxcsr!=0x3f80)ExitProcess(7);
    if(result!=0x12345678||GetLastError()!=0x88||budget!=1||output[2]!=0x1400000+36*9)ExitProcess(4);
    if(floating[0]!=0||floating[1]!=0x3ff00000)ExitProcess(5);
    for(u32 i=0;i<36;++i)if(((u8*)0x1400000)[i]!=0x55)ExitProcess(6);
    /* A depleted budget still executes the original function, without logging. */
    budget=0;SetLastError(0x77);
    if(((Fn)(fake_base+0xeaae0))((void*)(0x1000000+43*12),output,descriptor,prior,&budget)!=0x12345678
       ||budget!=0||output[2]!=0x1400000+36*9||GetLastError()!=0x88)ExitProcess(9);
    ExitProcess(0);
}
