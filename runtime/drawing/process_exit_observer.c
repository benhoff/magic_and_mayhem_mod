/* Independent PE32 observer: query and wait on the process owning a live
 * menu thread. Never inject, suspend, terminate or read target memory. */
#include "../shadow/win32_min.h"
API HANDLE WIN OpenThread(u32,i32,u32);
API u32 WIN GetProcessIdOfThread(HANDLE);
API HANDLE WIN OpenProcess(u32,i32,u32);
API i32 WIN GetExitCodeProcess(HANDLE,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN GetTickCount(void);
API i32 WIN FlushFileBuffers(HANDLE);
void* memset(void* out,int value,u32 n){u8* p=out;while(n--)*p++=(u8)value;return out;}
static u32 number(const char* key){
 char value[16];u32 n=GetEnvironmentVariableA(key,value,sizeof(value)),result=0;
 if(!n||n>=sizeof(value))ExitProcess(201);
 for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9'||result>429496729u)ExitProcess(201);u32 digit=(u32)(value[i]-'0');if(result==429496729u&&digit>5)ExitProcess(201);result=result*10+digit;}
 return result;
}
static void write_record(HANDLE file,const void* data,u32 size){
 u32 written=0;if(!WriteFile(file,data,size,&written,0)||written!=size||!FlushFileBuffers(file))ExitProcess(205);
}
void start(void){
 u32 thread=number("MNM_PROCESS_EXIT_THREAD"),timeout=number("MNM_PROCESS_EXIT_TIMEOUT_MS");
 if(!thread||!timeout||timeout>600000)ExitProcess(201);
 char path[1024];u32 n=GetEnvironmentVariableA("MNM_PROCESS_EXIT_FILE",path,sizeof(path));if(!n||n>=sizeof(path))ExitProcess(201);
 HANDLE file=CreateFileA(path,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(202);
 u32 header[16]={0x444d4e4d,0x31305850,1,64,thread,timeout};write_record(file,header,64);
 SetLastError(0);HANDLE owner=OpenThread(0x40,0,thread);u32 pid=owner?GetProcessIdOfThread(owner):0,error=GetLastError();
 HANDLE process=pid?OpenProcess(0x101000,0,pid):0;if(!process)error=GetLastError();
 if(owner&&!CloseHandle(owner))ExitProcess(203);
 u32 code=0,wait=0xffffffffu,query=0;
 if(process){SetLastError(0);wait=WaitForSingleObject(process,0);error=GetLastError();SetLastError(0);query=GetExitCodeProcess(process,&code);if(!query)error=GetLastError();}
 u32 row[16]={1,1,thread,pid,wait,query,code,GetTickCount(),error};
 if(!process||wait!=258||!query||code!=259){row[1]=3;write_record(file,row,64);if(process)CloseHandle(process);CloseHandle(file);ExitProcess(204);}
 write_record(file,row,64);u32 began=GetTickCount();
 do{SetLastError(0);wait=WaitForSingleObject(process,500);error=GetLastError();}while(wait==258&&(u32)(GetTickCount()-began)<timeout);
 SetLastError(0);query=GetExitCodeProcess(process,&code);if(!query)error=GetLastError();
 row[0]=2;row[1]=wait==0&&query?2:3;row[4]=wait;row[5]=query;row[6]=code;row[7]=GetTickCount();row[8]=error;
 write_record(file,row,64);if(!CloseHandle(process)||!CloseHandle(file))ExitProcess(203);
 ExitProcess(row[1]==2?0:204);
}
