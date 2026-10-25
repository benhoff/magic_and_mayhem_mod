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
static void metadata_checkpoint_fixture(void){
 struct Surface* s=surfaces;struct Surface saved=*s;
 HANDLE saved_file=file;file=0;u32 saved_session=session_producers,saved_oracle=oracle,saved_bytes=oracle_bytes,saved_failures=failures;
 s->id=2;s->width=s->height=s->stride=4;s->pixels=1;s->dirty=s->sampled+1;
 session_producers=1;oracle=9999;oracle_bytes=oracle_byte_limit+1;
 checkpoint(s,5);if(failures!=saved_failures||oracle!=10000||s->sampled!=s->dirty||oracle_bytes!=oracle_byte_limit+1)ExitProcess(126);
 *s=saved;file=saved_file;session_producers=saved_session;oracle=saved_oracle;oracle_bytes=saved_bytes;
}
static void bypass_fixture(void){
 generic_bypass_fixture=1;
 struct Surface* s=surfaces;s->object=0x700100;s->id=99;s->width=s->height=s->stride=4;s->pixels=0x700000;
 put((void*)0x658174,s->pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,4);put((void*)0x656618,4);
 u32 frame=0x700200,node=0x710000;zero((void*)frame,64);put((void*)frame,44);put((void*)(frame+4),1);put((void*)(frame+8),1);put((void*)(frame+28),node);put((void*)node,node);put((void*)(node+4),0);put((void*)(node+8),0);
 put((void*)(frame+12),5);put((void*)(frame+16),6);
 zero((void*)(node+12),4096);queue=1;depth=0;
 char caller_mode[8]={0};GetEnvironmentVariableA("MNM_GENERIC_CALLER_FIXTURE",caller_mode,sizeof(caller_mode));
 if(caller_mode[0]){
  generic_bypass_fixture=0;bypass_limit=bypass_queue=1;bypass_completed=0;
  const u8 continuation[]={0x83,0xc4,0x14,0x66,0x3d,1,0,0x0f,0x85,0x29};
  copy((void*)0x500512,continuation,10);
  u32 regs[18]={0};regs[9]=caller_mode[0]=='u'?0x500513:0x500512;regs[10]=frame;
  if(caller_mode[0]=='m')*(u8*)0x500512^=1;
  /* A null source context is deliberate: caller rejection must occur first. */
  producer_enter_observe(regs,13,0);ExitProcess(124);
 }
 SetLastError(1234);world_bypass_test_call(hooks[13].address,0x700100,4,0,bypass_before,frame);
 // If the original synthetic body executes, this counter increments. The
 // Selected bypass preserves incoming state except the recovered zero EAX result.
 u8* tail=(u8*)hooks[13].address+hooks[13].length;tail[0]=0xff;tail[1]=0x05;put(tail+2,0x700800);tail[6]=0x5b;tail[7]=0x5f;tail[8]=0x5e;tail[9]=0x5d;tail[10]=0xc3;put((void*)0x700800,0);
 u32 header[16]={0};u16 pixels[16];for(u32 i=0;i<16;++i)pixels[i]=0x4a4a;
 copy(header,MNM_WORLD_BYPASS_REPLY,8);header[2]=1;header[3]=64;header[4]=1;header[5]=1;header[6]=sequence+1;header[7]=99;header[8]=header[9]=header[10]=4;header[11]=32;header[12]=bypass_hash((u8*)pixels,32);header[13]=1;header[14]=hooks[13].address;
 char name[260];path(name,"world-bypass-",1,".reply");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1||!write(file,header,64)||!write(file,pixels,32)||!CloseHandle(file))ExitProcess(80);
 bypass_limit=bypass_queue=1;bypass_completed=0;
 SetLastError(1234);world_bypass_test_call(hooks[13].address,0x700100,4,0,after,frame);
 if(GetLastError()!=1234||get((void*)0x700800)||bypass_completed!=1)ExitProcess(81);
 if(*(u32*)(after+28)!=0)ExitProcess(82);
 for(u32 i=0;i<560;++i)if((i<28||i>=32)&&bypass_before[i]!=after[i])ExitProcess(82);
 for(u32 i=0;i<16;++i)if(((u16*)s->pixels)[i]!=pixels[i])ExitProcess(83);
 // Wrong request identity and bad payload checksum refuse before writeback.
 u32 r[24]={0};r[1]=header[6];r[3]=99;r[5]=r[6]=4;bypass_completed=0;header[6]++;
 for(u32 trial=0;trial<2;++trial){
  file=CreateFileA(name,0x40000000,0,0,2,0x80,0);if(file==(HANDLE)-1||!write(file,header,64)||!write(file,pixels,32)||!CloseHandle(file))ExitProcess(84);
  ((u16*)s->pixels)[0]=0x1234;if(bypass_reply(r,s,hooks[13].address)||((u16*)s->pixels)[0]!=0x1234||bypass_completed)ExitProcess(85);
  header[6]--;header[12]^=1;
 }
 bypass_limit=0;generic_bypass_fixture=0;put((void*)(frame+12),0);put((void*)(frame+16),0);
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
static void journal_fixture(void){
 HANDLE saved_file=file;u32 saved_limit=limit,saved_stop=stopped;limit=32;stopped=0;
 char name[260];path(name,"journal-buffer-",1,".bin");file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(124);
 u8* pattern=HeapAlloc(producer_heap(),0,65549);if(!pattern)ExitProcess(125);for(u32 i=0;i<65549;++i)pattern[i]=(u8)(i*17);
 if(!journal_write(pattern,7)||journal_used!=7||GetFileSize(file,0)!=0||!journal_write(pattern+7,65542)||journal_used!=13||GetFileSize(file,0)!=65536||!journal_flush()||journal_used||GetFileSize(file,0)!=65549)ExitProcess(126);
 CloseHandle(file);file=CreateFileA(name,0x80000000,1,0,3,0x80,0);u8* readback=HeapAlloc(producer_heap(),0,65549);if(!readback||!bypass_read(file,readback,65549)||!bypass_equal(pattern,readback,65549))ExitProcess(127);CloseHandle(file);
 file=(HANDLE)-1;if(!journal_write(pattern,7)||journal_flush()||!journal_failed||!stopped||journal_write(pattern,1))ExitProcess(128);
 journal_used=journal_failed=0;file=saved_file;limit=saved_limit;stopped=saved_stop;HeapFree(producer_heap(),0,pattern);HeapFree(producer_heap(),0,readback);
}
static void tail(u8* p,u32 tag){
 const u8* bytes=0;u32 n=0;
 static const u8 release[]={0x5f,0x5e,0x5b},lock[]={0x83,0xc4,8,0xb8,0,0,0x70,0},fill[]={0x5e,0x83,0xc4,0x6c},copies[]={0x5d,0x5b,0x83,0xc4,0x30},jpeg[]={0x5f,0x5e},font[]={0x83,0xc4,0x28},slow[]={0x5d,0x5b,0x83,0xc4,0x14},normal[]={0x5b,0x5f,0x5e,0x5d},clipped[]={0x83,0xc4,0x18,0x5d},half[]={0x5e,0x5d,0x5b,0x83,0xc4,0x1c},quarter[]={0x5e,0x5d,0x5b,0x83,0xc4,0x24},wave[]={0x83,0xc4,0x20},shadow[]={0x83,0xc4,0x18};
#define T(name) bytes=name;n=sizeof(name)
 if(tag==57){static const u8 effect_add[]={0x5d,0x5b,0x83,0xc4,0x30};T(effect_add);}
 else if(tag==58){static const u8 effect_outline[]={0x5f,0x5e,0x5d,0x5b,0x83,0xc4,0x20};T(effect_outline);}
 else if(tag==59){static const u8 effect_tint[]={0x5e,0x5d,0x5b,0x83,0xc4,0x18};T(effect_tint);}
 else if(tag==60){static const u8 effect_source[]={0x83,0xc4,0x28};T(effect_source);}
 else if(tag==56){static const u8 source_dib[]={0x5e,0x59};T(source_dib);}
 else if(tag==1){T(release);}else if(tag==2){T(lock);}else if(tag==5||tag==6){T(fill);}else if(tag==7||tag==8){T(copies);}else if(tag==9){T(jpeg);}else if(tag==10){T(font);}else if(tag==11){T(slow);}else if(tag==17){T(clipped);}else if(tag==18){T(half);}else if(tag==19||tag==20){T(quarter);}else if(tag==21){T(wave);}else if(tag==25){T(shadow);}else if(tag==30){static const u8 pcx[]={0x5f,0x5b,0x83,0xc4,0x18};T(pcx);}else if(tag==31){static const u8 dib[]={0x5e,0x59};T(dib);}else if(tag==32||tag==33){u8 close[]={0x5b,0x83,0xc4,tag==32?0x18:0x20};for(u32 i=0;i<4;++i)*p++=close[i];}else if(tag==34){static const u8 word[]={0x83,0xc4,0x1c};T(word);}else if(tag==35){static const u8 bmp[]={0x83,0xc4,8};T(bmp);}else if(tag==36){T(normal);}else if(tag==37||tag==38){static const u8 partial[]={0x83,0xc4,0x20};T(partial);}else if(tag==39){static const u8 keyed[]={0x5b,0x83,0xc4,0x18};T(keyed);}else if(tag==40){static const u8 bmp0[]={0x83,0xc4,8};T(bmp0);}else if(tag==41){static const u8 key[]={0x83,0xc4,0x10};T(key);}else if(tag==52){static const u8 colour[]={0x83,0xc4,0x24};T(colour);}else if(tag==53){static const u8 generator[]={0x5b,0x83,0xc4,8};T(generator);}else if(tag==54){}else if(tag==55){static const u8 resize[]={0x5b};T(resize);}else if(tag>=49){static const u8 rotated[]={0x5e,0x5d,0x5b,0x83,0xc4,0x28};T(rotated);}else if(tag==46){static const u8 outline[]={0x5f,0x5e,0x5d,0x5b,0x83,0xc4,0x28};T(outline);}else if(tag==47){static const u8 mark[]={0x5b};T(mark);}else if(tag==48){static const u8 units[]={0x5d,0x5b,0x83,0xc4,8};T(units);}else if(tag==45){static const u8 map[]={0x5e,0x83,0xc4,0x20};T(map);}else if(tag==44){static const u8 fade[]={0x5e,0x5d,0x59};T(fade);}else if(tag>=42){static const u8 rect[]={0x5b,0x83,0xc4,0x20};T(rect);}else if(tag>=27){u8 close[]={0x83,0xc4,tag==27?0x40:tag==28?0x38:0x24};for(u32 i=0;i<3;++i)*p++=close[i];}else if(tag==26){static const u8 add[]={0x83,0xc4,0x1c};T(add);}else if(tag>=12){T(normal);}
 for(u32 i=0;i<n;++i)*p++=bytes[i];if(hooks[tag].pop){*p++=0xc2;*p++=(u8)hooks[tag].pop;*p=0;}else *p=0xc3;
}
static void uniform_colour_source_fixture(void){
 struct Surface saved_surface;copy(&saved_surface,surfaces,sizeof(saved_surface));
 u32 saved_mode=producer_trace_mode,saved_bound=get((void*)0x658174),saved_format=get((void*)0x6e1f88),saved_clip[4]={get((void*)0x6e0008),get((void*)0x6cbb6c),get((void*)0x6a49b8),get((void*)0x656618)};
 producer_trace_mode=1;surfaces[0].object=0x700100;surfaces[0].id=99;surfaces[0].pixels=0x700000;surfaces[0].width=5;surfaces[0].height=3;surfaces[0].stride=5;
 put((void*)0x658174,0x700000);put((void*)0x6e1f88,0);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,5);put((void*)0x656618,3);
 u8 shape[24]={0};put(shape+4,4);put(shape+8,75);struct Frame f={0};f.tag=52;f.ecx=(u32)shape;f.args[0]=1;f.args[1]=0;
 u8* data=request(&f);if(!data)ExitProcess(137);u32* r=(u32*)data;
 if(r[2]!=MNM_PRODUCER_FILL||r[3]!=99||r[10]!=1||r[11]!=0||r[12]!=5||r[13]!=3||r[14]||r[18])ExitProcess(138);
 HeapFree(producer_heap(),0,data);f.args[0]=6;if(request(&f))ExitProcess(139);
 producer_trace_mode=saved_mode;copy(surfaces,&saved_surface,sizeof(saved_surface));put((void*)0x658174,saved_bound);put((void*)0x6e1f88,saved_format);put((void*)0x6e0008,saved_clip[0]);put((void*)0x6cbb6c,saved_clip[1]);put((void*)0x6a49b8,saved_clip[2]);put((void*)0x656618,saved_clip[3]);
}
static void vertical_bar_source_fixture(void){
 struct Surface saved_surface=surfaces[0];u32 saved_failures=failures;
 u32 old_h=get((void*)0x6a49f4),old_pitch=get((void*)0x6a49f8),old_clip=get((void*)0x6c482c),old_format=get((void*)0x6e1f88);
 u32 shape[12]={4,75,1,2,0,100,0,0x77770000,600,4,75,1};
 u32 rect[4]={0,0,5,80};put((void*)0x6c482c,(u32)rect);put((void*)0x6a49f4,80);put((void*)0x6a49f8,10);put((void*)0x6e1f88,0);
 surfaces[0]=(struct Surface){0x1234,7,0x55550000,5,80,5,0,0};
 struct Frame generator={0};generator.tag=53;generator.ecx=(u32)shape;
 bar_source_enter(&generator);if(bar_source((u32)shape,0)->valid)ExitProcess(136);
 bar_source_return(&generator);if(!bar_source((u32)shape,0)->valid)ExitProcess(137);
 struct Frame draw={0};draw.tag=54;draw.ecx=(u32)shape;draw.args[0]=0x55550000;draw.args[1]=0x1234;draw.caller=0x4fd590;
 u8* data=bar_draw_request(&draw);if(!data||get(data+56)!=300||get(data+60)!=0x58f890)ExitProcess(138);
 for(u32 i=0;i<300;++i)if(get(data+96+i*12+8))ExitProcess(139);HeapFree(producer_heap(),0,data);
 /* Changing current scalar state does not regenerate the retained bitmap. */
 shape[6]=73;data=bar_draw_request(&draw);if(!data)ExitProcess(140);
 for(u32 i=0;i<300;++i)if(get(data+96+i*12+8))ExitProcess(141);HeapFree(producer_heap(),0,data);
 bar_source_enter(&generator);bar_source_return(&generator);data=bar_draw_request(&draw);
 if(!data||get(data+96+(1*4+1)*12+8)!=0xffe0)ExitProcess(142);HeapFree(producer_heap(),0,data);
 rect[2]=2;data=bar_draw_request(&draw);if(!data||get(data+56)!=75)ExitProcess(143);HeapFree(producer_heap(),0,data);
 shape[7]^=2;if(bar_draw_request(&draw)||failures!=saved_failures+1)ExitProcess(144);shape[7]^=2;
 bar_source_enter(&generator);if(bar_draw_request(&draw)||failures!=saved_failures+2)ExitProcess(145);
 struct Frame resize={0};resize.tag=55;resize.ecx=(u32)shape;resize.args[0]=4;resize.args[1]=75;bar_source_enter(&resize);bar_source_return(&resize);
 data=bar_draw_request(&draw);if(!data)ExitProcess(146);for(u32 i=0;i<75;++i)if(get(data+96+i*12+8)!=31)ExitProcess(147);HeapFree(producer_heap(),0,data);
 zero(bar_sources,sizeof(bar_sources));failures=saved_failures;surfaces[0]=saved_surface;put((void*)0x6a49f4,old_h);put((void*)0x6a49f8,old_pitch);put((void*)0x6c482c,old_clip);put((void*)0x6e1f88,old_format);
}
static void source_point_fixture(void){
 struct Surface saved_surfaces[2];struct Frame saved_frame;struct ProducerTraceThread saved_threads[32];
 copy(saved_surfaces,surfaces,sizeof(saved_surfaces));copy(&saved_frame,frames,sizeof(saved_frame));copy(saved_threads,producer_trace_threads,sizeof(saved_threads));
 u32 saved_enabled=enabled,saved_stopped=stopped,saved_mode=producer_trace_mode,saved_failures=failures;
 u32 out[16],other[16],pair[32],wrapper[12]={0};for(u32 i=0;i<16;++i)out[i]=0x77777777;
 producer_trace_mode=0;SetLastError(0xbeef);if(SceneDrawingSourcePointV1((void*)0x1111,out,64)||GetLastError()!=0xbeef||out[0]!=0x77777777)ExitProcess(126);
 InitializeCriticalSection(producer_trace_mutex);producer_trace_mode=1;enabled=1;stopped=failures=0;
 zero(surfaces,sizeof(saved_surfaces));wrapper[1]=0x1111;wrapper[2]=0x2222;
 surfaces[0].object=(u32)wrapper;surfaces[0].id=77;surfaces[0].width=4;surfaces[0].height=3;surfaces[0].stride=6;surfaces[0].dirty=9;surfaces[0].sampled=7;
 frames[0].sp=1;zero(producer_trace_threads,sizeof(saved_threads));producer_trace_threads[0].id=GetCurrentThreadId();producer_trace_threads[0].depth=2;
 SetLastError(0xbeef);if(!SceneDrawingSourcePointV1((void*)0x1111,out,64)||GetLastError()!=0xbeef||out[0]!=64||out[1]!=1||out[2]!=19||out[6]!=77||out[7]!=4||out[8]!=3||out[9]!=6||out[10]!=9||out[11]!=7||out[12]!=1||out[13]!=2||out[14]!=GetCurrentThreadId()||out[15])ExitProcess(127);
 if(!SceneDrawingSourcePointV1((void*)0x2222,other,64)||!bypass_equal(out,other,64))ExitProcess(128);
 SetLastError(0xbeef);if(!SceneDrawingSourcePairV1((void*)0x1111,(void*)0x2222,pair,128)||GetLastError()!=0xbeef||!bypass_equal(pair,pair+16,64)||SceneDrawingSourcePairV1(0,0,pair,127))ExitProcess(136);
 if(!SceneDrawingSourcePointV1((void*)0x3333,other,64)||other[2]!=3||other[6]||other[7])ExitProcess(129);
 copy(surfaces+1,surfaces,sizeof(*surfaces));surfaces[1].id=78;
 if(!SceneDrawingSourcePointV1((void*)0x1111,other,64)||other[2]!=11||other[6]||other[7])ExitProcess(130);
 zero(surfaces+1,sizeof(*surfaces));stopped=1;failures=3;
 if(!SceneDrawingSourcePointV1((void*)0x1111,other,64)||other[2]!=23||other[5]!=3)ExitProcess(131);
 u32* readonly=VirtualAlloc(0,4096,0x3000,4);u32 prior,ignored;
 if(!readonly)ExitProcess(132);for(u32 i=0;i<16;++i)readonly[i]=0x77777777;
 if(!VirtualProtect(readonly,4096,2,&prior))ExitProcess(133);
 SetLastError(0xbeef);if(SceneDrawingSourcePointV1((void*)0x1111,readonly,64)||GetLastError()!=0xbeef||readonly[0]!=0x77777777||SceneDrawingSourcePointV1(0,out,63)||SceneDrawingSourcePointV1(0,0,64))ExitProcess(134);
 if(!VirtualProtect(readonly,4096,prior,&ignored))ExitProcess(135);
 copy(surfaces,saved_surfaces,sizeof(saved_surfaces));copy(frames,&saved_frame,sizeof(saved_frame));copy(producer_trace_threads,saved_threads,sizeof(saved_threads));
 enabled=saved_enabled;stopped=saved_stopped;producer_trace_mode=saved_mode;failures=saved_failures;
}
#include "world-effect-hook-fixture.h"
static void extent640_fixture(void){
 struct Surface saved=surfaces[0];HANDLE saved_file=file;const u32 saved_limit=limit,saved_sequence=sequence,saved_bytes=bytes,saved_failures=failures,saved_stop=stopped,saved_enabled=enabled;
 if(!journal_flush())ExitProcess(176);char name[260];path(name,"world-extent640-fixture-",0,".bin");file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(177);
 u32 header[16]={0};copy(header,MNM_PRODUCER_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=MNM_PRODUCER_V3_MAX_RECORDS;header[6]=32;if(!write(file,header,64))ExitProcess(178);
 bytes=64;sequence=0;limit=32;stopped=0;enabled=1;depth=0;zero(source_packets,sizeof(source_packets));source_packet_count=source_packet_bytes=0;
 struct Surface* dst=surfaces;dst->object=0x700100;dst->id=99;dst->width=dst->stride=640;dst->height=480;dst->pixels=(u32)VirtualAlloc(0,614400,0x3000,4);if(!dst->pixels)ExitProcess(179);const u32 pixels=dst->pixels;
 for(u32 i=0;i<307200;++i)((u16*)pixels)[i]=0x4321;
 put((void*)0x658174,pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,640);put((void*)0x656618,480);put((void*)0x6e1f88,0);
 queue=raster_first=raster_last=1;raster_world_open=1;batch_enabled=1;batch_guard_open=0;bypass_completed=0;
 const u32 invalid[4][3]={{640,600,640},{800,480,800},{641,480,641},{640,480,642}};
 for(u32 i=0;i<4;++i){dst->width=invalid[i][0];dst->height=invalid[i][1];dst->stride=invalid[i][2];if(batch_start(dst)||batch_guard_open)ExitProcess(180);}
 dst->width=dst->stride=640;dst->height=480;continuous_map=(u8*)1;continuous_resident=0;if(batch_start(dst)||batch_guard_open)ExitProcess(180);continuous_map=0;
 dst->width=dst->stride=640;dst->height=480;if(!batch_start(dst)||!batch_identity(dst)||batch_pixel_bytes!=614400)ExitProcess(181);
 u32 region[7];if(VirtualQuery((void*)pixels,region,28)!=28||region[5]!=1)ExitProcess(182);
 const u32 args[6]={1,0,0,0,0,0},counter=get((void*)0x700804);SetLastError(1234);world_effect_test_call((u32)hooks[57].entry,0x700200,1,12,after,args);
 if(GetLastError()!=1234||get(after+28)||get(after+32)!=0x247||get((void*)0x700804)!=counter||batch_count!=1||bypass_completed!=1)ExitProcess(183);
 ++dst->width;if(batch_identity(dst))ExitProcess(184);--dst->width;++dst->height;if(batch_identity(dst))ExitProcess(184);--dst->height;++dst->stride;if(batch_identity(dst))ExitProcess(184);--dst->stride;dst->pixels+=2;if(batch_identity(dst))ExitProcess(184);dst->pixels=pixels;
 struct Surface other=*dst;if(batch_identity(&other))ExitProcess(184);
 u32 request[16];batch_header(request,dst);if(request[8]!=640||request[9]!=480||request[10]!=640||!batch_identity(dst)||!batch_restore())ExitProcess(185);
 for(u32 i=0;i<307200;++i)if(((u16*)pixels)[i]!=0x4321)ExitProcess(186);
 if(!journal_flush()||!CloseHandle(file))ExitProcess(187); /* Test allocation retires with ExitProcess. */
 surfaces[0]=saved;file=saved_file;limit=saved_limit;sequence=saved_sequence;bytes=saved_bytes;failures=saved_failures;stopped=saved_stop;enabled=saved_enabled;batch_enabled=0;raster_first=raster_last=0;
 put((void*)0x658174,saved.pixels);put((void*)0x6a49b8,saved.width);put((void*)0x656618,saved.height);
}
#include "kind8-sprite-hook-fixture.h"
void start(void){
 oracle_config_fixture();
 reserve[sizeof(reserve)-1]=1;u32 old;
 if((u32)GetModuleHandleA(0)!=0x400000||!VirtualProtect((void*)0x480000,0x120000,0x40,&old))ExitProcess(1);
 for(u32 i=0;i<HOOKS;++i){copy((void*)hooks[i].address,hooks[i].bytes,hooks[i].length);tail((u8*)(hooks[i].address+hooks[i].length),i);}
 u32* surface=(u32*)0x700100;surface[4]=4;surface[5]=4;surface[6]=8;
 for(u32 i=0;i<HOOKS;++i){SetLastError(1234);canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,before[i]);if(GetLastError()!=1234)ExitProcess(2);}
 *(u8*)hooks[52].address^=1;
 if(canvas_producers_install()||*(u8*)hooks[0].address!=hooks[0].bytes[0])ExitProcess(3);
 *(u8*)hooks[52].address^=1;
 if(!canvas_producers_install()){
  for(u32 i=0;i<HOOKS;++i)for(u32 j=0;j<hooks[i].length;++j)if(*(u8*)(hooks[i].address+j)!=hooks[i].bytes[j])ExitProcess(123);
  ExitProcess(4);
 }
 for(u32 i=0;i<HOOKS;++i){SetLastError(1234);canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,after);if(GetLastError()!=1234)ExitProcess(5);for(u32 j=0;j<560;++j)if(before[i][j]!=after[j])ExitProcess(20+i);}
 bypass_fixture();
 raster_fixture();
 oracle_budget_fixture();
 metadata_checkpoint_fixture();
 batch_fixture();
 journal_fixture();
 uniform_colour_source_fixture();
 vertical_bar_source_fixture();
 source_point_fixture();
 effect_hook_fixture();
 extent640_fixture();
 sprite_hook_fixture();
 canvas_producers_close();
 for(u32 i=0;i<HOOKS;++i)canvas_test_call(hooks[i].address,i==3?0x700000:0x700100,4,hooks[i].pop,after);
 char name[260];path(name,"producer-fixture-pass-",0,".bin");
 u32 mark[8]={0};copy(mark,"MNMPFP01",8);mark[2]=1;mark[3]=HOOKS;mark[4]=mark[5]=mark[6]=mark[7]=1;
 HANDLE done=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(done==(HANDLE)-1||!write(done,mark,32)||!CloseHandle(done))ExitProcess(125);
 ExitProcess(0);
}
