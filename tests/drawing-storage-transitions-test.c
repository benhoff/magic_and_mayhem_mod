#include "../runtime/shadow/win32_min.h"
static u32 producer_trace_mode=1,sequence,queue,failures;
static u32 producer_trampolines[10];
static char directory[220];
struct Frame{u32 tag,owner,caller,ecx,storage_call;};
static struct Surface{u32 object,id,pixels,width,height,stride;} surface;
static struct Surface* object(u32 p){return p&&p==surface.object?&surface:0;}
static u32 get(const void* p){return *(const u32*)p;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int readable(u32 p,u32 n){u32 m[7];return p&&p+n>=p&&VirtualQuery((void*)p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(m[5]&0xee)&&p+n<=m[0]+m[3];}
static int write(HANDLE h,const void* p,u32 n){u32 done;return WriteFile(h,p,n,&done,0)&&done==n;}
static void path(char* out,const char* name,u32 number,const char* suffix){u32 i=0;(void)number;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;copy(out+i,"0000",4);i+=4;while(*suffix)out[i++]=*suffix++;out[i]=0;}
#include "../runtime/scene/drawing_storage_transitions.h"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
void start(void){
 require(GetEnvironmentVariableA("MNM_SCENE_DIR",directory,220)>0,201);require(storage_transitions_open(),202);
 u32 wrapper[7]={0};wrapper[1]=0xdead;wrapper[2]=0xbeef;wrapper[4]=800;wrapper[5]=600;wrapper[6]=1600;
 u32 mapped=99;producer_trampolines[9]=0x900000;
 require(storage_caller_rva(0x900009,0x900000,&mapped)==0x18d329&&mapped==1,205);
 require(!storage_caller_rva(0x900008,0x900000,&mapped)&&!mapped,206);
 require(!storage_caller_rva(0x90000a,0x900000,&mapped)&&!mapped,207);
 require(!storage_caller_rva(9,0,&mapped)&&!mapped,208);
 require(!storage_caller_rva(0,0xfffffff7,&mapped)&&!mapped,209);
 require(storage_caller_rva(0x58d329,0x900000,&mapped)==0x18d329&&!mapped,210);
 struct Frame f={0,18,0x4bdd10,(u32)wrapper,0};
 storage_transition_record(&f,1,0);surface=(struct Surface){(u32)wrapper,7,0,800,600,800};++sequence;storage_transition_record(&f,2,0x12345678);
 f.tag=2;f.caller=0x900009;storage_transition_record(&f,1,0);surface.pixels=0x1234;storage_transition_record(&f,2,0x1234);
 /* Same address on another returned Lock gets a distinct observed epoch. */
 storage_transition_record(&f,1,0);storage_transition_record(&f,2,0x1234);
 storage_transition_record(&f,1,0);surface.pixels=0;storage_transition_record(&f,2,0);
 f.tag=1;storage_transition_record(&f,1,0);surface.object=0;wrapper[1]=wrapper[2]=0;storage_transition_record(&f,2,0xffffffff);
 f.tag=0;storage_transition_record(&f,1,0);surface=(struct Surface){(u32)wrapper,8,0x1234,800,600,800};storage_transition_record(&f,2,0x87654321);
 require(storage_transition_calls==6&&storage_transition_returns==6&&storage_transition_epoch==3&&storage_transition_address_count==3,203);
 storage_transitions_close();require(!storage_transition_file,204);ExitProcess(0);
}
