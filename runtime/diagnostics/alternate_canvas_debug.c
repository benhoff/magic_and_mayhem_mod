/* Read-only child-memory observer. Every created child thread gets two DWORD
 * write watches and execution watches at dispatch/shutdown. No software traps,
 * pixel reads, pointer publication, CPU-family edits or WriteProcessMemory. */
#include "win32_debug_min.h"
#include "alternate_canvas_pinned.h"
API DWORD WIN GetFileAttributesA(const char*);
API int WIN TerminateProcess(HANDLE,u32);
API u32 WIN SuspendThread(HANDLE);
API u32 WIN ResumeThread(HANDLE);
API void* WIN FindWindowA(const char*,const char*);
API u32 WIN GetWindowThreadProcessId(void*,u32*);
API void* WIN GetWindow(void*,u32);
API int WIN GetWindowTextA(void*,char*,int);
static HANDLE process,output;
static struct {u32 id;HANDLE handle;} threads[256];
static u32 header[64]={0x414d4e4d,0x31305743,2,256},rows[256][16],row_count;
static u32 last_values[5],have_values;
static char error_text[256];
static void save(void){
 u32 done;if(SetFilePointer(output,0,0,0)!=0||!WriteFile(output,header,256,&done,0)||done!=256||
  !WriteFile(output,rows,row_count*64,&done,0)||done!=row_count*64||!WriteFile(output,error_text,256,&done,0)||done!=256||!FlushFileBuffers(output))ExitProcess(91);
}
static void fail(u32 code){header[5]=code;save();ExitProcess(code);}
static void read_child(u32 address,void* data,u32 size){u32 done;if(!ReadProcessMemory(process,(void*)address,data,size,&done)||done!=size)fail(92);}
static void emit(u32 event,u32 tid,CONTEXT* c,u32 hit){
 u32 values[5];read_child(0x6e1f68,values,20);
 u32 changed=!have_values;for(u32 i=0;i<5;++i)changed|=values[i]!=last_values[i];
 if(event==3&&!changed&&header[12]>8&&!values[0]&&!values[1])return;
 if(row_count==256)fail(93);u32* r=rows[row_count++];r[0]=row_count;r[1]=event;r[2]=tid;r[3]=c?c->Eip:0;r[4]=hit;
 for(u32 i=0;i<5;++i){r[5+i]=values[i];last_values[i]=values[i];}have_values=1;
 if(c){r[10]=c->Esp;r[11]=c->Ecx;r[12]=c->Edx;r[13]=c->Eax;}
 r[14]=header[12];r[15]=header[13];header[16]=row_count;save();
}
static CONTEXT get_context(HANDLE thread){CONTEXT c={0};c.ContextFlags=CONTEXT_CONTROL|CONTEXT_INTEGER|CONTEXT_DEBUG_REGISTERS;
 if(!GetThreadContext(thread,&c))fail(94);return c;}
static void set_debug(HANDLE thread,CONTEXT* c){c->Dr6=0;c->EFlags|=0x10000;
 /* Preserve all integer/control state except the debugger resume flag. */
 c->ContextFlags=CONTEXT_CONTROL|CONTEXT_DEBUG_REGISTERS;if(!SetThreadContext(thread,c))fail(95);}
static HANDLE find_thread(u32 tid){for(u32 i=0;i<256;++i)if(threads[i].id==tid)return threads[i].handle;fail(96);return 0;}
static void add_thread(u32 tid,HANDLE thread){
 for(u32 i=0;i<256;++i)if(!threads[i].id){threads[i].id=tid;threads[i].handle=thread;CONTEXT c=get_context(thread);
  if((c.Dr7&255)&&!(c.Dr0==0x6e1f68&&c.Dr1==0x6e1f6c&&c.Dr2==0x4fd438&&c.Dr3==0x58acf0&&c.Dr7==0xdd0055))fail(97);
  c.Dr0=0x6e1f68;c.Dr1=0x6e1f6c;c.Dr2=0x4fd438;c.Dr3=0x58acf0;c.Dr7=0xdd0055;set_debug(thread,&c);++header[9];return;
 }
 fail(98);
}
static void observe_error(void){
 if(header[34])return;void* dialog=FindWindowA(0,"DirectDraw Error");u32 pid=0;
 if(!dialog||!GetWindowThreadProcessId(dialog,&pid)||pid!=header[6])return;
 u32 used=0;for(void* child=GetWindow(dialog,5);child;child=GetWindow(child,2)){
  char text[256];int n=GetWindowTextA(child,text,sizeof(text));if(n<=0)continue;
  for(int i=0;i<n&&used<254;++i)error_text[used++]=text[i];if(used<254)error_text[used++]='\n';
 }error_text[used]=0;header[33]=(u32)dialog;header[34]=1;header[35]=used;
 read_child(0x6e1f78,header+36,4);read_child(0x6e1f70,header+37,8);read_child(0x6de6d5,header+23,4);read_child(0x6a49f4,header+24,8);save();
}
static void check_code(void){u8 bytes[sizeof(alternate_canvas_pinned)];read_child(0x4fd438,bytes,sizeof(bytes));
 for(u32 i=0;i<sizeof(bytes);++i)if(bytes[i]!=alternate_canvas_pinned[i])fail(99);++header[17];}
