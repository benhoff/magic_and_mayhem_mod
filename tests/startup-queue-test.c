#include "../runtime/shadow/win32_min.h"
API int WIN SceneInstallForTest(u32);
static u32 rows[45][9],queue[6];
typedef u32 (__attribute__((thiscall))*Run)(void*);
static void run(Run draw,void* q){SetLastError(0xabc123);if(draw(q)!=0x88ca6c00||GetLastError()!=0xabc123)ExitProcess(3);}
void start(void){
    const u32 base=0x20000000;
    u8* image=VirtualAlloc((void*)base,0x310000,0x3000,0x40);if(image!=(void*)base)ExitProcess(1);
    u8 body[]={0x83,0xec,0x18,0x53,0xb8,0x00,0x6c,0xca,0x88,0x5b,0x83,0xc4,0x18,0xc3};
    for(u32 i=0;i<sizeof(body);++i)image[0x1002a0+i]=body[i];
    if(SceneInstallForTest(base)!=1)ExitProcess(2);
    for(u32 i=0;i<45;++i){for(u32 j=0;j<9;++j)rows[i][j]=i*100+j;rows[i][1]=0;rows[i][6]=i<41?i:i==41?0xfffffffe:i==42?0xffffffff:i==43?0x7fffffff:0x80000000;}
    queue[0]=1;queue[2]=45;queue[3]=45;queue[5]=2;
    Run draw=(Run)(image+0x1002a0);run(draw,queue);
    queue[0]=(u32)rows;run(draw,queue);
    queue[2]=0;run(draw,queue);
    queue[2]=45;queue[3]=44;run(draw,queue);
    run(draw,(void*)1);
    queue[3]=45;run(draw,queue);
    run(draw,queue);
    queue[5]=19;run(draw,queue);
    /* The bounded journal must stop exactly at eight, preserving forwarding. */
    run(draw,queue);run(draw,queue);
    ExitProcess(0);
}
