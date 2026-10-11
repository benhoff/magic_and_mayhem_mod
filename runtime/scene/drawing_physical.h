/* Diagnostic original-active post-copy fence. No physical pixels enter replay. */
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
API void* WIN GetProcAddress(void*,const char*);
static u32 physical_enabled;
static int physical_init(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW02_PHYSICAL_FENCE",flag,2);
 if(!n)return 1;if(n!=1||flag[0]!='1')return 0;physical_enabled=1;return 1;
}
static void physical_fence(u32* r){
 if(!physical_enabled||!r[19])return;
 /* Flush queued GDI presentation, without acquiring another driver lock.
  * The containing assembly restores the original FP/register state. */
 void* module=GetModuleHandleA("gdi32.dll");
 void* flush=module?GetProcAddress(module,"GdiFlush"):0;
 if(!flush||!((int(WIN*)(void))flush)())ExitProcess(96);
 char name[260];path(name,"physical-request-",r[19],".bin");
 HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(file==(HANDLE)-1||!write(file,r,96)||!CloseHandle(file))ExitProcess(96);
 path(name,"physical-ack-",r[19],".bin");u32 start=GetTickCount();
 for(;;){
  file=CreateFileA(name,0x80000000,3,0,3,0x80,0);
  if(file!=(HANDLE)-1){u32 ack[4],got=0,size=GetFileSize(file,0);int okay=size==16&&ReadFile(file,ack,16,&got,0)&&got==16;CloseHandle(file);
   if(size>16)ExitProcess(96);
   if(okay){if(ack[0]!=0x504d4e4d||ack[1]!=0x314b4341||ack[2]!=r[19]||ack[3]!=r[0])ExitProcess(96);return;}}
  if(GetTickCount()-start>=5000)ExitProcess(96);Sleep(1);
 }
}
