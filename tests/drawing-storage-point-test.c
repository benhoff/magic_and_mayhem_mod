#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
static u32 depth,enabled=1,stopped,sequence=7,queue,failures;
#include "../runtime/scene/producer_trace_guard.h"
#define MAX_SURFACES 2
#define MAX_FRAMES 2
static struct Surface{u32 object,id,pixels,width,height,stride,dirty,sampled;} surfaces[2];
static struct{u32 sp,tag,ecx;} frames[2];
static void copy(void* out,const void* in,u32 n){u8* a=out;const u8* b=in;while(n--)*a++=*b++;}
static int bypass_writable(u32 p,u32 n){u32 m[7];return p&&p+n>=p&&VirtualQuery((void*)p,m,28)==28&&m[4]==0x1000&&(m[5]&0xcc)&&!(m[5]&0x101)&&p+n<=m[0]+m[3];}
#include "../runtime/scene/drawing_storage_point.h"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
void start(void){
 require(producer_trace_configure(),130);u32 out[16]={0};
 /* The synthetic address is deliberately unreadable. Export must not read it. */
 surfaces[0]=(struct Surface){1,77,0x1234,3,3,4,9,7};
 SetLastError(0xbeef);require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)==1&&GetLastError()==0xbeef,131);
 require(out[2]==19&&out[6]==77&&out[7]==3&&out[8]==3&&out[9]==4&&out[10]==9&&out[11]==7&&out[14]==GetCurrentThreadId()&&!out[15],132);
 require(SceneDrawingStoragePointV1((void*)0x5678,8,24,out,64)&&out[2]==3&&!out[6],133);
 require(SceneDrawingStoragePointV1((void*)0x1234,6,24,out,64)&&out[2]==35&&!out[6],134);
 require(SceneDrawingStoragePointV1((void*)0x1234,8,23,out,64)&&out[2]==35&&!out[6],135);
 surfaces[1]=surfaces[0];surfaces[1].id=78;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==11&&!out[6],136);surfaces[1].object=0;
 failures=1;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==3&&out[5]==1&&!out[6],137);failures=0;
 stopped=1;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==7&&!out[6],138);stopped=0;
 frames[0].sp=1;*producer_trace_depth(GetCurrentThreadId())=2;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[12]==1&&out[13]==2,139);
 u32* readonly=VirtualAlloc(0,4096,0x3000,4);require(readonly!=0,140);readonly[0]=0x77777777;u32 old;require(VirtualProtect(readonly,4096,2,&old),141);
 require(!SceneDrawingStoragePointV1((void*)0x1234,8,24,readonly,64)&&readonly[0]==0x77777777,142);
 require(!SceneDrawingStoragePointV1(0,0,0,0,64)&&!SceneDrawingStoragePointV1(0,0,0,out,63),143);
 require(SceneDrawingClientStoragePointV1((void*)0x1234,8,out,64)&&out[2]==83&&out[6]==77,145);
 require(SceneDrawingClientStoragePointV1((void*)0x1234,6,out,64)&&out[2]==35&&!out[6],146);
 /* Same-wrapper transitions refuse even with a still-matching cached address. */
 frames[0].sp=0;*producer_trace_depth(GetCurrentThreadId())=0;
 for(u32 tag=0;tag<3;++tag){
  frames[0].sp=1;frames[0].tag=tag;frames[0].ecx=1;
  SetLastError(0xbeef);require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==131&&!out[6]&&out[12]==1&&GetLastError()==0xbeef,147+tag);
  require(SceneDrawingClientStoragePointV1((void*)0x1234,8,out,64)&&out[2]==131&&!out[6],150+tag);
  frames[0].ecx=2;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==19&&out[6]==77,153+tag);
 }
 frames[0].ecx=1;frames[0].tag=7;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==19,156);
 frames[0].sp=0;frames[0].tag=1;require(SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64)&&out[2]==19,157);
 producer_trace_mode=0;require(!SceneDrawingStoragePointV1((void*)0x1234,8,24,out,64),144);ExitProcess(0);
}
