#include "../runtime/shadow/win32_min.h"
#include "../runtime/native-draw/cursor_copy_caller_thunks.h"
static u32 whole_slot,rect_slot,checks;
static const u32 rectangle[4]={1,2,9,10};
static void need(int value,u32 code){if(!value)ExitProcess(code);++checks;}
static void __attribute__((thiscall,noinline)) rect(void* source,void* destination,const u32* r,u32 x,u32 y){
 u32 slot=(u32)__builtin_frame_address(0)+4;
 need(source==(void*)0x1122 && destination==(void*)0x3344 && r==rectangle && x==17 && y==19,91);
 need(slot==rect_slot && *(u32*)slot==(u32)nd_copy_caller_rect_return && *(u32*)(slot+4)==(u32)destination,92);
 SetLastError(0x789abc);
}
static void __attribute__((thiscall,noinline)) whole(void* source,void* destination,u32 x,u32 y){
 u32 slot=(u32)__builtin_frame_address(0)+4;
 need(source==(void*)0x5566 && destination==(void*)0x7788 && x==23 && y==29,93);
 need(slot==whole_slot && *(u32*)slot==(u32)nd_copy_caller_whole_return && *(u32*)(slot+4)==(u32)destination,94);
 const u32 saved=whole_slot;
 nd_copy_caller_rect((void*)rect,(void*)0x1122,(void*)0x3344,rectangle,17,19,&rect_slot);
 need(whole_slot==saved && rect_slot!=whole_slot && GetLastError()==0x789abc,95);
 SetLastError(0x123456);
}
void main(void){
 SetLastError(0x123);
 nd_copy_caller_whole((void*)whole,(void*)0x5566,(void*)0x7788,23,29,&whole_slot);
 need(GetLastError()==0x123456,96);
 nd_copy_caller_rect((void*)rect,(void*)0x1122,(void*)0x3344,rectangle,17,19,&rect_slot);
 need(GetLastError()==0x789abc,97);
 u32 result[8]={0x3143434d,1,checks,whole_slot,rect_slot,(u32)nd_copy_caller_whole_return,(u32)nd_copy_caller_rect_return,1},written=0;
 HANDLE file=CreateFileA("thunk-results.bin",0x40000000,1,0,1,0x80,0);
 need(file!=(HANDLE)-1 && WriteFile(file,result,sizeof(result),&written,0) && written==sizeof(result),98);
 CloseHandle(file);ExitProcess(0);
}
