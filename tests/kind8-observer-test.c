#include "../runtime/shadow/win32_min.h"
API int WIN SceneInstallForTest(u32);
static u32 rows[600][9],queue[6],object[20],bad[20],table[5];
typedef u32 (__attribute__((thiscall))*Run)(void*);
static void run(Run draw){SetLastError(0xabc123);if(draw(queue)!=0x88ca6c00||GetLastError()!=0xabc123)ExitProcess(3);}
void start(void){
    const u32 base=0x20000000;u8* image=VirtualAlloc((void*)base,0x310000,0x3000,0x40);if(image!=(void*)base)ExitProcess(1);
    const u8 body[]={0x83,0xec,0x18,0x53,0xb8,0x00,0x6c,0xca,0x88,0x5b,0x83,0xc4,0x18,0xc3};
    for(u32 i=0;i<sizeof(body);++i)image[0x1002a0+i]=body[i];if(SceneInstallForTest(base)!=1)ExitProcess(2);
    for(u32 i=0;i<20;++i)object[i]=0x9000+i;object[0]=(u32)table;bad[0]=1;table[3]=0x12345678;
    for(u32 i=0;i<600;++i){rows[i][1]=(u32)object;rows[i][2]=i;rows[i][3]=(u32)-4;rows[i][6]=8;}
    rows[0][6]=0;rows[2][1]=1;rows[3][1]=(u32)bad;queue[0]=(u32)rows;queue[2]=4;queue[3]=600;
    Run draw=(Run)(image+0x1002a0);run(draw);queue[2]=600;run(draw);run(draw);ExitProcess(0);
}
