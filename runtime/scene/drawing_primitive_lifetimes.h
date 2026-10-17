/* Opt-in observations. The recorder lock never owns original source bytes.
 * QPC bounds bracket observer snapshots and original execution conservatively. */
API u32 WIN QueryPerformanceCounter(void*);
API u32 WIN QueryPerformanceFrequency(void*);
#define MNM_PRIMITIVE_LIFETIME_MAX_ROWS 2097152u
static HANDLE source_windows_file;
static HANDLE primitive_lifetimes_file;
static u32 primitive_lifetime_rows,primitive_lifetime_next,primitive_lifetime_active;
static u8* primitive_lifetime_buffer;
static u32 primitive_lifetime_used;
static void primitive_lifetimes_flush(void){
 if(primitive_lifetime_used){if(!write(primitive_lifetimes_file,primitive_lifetime_buffer,primitive_lifetime_used))ExitProcess(99);primitive_lifetime_used=0;}
}
static int primitive_lifetimes_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_PRIMITIVE_LIFETIMES",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!producer_trace_mode||!whole_update_capture||!source_windows_file)return 0;
 u32 frequency[2];if(!QueryPerformanceFrequency(frequency)||(!frequency[0]&&!frequency[1]))return 0;
 primitive_lifetime_buffer=HeapAlloc(producer_heap(),0,65536);if(!primitive_lifetime_buffer)return 0;
 char name[260];path(name,"primitive-lifetimes-",0,".bin");primitive_lifetimes_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(primitive_lifetimes_file==(HANDLE)-1){primitive_lifetimes_file=0;return 0;}
 u32 header[16]={0};copy(header,"MNMPLIF1",8);header[2]=1;header[3]=256;header[4]=0x40209ca7;header[5]=MNM_PRIMITIVE_LIFETIME_MAX_ROWS;header[6]=frequency[0];header[7]=frequency[1];
 return write(primitive_lifetimes_file,header,64);
}
static void primitive_lifetime_record(struct Frame* f,u32 phase,u32 result,u32 cadence,u32 accepted){
 if(!primitive_lifetimes_file)return;
 u32 error=GetLastError(),row[64];
 /* Volatile stores keep this freestanding PE32 recorder independent of CRT memset. */
 for(u32 i=0;i<64;++i)((volatile u32*)row)[i]=0;
 if(primitive_lifetime_rows>=MNM_PRIMITIVE_LIFETIME_MAX_ROWS-1||!QueryPerformanceCounter(row+6))ExitProcess(99);
 row[0]=++primitive_lifetime_rows;row[1]=phase;row[8]=sequence;row[9]=queue;row[13]=primitive_lifetime_active;row[48]=result;row[49]=cadence;row[50]=accepted;
 if(f){
  row[2]=f->lifetime_id;row[3]=f->owner;row[4]=f->tag;row[5]=f->caller;row[10]=f->lifetime_parent;row[11]=f->primitive;
  row[12]=*producer_trace_depth(f->owner);row[16]=f->ecx;row[17]=f->edx;copy(row+18,f->args,24);
  if(f->record){copy(row+24,f->record,96);row[59]=1;}
  struct Surface* surface=object(f->ecx);
  if(surface){row[51]=surface->object;row[52]=surface->id;row[53]=surface->pixels;row[54]=surface->width;row[55]=surface->height;row[56]=surface->stride;row[57]=surface->dirty;row[58]=surface->sampled;}
 }else row[3]=GetCurrentThreadId();
 if(primitive_lifetime_used==65536)primitive_lifetimes_flush();
 copy(primitive_lifetime_buffer+primitive_lifetime_used,row,256);primitive_lifetime_used+=256;SetLastError(error);
}
static void primitive_lifetime_enter(struct Frame* f){
 if(!primitive_lifetimes_file)return;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp&&frames[i].owner==f->owner&&frames[i].lifetime_id>f->lifetime_parent)f->lifetime_parent=frames[i].lifetime_id;
 if(primitive_lifetime_next==~0u)ExitProcess(99);f->lifetime_id=++primitive_lifetime_next;++primitive_lifetime_active;
 primitive_lifetime_record(f,1,0,0,0);
}
static void primitive_lifetimes_boundary(u32 cadence,u32 accepted,u32 pending){
 if(!primitive_lifetimes_file)return;
 if(pending!=primitive_lifetime_active)ExitProcess(99);
 primitive_lifetime_record(0,5,0,cadence,accepted);
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)primitive_lifetime_record(frames+i,6,0,cadence,accepted);
 primitive_lifetimes_flush();
}
static void primitive_lifetime_retire(struct Frame* f,u32 result){
 if(!primitive_lifetimes_file)return;
 if(!f->lifetime_id||!primitive_lifetime_active)ExitProcess(99);--primitive_lifetime_active;primitive_lifetime_record(f,4,result,0,0);
}
static void primitive_lifetimes_close(void){
 if(!primitive_lifetimes_file)return;u32 error=GetLastError();
 primitive_lifetime_record(0,7,0,0,0);primitive_lifetimes_flush();
 if(!CloseHandle(primitive_lifetimes_file))ExitProcess(99);primitive_lifetimes_file=0;
 HeapFree(producer_heap(),0,primitive_lifetime_buffer);primitive_lifetime_buffer=0;SetLastError(error);
}
