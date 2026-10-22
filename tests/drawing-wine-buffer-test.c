#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls;static void* returned;
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static void* result(void){require(GetLastError()==0xbeef,91);++calls;SetLastError(0xcafe);u16 cw=0x0b7f;__asm__ volatile("fldcw %0; fnclex"::"m"(cw));return returned;}
static void* __attribute__((cdecl)) front(void* object){require(object==(void*)9,92);return result();}
static void* __attribute__((cdecl)) back(void* object,u32 index){require(object==(void*)9&&index==3,93);return result();}
void start(void){
 require(initialize(),90);wine_buffer_original[0]=(void*)&front;wine_buffer_original[1]=(void*)&back;
 void* objects[2]={(void*)9,(void*)2};for(u32 i=0;i<2;++i){u32 r[24]={0};wine_lifetime_entry(r,i?1:3,0);wine_lifetime_return(r,0,0,objects+i);}
 for(u32 method=1;method<=2;++method)for(u32 scenario=0;scenario<3;++scenario){returned=scenario==0?0:scenario==1?(void*)2:(void*)3;SetLastError(0xbeef);u32 before=calls;
  void* value=method==1?wine_buffer_front((void*)9):wine_buffer_back((void*)9,3);u16 cw;__asm__ volatile("fnstcw %0":"=m"(cw));require(value==returned&&calls==before+1&&GetLastError()==0xcafe&&cw==0x0b7f,94);
 }
 require(calls==6,95);
 /* A retired texture leaves an empty slot of kind1. Null getters must still
  * report generation0; the old pointer-only search returned that retired ID. */
 u32 retirement[24]={0};wine_lifetime_entry(retirement,2,(void*)2);wine_lifetime_return(retirement,0,0,0);
 require(!wine_lifetime_find(0,1),96);returned=0;
 for(u32 method=1;method<=2;++method){SetLastError(0xbeef);u32 before=calls;
  void* value=method==1?wine_buffer_front((void*)9):wine_buffer_back((void*)9,3);
  require(!value&&calls==before+1&&GetLastError()==0xcafe,97);
 }
 require(calls==8,98);wine_buffer_close(1);wine_lifetime_close(1);ExitProcess(0);
}
