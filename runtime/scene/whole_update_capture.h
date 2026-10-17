/* Recorder policy only: retain post-queue composition until next acceptance. */
static HANDLE source_windows_file;
static u32 source_windows_records;
static int source_windows_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_UPDATE_ROUTES",flag,2);
 if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1')return 0;
 char name[260];path(name,"source-update-windows-",0,".bin");source_windows_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(source_windows_file==(HANDLE)-1){source_windows_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMDWIN1",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=512;
 return write(source_windows_file,h,64);
}
static int source_windows_record(u32 record,u32 accepted,u32 counter,u32 mode){
 if(!source_windows_file)return 1;u32 r[16]={0};r[0]=++source_windows_records;r[1]=1;r[2]=record;r[3]=accepted;r[4]=counter;r[5]=mode;r[6]=GetCurrentThreadId();r[7]=sequence;r[8]=queue;r[9]=failures;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)++r[10];
 return source_windows_records<512&&write(source_windows_file,r,64);
}
static int source_windows_close(void){
 if(!source_windows_file)return 1;u32 r[16]={0};r[0]=++source_windows_records;r[1]=4;r[6]=GetCurrentThreadId();r[7]=sequence;r[8]=queue;r[9]=failures;
 int okay=source_windows_records<=512&&write(source_windows_file,r,64)&&CloseHandle(source_windows_file);source_windows_file=0;return okay;
}
static void whole_update_refusal(u32 reason,u32 record,u32 accepted,u32 counter,u32 mode,u32 frame){
 char name[260];path(name,"whole-update-refusal-",0,".bin");HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1)return;
 u32 r[32]={0};copy(r,"MNMWREF1",8);r[2]=reason;r[3]=record;r[4]=accepted;r[5]=counter;r[6]=mode;r[7]=GetCurrentThreadId();r[8]=queue;r[9]=limit;r[10]=update_point_active;r[11]=update_point_completed;r[12]=failures;r[13]=frame;
 if(frame<MAX_FRAMES){r[14]=frames[frame].tag;r[15]=frames[frame].owner;}
 copy(r+16,update_point_first,64);write(file,r,128);CloseHandle(file);
}
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
static int whole_update_wait_frames(u32 record,u32 accepted,u32 counter,u32 mode){
 u32 start=GetTickCount();for(;;){u32 active=MAX_FRAMES;EnterCriticalSection(producer_trace_mutex);
  for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){active=i;break;}
  if(active==MAX_FRAMES){LeaveCriticalSection(producer_trace_mutex);return 1;}
  if(frames[active].owner==GetCurrentThreadId()||GetTickCount()-start>=5000){whole_update_refusal(2,record,accepted,counter,mode,active);LeaveCriticalSection(producer_trace_mutex);return 0;}
  LeaveCriticalSection(producer_trace_mutex);Sleep(1);
 }
}
static int whole_update_configure(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_WHOLE_UPDATES",flag,2);if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!producer_trace_mode||GetEnvironmentVariableA("MNM_DRAW_UPDATE_BINDING",flag,2)!=1||flag[0]!='1')return 0;whole_update_capture=1;return 1;
}
u32 WIN SceneDrawingUpdateBoundaryV1(u32 record,u32 accepted,u32 counter,u32 mode){
 u32 error=GetLastError();if(!whole_update_capture||stopped){SetLastError(error);return 1;}if(queue>=limit&&!whole_update_wait_frames(record,accepted,counter,mode)){SetLastError(error);return 0;}PRODUCER_TRACE_SCOPE;
 if(!source_windows_record(record,accepted,counter,mode)){SetLastError(error);return 0;}
 if(queue<limit){SetLastError(error);return 1;}
 if(queue!=limit||update_point_active||update_point_completed!=limit||!update_point_first[2]||record<=update_point_first[2]||accepted!=update_point_first[4]+1||counter!=update_point_first[3]||GetCurrentThreadId()!=update_point_first[6]||mode>255||failures){whole_update_refusal(1,record,accepted,counter,mode,MAX_FRAMES);SetLastError(error);return 0;}
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){whole_update_refusal(2,record,accepted,counter,mode,i);SetLastError(error);return 0;}
 for(u32 i=0;i<MAX_SURFACES;++i)checkpoint(surfaces+i,5);if(stopped||failures){SetLastError(error);return 0;}
 u32 r[24]={0};r[2]=MNM_PRODUCER_UPDATE_END;r[14]=queue;r[15]=update_point_first[4];r[16]=counter;r[17]=accepted;r[19]=record;r[20]=mode;r[21]=GetCurrentThreadId();r[22]=3;emit(r,0,0);if(stopped||!journal_flush()){SetLastError(error);return 0;}
 if(!source_windows_close()){SetLastError(error);return 0;}canvas_producers_close();if(journal_failed){SetLastError(error);return 0;}char name[260];u32 n=0;while(directory[n]){name[n]=directory[n];++n;}const char* suffix="\\canvas-producers.done";while(*suffix)name[n++]=*suffix++;name[n]=0;
 HANDLE done=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(done==(HANDLE)-1){SetLastError(error);return 0;}u32 mark[8]={0};copy(mark,"MNMPDONE",8);mark[2]=1;mark[3]=queue;mark[4]=sequence;mark[5]=failures;mark[6]=oracle;mark[7]=bytes;int okay=write(done,mark,32)&&CloseHandle(done);SetLastError(error);return okay;
}
