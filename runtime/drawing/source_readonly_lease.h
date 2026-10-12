/* APIintervalonlydiagnostic;returnisnotasyncEGLcompletion. */
typedef u32(WIN* SourceLease)(void*,u32,u32*,u32);
static SourceLease source_lease_getter;
static HANDLE source_lease_file;
static u32 source_lease_enabled,source_lease_records;
static int source_lease_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_LEASE",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!source_bytes_enabled)return 0;
 char name[600];path(name,"api-source-lease.bin");source_lease_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(source_lease_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMSLS01",8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=4096;if(!write(source_lease_file,h,64))return 0;source_lease_enabled=1;return 1;
}
static void source_lease_capture(struct Context* c,u32 phase){
 if(!source_lease_enabled||!c->observed||c->r[1]!=1||c->r[2]!=7||c->args[4]!=16)return;
 if(!source_lease_getter){void* scene=GetModuleHandleA("MnmScene.dll");source_lease_getter=scene?(SourceLease)GetProcAddress(scene,"SceneDrawingSourceLeaseV1"):0;if(!source_lease_getter)ExitProcess(99);}
 u32 r[32]={0};r[1]=phase;r[2]=c->r[29];r[3]=c->r[6];r[4]=GetTickCount();r[5]=c->r[4];
 u32 result=phase==1?source_lease_getter((void*)c->args[2],0,r+16,64):c->source_lease?source_lease_getter((void*)c->args[2],c->source_lease,r+16,64):2;
 r[6]=result;if(phase==1&&result&0x80000000u)c->source_lease=result&0x7fffffffu;r[7]=c->source_lease;
 if(phase==2&&c->source_lease&&result)ExitProcess(99);
 EnterCriticalSection(journal_lock);r[0]=++source_lease_records;if(source_lease_records>4096||!write(source_lease_file,r,128))ExitProcess(99);LeaveCriticalSection(journal_lock);
}
static void source_lease_close(u32 reason){if(!source_lease_file)return;u32 r[32]={0};r[0]=++source_lease_records;r[1]=4;r[2]=reason;if(!write(source_lease_file,r,128)||!CloseHandle(source_lease_file))ExitProcess(99);source_lease_file=0;}
