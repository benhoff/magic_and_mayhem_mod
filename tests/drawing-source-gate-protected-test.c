#include "../runtime/shadow/win32_min.h"
API unsigned WIN SetEnvironmentVariableA(const char*,const char*);
#define MNM_DRAWING_SELFTEST 1
#define DllMain unused_drawing_main
#include "../runtime/drawing/session.c"
#undef DllMain
void* memcpy(void* out,const void* in,unsigned bytes){copy(out,in,bytes);return out;}
static unsigned starts,ends,refuse,lease_calls,held_calls;
static unsigned mock_protection_active,snapshots;
static unsigned WIN gate_start(unsigned owner,void* output,unsigned bytes){
 unsigned* p=output;if(bytes!=128||owner!=GetCurrentThreadId())ExitProcess(201);
 p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=1;p[4]=refuse?3:0;p[5]=refuse?0:7;p[6]=owner;p[7]=1;p[8]=2;p[13]=refuse?0:3;++starts;return refuse?0:7;
}
static unsigned WIN gate_stop(unsigned owner,unsigned token,void* output,unsigned bytes){
 unsigned* p=output;if(bytes!=128||owner!=GetCurrentThreadId()||token!=7||mock_protection_active||source_worker_lease.api)ExitProcess(202);
 p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=2;p[5]=7;p[6]=owner;p[7]=1;p[8]=2;p[11]=p[12]=1;++ends;return 1;
}
static void check(int ok,unsigned code){if(!ok)ExitProcess(code);}
static unsigned WIN mock_source_lease(void* source,unsigned lease,unsigned* metadata,unsigned bytes){
 (void)source;check(bytes==64&&starts==1&&!ends,220);++lease_calls;
 for(unsigned i=0;i<16;++i)metadata[i]=0;metadata[0]=64;metadata[1]=1;metadata[2]=19;metadata[6]=61;metadata[7]=800;metadata[8]=600;metadata[9]=800;
 if(!lease){mock_protection_active=1;return 0x80000009;}check(lease==9&&mock_protection_active,221);mock_protection_active=0;return 0;
}
static unsigned WIN mock_source_bytes(void* source,unsigned* before,unsigned* after,unsigned char* pixels,unsigned capacity){
 (void)source;check(capacity==960000&&starts==1&&!ends,222);++held_calls;
 for(unsigned i=0;i<16;++i)before[i]=after[i]=0;
 before[0]=after[0]=64;before[1]=after[1]=1;before[2]=after[2]=19;before[6]=after[6]=61;before[7]=after[7]=800;before[8]=after[8]=600;before[9]=after[9]=800;
 for(unsigned i=0;i<capacity;++i)pixels[i]=(unsigned char)i;return 0;
}
static unsigned WIN snapshot(unsigned owner,void* output,unsigned bytes){
 check(owner==GetCurrentThreadId()&&bytes==128&&!mock_protection_active&&!source_worker_lease.api,230);++snapshots;memset(output,0,128);
 unsigned* p=output;p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=3;
 u16 control=0x017f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0xdeadbeef);return 1;
}
void start(void){
 check(SetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE",0)&&initialize(),203);
 check(SetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE","1"),209);source_lease_enabled=source_worker_lease_enabled=source_held_enabled=1;
 check(drawing_cursor_gate_open(),204);source_lease_enabled=source_worker_lease_enabled=source_held_enabled=0;drawing_cursor_gate_begin=gate_start;drawing_cursor_gate_end=gate_stop;
 source_lease_getter=mock_source_lease;source_bytes_getter=mock_source_bytes;
 char name[600];path(name,"api-source-lease.bin");source_lease_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(source_lease_file!=(HANDLE)-1,223);
 path(name,"api-source-worker-lease.bin");source_worker_lease_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(source_worker_lease_file!=(HANDLE)-1,224);
 path(name,"source-held-snapshot.bin");source_held_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(source_held_file!=(HANDLE)-1,225);
 source_lease_enabled=source_worker_lease_enabled=source_held_enabled=1;
 unsigned rectangle[4]={0,0,800,600};unsigned args[6]={0,0,0,(unsigned)rectangle,16,0};SetLastError(0xabcdef01);
 struct Context c=begin(0,7,args,5,0x4a1000,1);
 check(c.observed&&c.cursor_gate_token==7&&starts==1&&GetLastError()==0xabcdef01,205);
 check(mock_protection_active&&c.source_lease==9,226);
 source_worker_lease_packet(1,100,c.r[29],0);source_worker_lease_packet(2,100,c.r[29],200);source_worker_lease_packet(3,100,c.r[29],200);
 readbacks_enabled=0;SetLastError(0xdef01234);
 check(end(&c,0,0x80004005)==0x80004005&&GetLastError()==0xdef01234&&ends==1&&!c.cursor_gate_token&&!mock_protection_active&&lease_calls==2&&held_calls==2,206);
 refuse=1;SetLastError(0xabcdef01);c=begin(0,7,args,5,0x4a1000,1);
 check(c.observed&&!c.cursor_gate_token&&c.cursor_gate_reason==3&&starts==2,207);
 check(end(&c,0,0x80004005)==0x80004005&&ends==1,208);
 drawing_cursor_gate_snapshot_callback=snapshot;SetLastError(0x12345678);u16 control=0x027f;__asm__ volatile("fldcw %0"::"m"(control));
 DrawingActionV1(1,3,3,1,0,1,0);u16 actual;__asm__ volatile("fnstcw %0":"=m"(actual));check(snapshots==1&&GetLastError()==0x12345678&&actual==control,231);
 finish(1);ExitProcess(0);
}
