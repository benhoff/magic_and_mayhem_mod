#include "drawing_caller_state_codec.h"
#include "../../protocols/include/mnm/drawing_caller_state_v3.h"
#include "../../protocols/include/mnm/drawing_caller_bindings_v1.h"
API void WIN Sleep(u32);
/* Optional passive diagnostics for the shared producer entry/return boundary.
 * Native rows describe the saved output consumed by its tested POP/RET sequence;
 * they do not pretend to sample a later processor instruction. Retained rows
 * describe the normal return observer. Neither route supplies expected state. */
static HANDLE caller_state_file;
static HANDLE caller_bindings_file;
static u32 caller_bindings_rows;
static u32 caller_state_rows,caller_state_starts,caller_state_completed,caller_state_active,caller_state_failed,caller_state_gate;
static struct CallerState {u32 claimed,row[208],bindings[8];} caller_states[MAX_FRAMES];
static u32 caller_state_argc(u32 tag){
 if(tag>=HOOKS)return 0;
 if(tag==10)return 4;
 if(tag==12||tag==22||tag==23||tag==24||tag==36)return 3;
 if((tag>=13&&tag<=17)||tag==34)return 5;
 if(tag==26||tag==52)return 2;
 if(tag==11||(tag>=18&&tag<=21)||tag==25||tag>=57)return hooks[tag].pop/4;
 return 0;
}
static int caller_state_create_at(u32 number){
 if(caller_state_file||caller_bindings_file||caller_state_active)return 0;
 caller_state_rows=caller_state_starts=caller_state_completed=caller_state_failed=0;
 caller_bindings_rows=0;
 zero(caller_states,sizeof(caller_states));
 char name[260];path(name,"caller-state-",number,".bin");
 caller_state_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(caller_state_file==(HANDLE)-1){caller_state_file=0;return 0;}
 u32 header[16]={0};copy(header,MNM_CALLER_STATE_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=832;
 header[5]=MNM_CALLER_STATE_MAX_ROWS;header[6]=0x40209ca7;
 if(!write(caller_state_file,header,sizeof(header)))return 0;
 path(name,"caller-bindings-",number,".bin");caller_bindings_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(caller_bindings_file==(HANDLE)-1){caller_bindings_file=0;return 0;}
 copy(header,MNM_CALLER_BINDINGS_MAGIC,8);header[2]=1;header[4]=32;
 return write(caller_bindings_file,header,sizeof(header));
}
static int caller_state_create(void){return caller_state_create_at(0);}
static int caller_state_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_CALLER_STATE",flag,sizeof(flag));
 if(!n)return GetLastError()==203;
 return n==1&&flag[0]=='1'&&caller_state_create();
}
static void caller_state_emit(const u32 row[208],const u32 bindings[8]){
 while(__sync_lock_test_and_set(&caller_state_gate,1))Sleep(0);
 if(caller_state_rows>=MNM_CALLER_STATE_MAX_ROWS-1||!write(caller_state_file,row,832))ExitProcess(99);
 if(row[190]==2){
  if(!caller_bindings_file||!write(caller_bindings_file,bindings,32))ExitProcess(99);
  ++caller_bindings_rows;
 }
 ++caller_state_rows;__sync_lock_release(&caller_state_gate);
}
static void caller_state_context(u32 row[192],const u32* stack){
 const u32 caller=row[4],tag=row[2];u32 kind=0,word=0;
 if((tag==26&&(caller==0x54ab16||caller==0x54aab2||caller==0x54657c))||(tag==52&&caller==0x54ab72)){kind=1;word=3;}
 else if(caller==0x57e8fb||caller==0x57e915||caller==0x57e985||caller==0x57e9a8){kind=2;word=8;}
 else if(caller==0x57e940||caller==0x57e957||caller==0x57e9c7){kind=2;word=10;}
 else if(tag==57&&mnm_world_trail_site(caller)){kind=4;word=11;}
 else for(u32 i=0;i<sizeof(sprite_raster_sites)/sizeof(sprite_raster_sites[0]);++i)
  if(sprite_raster_sites[i].address==caller&&(sprite_raster_sites[i].tags&(1ull<<tag))){kind=3;word=sprite_raster_sites[i].parent_word;break;}
 if(kind&&readable((u32)stack,(word+1)*4)){row[190]=kind;row[191]=stack[word];}
}
static u32 caller_state_begin(const u32* saved,u32 tag,const u8* fx,u32 error){
 if(!caller_state_file||!enabled||stopped||tag>=HOOKS)return 0;
 const u32 count=caller_state_argc(tag);if(!count)return 0;
 if(!readable((u32)(saved+10),count*4))ExitProcess(99);
 for(u32 i=0;i<MAX_FRAMES;++i)if(__sync_bool_compare_and_swap(&caller_states[i].claimed,0,~0u)){
  u32* row=caller_states[i].row;zero(row,832);row[0]=__sync_add_and_fetch(&caller_state_starts,1);
  row[2]=tag;row[3]=GetCurrentThreadId();row[4]=saved[9];row[5]=(u32)(saved+9);
  row[6]=row[5]+4+hooks[tag].pop;row[7]=queue;row[8]=sequence;row[11]=count;row[12]=error;
  row[14]=hooks[tag].pop?0:1;row[15]=get((void*)0x658174);
  copy(row+16,saved,36);row[19]=row[5];copy(row+34,saved+10,count*4);
  copy(row+46,(void*)0x5f1e50,64);mnm_caller_state_fx((u8*)(row+78),fx);
  caller_state_context(row,saved+9);
  if(continuous_map){row[192]=continuous_generation_low;row[193]=continuous_generation_high;
   row[194]=continuous_source_low;row[195]=continuous_source_high;row[200]=continuous_nonce_low;row[201]=continuous_nonce_high;
   row[202]=1;row[203]=!!raster_world_open;}
  u32* bindings=caller_states[i].bindings;zero(bindings,32);
  if(row[190]==2){bindings[0]=row[0];bindings[1]=row[3];bindings[2]=row[4];bindings[3]=row[2];bindings[4]=row[7];bindings[5]=row[8];
   bindings[6]=get((void*)0x6903dc);bindings[7]=get((void*)0x6c4e3c);}
  __sync_add_and_fetch(&caller_state_active,1);__atomic_store_n(&caller_states[i].claimed,1,__ATOMIC_RELEASE);return i+1;
 }
 ExitProcess(99);return 0;
}
static void caller_state_finish(u32 token,u32 kind,const u32* saved,const u8* fx,u32 error){
 if(!token||token>MAX_FRAMES||!caller_state_file)ExitProcess(99);
 struct CallerState* state=caller_states+token-1;u32* row=state->row;
 if(__atomic_load_n(&state->claimed,__ATOMIC_ACQUIRE)!=1||row[3]!=GetCurrentThreadId())ExitProcess(99);
 row[1]=kind;row[9]=queue;row[10]=sequence;row[13]=error;
 if(row[202]){if(!continuous_map||row[200]!=continuous_nonce_low||row[201]!=continuous_nonce_high)ExitProcess(99);
  row[196]=continuous_generation_low;row[197]=continuous_generation_high;row[198]=continuous_source_low;row[199]=continuous_source_high;row[204]=!!raster_world_open;}
 copy(row+25,saved,36);row[28]=row[6];
 if(row[14]&1){const u32 args=kind==MNM_CALLER_STATE_NATIVE?(u32)(saved+10):row[6];
  if(!readable(args,row[11]*4))ExitProcess(99);copy(row+40,(void*)args,row[11]*4);}
 copy(row+62,(void*)0x5f1e50,64);mnm_caller_state_fx((u8*)(row+134),fx);
 caller_state_emit(row,state->bindings);__sync_add_and_fetch(&caller_state_completed,1);__sync_sub_and_fetch(&caller_state_active,1);
 __atomic_store_n(&state->claimed,0,__ATOMIC_RELEASE);
}
static void caller_state_return(u32 sp,u32 tag,const u32* saved,const u8* fx,u32 error){
 if(!caller_state_file||!caller_state_argc(tag))return;
 const u32 owner=GetCurrentThreadId();
 for(u32 i=0;i<MAX_FRAMES;++i)if(__atomic_load_n(&caller_states[i].claimed,__ATOMIC_ACQUIRE)==1&&
    caller_states[i].row[6]==sp&&caller_states[i].row[2]==tag&&caller_states[i].row[3]==owner){
  caller_state_finish(i+1,MNM_CALLER_STATE_RETAINED,saved,fx,error);return;
 }
 /* A native glyph can also use the resident normal return observer: its
  * saved native output was already recorded and its active slot retired. */
 if(tag!=10)ExitProcess(99);
}
static void caller_state_close(void){
 if(!caller_state_file)return;u32 error=GetLastError();
 u32 row[208];zero(row,sizeof(row));row[0]=caller_state_rows+1;row[1]=MNM_CALLER_STATE_FOOTER;row[2]=caller_state_starts;
 row[3]=caller_state_completed;row[4]=caller_state_active;row[5]=caller_state_failed;
 if(caller_state_rows>=MNM_CALLER_STATE_MAX_ROWS||!write(caller_state_file,row,832)||!CloseHandle(caller_state_file))ExitProcess(99);
 caller_state_file=0;u32 terminal[8]={0,caller_bindings_rows,caller_state_completed,caller_state_active,caller_state_failed,0,0,0};
 if(!caller_bindings_file||!write(caller_bindings_file,terminal,32)||!CloseHandle(caller_bindings_file))ExitProcess(99);
 caller_bindings_file=0;SetLastError(error);
}
