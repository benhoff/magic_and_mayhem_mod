/* Recorder policy: closed bounded session inputs, never destination planes. */
static int session_producers_configure(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_CANVAS_SESSION_PRODUCERS",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!producer_trace_mode||whole_update_capture)return 0;
 session_producers=1;return 1;
}
void canvas_producers_session_finish(void){
 if(!session_producers||stopped||!file)return;
 /* Quiescent interval end is corroborated by the collector's normal Quit. */
 if(failures||raster_world_open||queue>limit)return;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)return;
 if(!journal_flush())return;
 u32 h[16]={0};copy(h,"MNMPCS01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=sequence;h[6]=queue;h[7]=bytes;h[8]=oracle;h[9]=GetCurrentThreadId();h[10]=1;h[11]=limit;
 char name[260];path(name,"canvas-session-",0,".done");HANDLE out=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(out==(HANDLE)-1)return;if(!write(out,h,64)||!CloseHandle(out))ExitProcess(95);
}

API u32 WIN GetFileAttributesA(const char*);
static void session_producers_poll(void){
 if(!session_producers||stopped||!file||raster_world_open||failures)return;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)return;
 char name[260];path(name,"canvas-session-",0,".close");
 if(GetFileAttributesA(name)==0xffffffffu)return;
 canvas_producers_session_finish();canvas_producers_close();
}
