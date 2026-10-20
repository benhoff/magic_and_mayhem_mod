/* Included after the shared producer fixture helpers. Actual entry ASM,
 * synthetic original prologues and a protected800x600 original canvas. */
extern void world_effect_test_call(u32,u32,u32,u32,void*,const u32*);
static void effect_hook_fixture(void){
 struct Surface saved_surface=surfaces[0];const u32 saved_limit=limit,saved_sequence=sequence,saved_bytes=bytes,saved_failures=failures,saved_stop=stopped,saved_enabled=enabled;
 HANDLE saved_file=file;if(!journal_flush())ExitProcess(150);
 char name[260];path(name,"world-effect-fixture-",0,".bin");file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(151);
 u32 header[16]={0};copy(header,MNM_PRODUCER_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=MNM_PRODUCER_V3_MAX_RECORDS;header[6]=32;
 if(!write(file,header,64))ExitProcess(152);bytes=64;sequence=0;limit=32;stopped=0;enabled=1;depth=0;
 const u32 frame=0x700200,node=0x710000;
 zero((void*)frame,64);put((void*)frame,64);put((void*)(frame+4),2);put((void*)(frame+8),2);put((void*)(frame+28),node);
 put((void*)(frame+40),56);put((void*)(frame+44),60);put((void*)(frame+48),58);put((void*)(frame+52),62);
 *(u8*)(frame+56)=0;*(u8*)(frame+57)=2;*(u8*)(frame+58)=0;*(u8*)(frame+59)=2;
 *(u8*)(frame+60)=1;*(u8*)(frame+61)=2;*(u8*)(frame+62)=3;*(u8*)(frame+63)=4;
 zero((void*)node,4096);put((void*)node,node);for(u32 i=0;i<256;++i){*(u16*)(node+12+i*2)=(u16)(i*7);}
 put((void*)0x5e15f8,8);put((void*)0x6c4830,0);put((void*)0x6e1f88,0);
 *(u16*)0x6e1f7c=0xf800;*(u16*)0x6e1f7e=0x07e0;*(u16*)0x6e1f80=0x1f;
 for(u32 i=0;i<32;++i)put((void*)(0x5f1450+i*4),i%17);
 put((void*)0x5f14d0,0);for(u32 i=0;i<32;++i)put((void*)(0x5f14d0+(9*16+i)*4),i%19);
 struct Surface* s=surfaces;s->object=0x700100;s->id=99;s->width=s->stride=800;s->height=600;s->pixels=(u32)VirtualAlloc(0,960000,0x3000,4);if(!s->pixels)ExitProcess(153);
 for(u32 i=0;i<480000;++i)((u16*)s->pixels)[i]=0x4321;
 put((void*)0x658174,s->pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,800);put((void*)0x656618,600);
 queue=raster_first=raster_last=1;raster_world_open=1;bypass_limit=0;batch_enabled=1;bypass_completed=0;batch_guard_open=0;
 if(!batch_start(s))ExitProcess(154);
 const u32 tags[]={57,58,59,60,21};
 for(u32 i=0;i<5;++i){const u32 tag=tags[i];u32 args[6]={1,0,0,0,0,0};
  if(tag==59){args[2]=244;args[3]=198;args[4]=13;}if(tag==21)args[1]=9;
  if(tag>=57)producer_trampolines[tag]=hooks[tag].address; /* Default V1 does not install extended hooks. */
  u8* tail_at=(u8*)(hooks[tag].address+hooks[tag].length);u8 old_tail[32];copy(old_tail,tail_at,32);
  tail_at[0]=0x9c;tail_at[1]=0xff;tail_at[2]=5;put(tail_at+3,0x700804);tail_at[7]=0x9d;copy(tail_at+8,old_tail,32);
  enabled=0;SetLastError(1234);world_effect_test_call((u32)hooks[tag].entry,frame,1,hooks[tag].pop,bypass_before,args);const u32 counter=get((void*)0x700804);
  enabled=1;SetLastError(1234);world_effect_test_call((u32)hooks[tag].entry,frame,1,hooks[tag].pop,after,args);
  if(get((void*)0x700804)!=counter||GetLastError()!=1234||bypass_completed!=i+1||batch_count!=i+1||get(after+28)||get(after+32)!=0x247)ExitProcess(155+i);
  for(u32 j=0;j<560;++j)if((j<28||j>=36)&&after[j]!=bypass_before[j])ExitProcess(160+i);
  copy(tail_at,old_tail,32);
 }
 if(!batch_restore()||batch_count!=5)ExitProcess(165);
 for(u32 i=0;i<480000;++i)if(((u16*)s->pixels)[i]!=0x4321)ExitProcess(166);
 if(!journal_flush()||!CloseHandle(file))ExitProcess(167);file=0;
 struct Frame f={0};f.tag=57;f.ecx=frame;f.edx=1;f.args[0]=1;
 put((void*)0x6e1f88,1);if(request(&f))ExitProcess(168);put((void*)0x6e1f88,0);
 *(u16*)0x6e1f7c=0;if(request(&f))ExitProcess(169);*(u16*)0x6e1f7c=0xf800;
 f.args[2]=19;if(request(&f))ExitProcess(170);f.args[2]=0;
 put((void*)(frame+28),0xffffffff);if(request(&f))ExitProcess(171);put((void*)(frame+28),node);
 f.tag=21;f.args[1]=17;if(request(&f))ExitProcess(172);f.args[1]=9;
 f.tag=60;f.args[1]=0;put((void*)0x5e15f8,17);if(request(&f))ExitProcess(173);put((void*)0x5e15f8,8);
 u8* data=request(&f);if(!data)ExitProcess(174);put((void*)0x5f1454,33);if(effect_row_state(data,60))ExitProcess(175);HeapFree(producer_heap(),0,data);
 batch_enabled=0;raster_first=raster_last=0;surfaces[0]=saved_surface;file=saved_file;limit=saved_limit;sequence=saved_sequence;bytes=saved_bytes;failures=saved_failures;stopped=saved_stop;enabled=saved_enabled;
}
