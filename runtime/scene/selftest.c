#include "../shadow/win32_min.h"
API int WIN SceneInstallForTest(u32);
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static u8 frame[54];
static u32 records[3][9],queue[6];
void start(void){
    const u32 base=0x20000000;
    u8* image=VirtualAlloc((void*)base,0x310000,0x3000,0x40);if(image!=(void*)base)ExitProcess(1);
    u8 body[]={0x83,0xec,0x18,0x53,0xb8,0x00,0x6c,0xca,0x88,0x5b,0x83,0xc4,0x18,0xc3};
    for(u32 i=0;i<sizeof(body);++i)image[0x1002a0+i]=body[i];
    image[0x1002a0]=0x90;if(SceneInstallForTest(base))ExitProcess(2);image[0x1002a0]=0x83;
    if(SceneInstallForTest(base)!=1||SceneInstallForTest(base)!=0)ExitProcess(2);
    put(image+0x2a49b8,8);put(image+0x256618,4);
    put(frame,54);put(frame+4,2);put(frame+8,1);put(frame+28,0xffffffff);put(frame+40,48);put(frame+44,50);frame[48]=0;frame[49]=2;frame[50]=0;frame[51]=0;frame[52]=0;frame[53]=248;
    for(u32 i=0;i<3;++i){records[i][0]=i;records[i][1]=(u32)frame;records[i][2]=2+i;records[i][3]=1;records[i][6]=i==2?0xfffffffe:0;records[i][7]=0x8ad08ad0;}
    queue[0]=(u32)records;queue[1]=(u32)(records+3);queue[2]=3;queue[3]=3;queue[5]=2;
    typedef u32 (__attribute__((thiscall))*Run)(void*);
    queue[0]=1;SetLastError(0xabc123);if(((Run)(image+0x1002a0))(queue)!=0x88ca6c00||GetLastError()!=0xabc123)ExitProcess(4);queue[0]=(u32)records;
    u32 initial=0x76543210,mxcsr=0x3f80;
    __asm__ volatile("movd %0, %%xmm0; ldmxcsr %1; fld1"::"r"(initial),"m"(mxcsr));
    for(u32 n=0;n<6;++n){SetLastError(0xabc123);u32 result=((Run)(image+0x1002a0))(queue);
        if(result!=0x88ca6c00||GetLastError()!=0xabc123||records[0][1]!=(u32)frame||queue[2]!=3)ExitProcess(3);}
    u32 xmm,floating[2];__asm__ volatile("movd %%xmm0, %0; stmxcsr %1; fstpl %2":"=r"(xmm),"=m"(mxcsr),"=m"(floating));
    if(xmm!=initial||mxcsr!=0x3f80||floating[0]||floating[1]!=0x3ff00000)ExitProcess(5);
    // Invalid readable extents still forward the original function.
    queue[0]=1;if(((Run)(image+0x1002a0))(queue)!=0x88ca6c00)ExitProcess(4);
    ExitProcess(0);
}
