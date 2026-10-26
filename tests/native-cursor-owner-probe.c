/* Production owning hooks in a private original image. Real provider/kernel
 * resources; cursor position/window ports are authored. No game entrypoint. */
#include "graphics-lifetime-common.h"
#include "original-pe32-image.h"
static u32 checks,planes,px=9,py=7;
static void stage(u32 n){u32 done=0;HANDLE f=CreateFileA("stage.bin",0x40000000,0,0,2,0x80,0);gl_require(f!=(HANDLE)-1&&WriteFile(f,&n,4,&done,0)&&done==4,165);CloseHandle(f);}
static void need(int ok,u32 code){gl_require(ok,code);++checks;}
static u32 get(u32 a){return *(u32*)a;}static void put(u32 a,u32 v){*(u32*)a=v;}
static u32 WIN cursor(u32* p){p[0]=px;p[1]=py;return 1;}
static u32 WIN transform(u32 w,u32* p){(void)w;(void)p;return 1;}
static u32 WIN client(u32 w,u32* r){(void)w;r[0]=r[1]=0;r[2]=640;r[3]=480;return 1;}
static u32 WIN intersect(i32* r,const i32* a,const i32* b){
 for(u32 i=0;i<4;++i)r[i]=i<2?(a[i]>b[i]?a[i]:b[i]):(a[i]<b[i]?a[i]:b[i]);
 if(r[0]>=r[2]||r[1]>=r[3]){for(u32 i=0;i<4;++i)r[i]=0;return 0;}return 1;
}
static void* WIN fn(const char* name){return GetProcAddress(GetModuleHandleA("kernel32.dll"),name);}
static void bind_imports(void){
 put(0x5c50b8,(u32)fn("CreateMutexA"));put(0x5c50a0,(u32)fn("CloseHandle"));put(0x5c5140,(u32)fn("WaitForSingleObject"));put(0x5c513c,(u32)fn("ReleaseMutex"));
 put(0x5c5210,(u32)cursor);put(0x5c5298,(u32)transform);put(0x5c51fc,(u32)client);put(0x5c5294,(u32)intersect);
}
/* Owned constructor endpoints retain the actual six-byte original import call
 * instruction/address. Their following original initialization is not run. */
