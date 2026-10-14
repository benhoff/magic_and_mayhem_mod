/* External snapshots only, taken between accepted protection and restoration. */
typedef u32(WIN* SourceAdmission)(const u32*,u32);
static SourceAdmission source_admission_getter;
static u32 source_held_pending_copy;
static u32 source_held_enabled,source_held_records,source_held_planes,source_held_total;
static HANDLE source_held_file;
static u8* source_held_first;
static u32 source_held_lease,source_held_api,source_held_canvas;
static int source_held_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_HELD_SNAPSHOT",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!source_worker_lease_enabled)return 0;
 char name[600];path(name,"source-held-snapshot.bin");source_held_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(source_held_file==(HANDLE)-1)return 0;
 char pending_flag[2];u32 pending_n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_PENDING_COPY",pending_flag,2);if(pending_n&&(pending_n!=1||pending_flag[0]!='1'))return 0;source_held_pending_copy=pending_n!=0;
 u32 h[16]={0};copy(h,"MNMSHB01",8);h[2]=source_held_pending_copy?2:1;h[3]=192;h[4]=0x40209ca7;h[5]=2048;h[6]=1024;h[7]=1024u*1024u*1024u;if(!write(source_held_file,h,64))return 0;source_held_enabled=1;return 1;
}
/* One independent failure record; accepted journal format remains unchanged. */
static void source_held_refuse(struct Context* c,u32 phase,u32 reason,const u32* snapshot){
 u32 h[16]={0},r[48]={0};copy(h,"MNMSHR01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=1;h[6]=source_worker_lease.count;h[7]=source_worker_lease.completed;h[8]=source_held_records;h[9]=source_held_planes;h[10]=source_held_total;
 r[0]=1;r[1]=reason;r[2]=phase;r[3]=c->r[29];r[4]=c->source_lease;r[5]=GetCurrentThreadId();r[6]=GetTickCount();if(snapshot){r[7]=snapshot[7];copy(r+16,snapshot+16,128);}copy(r+8,c->args,24);
 /* The rectangle pointer becomes only an availability flag and owned scalars. */
 r[11]=0;if(readable((void*)c->args[3],16)){r[14]=1;copy(r+8,(void*)c->args[3],16);}else{r[8]=r[9]=r[10]=r[11]=0;}r[12]=c->args[0];r[13]=c->args[1];r[15]=c->args[4];
 char name[600];path(name,"source-held-refusal.bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f!=(HANDLE)-1){if(write(f,h,64))write(f,r,192);CloseHandle(f);}ExitProcess(99);
}
static void source_held_capture(struct Context* c,u32 phase){
 if(!source_held_enabled||!c->source_lease)return;
 if(!source_bytes_getter){void* scene=GetModuleHandleA("MnmScene.dll");source_bytes_getter=scene?(SourceBytes)GetProcAddress(scene,"SceneDrawingSourceBytesV1"):0;if(!source_bytes_getter)ExitProcess(99);}
 if(phase==1){if(source_held_first)source_held_refuse(c,phase,7,0);}else if(phase!=2||!source_held_first||source_held_lease!=c->source_lease||source_held_api!=c->r[29])source_held_refuse(c,phase,8,0);
 u8* pixels=VirtualAlloc(0,960000,0x3000,4);if(!pixels)source_held_refuse(c,phase,9,0);u32 r[48]={0};r[1]=phase;r[2]=c->r[29];r[3]=GetCurrentThreadId();r[4]=GetTickCount();r[5]=c->r[4];r[6]=c->r[3];r[10]=c->source_lease;
 r[7]=source_bytes_getter((void*)c->args[2],r+16,r+32,pixels,960000);
 if(r[7])source_held_refuse(c,phase,1,r);
 if(!equal(r+16,r+32,64)||r[16]!=64||r[17]!=1||r[18]!=19||!r[22]||r[23]!=800||r[24]!=600||r[25]!=800||r[21]||(!source_held_pending_copy&&r[28]>1)||r[29])source_held_refuse(c,phase,2,r);
 if(r[28]>1){
  if(!source_admission_getter){void* scene=GetModuleHandleA("MnmScene.dll");source_admission_getter=scene?(SourceAdmission)GetProcAddress(scene,"SceneDrawingSourceAdmissionV1"):0;}
  if(!source_held_pending_copy||!source_admission_getter||!(r[13]=source_admission_getter(r+16,64)))source_held_refuse(c,phase,11,r);
 }
 if(c->args[0]||c->args[1]||!readable((void*)c->args[3],16)||((u32*)c->args[3])[0]||((u32*)c->args[3])[1]||((u32*)c->args[3])[2]!=800||((u32*)c->args[3])[3]!=600)source_held_refuse(c,phase,3,r);
 if(phase==1){source_held_first=pixels;source_held_lease=c->source_lease;source_held_api=c->r[29];source_held_canvas=r[22];}
 else{if(source_held_canvas!=r[22])source_held_refuse(c,phase,4,r);if(!equal(source_held_first,pixels,960000))source_held_refuse(c,phase,5,r);if(source_worker_lease.count!=source_worker_lease.completed||!source_worker_lease.count)source_held_refuse(c,phase,6,r);}
 r[11]=source_worker_lease.count;r[12]=source_worker_lease.completed;
 if(source_held_planes>=1024||source_held_total>1024u*1024u*1024u-960000)source_held_refuse(c,phase,10,r);r[8]=++source_held_planes;r[9]=960000;source_held_total+=960000;
 char name[600],suffix[40];static const char prefix[]="source-held-oracle-000000.565";copy(suffix,prefix,sizeof(prefix));u32 number=r[8];for(u32 i=0;i<6;++i){suffix[sizeof(prefix)-6-i]=(char)('0'+number%10);number/=10;}path(name,suffix);
 HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f==(HANDLE)-1||!write(f,pixels,960000)||!CloseHandle(f))ExitProcess(99);
 r[0]=++source_held_records;if(source_held_records>2048||!write(source_held_file,r,192))ExitProcess(99);
 if(phase==2){if(!VirtualFree(pixels,0,0x8000)||!VirtualFree(source_held_first,0,0x8000))ExitProcess(99);source_held_first=0;}
}
static void source_held_close(u32 reason){if(!source_held_file)return;if(source_held_first)ExitProcess(99);u32 r[48]={0};r[0]=++source_held_records;r[1]=4;r[2]=reason;r[8]=source_held_planes;r[9]=source_held_total;if(!write(source_held_file,r,192)||!CloseHandle(source_held_file))ExitProcess(99);source_held_file=0;}