void mainCRTStartup(void){
 output=CreateFileA("alternate-watch.bin",GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);if(output==INVALID_HANDLE_VALUE)ExitProcess(80);save();
 char* command=GetCommandLineA();if(*command=='"'){++command;while(*command&&*command!='"')++command;if(*command)++command;}else while(*command&&*command!=' '&&*command!='\t')++command;
 while(*command==' '||*command=='\t')++command;if(!*command)fail(81);
 STARTUPINFOA startup={0};PROCESS_INFORMATION child={0};startup.cb=sizeof(startup);
 char directory[1024];u32 length=GetEnvironmentVariableA("MNM_ALTERNATE_CWD",directory,sizeof(directory));if(length>=sizeof(directory))fail(102);
 if(!CreateProcessA(0,command,0,0,0,DEBUG_ONLY_THIS_PROCESS,0,length?directory:0,&startup,&child))fail(82);
 process=child.hProcess;header[6]=child.dwProcessId;header[7]=child.dwThreadId;save();u32 stopping=0,events=0;
 for(;;){DEBUG_EVENT e;
  if(!stopping&&GetFileAttributesA("alternate-stop.flag")!=0xffffffffu){emit(5,0,0,0);stopping=1;header[18]=1;if(!TerminateProcess(process,238))fail(84);}
  if(!WaitForDebugEvent(&e,100)){
   if(GetLastError()!=121)fail(83);
   observe_error();
   if(!stopping){HANDLE t=find_thread(child.dwThreadId);u32 previous=SuspendThread(t);if(previous==0xffffffffu)fail(104);
    CONTEXT c=get_context(t);header[28]=c.Eip;header[29]=c.Esp;
    u32 done=0;ReadProcessMemory(process,(void*)c.Esp,header+30,8,&done);
    if(ResumeThread(t)!=previous+1)fail(105);++header[32];save();}
   if(!stopping&&GetFileAttributesA("alternate-stop.flag")!=0xffffffffu){emit(5,0,0,0);stopping=1;header[18]=1;if(!TerminateProcess(process,238))fail(84);}
   continue;
  }
  if(e.dwProcessId!=child.dwProcessId||++events>200000)fail(85);u32 status=DBG_CONTINUE;
  if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT){
   header[8]=(u32)e.u.CreateProcessInfo.lpBaseOfImage;if(header[8]!=0x400000)fail(86);
   if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);
   if(e.u.CreateProcessInfo.hThread!=child.hThread)CloseHandle(e.u.CreateProcessInfo.hThread);
   add_thread(e.dwThreadId,child.hThread);check_code();emit(1,e.dwThreadId,0,0);
  }else if(e.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT)add_thread(e.dwThreadId,e.u.CreateThread.hThread);
  else if(e.dwDebugEventCode==4){for(u32 i=0;i<256;++i)if(threads[i].id==e.dwThreadId){CloseHandle(threads[i].handle);threads[i].id=0;++header[10];break;}}
  else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT){if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
  else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT){u32 code=e.u.Exception.ExceptionRecord.ExceptionCode;
   if(code==EXCEPTION_SINGLE_STEP){HANDLE thread=find_thread(e.dwThreadId);CONTEXT c=get_context(thread);u32 hits=c.Dr6&15;
    if(!hits)fail(87);
    if(hits&3){++header[11];if(hits&1)++header[14];if(hits&2)++header[15];emit(2,e.dwThreadId,&c,hits);}
    if(hits&4){if(c.Eip!=0x4fd438)fail(88);++header[12];if(alternate_canvas_pinned[0]&&c.Ebx)fail(103);read_child(0x6de6d5,header+23,4);read_child(0x6a49f4,header+24,8);emit(3,e.dwThreadId,&c,hits);}
    if(hits&8){if(c.Eip!=0x58acf0)fail(89);++header[13];emit(4,e.dwThreadId,&c,hits);}
    set_debug(thread,&c);save();
   }else if(code==EXCEPTION_BREAKPOINT&&e.u.Exception.dwFirstChance&&!header[19]++){}
   else{++header[20];status=DBG_EXCEPTION_NOT_HANDLED;if(!e.u.Exception.dwFirstChance)fail(90);}
  }else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){header[21]=e.u.ExitProcess.dwExitCode;
   header[4]=!header[5]&&(!header[21]||(stopping&&header[21]==238));header[22]=events;save();
   if(!ContinueDebugEvent(e.dwProcessId,e.dwThreadId,status))fail(100);
   for(u32 i=0;i<256;++i)if(threads[i].id)CloseHandle(threads[i].handle);CloseHandle(process);CloseHandle(output);ExitProcess(header[4]?0:101);
  }
  if(!ContinueDebugEvent(e.dwProcessId,e.dwThreadId,status))fail(100);
 }
}
