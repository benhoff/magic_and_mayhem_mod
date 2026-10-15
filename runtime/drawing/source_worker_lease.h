API void WIN Sleep(u32);
/* Diagnostic only. Packet callbacks run with journal_lock held. */
static u32 source_worker_lease_enabled,source_worker_lease_records;
static HANDLE source_worker_lease_file;
static struct {u32 api,lease,owner,count,completed,unknown;u32 commands[2048],workers[2048],returned[2048];} source_worker_lease;
static void source_worker_lease_emit(u32 phase,u32 command,u32 worker){
 u32 r[16]={0};r[0]=++source_worker_lease_records;r[1]=phase;r[2]=source_worker_lease.api;r[3]=source_worker_lease.lease;r[4]=GetCurrentThreadId();r[5]=GetTickCount();r[6]=command;r[7]=worker;r[8]=source_worker_lease.count;r[9]=source_worker_lease.completed;r[10]=source_worker_lease.unknown;
 if(source_worker_lease_records>16384||!write(source_worker_lease_file,r,64))ExitProcess(99);
}
static int source_worker_lease_mode(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_WORKER_LEASE",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!wine_blit_packet_enabled)return 0;
 char name[600];path(name,"api-source-worker-lease.bin");source_worker_lease_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(source_worker_lease_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMSWL01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=16384;h[6]=2048;h[7]=5000;if(!write(source_worker_lease_file,h,64))return 0;source_worker_lease_enabled=1;return 1;
}
static void source_worker_lease_begin(struct Context* c){
 if(!source_worker_lease_enabled)return;if(source_worker_lease.api)ExitProcess(99);memset(&source_worker_lease,0,sizeof(source_worker_lease));source_worker_lease.api=c->r[29];source_worker_lease.lease=c->source_lease;source_worker_lease.owner=GetCurrentThreadId();source_worker_lease_emit(1,0,0);
}
static void source_worker_lease_packet(u32 phase,u32 command,u32 api,u32 worker){
 if(!source_worker_lease_enabled||!source_worker_lease.api)return;
 if(!command){++source_worker_lease.unknown;return;}
 if(phase==1){if(api!=source_worker_lease.api)return;u32 n=source_worker_lease.count;if(n==2048)ExitProcess(99);for(u32 i=0;i<n;++i)if(source_worker_lease.commands[i]==command)ExitProcess(99);source_worker_lease.commands[n]=command;++source_worker_lease.count;source_worker_lease_emit(2,command,0);return;}
 for(u32 i=0;i<source_worker_lease.count;++i)if(source_worker_lease.commands[i]==command){
  if(phase==2){if(!worker||source_worker_lease.workers[i]||source_worker_lease.returned[i])ExitProcess(99);source_worker_lease.workers[i]=worker;}
  else if(phase==3){if(!worker||source_worker_lease.workers[i]!=worker||source_worker_lease.returned[i])ExitProcess(99);source_worker_lease.returned[i]=1;++source_worker_lease.completed;source_worker_lease_emit(3,command,worker);}else ExitProcess(99);return;
 }
}
static void source_worker_lease_wait(struct Context* c){
 if(!source_worker_lease_enabled||!c->source_lease)return;u32 start=GetTickCount();
 for(;;){EnterCriticalSection(journal_lock);
  if(source_worker_lease.api!=c->r[29]||source_worker_lease.lease!=c->source_lease||source_worker_lease.owner!=GetCurrentThreadId()||source_worker_lease.unknown||!source_worker_lease.count)ExitProcess(99);
  if(source_worker_lease.count==source_worker_lease.completed){source_worker_lease_emit(4,0,0);source_worker_lease.api=0;LeaveCriticalSection(journal_lock);return;}
  LeaveCriticalSection(journal_lock);if(GetTickCount()-start>=5000)ExitProcess(99);Sleep(1);
 }
}
static void source_worker_lease_close(u32 reason){if(!source_worker_lease_file)return;if(source_worker_lease.api)ExitProcess(99);u32 r[16]={0};r[0]=++source_worker_lease_records;r[1]=5;r[2]=reason;if(!write(source_worker_lease_file,r,64)||!CloseHandle(source_worker_lease_file))ExitProcess(99);source_worker_lease_file=0;}
