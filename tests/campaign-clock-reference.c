#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API u32 WIN timeGetTime(void);
API void WIN Sleep(u32);
typedef u32(WIN *PacerTickFn)(void);
#include "../runtime/render/precise_clock.h"
static u32 calls,fine_calls;
static u32 WIN original(void){if(GetLastError()!=77)ExitProcess(1);++calls;SetLastError(88);return 0xffffffff;}
static u32 WIN fine(void){if(GetLastError()!=77)ExitProcess(2);++fine_calls;SetLastError(99);return 1;}
void start(void){
 if(!precise_clock_epoch_matches(0xffffffff,1)||!precise_clock_epoch_matches(0,32)||!precise_clock_epoch_matches(32,0)||precise_clock_epoch_matches(0,33)||precise_clock_epoch_matches(33,0))ExitProcess(3);
 SetLastError(77);if(precise_clock_read(original,0)!=0xffffffff||GetLastError()!=88)ExitProcess(4);
 pacer_precise_tick=fine;SetLastError(77);if(precise_clock_read(original,pacer_precise_tick)!=1||GetLastError()!=88||calls!=2||fine_calls!=1)ExitProcess(5);
 HANDLE out=CreateFileA("clocks.bin",0x40000000,0,0,1,0,0);u32 bytes;
 for(u32 i=0;i<1024;i++){
  u32 row[4]={i,GetTickCount(),timeGetTime(),GetTickCount()};
  if(!WriteFile(out,row,sizeof(row),&bytes,0)||bytes!=sizeof(row)||!precise_clock_epoch_matches(row[1],row[2]))ExitProcess(6);
  Sleep(1);
 }
 CloseHandle(out);ExitProcess(0);
}
