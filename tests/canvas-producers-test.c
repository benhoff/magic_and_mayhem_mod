/* Actual entry/return trampolines against compatible synthetic PE32 prologues.
   ABI/error/floating-point preservation and atomic signature refusal only. */
#define MNM_SCENE_SELFTEST 1
#include "../runtime/scene/canvas_producers.c"
extern void canvas_test_call(u32,u32,u32,u32,void*);
extern void world_bypass_test_call(u32,u32,u32,u32,void*,u32);
static volatile u8 reserve[0x310000];
static u8 before[HOOKS][560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16))),bypass_before[560] __attribute__((aligned(16)));
static u8 raster_before[13][560] __attribute__((aligned(16)));
static void oracle_config_fixture(void){
 const char* invalid[]={"","0","3073","4096","-1","1x","99999"};
 for(u32 i=0;i<7;++i){u32 n=0;while(invalid[i][n])++n;if(oracle_configure(invalid[i],n))ExitProcess(115);}
 if(!oracle_configure("1",1)||oracle_byte_limit!=1048576u||!oracle_configure("3072",4)||oracle_byte_limit!=3221225472u)ExitProcess(116);
 oracle_byte_limit=1073741824u;
}
static void oracle_budget_fixture(void){
 struct Surface* s=surfaces;u32 saved_bytes=oracle_bytes,saved_limit=oracle_byte_limit,saved_dirty=s->dirty,saved_sampled=s->sampled;
 u32 count=oracle,errors=failures;s->dirty=s->sampled+1;
 oracle_byte_limit=1073741824u;oracle_bytes=oracle_byte_limit-31;
 checkpoint(s,5);if(oracle!=count||failures!=errors+1||s->dirty==s->sampled)ExitProcess(117);
 if(!oracle_configure("3072",4))ExitProcess(118);
 checkpoint(s,5);if(oracle!=count+1||oracle_bytes!=1073741825u||s->dirty!=s->sampled)ExitProcess(119);
 ++s->dirty;oracle_bytes=oracle_byte_limit-32;
 checkpoint(s,5);if(oracle!=count+2||oracle_bytes!=oracle_byte_limit||s->dirty!=s->sampled)ExitProcess(120);
 ++s->dirty;checkpoint(s,5);if(oracle!=count+2||failures!=errors+2||s->dirty==s->sampled)ExitProcess(121);
 oracle_bytes=oracle_byte_limit+1;checkpoint(s,5);if(oracle!=count+2||failures!=errors+3)ExitProcess(122);
 oracle_bytes=saved_bytes+64;oracle_byte_limit=saved_limit;s->dirty=saved_dirty;s->sampled=saved_sampled;
}
static void bypass_fixture(void){
 struct Surface* s=surfaces;s->object=0x700100;s->id=99;s->width=s->height=s->stride=4;s->pixels=0x700000;
 put((void*)0x658174,s->pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,4);put((void*)0x656618,4);
 u32 frame=0x700200,node=0x710000;zero((void*)frame,64);put((void*)frame,44);put((void*)(frame+4),1);put((void*)(frame+8),1);put((void*)(frame+28),node);put((void*)node,node);put((void*)(node+4),0);put((void*)(node+8),0);
 zero((void*)(node+12),4096);queue=1;depth=0;
 SetLastError(1234);world_bypass_test_call(hooks[13].address,0x700100,4,0,bypass_before,frame);
 // If the original synthetic body executes, this counter increments. The
 // selected bypass must preserve the original caller's incoming state instead.
 u8* tail=(u8*)hooks[13].address+hooks[13].length;tail[0]=0xff;tail[1]=0x05;put(tail+2,0x700800);tail[6]=0x5b;tail[7]=0x5f;tail[8]=0x5e;tail[9]=0x5d;tail[10]=0xc3;put((void*)0x700800,0);
 u32 header[16]={0};u16 pixels[16];for(u32 i=0;i<16;++i)pixels[i]=0x4a4a;
 copy(header,MNM_WORLD_BYPASS_REPLY,8);header[2]=1;header[3]=64;header[4]=1;header[5]=1;header[6]=sequence+1;header[7]=99;header[8]=header[9]=header[10]=4;header[11]=32;header[12]=bypass_hash((u8*)pixels,32);header[13]=1;header[14]=hooks[13].address;
 char name[260];path(name,"world-bypass-",1,".reply");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1||!write(file,header,64)||!write(file,pixels,32)||!CloseHandle(file))ExitProcess(80);
 bypass_limit=bypass_queue=1;bypass_completed=0;
 SetLastError(1234);world_bypass_test_call(hooks[13].address,0x700100,4,0,after,frame);
 if(GetLastError()!=1234||get((void*)0x700800)||bypass_completed!=1)ExitProcess(81);
 for(u32 i=0;i<560;++i)if(bypass_before[i]!=after[i])ExitProcess(82);
 for(u32 i=0;i<16;++i)if(((u16*)s->pixels)[i]!=pixels[i])ExitProcess(83);
 // Wrong request identity and bad payload checksum refuse before writeback.
 u32 r[24]={0};r[1]=header[6];r[3]=99;r[5]=r[6]=4;bypass_completed=0;header[6]++;
 for(u32 trial=0;trial<2;++trial){
  file=CreateFileA(name,0x40000000,0,0,2,0x80,0);if(file==(HANDLE)-1||!write(file,header,64)||!write(file,pixels,32)||!CloseHandle(file))ExitProcess(84);
  ((u16*)s->pixels)[0]=0x1234;if(bypass_reply(r,s,hooks[13].address)||((u16*)s->pixels)[0]!=0x1234||bypass_completed)ExitProcess(85);
  header[6]--;header[12]^=1;
 }
 bypass_limit=0;
}
static void raster_fixture(void){
 struct Surface* surface=surfaces;put((void*)(0x710000+4),7);put((void*)0x5f14d0,0);queue=0;depth=0;limit=2;raster_first=raster_last=1;
 u32 return_registers[16]={0};return_registers[9]=0x700900;canvas_producers_queue(return_registers);canvas_producers_protect();
 const u32 tags[]={12,13,14,15,16,17,18,19,20,21,23,24,25};
 for(u32 k=0;k<sizeof(tags)/sizeof(tags[0]);++k){
  const u32 tag=tags[k],fast=(tag>=18&&tag<=21)||tag==25;u32 ecx=fast?0x700200:0x700100;
  raster_first=raster_last=0;bypass_completed=k;
  u8 saved[32];u8* tail=(u8*)hooks[tag].address+hooks[tag].length;copy(saved,tail,32);tail[0]=0x9c;tail[1]=0xff;tail[2]=0x05;put(tail+3,0x700804);tail[7]=0x9d;copy(tail+8,saved,32);
  SetLastError(1234);world_bypass_test_call(hooks[tag].address,ecx,4,hooks[tag].pop,bypass_before,0x700200);
  copy(raster_before[k],bypass_before,560);
  u32 counter=get((void*)0x700804);
  u32 header[16]={0};u16 pixels[16];for(u32 j=0;j<16;++j)pixels[j]=(u16)(0x1000+tag);
  copy(header,MNM_WORLD_RASTER_REPLY,8);header[2]=2;header[3]=64;header[4]=k+1;header[5]=1;header[6]=sequence+1;header[7]=99;header[8]=header[9]=header[10]=4;header[11]=32;header[12]=bypass_hash((u8*)pixels,32);header[13]=1;header[14]=hooks[tag].address;header[15]=tag==12||tag==23||tag==24?1:0;
  raster_first=raster_last=1;char name[260];bypass_path(name,k+1,".reply");HANDLE file=CreateFileA(name,0x40000000,0,0,2,0x80,0);if(file==(HANDLE)-1||!write(file,header,64)||!write(file,pixels,32)||!CloseHandle(file))ExitProcess(86);
  SetLastError(1234);world_bypass_test_call(hooks[tag].address,ecx,4,hooks[tag].pop,after,0x700200);
  if(get((void*)0x700804)!=counter||GetLastError()!=1234||bypass_completed!=k+1||*(u32*)(after+28)!=header[15]||*(u32*)(after+32)!=0x247)ExitProcess(87);
  for(u32 j=0;j<560;++j)if((j<28||j>=36)&&bypass_before[j]!=after[j])ExitProcess(88);
  for(u32 j=0;j<16;++j)if(((u16*)surface->pixels)[j]!=pixels[j])ExitProcess(89);
 }
 /* Exercise the actual World return callback, then a HUD raster while the
  * selected queue ordinal remains1. The original body must forward. */
 raster_first=raster_last=1;
 producer_leave_observe(return_registers);
 if(raster_world_open||raster_active())ExitProcess(90);
 u32 counter=get((void*)0x700804),completed=bypass_completed;
 SetLastError(1234);world_bypass_test_call(hooks[24].address,0x700100,4,0,after,0x700200);
 if(get((void*)0x700804)!=counter+1||bypass_completed!=completed||GetLastError()!=1234)ExitProcess(91);
 raster_first=raster_last=0;
}
static void batch_fixture(void){
 struct Surface* s=surfaces;u32 p=(u32)VirtualAlloc(0,960000,0x3000,4);if(!p||!AddVectoredExceptionHandler(1,(void*)batch_fault))ExitProcess(100);
 s->pixels=p;s->width=s->stride=800;s->height=600;s->dirty=s->sampled=0;put((void*)0x658174,p);
 for(u32 i=0;i<480000;++i)((u16*)p)[i]=0x4321;
 queue=1;depth=0;limit=3;raster_first=raster_last=2;batch_enabled=1;bypass_completed=batch_transfers=0;
 u32 ret[16]={0};ret[9]=0x700900;canvas_producers_queue(ret);canvas_producers_protect();if(!batch_guard_open)ExitProcess(101);
 char mode[16]={0};GetEnvironmentVariableA("MNM_BATCH_FIXTURE",mode,sizeof(mode));
 if(mode[0]=='r'){volatile u16 value=*(volatile u16*)p;(void)value;ExitProcess(102);}
 if(mode[0]=='w'){*(volatile u16*)p=0x7777;ExitProcess(102);}
 if(mode[0]=='s'){world_bypass_test_call(hooks[13].address,0x700100,4,0,after,p);ExitProcess(102);}
 if(mode[0]=='p'){canvas_test_call(hooks[5].address,0x700100,4,hooks[5].pop,after);ExitProcess(102);}
 const u32 tags[]={12,13,14,15,16,17,18,19,20,21,23,24,25};
 for(u32 k=0;k<13;++k){const u32 tag=tags[k],fast=(tag>=18&&tag<=21)||tag==25,counter=get((void*)0x700804);
  SetLastError(1234);world_bypass_test_call(hooks[tag].address,fast?0x700200:0x700100,4,hooks[tag].pop,after,0x700200);
  if(get((void*)0x700804)!=counter||GetLastError()!=1234||bypass_completed!=k+1||batch_count!=k+1||batch_transfers||!batch_guard_open||*(u32*)(after+28)!=(tag==12||tag==23||tag==24?1u:0u))ExitProcess(103);
  if(*(u32*)(after+32)!=0x247)ExitProcess(113);for(u32 j=0;j<560;++j)if((j<28||j>=36)&&after[j]!=raster_before[k][j])ExitProcess(114);
 }
 u32 request[16];batch_header(request,s);if(!batch_restore()||*(u16*)p!=0x4321)ExitProcess(104);
 u16* pixels=HeapAlloc(GetProcessHeap(),0,960000);if(!pixels)ExitProcess(105);for(u32 i=0;i<480000;++i)pixels[i]=0x5678;
 u32 h[16];copy(h,request,64);copy(h,MNM_WORLD_BATCH_REPLY,8);h[11]=960000;h[12]=bypass_hash((u8*)pixels,960000);char name[260];path(name,"world-batch-",2,".reply");
 for(u32 k=0;k<3;++k){u32 bad[16];copy(bad,h,64);if(k==0)++bad[6];else if(k==1)bad[12]^=1;else ++bad[15];HANDLE f=CreateFileA(name,0x40000000,0,0,2,0x80,0);if(f==(HANDLE)-1||!write(f,bad,64)||!write(f,pixels,960000)||!CloseHandle(f))ExitProcess(106);if(batch_reply(s,request)||*(u16*)p!=0x4321||batch_transfers)ExitProcess(107);}
 HANDLE f=CreateFileA(name,0x40000000,0,0,2,0x80,0);if(f==(HANDLE)-1||!write(f,h,64)||!write(f,pixels,960000)||!CloseHandle(f))ExitProcess(108);
 if(!VirtualProtect((void*)batch_guard_base,batch_guard_bytes,1,&batch_guard_old))ExitProcess(109);batch_guard_open=1;
 SetLastError(1234);producer_leave_observe(ret);
 if(GetLastError()!=1234||batch_guard_open||batch_transfers!=1||raster_world_open||bypass_completed!=13)ExitProcess(110);
 for(u32 i=0;i<480000;++i)if(((u16*)p)[i]!=0x5678)ExitProcess(111);
 u32 completed=bypass_completed,counter=get((void*)0x700804);world_bypass_test_call(hooks[24].address,0x700100,4,0,after,0x700200);
 if(bypass_completed!=completed||get((void*)0x700804)!=counter+1)ExitProcess(112);
 batch_enabled=0;raster_first=raster_last=0;HeapFree(GetProcessHeap(),0,pixels);
}
static void tail(u8* p,u32 tag){
 const u8* bytes=0;u32 n=0;
 static const u8 release[]={0x5f,0x5e,0x5b},lock[]={0x83,0xc4,8,0xb8,0,0,0x70,0},fill[]={0x5e,0x83,0xc4,0x6c},copies[]={0x5d,0x5b,0x83,0xc4,0x30},jpeg[]={0x5f,0x5e},font[]={0x83,0xc4,0x28},slow[]={0x5d,0x5b,0x83,0xc4,0x14},normal[]={0x5b,0x5f,0x5e,0x5d},clipped[]={0x83,0xc4,0x18,0x5d},half[]={0x5e,0x5d,0x5b,0x83,0xc4,0x1c},quarter[]={0x5e,0x5d,0x5b,0x83,0xc4,0x24},wave[]={0x83,0xc4,0x20},shadow[]={0x83,0xc4,0x18};
#define T(name) bytes=name;n=sizeof(name)
 if(tag==1){T(release);}else if(tag==2){T(lock);}else if(tag==5||tag==6){T(fill);}else if(tag==7||tag==8){T(copies);}else if(tag==9){T(jpeg);}else if(tag==10){T(font);}else if(tag==11){T(slow);}else if(tag==17){T(clipped);}else if(tag==18){T(half);}else if(tag==19||tag==20){T(quarter);}else if(tag==21){T(wave);}else if(tag==25){T(shadow);}else if(tag==30){static const u8 pcx[]={0x5f,0x5b,0x83,0xc4,0x18};T(pcx);}else if(tag==31){static const u8 dib[]={0x5e,0x59};T(dib);}else if(tag==32||tag==33){u8 close[]={0x5b,0x83,0xc4,tag==32?0x18:0x20};for(u32 i=0;i<4;++i)*p++=close[i];}else if(tag==34){static const u8 word[]={0x83,0xc4,0x1c};T(word);}else if(tag==35){static const u8 bmp[]={0x83,0xc4,8};T(bmp);}else if(tag==36){T(normal);}else if(tag==37||tag==38){static const u8 partial[]={0x83,0xc4,0x20};T(partial);}else if(tag==39){static const u8 keyed[]={0x5b,0x83,0xc4,0x18};T(keyed);}else if(tag==40){static const u8 bmp0[]={0x83,0xc4,8};T(bmp0);}else if(tag==41){static const u8 key[]={0x83,0xc4,0x10};T(key);}else if(tag==46){static const u8 outline[]={0x5f,0x5e,0x5d,0x5b,0x83,0xc4,0x28};T(outline);}else if(tag==47){static const u8 mark[]={0x5b};T(mark);}else if(tag==48){static const u8 units[]={0x5d,0x5b,0x83,0xc4,8};T(units);}else if(tag==45){static const u8 map[]={0x5e,0x83,0xc4,0x20};T(map);}else if(tag==44){static const u8 fade[]={0x5e,0x5d,0x59};T(fade);}else if(tag>=42){static const u8 rect[]={0x5b,0x83,0xc4,0x20};T(rect);}else if(tag>=27){u8 close[]={0x83,0xc4,tag==27?0x40:tag==28?0x38:0x24};for(u32 i=0;i<3;++i)*p++=close[i];}else if(tag==26){static const u8 add[]={0x83,0xc4,0x1c};T(add);}else if(tag>=12){T(normal);}
 for(u32 i=0;i<n;++i)*p++=bytes[i];if(hooks[tag].pop){*p++=0xc2;*p++=(u8)hooks[tag].pop;*p=0;}else *p=0xc3;
}
void start(void){
 oracle_config_fixture();
 reserve[sizeof(reserve)-1]=1;u32 old;
 if((u32)GetModuleHandleA(0)!=0x400000||!VirtualProtect((void*)0x480000,0x120000,0x40,&old))ExitProcess(1);
 for(u32 i=0;i<HOOKS;++i){copy((void*)hooks[i].address,hooks[i].bytes,hooks[i].length);tail((u8*)(hooks[i].address+hooks[i].length),i);}
 u32* surface=(u32*)0x700100;surface[4]=4;surface[5]=4;surface[6]=8;
 for(u32 i=0;i<HOOKS;++i){SetLastError(1234);canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,before[i]);if(GetLastError()!=1234)ExitProcess(2);}
 *(u8*)hooks[HOOKS-1].address^=1;
 if(canvas_producers_install()||*(u8*)hooks[0].address!=hooks[0].bytes[0])ExitProcess(3);
 *(u8*)hooks[HOOKS-1].address^=1;
 if(!canvas_producers_install()){
  for(u32 i=0;i<HOOKS;++i)for(u32 j=0;j<hooks[i].length;++j)if(*(u8*)(hooks[i].address+j)!=hooks[i].bytes[j])ExitProcess(123);
  ExitProcess(4);
 }
 for(u32 i=0;i<HOOKS;++i){SetLastError(1234);canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,after);if(GetLastError()!=1234)ExitProcess(5);for(u32 j=0;j<560;++j)if(before[i][j]!=after[j])ExitProcess(20+i);}
 bypass_fixture();
 raster_fixture();
 oracle_budget_fixture();
 batch_fixture();
 canvas_producers_close();
 for(u32 i=0;i<HOOKS;++i)canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,after);
 ExitProcess(0);
}
