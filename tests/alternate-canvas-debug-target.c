/* Positive control: real watch hits on main and created worker threads. */
#include "../runtime/shadow/win32_min.h"
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 WIN writes(void* unused){(void)unused;volatile u32* base=(u32*)0x6e1f68;
 base[0]=0x12340000;base[1]=0x56780000;((void(*)(void))0x4fd438)();base[0]=0;base[1]=0;((void(*)(void))0x58acf0)();return 0;}
void start(void){u32 old;if(!VirtualProtect((void*)0x4fd000,0x1e5000,0x40,&old))ExitProcess(60);
 *(u8*)0x4fd438=0xc3;*(u8*)0x58acf0=0xc3;writes(0);
 HANDLE worker=CreateThread(0,0,(void*)writes,0,0,0);if(!worker||WaitForSingleObject(worker,10000)!=0)ExitProcess(61);CloseHandle(worker);ExitProcess(0);}
