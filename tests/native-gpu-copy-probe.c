/* Explicit native route preserves physical CPU authority and validates leases.
 * The combined campaign separately verifies actual ordered GPU execution. */
#define start retained_resource_probe
#include "native-draw-resource-probe.c"
#undef start
typedef i32 (WIN *History)(const void*,u32,u32,u32,u32,u32);
typedef i32 (WIN *GpuCopy)(void*,const i32*,void*,const i32*,u32,u16*,u32);
typedef i32 (WIN *Transfer)(void*,u32*,const u16*,u32);
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");require(module!=0,101);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 Transfer transfer=(Transfer)GetProcAddress(module,"NativeDrawCpuTransferV1");
 GpuCopy gpu=(GpuCopy)GetProcAddress(module,"NativeDrawGpuCopyV1");
 void *draw=0,*src=0,*dst=0;u32 d[31];
 require(create && history && gpu && transfer && !create(0,&draw,0),102);
 desc(d,2);d[1]=7;d[2]=9;d[3]=13;d[26]=0x840;
 require(!((Surface)table(draw)[6])(draw,d,&src,0) && !((Surface)table(draw)[6])(draw,d,&dst,0),103);
 desc(d,2);require(!((Lock)table(src)[25])(src,0,d,1,0),104);u16* source=(u16*)d[9];
 source[0]=0x1234;require(!((i32 (WIN *)(void*,void*))table(src)[32])(src,source),105);
 desc(d,2);require(!((Lock)table(dst)[25])(dst,0,d,1,0),106);u16* target=(u16*)d[9];
 target[0]=0x5678;require(!((i32 (WIN *)(void*,void*))table(dst)[32])(dst,target),107);
 i32 rect[4]={0,0,13,9},bad[4]={0,0,14,9};
 require(gpu(dst,rect,src,rect,0,0,0)<0 && !history(target,2,0,0,0,0),108);
 require(!history(source,1,2,0,5,6),109);
 require(gpu(dst,bad,src,rect,0,0,0)<0 && !history(target,2,0,0,0,0),110);
 desc(d,2);require(!((Lock)table(dst)[25])(dst,0,d,1,0),111);
 require(gpu(dst,rect,src,rect,0,0,0)==(i32)0x887601aeu,112);
 require(!((i32 (WIN *)(void*,void*))table(dst)[32])(dst,target),113);
 require(gpu(src,rect,src,rect,0,0,0)<0,114);
 require(gpu(dst,rect,src,rect,0x8000,0,0)<0 && !history(target,2,0,0,0,0),115);
 u16 baseline[13*9];
 require(gpu(dst,rect,src,rect,0,baseline,sizeof(baseline)-2)<0 && !history(target,2,0,0,0,0),125);
 require(!gpu(dst,rect,src,rect,0,baseline,sizeof(baseline)) && baseline[0]==0x5678,116);
 require(source[0]==0x1234 && target[0]==0x5678,117);
 require(history(target,2,0,0,0,0)==(i32)0x887601aeu,118);
 require(history(target,1,2,0,5,6)<0 && history(target,1,3,0,7,8)<0,119);
 require(!history(target,1,3,0,5,6),120);
 u32 identity[8];require(!transfer(dst,identity,0,0),126);
 for(u32 i=0;i<13*9;++i)baseline[i]=0x2222;
 identity[7]^=1;require(transfer(dst,identity,baseline,sizeof(baseline))<0 && target[0]==0x5678 && history(target,2,0,0,0,0)<0,127);identity[7]^=1;
 require(transfer(dst,identity,baseline,sizeof(baseline)-2)<0 && history(target,2,0,0,0,0)<0,128);
 require(!transfer(dst,identity,baseline,sizeof(baseline)) && !history(target,2,0,0,0,0) && target[0]==0x2222 && target[12+8*(16)]==0x2222,129);
 require(transfer(dst,identity,baseline,sizeof(baseline))<0 && transfer(dst,identity,0,0)==1,130);
 require(!history(target,1,4,0,5,6),131);
 u32 fx[25]={100};fx[20]=0x7fff;
 require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(src)[5])(src,0,0,0,0x400,fx),121);
 require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(dst)[5])(dst,0,src,0,0,0),122);
 require(!history(target,2,0,0,0,0) && target[0]==0x7fff,123);
 require(!release(src) && !release(dst) && !release(draw),124);
 ExitProcess(0);
}
