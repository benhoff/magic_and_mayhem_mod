#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 calls,fault;
static i32 source_rect[4]={-2,3,8,9},destination_rect[4]={1,2,11,12};
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN reentrant(void* unused){(void)unused;EnterCriticalSection(journal_lock);LeaveCriticalSection(journal_lock);return 0;}
static u32 callback_return(void){
 require(GetLastError()==0xbeef,71);HANDLE thread=CreateThread(0,0,reentrant,0,0,0);require(thread&&WaitForSingleObject(thread,5000)==0,72);CloseHandle(thread);
 ++calls;SetLastError(0xcafe);u16 control=0x0b7f;__asm__ volatile("fldcw %0; fnclex"::"m"(control));return fault;
}
static u32 __attribute__((cdecl)) fake_blt(void* context,void* dst,u32 dstsub,const void* dr,void* src,u32 srcsub,const void* sr,u32 flags,const void* fx,u32 filter){
 require(context==(void*)1&&dst==(void*)2&&src==(void*)3&&dstsub==4&&srcsub==5&&dr==destination_rect&&sr==source_rect&&flags==6&&fx==(void*)7&&filter==8,73);return callback_return();
}
static u32 __attribute__((cdecl)) fake_present(void* swapchain,const void* sr,const void* dr,void* window,u32 interval,u32 flags){
 require(swapchain==(void*)9&&sr==source_rect&&dr==destination_rect&&window==(void*)10&&interval==11&&flags==12,74);return callback_return();
}
void start(void){
 require(initialize(),70);wine_submission_original[0]=(void*)&fake_blt;wine_submission_original[1]=(void*)&fake_present;
 struct Context parent={0};parent.observed=1;parent.r[1]=1;parent.r[29]=41;wine_submission_parent(&parent,1);
 struct Context nested;copy(&nested,&parent,sizeof(nested));nested.r[29]=42;wine_submission_parent(&nested,1);
 const u32 faults[]={0,0x80004005,0x887601c2,0x887601ae};
 for(u32 method=0;method<2;++method)for(u32 i=0;i<4;++i){fault=faults[i];SetLastError(0xbeef);u32 before=calls;
  u32 result=method?wine_submission_present((void*)9,source_rect,destination_rect,(void*)10,11,12):wine_submission_blt((void*)1,(void*)2,4,destination_rect,(void*)3,5,source_rect,6,(void*)7,8);
  u16 control;__asm__ volatile("fnstcw %0":"=m"(control));require(result==fault&&GetLastError()==0xcafe&&control==0x0b7f&&calls==before+1,75);
 }
 wine_submission_parent(&nested,2);wine_submission_parent(&parent,2);SetLastError(0xbeef);fault=0;require(wine_submission_present((void*)9,source_rect,destination_rect,(void*)10,11,12)==0,76);
 wine_submission_close(1);ExitProcess(0);
}
