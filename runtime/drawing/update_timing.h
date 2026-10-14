/* Opt-in QPC observation of every admitted surface/input/action call. This
 * timestamps observation boundaries, not exclusive source or screen ownership. */
API int WIN QueryPerformanceCounter(void*);
API int WIN QueryPerformanceFrequency(void*);
static HANDLE update_timing_file;
static u32 update_timing_enabled,update_timing_records,update_timing_frequency[2];
static int update_timing_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_UPDATE_ROUTES",flag,2);
 if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1')return 0;
 if(!QueryPerformanceFrequency(update_timing_frequency)||(!update_timing_frequency[0]&&!update_timing_frequency[1]))return 0;
 char name[600];path(name,"update-call-timing.bin");update_timing_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(update_timing_file==(HANDLE)-1){update_timing_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMUTIM1",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=786433;h[6]=update_timing_frequency[0];h[7]=update_timing_frequency[1];
 if(!write(update_timing_file,h,64))return 0;update_timing_enabled=1;return 1;
}
static void update_timing_capture(const struct Context* c,u32 phase){
 if(!update_timing_enabled||!c->observed)return;
 u32 r[16]={0};r[1]=phase;r[2]=c->r[29];r[3]=c->r[6];r[4]=c->r[1];r[5]=c->r[2];
 if(!QueryPerformanceCounter(r+6))ExitProcess(99);
 if(phase>=2){r[8]=c->r[8];r[9]=c->r[9];r[10]=c->r[21];}
 if(phase==3){r[11]=c->r[0];r[12]=c->r[12];}
 EnterCriticalSection(journal_lock);r[0]=++update_timing_records;
 if(!update_timing_file||update_timing_records>=786433||!write(update_timing_file,r,64))ExitProcess(99);
 LeaveCriticalSection(journal_lock);
}
/* Caller holds journal_lock after all admitted entries complete. */
static void update_timing_close(u32 reason){
 if(!update_timing_file)return;u32 r[16]={0};r[0]=++update_timing_records;r[1]=4;r[2]=reason;r[3]=records;r[4]=pending;r[5]=planes;r[8]=drops;r[9]=gaps;
 if(!QueryPerformanceCounter(r+6)||update_timing_records>786433||!write(update_timing_file,r,64)||!CloseHandle(update_timing_file))ExitProcess(99);
 update_timing_file=0;update_timing_enabled=0;
}
