/* Actual retained wrappers/primary continuation, supplied synthetic raster
 * tails and a protected legacy plane. This checks callback/trace integration,
 * not original rectangle pixels or the absent host compositor. */
static int WIN rectangle_fixture_bind(u32 a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 h){
 (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return 1;
}
static void rectangle_hook_fixture(void){
 struct Surface saved=surfaces[0];HANDLE saved_file=file;
 const u32 saved_limit=limit,saved_sequence=sequence,saved_bytes=bytes,saved_failures=failures,saved_stop=stopped,saved_enabled=enabled;
 ResidentWorldBind saved_public=continuous_public_bind;
 if(!journal_flush())ExitProcess(211);char name[260];path(name,"world-rectangle-fixture-",0,".bin");
 file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(212);
 u32 header[16]={0};copy(header,MNM_PRODUCER_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=MNM_PRODUCER_V3_MAX_RECORDS;header[6]=32;
 if(!write(file,header,64))ExitProcess(213);bytes=64;sequence=0;limit=32;stopped=0;enabled=1;depth=0;
 struct Surface* dst=surfaces;dst->object=0x700100;dst->id=99;dst->width=dst->stride=800;dst->height=600;
 dst->pixels=(u32)VirtualAlloc(0,960000,0x3000,4);if(!dst->pixels)ExitProcess(214);
 for(u32 i=0;i<480000;++i)((u16*)dst->pixels)[i]=0x4321;
 put((void*)0x658174,dst->pixels);put((void*)0x6a2dc8,800);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,800);put((void*)0x656618,600);put((void*)0x6e1f88,0);
 queue=raster_first=raster_last=1;raster_world_open=1;batch_enabled=1;batch_guard_open=0;continuous_public_bind=rectangle_fixture_bind;
 copy((void*)0x54ab00,sprite_rectangle_add,sizeof(sprite_rectangle_add));copy((void*)0x54ab60,sprite_rectangle_colour,sizeof(sprite_rectangle_colour));
 if(!FlushInstructionCache(GetCurrentProcess(),(void*)0x54ab00,0x80)||!batch_start(dst))ExitProcess(215);
 const u32 object=0x700300,row=0x700600;
 for(u32 kind=0;kind<2;++kind){
  zero((void*)object,100);zero((void*)row,64);const u32 table=kind?0x5c7494:0x5c7420,shape=object+(kind?0x1c:0x30);
  put((void*)object,table);put((void*)(table+12),kind?0x54ab60:0x54ab00);put((void*)(row+4),object);put((void*)(row+8),1);put((void*)(row+12),2);
  if(!kind){put((void*)shape,3);put((void*)(shape+4),2);put((void*)(shape+8),5);put((void*)(shape+12),7);put((void*)(shape+16),9);}
  else{put((void*)shape,3);put((void*)(shape+4),3);put((void*)(shape+8),2);*(u16*)(shape+12)=5;*(u16*)(shape+14)=7;*(u16*)(shape+16)=9;put((void*)(shape+18),0);}
  SetLastError(1234);kind8_sprite_primary_call((u32*)row);
  if(GetLastError()!=1234||(*(u16*)(row+30))||(!kind&&*(u8*)(object+0x44)!=1))ExitProcess(216);
 }
 if(!batch_restore())ExitProcess(217);for(u32 i=0;i<480000;++i)if(((u16*)dst->pixels)[i]!=0x4321)ExitProcess(218);
 if(!journal_flush()||!CloseHandle(file))ExitProcess(219);
 surfaces[0]=saved;file=saved_file;limit=saved_limit;sequence=saved_sequence;bytes=saved_bytes;failures=saved_failures;stopped=saved_stop;enabled=saved_enabled;continuous_public_bind=saved_public;
 batch_enabled=0;raster_first=raster_last=0;put((void*)0x658174,saved.pixels);put((void*)0x6a49b8,saved.width);put((void*)0x656618,saved.height);
}
/* Supplied observer frames, not additional callback/raster executions. Exercise
 * both opaque stack offsets and exact known/unknown selector custody. */
static void caller_bindings_fixture(void){
 if(!caller_state_create_at(1))ExitProcess(221);
 const u32 word=get((void*)0x6903dc),generic=get((void*)0x6c4e3c);
 u8* saved_map=continuous_map;const u32 gen=continuous_generation_low,genhigh=continuous_generation_high;
 const u32 source=continuous_source_low,sourcehigh=continuous_source_high,nonce=continuous_nonce_low,noncehigh=continuous_nonce_high;
 const u32 saved_world=raster_world_open,saved_sequence=sequence;
 static const u32 input_addresses[7]={0x5f14d0,0x5f1450,0x6f5eac,0x6c4830,0x5e15f8,0x6e1f88,0x6a2dc8};
 u32 previous_inputs[7];for(u32 j=0;j<7;++j)previous_inputs[j]=get((void*)input_addresses[j]);
 /* Supplied channel identity only; these direct diagnostic helper calls never
  * dereference the dummy channel or publish a World/source chunk. */
 continuous_map=(u8*)1;continuous_generation_low=7;continuous_generation_high=0;
 continuous_source_low=3;continuous_source_high=0;continuous_nonce_low=123;continuous_nonce_high=456;raster_world_open=0;
 u8 fx[512] __attribute__((aligned(16)));__asm__ volatile("fxsave %0":"=m"(fx));
 for(u32 i=0;i<2;++i){
  u32 saved[24];zero(saved,sizeof(saved));saved[8]=0x246;saved[9]=i?0x57e8fb:0x57e957;
  saved[17]=0x500a8c;saved[18]=0x444444;saved[19]=0x500566;saved[20]=0x555555;
  put((void*)0x6903dc,i?0x12345678:0x597086);put((void*)0x6c4e3c,i?0:0x5947b2);
  const u32 input_values[7]={i?0:0x7d00,i?0:0x7d00,i?0x3f800000:0,i?0x80000000:7,i?0:13,i?0x1234:0,i?0:800};
  for(u32 j=0;j<7;++j)put((void*)input_addresses[j],input_values[j]);
  u32 token=caller_state_begin(saved,i?22:17,fx,1234);if(!token)ExitProcess(222);
  if(i){++continuous_source_low;sequence=0;}
  caller_state_finish(token,MNM_CALLER_STATE_NATIVE,saved,fx,1234);
 }
 if(caller_state_starts!=2||caller_state_completed!=2||caller_state_active||caller_bindings_rows!=2)ExitProcess(223);
 caller_state_close();put((void*)0x6903dc,word);put((void*)0x6c4e3c,generic);
 continuous_map=saved_map;continuous_generation_low=gen;continuous_generation_high=genhigh;
 continuous_source_low=source;continuous_source_high=sourcehigh;continuous_nonce_low=nonce;continuous_nonce_high=noncehigh;
 raster_world_open=saved_world;sequence=saved_sequence;
 for(u32 j=0;j<7;++j)put((void*)input_addresses[j],previous_inputs[j]);
}
/* Supplied raw stack controls, not original wrapper executions. The retained
 * original57e8e0 pushes four nonvolatile words; all three auxiliary tails have
 * three callee-popped arguments. Their caller return is word8, while the
 * five-argument cdecl calls have word10. Include all four secondary-frame
 * entries imported from master. Distinct canaries expose swaps. */
static void caller_opaque_context_fixture(void){
 static const u32 callers[8][3]={{0x57e985,19,8},{0x57e9a8,18,8},{0x57e9c7,20,8},{0x57e957,17,10},
  {0x57e9ec,22,8},{0x57ea02,12,8},{0x57ea29,13,10},{0x57ea40,17,10}};
 for(u32 i=0;i<8;++i){
  u32 row[192],stack[16];zero(row,sizeof(row));zero(stack,sizeof(stack));
  row[2]=callers[i][1];row[4]=callers[i][0];
  stack[8]=callers[i][2]==8?0x500566:0xcccccccc;stack[10]=callers[i][2]==10?0x500566:0xdddddddd;
  caller_state_context(row,stack);
  if(row[190]!=2||row[191]!=0x500566)ExitProcess(231);
 }
}
/* Supplied diagnostic return, not an actual glyph adapter/pixel execution.
 * Snapshot before observer work, then simulate prefix drain/publication and
 * mutate scratch inputs. Completion must keep the earlier engine state. */
static void caller_native_return_fixture(void){
 if(!caller_state_create_at(2))ExitProcess(224);
 u32 saved[24];zero(saved,sizeof(saved));saved[8]=0x246;saved[9]=0x4a5a68;saved[7]=0x12345678;
 u8 fx[512] __attribute__((aligned(16)));__asm__ volatile("fxsave %0":"=m"(fx));
 const u32 saved_sequence=sequence;u32 token=caller_state_begin(saved,10,fx,1234);if(!token)ExitProcess(225);
 caller_states[token-1].native_return=1;
 const u32 returned=caller_state_capture_return((u32)(saved+9)+20,10,saved,fx,1234);
 if(returned!=token||caller_state_completed||caller_state_active!=1)ExitProcess(226);
 saved[7]=0xdeadbeef;sequence+=3;caller_state_publish(token);
 if(caller_state_completed!=1||caller_state_active||caller_states[token-1].row[32]!=0x12345678||caller_states[token-1].row[10]!=sequence)ExitProcess(227);
 caller_state_close();sequence=saved_sequence;
}
/* Supplied quiescent source prefix, with no World or drawing execution. */
static void caller_tail_fixture(void){
 u8* map=continuous_map;
 u32* variables[]={&continuous_slot_index,&continuous_generation_low,&continuous_generation_high,
  &continuous_source_low,&continuous_source_high,&continuous_nonce_low,&continuous_nonce_high,
  &bytes,&continuous_written,&sequence,&queue,&raster_world_open,&failures,&journal_failed,&continuous_deferred_bytes};
 u32 previous[15];for(u32 i=0;i<15;++i)previous[i]=*variables[i];
 continuous_map=HeapAlloc(GetProcessHeap(),8,4096);if(!continuous_map)ExitProcess(228);
 continuous_slot_index=0;continuous_generation_low=7;continuous_generation_high=0;
 continuous_source_low=3;continuous_source_high=0;continuous_nonce_low=123;continuous_nonce_high=456;
 bytes=continuous_written=160;sequence=1;queue=raster_world_open=failures=journal_failed=continuous_deferred_bytes=0;
 u8* slot=continuous_slot();put(slot,MNM_CONT_WRITING);u32* h=(u32*)(slot+MNM_CONT_SLOT_HEADER);zero(h,160);
 copy(h,MNM_PRODUCER_V3_MAGIC,8);h[2]=3;h[3]=64;h[4]=MNM_PRODUCER_BUILD;h[5]=MNM_PRODUCER_V3_MAX_RECORDS;h[6]=32;
 u32* r=h+16;r[0]=96;r[1]=1;r[2]=MNM_PRODUCER_FILL;r[3]=99;r[5]=3;r[6]=2;r[7]=3;r[12]=3;r[13]=2;r[14]=0x1234;
 caller_state_tail_at(3);
 if(sequence!=1||bytes!=160||continuous_written!=160||continuous_load(slot,0)!=MNM_CONT_WRITING)ExitProcess(229);
 if(!HeapFree(GetProcessHeap(),0,continuous_map))ExitProcess(230);continuous_map=map;
 for(u32 i=0;i<15;++i)*variables[i]=previous[i];
}
