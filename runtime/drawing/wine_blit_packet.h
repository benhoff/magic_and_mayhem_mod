/* Exact local packet identity only; no source pixel/storage ownership claim. */
static u32 wine_blit_packet_enabled,wine_blit_packet_records,wine_blit_packet_commands;
static HANDLE wine_blit_packet_file;
static struct {const void* pointer;u32 command,submission,api_entry,words[25];} wine_blit_packets[2048];
void* wine_blit_packet_resume;
extern void wine_blit_packet_entry(void);
static void wine_blit_packet_emit(u32* r){r[0]=++wine_blit_packet_records;if(wine_blit_packet_records>131072||!write(wine_blit_packet_file,r,64))ExitProcess(99);}
void wine_blit_packet_observe(u32* registers){
 u32 error=GetLastError();if(wine_blit_packet_enabled&&wine_submission_enabled){const u32* packet=(void*)registers[7];EnterCriticalSection(journal_lock);
  if(!readable(packet,100)||packet[0]!=42)ExitProcess(99);u32 slot=wine_submission_slot(),depth=wine_submission_parents[slot].submission_depth,free=2048;
  for(u32 i=0;i<2048;++i){if(wine_blit_packets[i].pointer==packet)ExitProcess(99);if(!wine_blit_packets[i].pointer&&free==2048)free=i;}if(free==2048)ExitProcess(99);
  wine_blit_packets[free].pointer=packet;wine_blit_packets[free].command=++wine_blit_packet_commands;wine_blit_packets[free].api_entry=wine_submission_parents[slot].depth?wine_submission_parents[slot].entry[wine_submission_parents[slot].depth-1]:0;wine_blit_packets[free].submission=depth?wine_submission_parents[slot].submission[depth-1]:0;copy(wine_blit_packets[free].words,packet,100);
  u32 r[16]={0};r[1]=1;r[2]=wine_blit_packets[free].command;r[3]=wine_blit_packets[free].submission;r[4]=GetCurrentThreadId();r[5]=GetTickCount();r[6]=wine_lifetime_records;wine_blit_packet_emit(r);wine_source_version_packet(r[3],r[2]);source_worker_lease_packet(1,r[2],wine_blit_packets[free].api_entry,0);LeaveCriticalSection(journal_lock);
 }SetLastError(error);
}
static void wine_blit_packet_worker(const void* packet,u32 worker,u32 phase){
 if(!wine_blit_packet_enabled)return;u32 r[16]={0};r[1]=phase;r[4]=GetCurrentThreadId();r[5]=GetTickCount();r[7]=worker;
 for(u32 i=0;i<2048;++i)if(wine_blit_packets[i].pointer==packet){if(!equal(wine_blit_packets[i].words,packet,100))ExitProcess(99);r[2]=wine_blit_packets[i].command;r[3]=wine_blit_packets[i].submission;wine_source_version_worker(packet,r[3],r[2],worker,phase);if(phase==3)wine_blit_packets[i].pointer=0;break;}
 if(!r[2]){r[8]=1;if(wine_source_version_enabled)wine_source_version_worker(packet,0,0,worker,phase);}wine_blit_packet_emit(r);source_worker_lease_packet(phase,r[2],0,worker);
}
static int wine_blit_packet_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_BLIT_PACKET",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!wine_blit_worker_enabled)return 0;
 char name[600];path(name,"wine-blit-packet.bin");wine_blit_packet_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_blit_packet_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMWPK01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;if(!write(wine_blit_packet_file,h,64))return 0;wine_blit_packet_enabled=1;return 1;
}
static int wine_blit_packet_install_site(u8* site){
 if(!wine_blit_packet_enabled)return 1;const u8 expected[6]={0x8b,0x03,0x89,0x74,0x24,0x04};if(!readable(site,6)||!equal(site,expected,6))return 0;u32 old,unused;if(!VirtualProtect(site,6,0x40,&old))return 0;wine_blit_packet_resume=site+6;u8 patch[6]={0xe9,0,0,0,0,0x90};u32 delta=(u32)&wine_blit_packet_entry-(u32)site-5;copy(patch+1,&delta,4);copy(site,patch,6);int okay=FlushInstructionCache(GetCurrentProcess(),site,6);if(!VirtualProtect(site,6,old,&unused)||!okay)return 0;return 1;
}
static void wine_blit_packet_close(u32 reason){if(!wine_blit_packet_file)return;u32 r[16]={0};r[1]=4;r[2]=reason;r[3]=wine_blit_packet_commands;r[4]=GetCurrentThreadId();r[5]=GetTickCount();for(u32 i=0;i<2048;++i)if(wine_blit_packets[i].pointer)++r[8];wine_blit_packet_emit(r);if(!CloseHandle(wine_blit_packet_file))ExitProcess(99);wine_blit_packet_file=0;}

static int wine_blit_packet_install(void* module){return wine_blit_packet_install_site((u8*)module+0x1dff8c);}
