/* Additive original-active metadata. Never transport COM/process pointers or
 * original destination bytes, and never hold journal_lock over the getter. */
static u32 producer_points_enabled,producer_point_records;
static HANDLE producer_point_file;
typedef u32(WIN *ProducerPoint)(void*,void*,u32*,u32);
static ProducerPoint producer_point;
static int producer_points_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW02_PRODUCER_BIND",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!physical_enabled)return 0;
 producer_points_enabled=1;char name[600];path(name,"api-producer-points.bin");
 producer_point_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(producer_point_file==(HANDLE)-1){producer_point_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMDSRC1",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=786433;
 return write(producer_point_file,h,64);
}
static void producer_points_capture(struct Context* c,void* object,u32 phase){
 if(!producer_points_enabled||!c->observed||c->r[1]!=1)return;
 if(phase<1||phase>3)ExitProcess(97);
 ProducerPoint getter=__atomic_load_n(&producer_point,__ATOMIC_ACQUIRE);
 if(!getter){void* scene=GetModuleHandleA("MnmScene.dll");
  getter=scene?(ProducerPoint)GetProcAddress(scene,"SceneDrawingSourcePairV1"):0;
  if(!getter)ExitProcess(97);
  __atomic_store_n(&producer_point,getter,__ATOMIC_RELEASE);
 }
 void* source=0;u32 method=c->r[2];
 if(method==5)source=(void*)c->args[1];else if(method==7)source=(void*)c->args[2];else if(method==11)source=(void*)c->args[0];
 u32 r[48]={0};r[1]=phase;r[2]=c->r[29];r[4]=method;r[5]=c->r[3];r[6]=c->interval_related;r[7]=c->r[6];r[8]=GetTickCount();
 if(phase>=2)r[9]=c->r[8];if(phase==3){r[3]=c->r[0];r[10]=c->r[12];}
 if(!getter(object,source,r+16,128))ExitProcess(97);
 for(u32 at=16;at<=32;at+=16){u32* p=r+at;
  if(p[0]!=64||p[1]!=1||!(p[2]&1)||p[2]&~31u||(p[2]&8&&p[2]&16)||p[14]!=r[7]||p[15]||p[12]>128||p[13]>128)ExitProcess(97);
  if(p[2]&16){if(!p[6])ExitProcess(97);}else for(u32 i=6;i<12;++i)if(p[i])ExitProcess(97);
 }
 for(u32 i=3;i<=5;++i)if(r[16+i]!=r[32+i])ExitProcess(97);
 for(u32 i=12;i<=14;++i)if(r[16+i]!=r[32+i])ExitProcess(97);
 EnterCriticalSection(journal_lock);r[0]=++producer_point_records;
 if(!producer_point_file||producer_point_records>=786433||!write(producer_point_file,r,192))ExitProcess(97);
 LeaveCriticalSection(journal_lock);
}
/* Caller owns journal_lock; all pending API entries must have completed. */
static void producer_points_close(u32 reason){
 if(!producer_point_file)return;u32 r[48]={0};r[0]=++producer_point_records;r[1]=4;r[2]=reason;r[3]=records;r[7]=GetCurrentThreadId();r[8]=GetTickCount();r[9]=pending;r[10]=planes;r[11]=drops;r[12]=gaps;
 if(producer_point_records>786433||!write(producer_point_file,r,192)||!CloseHandle(producer_point_file))ExitProcess(97);producer_point_file=0;
}
