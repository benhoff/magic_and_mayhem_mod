/* Actual menu callback relay into the production drawing DLL; owned dispatch. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
#include "../runtime/menu/drawing_action.h"
static u32 forwards;
static void require(int okay,u32 code){if(!okay)ExitProcess(code);}
static u32 original_dispatch(u32 result){++forwards;SetLastError(0xcafe);return result;}
void start(void){
 require(LoadLibraryA("MnmDrawing.dll")&&drawing_action_init(),70);
 for(u32 status=1;status<=5;++status){
  SetLastError(0xbeef);drawing_action_record(status,1,3,9,status+20,1,~0u);
  require(GetLastError()==0xbeef,71);
  u32 before=forwards,result=original_dispatch(status);
  drawing_action_record(status,1,3,9,status+20,2,result);
  require(forwards==before+1&&GetLastError()==0xcafe&&result==status,72);
 }
 drawing_action=0;SetLastError(0xabcd);drawing_action_record(99,1,3,9,0,1,~0u);require(GetLastError()==0xabcd&&forwards==5,73);
 ExitProcess(0);
}