static HANDLE make_mutex(u32 call,u32 global){
 u8* start=(u8*)(call-6);for(u32 i=0;i<3;++i){start[i*2]=0x6a;start[i*2+1]=0;}
 start[6]=0xff;start[7]=0x15;*(u32*)(start+8)=0x5c50b8;start[12]=0xc3;
 HANDLE h=((HANDLE(*)(void))start)();need(h!=0,201);put(global,(u32)h);return h;
}
static void wrap(u32 address,u32 w,u32 h){
 u32 d[31];gl_desc(d,0);d[2]=h;d[3]=w;
 ((void(__attribute__((thiscall))*)(u32,u32*,u32,u32))0x58ad90)(address,d,0,0);
 need(get(address+4)&&get(address+8),202);
}
static void drop(u32 address){((void(__attribute__((thiscall))*)(u32))0x58b3e0)(address);}
static void fill(u32 wrapper,u32 w,u32 h,u32 seed,u32 atlas){
 void* p=(void*)get(wrapper+8);u32 d[31];gl_desc(d,0);need(!((i32(WIN*)(void*,void*,u32*,u32,HANDLE))gl_vt(p)[25])(p,0,d,1,0),203);
 need(d[2]==h&&d[3]==w,204);
 for(u32 y=0;y<h;++y)for(u32 x=0;x<w;++x)((u16*)(d[9]+y*d[4]))[x]=(u16)(atlas&&(x+y)%3==0?31:seed+x*337+y*971);
 need(!gl_unlock(p,1,d[9]),205);
}
static void compare(void* p,u32 w,u32 h,u32 bg,u32 atlas_seed,i32 at_x,i32 at_y,u32 visible){
 u32 d[31];gl_desc(d,0);need(!((i32(WIN*)(void*,void*,u32*,u32,HANDLE))gl_vt(p)[25])(p,0,d,1,0),206);
 for(u32 y=0;y<h;++y)for(u32 x=0;x<w;++x){u16 expected=(u16)(bg+x*337+y*971);i32 sx=(i32)x-at_x,sy=(i32)y-at_y;
  if(visible && sx>=0 && sy>=0 && sx<40 && sy<40 && (sx+sy)%3!=0)expected=(u16)(atlas_seed+(u32)sx*337+(u32)sy*971);
  need(((u16*)(d[9]+y*d[4]))[x]==expected,207);
 }
 need(!gl_unlock(p,1,d[9]),208);++planes;
}
static void enqueue(u32 source,u32 dest,u32 kind,const u32* r,u32 x,u32 y){((void(__attribute__((thiscall))*)(u32,u32,u32,const u32*,u32,u32,u32))0x58d5b0)(source,kind,dest,r,x,y,1);}
static void drain(void){((void(*)(void))0x58d660)();}
void start(void){
 original_map();bind_imports();stage(0);u32 native=0;void* module=gl_module(&native);need(native,209);
 char early_flag[2];HANDLE early_outer=0;if(GetEnvironmentVariableA("MNM_CURSOR_EARLY_OUTER",early_flag,2))early_outer=make_mutex(0x4e8f6f,0x6e1ec0);
 char bad[4];u32 bad_n=GetEnvironmentVariableA("MNM_CURSOR_BAD_ANCHOR",bad,sizeof(bad));
 if(bad_n){u32 index=(u32)(bad[0]-'0');if(bad_n==2)index=index*10+(u32)(bad[1]-'0');const u32 sites[18]={0x4a30b0,0x4a37d0,0x4a39a0,0x4a4370,0x4a4380,0x4a44b0,0x4a45b0,0x4a4910,0x4a2f90,0x58d5b0,0x58d660,0x58ad90,0x58b3e0,0x58c140,0x58c360,0x58c4a0,0x58c8a0,0x58ca90};need(index<18,210);*(u8*)sites[index]^=1;}
 GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");void* draw=0;need(create&&!create(0,&draw,0)&&draw,211);
 if(bad_n)ExitProcess(212);stage(1);
 need(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),213);
 const u8 dd2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
 const u8 dd4[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
 put(0x6f98c4,(u32)draw);put(0x6f98c8,(u32)gl_query(draw,dd2));put(0x6f98cc,(u32)gl_query(draw,dd4));put(0x6f98d0,0);put(0x6e1f1c,0);put(0x6f8154,0);put(0x6f98c0,0);
 wrap(0x641fc8,40,40);wrap(0x642000,80,80);wrap(0x6a49e0,640,480);wrap(0x6a4fe0,640,480);put(0x6f98c0,0x6a4fe0);
 fill(0x642000,80,80,0x1100,1);fill(0x6a49e0,640,480,0x5500,0);fill(0x6a4fe0,640,480,0x7700,0);
 {u32 key[2]={31,31};void* a=(void*)get(0x642008);need(!((i32(WIN*)(void*,u32,u32*))gl_vt(a)[29])(a,8,key),232);put(0x64202c,31);}
 stage(2);HANDLE inner=make_mutex(0x4a3700,0x642c58),outer=early_outer?early_outer:make_mutex(0x4e8f6f,0x6e1ec0);(void)inner;stage(3);
 put(0x642c64,1);put(0x642c60,1);put(0x6c5038,1);put(0x5e04c8,0);put(0x6de6d5,0);put(0x6de6dd,1);put(0x6f68d8,1);put(0x6e1ee4,1);put(0x641a8c,2);put(0x642c50,2);put(0x641fc0,0);put(0x642c5c,0);
 for(u32 k=0;k<2;++k){put(0x642678+k*68,2+k);put(0x64267c+k*68,1+k);put(0x642680+k*68,1);put(0x642684+k*68,0);put(0x6426b8+k*68,1);}
 put(0x641a90,0);put(0x641a94,0);put(0x641a98,40);put(0x641a9c,40);put(0x642c54,0);
 ((void(*)(void))0x4a45b0)();need(get(0x6f98d4)==2,214);stage(4);
 typedef i32(WIN* Pin)(void*,u32*);typedef i32(WIN* Unpin)(u32);Pin pin=(Pin)GetProcAddress(module,"NativeDrawPinV1");Unpin unpin=(Unpin)GetProcAddress(module,"NativeDrawUnpinV1");
 void* old=(void*)get(0x6a49e8);u32 inspection=0;need(pin&&!pin(old,&inspection),215);
 drop(0x641fc8);drop(0x642000);drop(0x6a49e0);
 wrap(0x641fc8,40,40);wrap(0x642000,80,80);wrap(0x6a49e0,640,480);fill(0x642000,80,80,0x3300,1);fill(0x6a49e0,640,480,0x9900,0);
 {u32 key[2]={31,31};void* a=(void*)get(0x642008);need(!((i32(WIN*)(void*,u32,u32*))gl_vt(a)[29])(a,8,key),232);put(0x64202c,31);}
 stage(5);drain();need(!get(0x6f98d4),216);compare(old,640,480,0x5500,0x1100,7,6,1);compare((void*)get(0x6a49e8),640,480,0x9900,0,0,0,0);need(!unpin(inspection),217);stage(6);
 // A busy external writer retains both backing and the queued operation.
 u32 rect[4]={1,1,4,4},desc[31];gl_desc(desc,0);void* target=(void*)get(0x6a49e8);need(!gl_lock(target,1,desc),218);
 enqueue(0x642000,0x6a49e0,2,rect,0,0);drain();need(get(0x6f98d4)==1,219);need(!gl_unlock(target,1,desc[9]),220);drain();need(!get(0x6f98d4),221);stage(7);
 // Actual immediate foreground save/paint and both recovered background clients.
 fill(0x6a49e0,640,480,0x9900,0);put(0x6e1ee4,0);((void(*)(void))0x4a45b0)();compare(target,640,480,0x9900,0x3300,7,6,1);
 ((void(*)(void))0x4a4380)();compare(target,640,480,0x9900,0,0,0,0);
 put(0x642c54,0);put(0x641aa0,0xffffffffu);put(0x641aa4,0xffffffffu);((void(*)(void))0x4a4910)();compare((void*)get(0x6a4fe8),640,480,0x7700,0x3300,7,6,1);
 ((void(*)(void))0x4a44b0)();compare((void*)get(0x6a4fe8),640,480,0x7700,0,0,0,0);
 stage(9);((void(__attribute__((fastcall))*)(u32))0x4a39a0)(1);need(get(0x5e04c8)==1,222);((void(*)(void))0x4a4370)();need(!get(0x642c54),223);
 // Queue saturation owns exactly256 operand pins; cancellation is terminal.
 for(u32 i=0;i<128;++i)enqueue(0x642000,0x6a49e0,2,rect,0,0);need(get(0x6f98d4)==128,224);enqueue(0x642000,0x6a49e0,2,rect,0,0);need(get(0x6f98d4)==128,225);
 u32(WIN* stop)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawCursorStopV1");u32(WIN* status)(u32*,u32)=(u32(WIN*)(u32*,u32))GetProcAddress(module,"NativeDrawCursorStatusV1");
 need(stop&&status&&stop()&&!get(0x6f98d4),226);stage(10);((void(*)(void))0x4a45b0)();need(!get(0x6f98d4),227);
 ((void(*)(void))0x4a37d0)();need(!get(0x642c58),228);((void(*)(void))0x58acf0)();gl_counts(module,0,0,0,0,0);stage(11);
 need(((u32(WIN*)(HANDLE))get(0x5c50a0))(outer),229);put(0x6e1ec0,0);
 u32 words[28];need(status(words,sizeof(words))&&words[1]&&words[2]&&!words[5]&&!words[6]&&!words[7]&&!words[8]&&!words[9]&&!words[10],230);
 need(words[12]==words[13]+words[14]&&words[12]>=131&&words[15]==1&&words[17]>=1&&words[19]>=1&&words[21]==1,231);
 stage(12);u32 result[32]={1,checks,planes,18};for(u32 i=0;i<28;++i)result[4+i]=words[i];gl_save("cursor-owner.bin",result,sizeof(result));ExitProcess(0);
}
