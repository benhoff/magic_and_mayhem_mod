/* Public current-context GL sync, outside journal_lock; no all-context claim. */
static HANDLE wine_gpu_file;
static u32 wine_gpu_enabled,wine_gpu_records,wine_gpu_calls,wine_gpu_signaled,wine_gpu_unwitnessed,wine_gpu_failed;
static void* (WIN *wine_gpu_current)(void);
static void* (WIN *wine_gpu_proc)(const char*);
static void wine_gpu_emit(u32* r){r[0]=++wine_gpu_records;if(wine_gpu_records>131072||!write(wine_gpu_file,r,128))ExitProcess(99);}
static int wine_gpu_callable(void* p){u32 m[7];return (u32)p>3&&(u32)p!=0xffffffff&&VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(m[5]&0xf0);}
static void wine_gpu_complete(const void* packet,u32 worker){
 if(!wine_gpu_enabled)return;u32 r[32]={0};r[1]=1;r[2]=worker;r[5]=GetCurrentThreadId();r[6]=GetTickCount();r[16]=1000000000;
 EnterCriticalSection(journal_lock);++wine_gpu_calls;
 for(u32 i=0;i<2048;++i)if(wine_blit_packets[i].pointer==packet){if(!equal(wine_blit_packets[i].words,packet,100))ExitProcess(99);r[3]=wine_blit_packets[i].command;r[4]=wine_blit_packets[i].submission;break;}
 LeaveCriticalSection(journal_lock);
 void* context=wine_gpu_current();if(!context){r[8]=1;}else{
  EnterCriticalSection(journal_lock);r[9]=wine_submission_identity(context,4);LeaveCriticalSection(journal_lock);
  void* (WIN *fence)(u32,u32)=(void*)wine_gpu_proc("glFenceSync");
  u32 (WIN *wait)(void*,u32,unsigned long long)=(void*)wine_gpu_proc("glClientWaitSync");
  void (WIN *destroy)(void*)=(void*)wine_gpu_proc("glDeleteSync");
  if(wine_gpu_callable((void*)fence))r[12]|=1;if(wine_gpu_callable((void*)wait))r[12]|=2;if(wine_gpu_callable((void*)destroy))r[12]|=4;
  if(r[12]!=7)r[8]=3;else{
   void* sync=fence(0x9117,0);if(!sync)r[8]=4;else{
    r[13]=1;r[10]=wait(sync,1,1000000000ULL);destroy(sync);r[14]=1;
    r[8]=(r[10]==0x911a||r[10]==0x911c)?2:r[10]==0x911b?5:r[10]==0x911d?6:8;
   }
  }
  r[15]=wine_gpu_current()==context;if(!r[15])r[8]=7;
 }
 r[7]=GetTickCount();r[11]=r[7]-r[6];EnterCriticalSection(journal_lock);
 if(r[8]==2)++wine_gpu_signaled;else if(r[8]==1)++wine_gpu_unwitnessed;else ++wine_gpu_failed;wine_gpu_emit(r);LeaveCriticalSection(journal_lock);
 if(r[8]>2)ExitProcess(99);
}
static int wine_gpu_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_GPU_COMPLETION",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!wine_blit_packet_enabled)return 0;
 char name[600];path(name,"wine-gpu-completion.bin");wine_gpu_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_gpu_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWGC01",8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=131072;h[6]=1000000000;if(!write(wine_gpu_file,h,64))return 0;wine_gpu_enabled=1;return 1;
}
static int wine_gpu_install(void){
 if(!wine_gpu_enabled)return 1;void* module=GetModuleHandleA("opengl32.dll");if(!module)return 0;
 const u8 a[16]={0x64,0xa1,0x18,0,0,0,0x8b,0x80,0xec,0x0b,0,0,0xc3,0x90,0x90,0x90};
 const u8 b[16]={0x55,0x89,0xe5,0x83,0xec,0x28,0xe8,0xc5,0xba,0xf2,0xff,0x85,0xc0,0x0f,0x84,0xa4};
 void* current=GetProcAddress(module,"wglGetCurrentContext");void* proc=GetProcAddress(module,"wglGetProcAddress");
 if(current!=(void*)((u32)module+0xda210)||proc!=(void*)((u32)module+0xda900)||!readable(current,16)||!readable(proc,16)||!equal(current,a,16)||!equal(proc,b,16))return 0;
 wine_gpu_current=(void*)current;wine_gpu_proc=(void*)proc;return 1;
}
static void wine_gpu_close(u32 reason){if(!wine_gpu_file)return;u32 r[32]={0};r[1]=4;r[2]=reason;r[3]=wine_gpu_calls;r[8]=wine_gpu_failed;r[9]=wine_gpu_unwitnessed;r[10]=wine_gpu_signaled;wine_gpu_emit(r);if(!CloseHandle(wine_gpu_file))ExitProcess(99);wine_gpu_file=0;wine_gpu_enabled=0;}
