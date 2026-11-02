/* Optional fixed mapped sideband. Only init/terminal cleanup perform file I/O.
 * These observations have no pixel, command, lease or admission consumer. */
#include "../../protocols/include/mnm/native_cpu_reader_journal_v1.h"
static struct MnmCpuReaderJournal* cpu_journal;
struct CpuJournalContext {u32 row[32],result;struct CpuJournalContext* previous;};
static struct CpuJournalContext* cpu_journal_context;
struct CpuJournalBorrow {u32 object,dc,context;};
static struct CpuJournalBorrow cpu_journal_borrows[64];
static int cpu_journal_env(const char* name){char b[2];return GetEnvironmentVariableA(name,b,2)==1 && b[0]=='1';}
static void cpu_journal_module(u32 pc,u32* row){
 u32 memory[7];for(u32 i=7;i<=13;++i)row[i]=0;row[7]=pc;row[9]=0xffffffffu;
 if(!pc || VirtualQuery((void*)pc,memory,28)!=28 || memory[4]!=0x1000 || memory[6]!=0x1000000)return;
 u32 base=memory[1];if(!readable((void*)base,64) || *(u16*)base!=0x5a4d)return;
 u32 pe=*(u32*)(base+0x3c);
 if(pe>4096 || base>0xffffffffu-pe-88 || !readable((void*)(base+pe),88) || *(u32*)(base+pe)!=0x4550)return;
 u32 extent=*(u32*)(base+pe+80),bytes=*(u32*)(base+pe+84);
 if(!extent || pc<base || pc-base>=extent || !bytes || bytes>4096 || !readable((void*)base,bytes))return;
 u32 hash=2166136261u;for(u32 i=0;i<bytes;++i)hash=(hash^((u8*)base)[i])*16777619u;
 row[8]=base;row[9]=pc-base;row[10]=extent;row[11]=*(u32*)(base+pe+8);row[12]=hash;row[13]=bytes;
}
static void cpu_journal_emit(u32 event,u32* row){
 if(!cpu_journal)return;u32 error=GetLastError();
 row[1]=event;row[2]=GetCurrentThreadId();row[3]=GetTickCount();row[4]=command_channel_session;
 mnm_crj_record(cpu_journal,row);SetLastError(error);
}
static struct CpuJournalContext* cpu_journal_current(void){
 struct CpuJournalContext* c=__atomic_load_n(&cpu_journal_context,__ATOMIC_ACQUIRE);
 return c && c->row[2]==GetCurrentThreadId()?c:0;
}
static void cpu_journal_enter(struct CpuJournalContext* c,u32 reason,u32 caller,void* object,u32 family,u32 route){
 if(!cpu_journal)return;u32 error=GetLastError();
 c->previous=cpu_journal_current();
 if(cpu_journal_context && !c->previous){__atomic_fetch_or(cpu_journal->header+9,8,__ATOMIC_RELAXED);SetLastError(error);return;}
 c->row[2]=GetCurrentThreadId();c->row[5]=++cpu_journal->header[22];c->row[6]=reason;
 c->row[14]=(u32)object;c->row[19]=family;c->row[20]=route;c->row[31]=c->previous?c->previous->row[5]:0;
 cpu_journal_module(caller,c->row);++cpu_journal->header[24];
 __atomic_store_n(&cpu_journal_context,c,__ATOMIC_RELEASE);cpu_journal_emit(MNM_CRJ_BEGIN,c->row);SetLastError(error);
}
static void cpu_journal_leave(struct CpuJournalContext* c){
 if(!c->row[5] || !cpu_journal)return;u32 error=GetLastError();
 if(cpu_journal_context!=c){__atomic_fetch_or(cpu_journal->header+9,8,__ATOMIC_RELAXED);SetLastError(error);return;}
 c->row[26]=c->result;cpu_journal_emit(MNM_CRJ_END,c->row);
 __atomic_store_n(&cpu_journal_context,c->previous,__ATOMIC_RELEASE);
 ++cpu_journal->header[19];--cpu_journal->header[24];SetLastError(error);
}
static void cpu_journal_plain(u32 event,void* object,void* dc,i32 result){
 if(!cpu_journal)return;u32 error=GetLastError();struct CpuJournalContext* c=cpu_journal_current();u32 r[32];zero(r,sizeof(r));
 if(c)copy(r,c->row,sizeof(r));r[14]=(u32)object;r[25]=(u32)dc;r[26]=(u32)result;
 cpu_journal_emit(event,r);SetLastError(error);
}
static void cpu_journal_init(void){
 u32 error=GetLastError();
 if(!mnm_crj_gate(cpu_journal_env("MNM_NATIVE_CPU_READER_JOURNAL"),cpu_journal_env("MNM_ACCEPTANCE_DIAGNOSTIC"),cpu_journal_env("MNM_ACCEPTANCE_PILOT")) || !lock_capture_path_length){SetLastError(error);return;}
 char path[544];static const char suffix[]="\\cpu-reader-journal-v1.bin";
 if(lock_capture_path_length>sizeof(path)-sizeof(suffix)){SetLastError(error);return;}
 copy(path,lock_capture_path,lock_capture_path_length);copy(path+lock_capture_path_length,suffix,sizeof(suffix));
 HANDLE file=CreateFileA(path,0xc0000000,1,0,1,0x80,0);
 if(file!=(HANDLE)-1){HANDLE map=CreateFileMappingA(file,0,4,0,MNM_CRJ_BYTES,0);CloseHandle(file);
  if(map){cpu_journal=MapViewOfFile(map,2,0,0,MNM_CRJ_BYTES);CloseHandle(map);}}
 if(cpu_journal){zero(cpu_journal,MNM_CRJ_BYTES);u32* h=cpu_journal->header;copy(h,"MNMCRJ01",8);
  h[2]=1;h[3]=128;h[4]=128;h[5]=MNM_CRJ_CAPACITY;h[11]=command_channel_session;h[16]=GetTickCount();h[25]=7;
  u32 module[32];zero(module,sizeof(module));cpu_journal_module((u32)GetModuleHandleA(0),module);h[12]=module[8];h[13]=module[10];
  cpu_journal_module((u32)GetModuleHandleA("MnmRender.dll"),module);h[14]=module[8];h[15]=module[10];
 }
 SetLastError(error);
}
static void cpu_journal_close(u32 complete){
 if(!cpu_journal)return;u32 error=GetLastError();
 cpu_journal->header[17]=GetTickCount();cpu_journal->header[18]=complete;
 if(cpu_journal_context || cpu_journal->header[23] || cpu_journal->header[24])__atomic_fetch_or(cpu_journal->header+9,16,__ATOMIC_RELAXED);
 __atomic_store_n(cpu_journal->header+10,1,__ATOMIC_RELEASE);
 UnmapViewOfFile(cpu_journal);cpu_journal=0;SetLastError(error);
}
static void cpu_journal_detach(void){if(cpu_journal){UnmapViewOfFile(cpu_journal);cpu_journal=0;}}
