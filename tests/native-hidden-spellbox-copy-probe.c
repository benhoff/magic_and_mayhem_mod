/* Real provider, production wrapper hook and actual5770f5 E8. The caller's
 * surrounding UI is an authored endpoint, not a whole Spellbox ABI comparison. */
#define start hidden_existing_owner_start
#include "native-cursor-owner-probe.c"
#undef start
void WIN hidden_copy(u32,u32,const u32*,u32,u32);
__asm__(
 ".text\n.globl \"_hidden_copy@20\"\n\"_hidden_copy@20\":\n"
 "pushl %ebp\nmovl %esp,%ebp\npushl $_hidden_copy_return\n"
 "pushl 24(%ebp)\npushl 20(%ebp)\npushl 16(%ebp)\npushl 12(%ebp)\n"
 "movl 8(%ebp),%ecx\nmovl $0x5770f5,%eax\njmp *%eax\n"
 "_hidden_copy_return:\nmovl %ebp,%esp\npopl %ebp\nretl $20\n");
static void plane(void* p,u32 source,u32 copied){
 u32 d[31];gl_desc(d,0);need(!((i32(WIN*)(void*,void*,u32*,u32,HANDLE))gl_vt(p)[25])(p,0,d,1,0),401);
 for(u32 y=0;y<480;++y)for(u32 x=0;x<640;++x){
  u32 seed=source || (copied&&x<40&&y<40)?0x1100:0x5500;
  need(((u16*)(d[9]+y*d[4]))[x]==(u16)(seed+x*337+y*971),402);
 }
 need(!gl_unlock(p,1,d[9]),403);++planes;
}
void start(void){
 original_map();bind_imports();stage(0);u32 native=0;void* module=gl_module(&native);need(native,404);
 char early[2];HANDLE outer=0;if(GetEnvironmentVariableA("MNM_CURSOR_EARLY_OUTER",early,2))outer=make_mutex(0x4e8f6f,0x6e1ec0);
 char bad[4];u32 bad_n=GetEnvironmentVariableA("MNM_CURSOR_BAD_ANCHOR",bad,sizeof(bad));
 if(bad_n){u32 index=(u32)(bad[0]-'0');if(bad_n==2)index=index*10+(u32)(bad[1]-'0');
  const u32 sites[18]={0x4a30b0,0x4a37d0,0x4a39a0,0x4a4370,0x4a4380,0x4a44b0,0x4a45b0,0x4a4910,0x4a2f90,0x58d5b0,0x58d660,0x58ad90,0x58b3e0,0x58c140,0x58c360,0x58c4a0,0x58c8a0,0x58ca90};
  need(index<18,417);*(u8*)sites[index]^=1;}
 GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");void* draw=0;need(create&&!create(0,&draw,0)&&draw,405);
 need(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),406);
 const u8 dd2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
 const u8 dd4[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
 put(0x6f98c4,(u32)draw);put(0x6f98c8,(u32)gl_query(draw,dd2));put(0x6f98cc,(u32)gl_query(draw,dd4));
 put(0x6f98d0,0);put(0x6e1f1c,0);put(0x6f8154,0);put(0x6f98c0,0);
 wrap(0x641fc8,640,480);wrap(0x642000,80,80);wrap(0x6a49e0,640,480);wrap(0x6a4fe0,640,480);
 make_mutex(0x4a3700,0x642c58);if(!outer)outer=make_mutex(0x4e8f6f,0x6e1ec0);
 put(0x642c64,0);put(0x5e04c8,22);put(0x6e1ee4,0);put(0x6f98c0,0x6a4fe0);
 fill(0x641fc8,640,480,0x1100,0);fill(0x6a49e0,640,480,0x5500,0);
 void* source=(void*)get(0x641fd0),*dest=(void*)get(0x6a49e8);
 const u8 call[5]={0xe8,0xa6,0x53,1,0},continuation[10]={0xc7,5,0x70,0x51,0x6f,0,1,0,0,0};
 for(u32 i=0;i<5;++i)need(((u8*)0x5770f5)[i]==call[i],407);
 for(u32 i=0;i<10;++i)need(((u8*)0x5770fa)[i]==continuation[i],408);
 // Private mapped-image endpoint only: after the actual original E8 return,
 // return to the authored harness instead of entering the surrounding UI.
 *(u8*)0x5770fa=0xc3;
 u32(WIN* status)(u32*,u32)=(u32(WIN*)(u32*,u32))GetProcAddress(module,"NativeDrawCursorStatusV1");
 u32 before[28],after[28];need(status&&status(before,sizeof(before)),409);
 const u32 outside[4]={640,512,680,552};stage(1);
 hidden_copy(0x641fc8,0x6a49e0,outside,640,512);stage(2);
 plane(source,1,0);plane(dest,0,0);
 need(status(after,sizeof(after))&&after[26]==before[26]+1,410);
 const u32 inside[4]={0,0,40,40};hidden_copy(0x641fc8,0x6a49e0,inside,0,0);
 plane(source,1,0);plane(dest,0,1);need(status(before,sizeof(before))&&before[26]==after[26],411);
 i32 invalid[4]={640,512,680,552};i32 result=((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))gl_vt(dest)[7])(dest,640,512,source,invalid,0x10);
 need((u32)result==0x88760096,412);plane(dest,0,1);
 u32(WIN* stop)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawCursorStopV1");
 need(stop&&stop(),413);((void(*)(void))0x4a37d0)();((void(*)(void))0x58acf0)();
 gl_counts(module,0,0,0,0,0);need(((u32(WIN*)(HANDLE))get(0x5c50a0))(outer),414);put(0x6e1ec0,0);
 need(status(after,sizeof(after)),415);for(u32 i=5;i<=10;++i)need(!after[i],416);
 u32 report[32]={1,checks,planes,18};for(u32 i=0;i<28;++i)report[i+4]=after[i];
 gl_save("hidden-spellbox-copy.bin",report,sizeof(report));stage(3);ExitProcess(0);
}
