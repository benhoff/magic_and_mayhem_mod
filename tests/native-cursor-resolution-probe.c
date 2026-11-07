/* Real owning installer/provider and unchanged original cursor bodies. */
#define start cursor_existing_owner_start
#include "native-cursor-owner-probe.c"
#undef start
static void clear_background(void){put(0x642c54,0);put(0x641aa0,0x80000000u);put(0x641aa4,0x80000000u);}
static void foreground(void){((void(*)(void))0x4a45b0)();}
static void timer_cursor(void){((void(*)(void))0x4a4910)();}
static void restore_back(void){((void(*)(void))0x4a4380)();}
static u32 WIN cursor_mode_flip(u32* p){put(0x6de6d5,1);return cursor(p);}
void start(void){
 original_map();bind_imports();stage(0);u32 native=0;void* module=gl_module(&native);need(native,301);
 char early_flag[2];HANDLE early_outer=0;
 if(GetEnvironmentVariableA("MNM_CURSOR_EARLY_OUTER",early_flag,2))early_outer=make_mutex(0x4e8f6f,0x6e1ec0);
 char bad[4];u32 bad_n=GetEnvironmentVariableA("MNM_CURSOR_BAD_ANCHOR",bad,sizeof(bad));
 if(bad_n){u32 index=(u32)(bad[0]-'0');if(bad_n==2)index=index*10+(u32)(bad[1]-'0');
  const u32 sites[18]={0x4a30b0,0x4a37d0,0x4a39a0,0x4a4370,0x4a4380,0x4a44b0,0x4a45b0,0x4a4910,0x4a2f90,0x58d5b0,0x58d660,0x58ad90,0x58b3e0,0x58c140,0x58c360,0x58c4a0,0x58c8a0,0x58ca90};
  need(index<18,302);*(u8*)sites[index]^=1;
 }
 GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");void* draw=0;need(create&&!create(0,&draw,0)&&draw,303);
 if(bad_n)ExitProcess(304);
 need(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),305);
 const u8 dd2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
 const u8 dd4[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
 put(0x6f98c4,(u32)draw);put(0x6f98c8,(u32)gl_query(draw,dd2));put(0x6f98cc,(u32)gl_query(draw,dd4));
 put(0x6f98d0,0);put(0x6e1f1c,0);put(0x6f8154,0);put(0x6f98c0,0);
 wrap(0x641fc8,40,40);wrap(0x642000,80,80);wrap(0x6a49e0,640,480);wrap(0x6a4fe0,640,480);put(0x6f98c0,0x6a4fe0);
 fill(0x642000,80,80,0x1100,1);
 {u32 key[2]={31,31};void* a=(void*)get(0x642008);need(!((i32(WIN*)(void*,u32,u32*))gl_vt(a)[29])(a,8,key),306);put(0x64202c,31);}
 make_mutex(0x4a3700,0x642c58);HANDLE outer=early_outer?early_outer:make_mutex(0x4e8f6f,0x6e1ec0);
 put(0x642c64,1);put(0x642c60,1);put(0x6c5038,1);put(0x5e04c8,0);put(0x6de6dd,1);put(0x6f68d8,1);put(0x6e1ee4,0);
 put(0x641a8c,2);put(0x642c50,2);put(0x641fc0,0);put(0x642c5c,0);
 put(0x642678,0);put(0x64267c,0);put(0x642680,1);put(0x642684,0);put(0x6426b8,1);
 put(0x641a90,0);put(0x641a94,0);put(0x641a98,40);put(0x641a9c,40);
 void* back=(void*)get(0x6a49e8),*front=(void*)get(0x6a4fe8);
 // First regression is safely inside both modes: old code paints despite a
 // mismatched clip mode. Fail against a complete independently built plane.
 clear_background();px=9;py=7;put(0x6de6d5,1);fill(0x6a49e0,640,480,0x5500,0);
 foreground();compare(back,640,480,0x5500,0,0,0,0);need(!get(0x642c54),307);stage(1);
 // Exact retained off-screen requests must never reach a copy wrapper.
 px=640;py=512;foreground();need(!get(0x642c54)&&!get(0x6f98d4),308);
 compare(back,640,480,0x5500,0,0,0,0);
 fill(0x6a4fe0,640,480,0x7700,0);timer_cursor();compare(front,640,480,0x7700,0,0,0,0);stage(2);
 // Once the mode settles, stationary single-frame cursors repaint normally.
 put(0x6de6d5,0);px=9;py=7;foreground();compare(back,640,480,0x5500,0x1100,9,7,1);
 restore_back();compare(back,640,480,0x5500,0,0,0,0);clear_background();
 timer_cursor();compare(front,640,480,0x7700,0x1100,9,7,1);
 ((void(*)(void))0x4a44b0)();compare(front,640,480,0x7700,0,0,0,0);stage(3);
 // Stale high-resolution saved offsets while live targets are low: abort the
 // cursor transaction, clear its underlay and make a stationary timer retry.
 put(0x642c54,1);put(0x641a78,640);put(0x641a7c,512);put(0x641a80,680);put(0x641a84,552);
 put(0x641aa0,640);put(0x641aa4,512);px=9;py=7;timer_cursor();
 need(!get(0x642c54),309);compare(front,640,480,0x7700,0,0,0,0);
 timer_cursor();compare(front,640,480,0x7700,0x1100,9,7,1);
 ((void(*)(void))0x4a44b0)();compare(front,640,480,0x7700,0,0,0,0);stage(4);
 // A queued cursor must follow the same mode admission as immediate drawing.
 put(0x6e1ee4,1);put(0x6de6d5,1);clear_background();foreground();need(!get(0x6f98d4),310);
 put(0x6de6d5,0);foreground();need(get(0x6f98d4)==2,311);drain();need(!get(0x6f98d4),312);
 compare(back,640,480,0x5500,0x1100,9,7,1);restore_back();drain();compare(back,640,480,0x5500,0,0,0,0);stage(5);
 // Deferred stale restore: neither the invalid job nor a stale valid flag is
 // published, and the next ordinary save/paint pair still drains normally.
 put(0x642c54,1);put(0x641a78,640);put(0x641a7c,512);put(0x641a80,680);put(0x641a84,552);
 put(0x641aa0,640);put(0x641aa4,512);restore_back();need(!get(0x6f98d4)&&!get(0x642c54),318);
 compare(back,640,480,0x5500,0,0,0,0);foreground();drain();compare(back,640,480,0x5500,0x1100,9,7,1);
 restore_back();drain();compare(back,640,480,0x5500,0,0,0,0);put(0x6e1ee4,0);
 // General COM callers retain the exact strict invalid-rectangle result.
 {i32 rect[4]={0,0,40,40};i32 result=((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))gl_vt(back)[7])(back,640,512,(void*)get(0x642008),rect,0x10);
  need((u32)result==0x88760096u,319);compare(back,640,480,0x5500,0,0,0,0);}
 const i32 positions[4][2]={{0,0},{638,478},{-3,-2},{639,100}};
 for(u32 i=0;i<4;++i){clear_background();px=(u32)positions[i][0];py=(u32)positions[i][1];foreground();
  compare(back,640,480,0x5500,0x1100,positions[i][0],positions[i][1],1);restore_back();compare(back,640,480,0x5500,0,0,0,0);}
 // A controlled callback changes mode after entry admission. Per-copy checks
 // still abort the unsafe transaction before any original copy API is called.
 clear_background();px=640;py=512;put(0x5c5210,(u32)cursor_mode_flip);timer_cursor();put(0x5c5210,(u32)cursor);
 need(!get(0x642c54)&&get(0x641aa0)==0x80000000u&&get(0x641aa4)==0x80000000u,320);
 compare(front,640,480,0x7700,0,0,0,0);
 // The compatible high-resolution backing resumes real clipped cursor pixels.
 drop(0x6a49e0);drop(0x6a4fe0);wrap(0x6a49e0,800,600);wrap(0x6a4fe0,800,600);
 back=(void*)get(0x6a49e8);fill(0x6a49e0,800,600,0x6600,0);clear_background();px=790;py=590;
 foreground();compare(back,800,600,0x6600,0x1100,790,590,1);restore_back();compare(back,800,600,0x6600,0,0,0,0);stage(6);
 u32(WIN* stop)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawCursorStopV1");
 need(stop&&stop(),313);((void(*)(void))0x4a37d0)();((void(*)(void))0x58acf0)();
 gl_counts(module,0,0,0,0,0);need(((u32(WIN*)(HANDLE))get(0x5c50a0))(outer),314);put(0x6e1ec0,0);
 u32 words[28];u32(WIN* status)(u32*,u32)=(u32(WIN*)(u32*,u32))GetProcAddress(module,"NativeDrawCursorStatusV1");
 need(status&&status(words,sizeof(words))&&words[1]&&words[2],315);
 for(u32 k=5;k<=10;++k)need(!words[k],316);
 need(words[12]==words[13]+words[14]&&words[26]>=7,317);
 u32 result[32]={1,checks,planes,18};for(u32 i=0;i<28;++i)result[i+4]=words[i];
 gl_save("cursor-resolution.bin",result,sizeof(result));stage(7);ExitProcess(0);
}
