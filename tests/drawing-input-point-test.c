#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API u32 WIN GetCurrentThreadId(void);
API void WIN Sleep(u32);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN DrawingInstallCursorForTest(void*);
API i32 WIN DrawingObserveCursorForTest(i32*);
API u32 WIN DrawingInputPointV1(u32,void*,u32);
static u32 calls,callback_success=1,done,release_threads,foreign_owner;
static u32 points[5][16];
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void clear(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static int same(const void* a,const void* b,u32 n){const u8* x=a,*y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static i32 WIN original_cursor(i32* out){
 __sync_fetch_and_add(&calls,1);out[0]=400;out[1]=300;u16 control=0x0b7f;
 __asm__ volatile("fldcw %0; fnclex"::"m"(control));SetLastError(0xabcd);return callback_success;
}
static i32 forward(i32* point){
 i32 result=DrawingObserveCursorForTest(point);u32 error=GetLastError();u16 control;__asm__ volatile("fnstcw %0":"=m"(control));require(error==0xabcd&&control==0x0b7f,226);SetLastError(error);return result;
}
static u32 query(u32 owner,void* out,u32 bytes){
 u8 before[512] __attribute__((aligned(16))),after[512] __attribute__((aligned(16)));clear(before,512);clear(after,512);
 SetLastError(0x7351);__asm__ volatile("fxsave (%0)"::"r"(before):"memory");u32 result=DrawingInputPointV1(owner,out,bytes);
 u32 error=GetLastError();__asm__ volatile("fxsave (%0)"::"r"(after):"memory");require(error==0x7351&&same(before,after,512),210);return result;
}
static u32 WIN worker(void* first){
 if(first)foreign_owner=GetCurrentThreadId();i32 point[2];require(forward(point)==1&&GetLastError()==0xabcd,211);
 __sync_fetch_and_add(&done,1);while(!__atomic_load_n(&release_threads,__ATOMIC_ACQUIRE))Sleep(5);return 0;
}
static void wait_done(u32 count){u32 begin=GetTickCount();while(__atomic_load_n(&done,__ATOMIC_ACQUIRE)<count){require(GetTickCount()-begin<5000,212);Sleep(5);}}
void start(void){
 require(DrawingInstallCursorForTest((void*)&original_cursor),201);u32 owner=GetCurrentThreadId();i32 point[2];
 require(forward(point)==1&&GetLastError()==0xabcd&&calls==1,202);
 require(query(owner,points[0],64)&&points[0][3]==3&&points[0][11]==1&&points[0][12]==1&&points[0][14]==400&&points[0][15]==300,203);
 require(query(0xffffffff,points[1],64)&&points[1][3]==1&&!points[1][11],204);
 u32 untouched[16];for(u32 i=0;i<16;++i)untouched[i]=0x6b6b6b6b;
 require(!query(owner,untouched,63)&&!query(0,untouched,64)&&!query(owner,0,64)&&!query(owner,(void*)0xffffffe0,64),205);
 for(u32 i=0;i<16;++i)require(untouched[i]==0x6b6b6b6b,206);
 u8* readonly=VirtualAlloc(0,4096,0x3000,4);require(readonly!=0,213);for(u32 i=0;i<4096;++i)readonly[i]=0x6b;u32 old,ignored;
 require(VirtualProtect(readonly,4096,2,&old)&&!query(owner,readonly,64),214);
 for(u32 i=0;i<4096;++i)require(readonly[i]==0x6b,215);require(VirtualProtect(readonly,4096,old,&ignored),216);
 callback_success=0;require(forward(point)==0&&GetLastError()==0xabcd&&calls==2,207);
 require(query(owner,points[2],64)&&points[2][3]==1&&points[2][11]==2&&!points[2][14]&&!points[2][15],208);
 callback_success=1;HANDLE threads[16];threads[0]=CreateThread(0,0,worker,(void*)1,0,0);require(threads[0]!=0,217);wait_done(1);
 require(query(foreign_owner,points[3],64)&&points[3][3]==3&&points[3][11]==3,218);
 require(query(owner,points[4],64)&&points[4][11]==2&&points[4][3]==1&&calls==3,219);
 for(u32 i=1;i<16;++i){threads[i]=CreateThread(0,0,worker,0,0,0);require(threads[i]!=0,220);}wait_done(16);
 require(calls==18&&!query(owner,untouched,64),221);for(u32 i=0;i<16;++i)require(untouched[i]==0x6b6b6b6b,222);
 __atomic_store_n(&release_threads,1,__ATOMIC_RELEASE);for(u32 i=0;i<16;++i){require(WaitForSingleObject(threads[i],5000)==0,223);CloseHandle(threads[i]);}
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_DRAW_POINT_FIXTURE_FILE",name,sizeof(name));require(n&&n<sizeof(name),224);
 HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);u32 written;require(file!=(HANDLE)-1&&WriteFile(file,points,sizeof(points),&written,0)&&written==sizeof(points)&&CloseHandle(file),225);ExitProcess(0);
}
