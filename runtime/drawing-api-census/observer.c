/* Separate opt-in CALL diagnostics. Original APIs execute exactly once through
 * relocated calls; the observer itself issues no graphics/COM/GDI operations. */
#include "../shadow/win32_min.h"
#include "exception_i386.h"
#include "../../protocols/include/mnm/drawing_api_journal_v1.h"
#include "../../protocols/include/mnm/drawing_api_journal_v2.h"
#include "../../protocols/include/mnm/drawing_api_bindings_v1.h"
#include "../../protocols/include/mnm/drawing_census_v1.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(const void*);
API void* WIN GetProcAddress(HANDLE,const char*);
#include "api-hooks.inc"
u32 api_trampolines[API_HOOKS],api_remainders[API_HOOKS];
static struct ApiFrame {u32 live,tag,id,phase,thread,sp,state,slot,target;
 u32 arguments_read,args[16],receiver,vtable,candidate;} frames[2048];
static HANDLE journal;
static const volatile u32* control;
static volatile u32 mutex;
static u32 sequence,serial,lost,active,io_failed,used,installed;
static u32 module_base;
static void* exception_handler;
static u8 buffer[65536];
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void zero(void* d,u32 n){volatile u8* a=d;while(n--)*a++=0;}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
static void lock(void){while(__sync_lock_test_and_set(&mutex,1))Sleep(0);}
static void unlock(void){__sync_lock_release(&mutex);}
static int write(const void* p,u32 n){u32 at=0;while(at<n){u32 done=0;if(!WriteFile(journal,(const u8*)p+at,n-at,&done,0)||!done||done>n-at){io_failed=1;return 0;}at+=done;}return 1;}
static int write_binding(HANDLE file,const void* p,u32 n){u32 at=0;while(at<n){u32 done=0;if(!WriteFile(file,(const u8*)p+at,n-at,&done,0)||!done||done>n-at)return 0;at+=done;}return 1;}
static void flush(void){if(used&&!io_failed)write(buffer,used);used=0;}
static u32 phase(void){
 for(u32 attempt=0;attempt<64;++attempt){u32 before=control[4],value=control[5],alive=control[6];__atomic_thread_fence(__ATOMIC_ACQUIRE);
  if(!(before&1)&&before==control[4]){if(value<=14&&alive==1)return value;break;}if((attempt&15)==15)Sleep(0);}
 ++lost;return 0;
}
static void target(struct ApiFrame* f,u32* regs){
 const struct ApiHook* h=api_hooks+f->tag;u32 value=h->reg==9?(u32)(regs+9):regs[h->reg];
 if(h->mode==1){f->state=MNM_DRAW_API_REGISTER;f->target=value;}
 else if(h->mode==4){f->state=MNM_DRAW_API_DIRECT;f->target=h->operand;}
 else {f->slot=h->mode==3?h->operand:value+h->operand;
  if(readable(f->slot,4)){f->state=MNM_DRAW_API_SLOT_READ;f->target=*(u32*)f->slot;}
  else f->state=MNM_DRAW_API_SLOT_UNREADABLE;}
}
static void arguments(struct ApiFrame* f,u32* regs){
 const struct ApiHook* h=api_hooks+f->tag;u32 count=h->formal_words==0xffffffffu?16:h->formal_words;
 if(readable((u32)(regs+9),count*4)){f->arguments_read=1;copy(f->args,regs+9,count*4);}
 /* A pointer equality observation, not an inferred COM/interface identity.
  * Receiver/vtable are saved before the CALL and never read after Release. */
 if(f->arguments_read&&count&&h->mode==2&&readable(f->args[0],4)){
  u32 table=*(u32*)f->args[0];if(table&&table+h->operand==f->slot){f->receiver=f->args[0];f->vtable=table;f->candidate=1;}
 }
}
static void snapshots(u32* row,const struct ApiFrame* f,u32 event){
 const struct ApiHook* h=api_hooks+f->tag;
 for(u32 i=0;i<h->snapshot_count;++i){const struct ApiSnapshot* plan=h->snapshots+i;u32* out=row+56+i*36;
  if(!(plan->events&event))continue;
  out[1]=plan->bytes;
  if(!f->arguments_read){out[2]=4;continue;}
  out[0]=f->args[plan->argument];
  if(!out[0])out[2]=2;
  else if(!readable(out[0],plan->bytes))out[2]=3;
  else{out[2]=1;out[3]=plan->bytes;copy(out+4,(void*)out[0],plan->bytes);}
 }
}
static void emit(u32 event,const struct ApiFrame* f,u32* regs,u32 error){
 if(io_failed||sequence>=MNM_DRAW_API_V2_MAX_RECORDS){++lost;return;}
 u32 row[200];zero(row,sizeof(row));row[0]=++sequence;row[1]=event;row[2]=f->id;row[3]=f->tag;row[4]=f->phase;row[5]=f->thread;row[6]=api_hooks[f->tag].address;row[7]=(u32)(regs+9);
 row[8]=regs[6];row[9]=regs[5];row[10]=regs[7];row[11]=regs[8];row[12]=error;row[13]=GetTickCount();row[14]=f->state;row[15]=f->slot;row[16]=f->target;
 if(event==MNM_DRAW_API_BEFORE&&readable((u32)(regs+9),32)){row[17]=1;copy(row+18,regs+9,32);}
 row[26]=active;row[27]=f->sp;row[28]=regs[4];row[29]=regs[2];row[30]=regs[1];row[31]=regs[0];
 const struct ApiHook* h=api_hooks+f->tag;row[32]=h->formal_words;row[33]=h->formal_words==0xffffffffu?16:h->formal_words;row[35]=h->snapshot_count;
 if(event==MNM_DRAW_API_BEFORE&&f->arguments_read){row[34]=1;copy(row+36,f->args,row[33]*4);}
 row[52]=f->receiver;row[53]=f->vtable;row[54]=f->candidate;snapshots(row,f,event);
 if(used+800>sizeof(buffer))flush();if(!io_failed){copy(buffer+used,row,800);used+=800;}
}
void api_observe(u32* regs,u32 tag,u32 event){
 u32 error=GetLastError(),thread=GetCurrentThreadId();lock();
 /* On bookkeeping loss, stop assigning pairs. Original forwarding remains
  * independent: each AFTER stub has its own fixed continuation. */
 if(lost||io_failed){unlock();SetLastError(error);return;}
 if(tag>=API_HOOKS||(event!=1&&event!=2)){++lost;unlock();SetLastError(error);return;}
 if(event==MNM_DRAW_API_BEFORE){
  u32 value=phase();if(lost){unlock();SetLastError(error);return;}
  for(u32 i=0;i<2048;++i)if(!frames[i].live){struct ApiFrame f={0};f.live=1;f.tag=tag;f.id=++serial;f.phase=value;f.thread=thread;f.sp=(u32)(regs+9);target(&f,regs);arguments(&f,regs);frames[i]=f;++active;emit(event,&f,regs,error);unlock();SetLastError(error);return;}
  ++lost;
 }else{
  u32 match=2048;for(u32 i=0;i<2048;++i)if(frames[i].live&&frames[i].thread==thread&&(match==2048||frames[i].id>frames[match].id))match=i;
  if(match==2048||frames[match].tag!=tag)++lost;
  else{struct ApiFrame f=frames[match];zero(frames+match,sizeof(frames[match]));--active;emit(event,&f,regs,error);}
 }
 unlock();SetLastError(error);
}
static int prepare_trampoline(u32 index){
 const struct ApiHook* h=api_hooks+index;
 u8* t=VirtualAlloc(0,h->code_length,0x3000,0x40);if(!t)return 0;copy(t,h->code,h->code_length);
 for(u32 j=0;j<h->reloc_count;++j){u32 offset=h->reloc[j];*(u32*)(t+offset)-=(u32)t;}
 *(u32*)(t+h->after_offset)+=(u32)h->after;
 api_trampolines[index]=(u32)t;api_remainders[index]=(u32)t+h->rest;
 return FlushInstructionCache(GetCurrentProcess(),t,h->code_length);
}
static i32 WIN api_exception(struct ApiExceptionPointers* info){
 /* No OS/API calls: preserve LastError and change only saved EIP. A foreign
  * breakpoint or mismatched exception context follows the normal handler chain.
  * No temporary opcode restoration, single stepping or shared patch race. */
 if(!info||!info->record||!info->context||info->record->code!=0x80000003u)return 0;
 u32 address=(u32)info->record->address,pc=info->context->eip;
 for(u32 i=0;i<API_HOOKS;++i){const struct ApiHook* h=api_hooks+i;
  if(h->patch_bytes==1&&address==h->address&&(pc==address||pc==address+1)){
   info->context->eip=(u32)h->before;return -1;
  }
 }
 return 0;
}
static int patch_hook(u32 index){
 const struct ApiHook* h=api_hooks+index;u32 old,unused;
 u32 length=h->patch_bytes==1?1:h->length;
 if(!VirtualProtect((void*)h->address,length,0x40,&old))return 0;
 if(h->patch_bytes==1){*(volatile u8*)h->address=0xcc;
  int flushed=FlushInstructionCache(GetCurrentProcess(),(void*)h->address,1);int protected=VirtualProtect((void*)h->address,1,old,&unused);return flushed&&protected;}
 u8 bytes[20];zero(bytes,sizeof(bytes));bytes[0]=0xe9;*(u32*)(bytes+1)=(u32)h->before-h->address-5;for(u32 j=5;j<h->length;++j)bytes[j]=0x90;copy((void*)h->address,bytes,h->length);
 int flushed=FlushInstructionCache(GetCurrentProcess(),(void*)h->address,h->length);int protected=VirtualProtect((void*)h->address,h->length,old,&unused);return flushed&&protected;
}
static int path(char* out,const char* directory,const char* suffix){u32 at=0;while(directory[at]){if(at>=220)return 0;out[at]=directory[at];++at;}while(*suffix){if(at>=259)return 0;out[at++]=*suffix++;}out[at]=0;return 1;}
static u32 image_size(u32 base){
 if(!readable(base,64)||*(u16*)base!=0x5a4d)return 0;u32 offset=*(u32*)(base+60);
 if(offset>4096-84||!readable(base+offset,84)||*(u32*)(base+offset)!=0x4550||*(u16*)(base+offset+24)!=0x10b)return 0;
 u32 size=*(u32*)(base+offset+80);if(!size||base+size<base)return 0;return size;
}
static int write_bindings(const char* directory){
 char name[260];if(!path(name,directory,"\\api-bindings.bin"))return 0;
 u32 header[32]={0};copy(header,MNM_DRAW_API_BINDINGS,8);header[2]=1;header[3]=128;header[4]=48;header[5]=API_HOOKS;header[6]=0x400000;header[7]=module_base;header[8]=image_size(module_base);
 if(!header[8])return 0;
 header[9]=(u32)GetModuleHandleA("MnmDrawingCensus.dll");header[10]=image_size(header[9]);
 if(header[9]){header[11]=(u32)GetProcAddress((HANDLE)header[9],"census_leave");if(!header[10]||!header[11])return 0;}
 header[12]=(u32)GetModuleHandleA("MnmMenu.dll");header[13]=image_size(header[12]);if(header[12]&&!header[13])return 0;copy(header+16,original_sha256,32);
 HANDLE file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(file==(HANDLE)-1)return 0;
 int ok=write_binding(file,header,128);
 for(u32 i=0;ok&&i<API_HOOKS;++i){const struct ApiHook* h=api_hooks+i;u32 call_length=h->after_offset-1;
  u32 row[12]={i,h->address,h->length,call_length,api_trampolines[i],h->code_length,api_remainders[i],(u32)h->before,(u32)h->after,api_trampolines[i]+call_length,h->address+call_length,0};ok=write_binding(file,row,48);}
 u32 end[16]={0};copy(end,MNM_DRAW_API_BINDINGS_END,8);end[2]=1;end[3]=API_HOOKS;if(ok)ok=write_binding(file,end,64);return CloseHandle(file)&&ok;
}
static int install(void){
 char directory[220],name[260];u32 n=GetEnvironmentVariableA("MNM_DRAW_API_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory)||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));if(!(d&(1u<<24)))return 0;
 if(!path(name,directory,"\\census-control.bin"))return 0;HANDLE file=CreateFileA(name,0x80000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return 0;
 HANDLE mapping=CreateFileMappingA(file,0,2,0,64,0);CloseHandle(file);if(!mapping)return 0;control=MapViewOfFile(mapping,4,0,0,64);CloseHandle(mapping);
 if(!control||!same((const void*)control,MNM_DRAW_CENSUS_CONTROL,8)||control[2]!=1||control[3]!=64)return 0;
 /* Validate every original site before allocating or patching any. */
 for(u32 i=0;i<API_HOOKS;++i){const struct ApiHook* h=api_hooks+i;if(!readable(h->address,h->length)||!same((void*)h->address,h->signature,h->length))return 0;}
 for(u32 i=0;i<API_HOOKS;++i)if(!prepare_trampoline(i))return 0;
 for(u32 i=0;i<API_HOOKS;++i)if(api_hooks[i].patch_bytes==1){exception_handler=AddVectoredExceptionHandler(1,api_exception);if(!exception_handler)return 0;break;}
 if(!write_bindings(directory))return 0;
 if(!path(name,directory,"\\api-calls.bin"))return 0;journal=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(journal==(HANDLE)-1){journal=0;return 0;}
 u32 header[16]={0};copy(header,MNM_DRAW_API_V2_MAGIC,8);header[2]=2;header[3]=64;header[4]=MNM_DRAW_API_V2_RECORD;header[5]=(u32)GetModuleHandleA(0);header[6]=API_HOOKS;header[7]=MNM_DRAW_API_V2_MAX_RECORDS;copy(header+8,original_sha256,32);if(!write(header,64))return 0;
 for(u32 i=0;i<API_HOOKS;++i)if(!patch_hook(i))return 0;
 installed=1;return 1;
}
__declspec(dllexport) void DrawingApiCensusAnchor(void){}
int WIN DllMain(void* instance,u32 reason,void* reserved){
 (void)reserved;if(reason==1){module_base=(u32)instance;return install();}if(reason!=0)return 1;
 if(journal&&!installed){CloseHandle(journal);journal=0;}
 if(journal){/* Busy process detach cannot create a valid terminal. */
  if(__sync_lock_test_and_set(&mutex,1)){CloseHandle(journal);journal=0;return 1;}
  flush();u32 end[16]={0};copy(end,MNM_DRAW_API_END,8);end[2]=2;end[3]=sequence;end[4]=serial;end[5]=lost;end[6]=active;end[7]=io_failed;if(!io_failed)write(end,64);CloseHandle(journal);journal=0;unlock();}
 if(control){UnmapViewOfFile((const void*)control);control=0;}return 1;
}
