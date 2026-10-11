#include "../shadow/win32_min.h"
API u32 WIN DrawingInstallForTest(void*,u32);
API void WIN DrawingActionV1(u32,u32,u32,u32,u32,u32,u32);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 readbacks;
static u32 WIN nested(void* p){u32 id=(u32)p;DrawingActionV1(id,1,3,1,0,1,0);DrawingActionV1(id,1,3,1,0,2,1);return 0;}
static u16 pixels[48];static void* vt[36];static struct {void** vt;} object;
static u32 callback_slot,callback_result,calls;
static u32 result(void){++calls;SetLastError(0xa000+callback_slot);u16 control=0x0b7f;__asm__ volatile("fldcw %0; fnclex"::"m"(control));return callback_result;}
static u32 WIN desc(void* p,u32* d){(void)p;d[2]=6;d[3]=8;d[4]=16;d[9]=(u32)pixels;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x200;SetLastError(0xfeed);return 0;}
static u32 WIN lock(void* p,u32 rect,u32* d,u32 flags,u32 event){(void)rect;(void)flags;(void)event;HANDLE thread=CreateThread(0,0,nested,(void*)(100+readbacks++),0,0);if(!thread||WaitForSingleObject(thread,5000)!=0)ExitProcess(60);CloseHandle(thread);return desc(p,d);}
static u32 WIN unlock(void* p,u32 data){(void)p;(void)data;return result();}
static u32 WIN blt(void* p,u32 dest,u32 src,u32 rect,u32 flags,u32 effects){(void)p;(void)dest;(void)src;(void)rect;(void)effects;if(flags!=0x1000000)ExitProcess(40);return result();}
static u32 WIN fast(void* p,u32 x,u32 y,u32 src,u32 rect,u32 flags){(void)p;(void)x;(void)y;(void)src;(void)rect;(void)flags;return result();}
static u32 WIN flip(void* p,u32 target,u32 flags){(void)p;(void)target;(void)flags;return result();}
static u32 WIN single(void* p){(void)p;return result();}
static u32 WIN pair(void* p,u32 arg){(void)p;(void)arg;return result();}
static u32 WIN batch(void* p,u32 arg,u32 count,u32 flags){(void)p;(void)arg;(void)count;(void)flags;return result();}
static void require(int okay,u32 id){if(!okay)ExitProcess(id);}
void start(void){
 object.vt=vt;vt[5]=(void*)&blt;vt[7]=(void*)&fast;vt[11]=(void*)&flip;vt[22]=(void*)&desc;vt[25]=(void*)&lock;vt[32]=(void*)&unlock;vt[26]=(void*)&pair;vt[27]=(void*)&single;vt[31]=(void*)&pair;vt[6]=(void*)&batch;
 for(u32 i=0;i<48;++i)pixels[i]=(u16)(i*997);require(DrawingInstallForTest(&object,11),41);
 const u32 faults[]={0,0x887601c2,0x887601ae,0x80004005};
 for(u32 m=0;m<8;++m)for(u32 f=0;f<4;++f){callback_slot=m;callback_result=faults[f];SetLastError(0xbeef);u32 before=calls,r=0;
  switch(m){case 0:r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))vt[5])(&object,0,0,0,0x1000000,0);break;case 1:r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))vt[7])(&object,0,0,0,0,16);break;case 2:r=((u32(WIN*)(void*,u32,u32))vt[11])(&object,0,0);break;case 3:r=((u32(WIN*)(void*,u32))vt[32])(&object,0);break;case 4:r=((u32(WIN*)(void*,u32))vt[26])(&object,0);break;case 5:r=((u32(WIN*)(void*))vt[27])(&object);break;case 6:r=((u32(WIN*)(void*,u32))vt[31])(&object,0);break;case 7:r=((u32(WIN*)(void*,u32,u32,u32))vt[6])(&object,0,0,0);break;}
  u16 control;__asm__ volatile("fnstcw %0":"=m"(control));require(r==callback_result&&GetLastError()==0xa000+m&&control==0x0b7f&&calls-before==(f?1:2),42+m);
 }
 DrawingActionV1(1,1,3,1,0,1,0);DrawingActionV1(1,1,3,1,0,2,1);ExitProcess(0);
}
