/* Original control instructions copied from a hash-checked input at test build
 * time. Pixel entries are compatible synthetic tails; admitted leaves bypass
 * them while the entire original destination is PAGE_NOACCESS. */
#include "kind8-sprite-original.inc"
extern u32 kind8_sprite_primary_call(u32*);
static void sprite_fixture_require(int yes,u32 code){if(!yes)ExitProcess(code);}
static void sprite_source(u32 frame,u32 node,int direct){
 zero((void*)frame,68);put((void*)frame,direct?68:64);put((void*)(frame+4),2);put((void*)(frame+8),2);put((void*)(frame+28),direct?0xffffffffu:node);
 put((void*)(frame+40),56);put((void*)(frame+44),60);put((void*)(frame+48),58);put((void*)(frame+52),direct?64:62);
 *(u8*)(frame+57)=2;*(u8*)(frame+59)=2;
 if(direct){*(u16*)(frame+60)=0xf800;*(u16*)(frame+62)=0x07e0;*(u16*)(frame+64)=0x001f;*(u16*)(frame+66)=0xffff;}
 else {for(u32 i=0;i<4;++i)*(u8*)(frame+60+i)=(u8)(i+1);}
}
static void sprite_hook_fixture(void){
 struct Surface saved=surfaces[0];HANDLE saved_file=file;const u32 saved_limit=limit,saved_sequence=sequence,saved_bytes=bytes,saved_failures=failures,saved_stop=stopped,saved_enabled=enabled;
 sprite_fixture_require(journal_flush(),190);char name[260];path(name,"world-kind8-sprite-fixture-",0,".bin");file=CreateFileA(name,0x40000000,0,0,1,0x80,0);sprite_fixture_require(file!=(HANDLE)-1,191);
 u32 header[16]={0};copy(header,MNM_PRODUCER_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=MNM_PRODUCER_V3_MAX_RECORDS;header[6]=32;sprite_fixture_require(write(file,header,64),192);
 bytes=64;sequence=0;limit=32;stopped=0;enabled=1;depth=0;zero(source_packets,sizeof(source_packets));source_packet_count=source_packet_bytes=0;
 const u32 frame=0x700200,node=0x710000,object=0x700300,bank=0x700400,owner=0x700500,row=0x700600,offsets=0x700700;
 zero((void*)node,4096);put((void*)node,node);for(u32 i=0;i<256;++i)*(u16*)(node+12+i*2)=(u16)(i*7);
 struct Surface* dst=surfaces;dst->object=0x700100;dst->id=99;dst->width=dst->stride=800;dst->height=600;dst->pixels=(u32)VirtualAlloc(0,960000,0x3000,4);sprite_fixture_require(dst->pixels!=0,193);
 for(u32 i=0;i<480000;++i)((u16*)dst->pixels)[i]=0x4321;
 put((void*)0x658174,dst->pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,800);put((void*)0x656618,600);put((void*)0x6a2dc8,800);put((void*)0x6e1f88,0);put((void*)0x5e1404,1);put((void*)0x644520,0);
 *(u16*)0x6e1f7c=0xf800;*(u16*)0x6e1f7e=0x7e0;*(u16*)0x6e1f80=0x1f;
 queue=raster_first=raster_last=1;raster_world_open=1;batch_enabled=1;batch_guard_open=0;bypass_completed=0;continuous_map=0;
 sprite_fixture_require(batch_start(dst),194);
 const u32 tags[]={13,14,17,22,36};
 for(u32 i=0;i<5;++i){const u32 tag=tags[i];sprite_source(frame,node,tag==22||tag==36);const u32 args[6]={frame,1+i*3,1,0,0,0};
  u8* tail_at=(u8*)(hooks[tag].address+hooks[tag].length);u8 old_tail[32];copy(old_tail,tail_at,32);tail_at[0]=0x9c;tail_at[1]=0xff;tail_at[2]=5;put(tail_at+3,0x700804);tail_at[7]=0x9d;copy(tail_at+8,old_tail,32);
  enabled=0;SetLastError(1234);world_effect_test_call((u32)hooks[tag].entry,0x1234,0x5678,0,bypass_before,args);const u32 counter=get((void*)0x700804);
  enabled=1;SetLastError(1234);world_effect_test_call((u32)hooks[tag].entry,0x1234,0x5678,0,after,args);
  sprite_fixture_require(get((void*)0x700804)==counter&&GetLastError()==1234&&bypass_completed==i+1&&batch_count==i+1&&!get(after+28)&&get(after+32)==0x247,195);
  for(u32 j=0;j<560;++j)if((j<28||j>=36)&&after[j]!=bypass_before[j])ExitProcess(196);
  copy(tail_at,old_tail,32);
 }
 copy((void*)0x5492b0,sprite_method_flies,sizeof(sprite_method_flies));copy((void*)0x549730,sprite_method_cure,sizeof(sprite_method_cure));
 copy((void*)0x5006a2,sprite_primary_prefix,sizeof(sprite_primary_prefix));copy((void*)0x5006b2,sprite_primary_continuation,sizeof(sprite_primary_continuation));copy((void*)0x500848,sprite_primary_store,sizeof(sprite_primary_store));*(u8*)0x50084c=0xc3;
 /* Synthetic RNG binding counts calls and checks the original receiver. */
 const u8 rng[]={0x81,0xf9,0xc8,0x74,0x65,0x00,0x74,0x06,0xff,0x05,0x0c,0x08,0x70,0x00,0xff,0x05,0x08,0x08,0x70,0x00,0xb8,0x33,0,0,0,0xc3};copy((void*)0x54e200,rng,sizeof(rng));put((void*)0x700808,0);put((void*)0x70080c,0);
 const u8 ax_one[]={0xb8,1,0,0,0,0xc3};copy((void*)0x54e240,ax_one,sizeof(ax_one));
 sprite_fixture_require(FlushInstructionCache(GetCurrentProcess(),(void*)0x480000,0x120000),197);
 zero((void*)owner,68);put((void*)(owner+0x24),1);zero((void*)bank,16);put((void*)(bank+12),1);put((void*)offsets,0);
 kind8_sprite_guard_fixture=1;
 for(u32 family=0;family<2;++family){const u32 table=family?0x5c74fc:0x5c74e4,method=family?0x549730:0x5492b0,global=family?0x6f2268:0x6f2150;
  put((void*)(table+12),method);zero((void*)object,32);put((void*)object,table);put((void*)(object+(family?0x10:0x1c)),owner);put((void*)global,bank);put((void*)(global+8),offsets);put((void*)(global+12),frame);
  for(u32 i=0;i<5;++i){const u32 tag=tags[i];sprite_source(frame,node,tag==22||tag==36);zero((void*)row,36);put((void*)(row+4),object);put((void*)(row+8),1+i*3);put((void*)(row+12),5+family*3);put((void*)(row+24),8);put((void*)(row+28),0x8ad08ad0);put((void*)(row+32),0x12345678);
   put((void*)0x6903dc,hooks[tag==36?36:22].address);put((void*)0x6c4e3c,tag==17?0x54e240:hooks[tag==14?14:13].address);
   SetLastError(1234);const u32 result=kind8_sprite_primary_call((u32*)row);
   sprite_fixture_require(!(result&65535)&&GetLastError()==1234&&get((void*)(row+28))==0x8ad0&&get((void*)(row+32))==0x12345678&&batch_count==6+family*5+i&&bypass_completed==batch_count,198);
   sprite_fixture_require(get((void*)object)==table&&get((void*)(object+(family?0x10:0x1c)))==owner,199);
  }
 }
 sprite_fixture_require(get((void*)0x700808)==15&&!get((void*)0x70080c),200);
 /* Real Win32 VirtualQuery/VirtualProtect controls, including every shorter
  * stack and object span ending at an inaccessible second page. */
 u8* pages=VirtualAlloc(0,8192,0x3000,4);u32 prior;sprite_fixture_require(pages&&VirtualProtect(pages+4096,4096,1,&prior),201);u32 controls=0,shorts=0,pairs=0;
 for(u32 k=0;k<6;++k){const struct SpriteRasterSite* site=sprite_raster_sites+k;const struct MnmKind8SpriteClass type=mnm_kind8_sprite_class(site->table,site->method);
  put((void*)(site->table+12),site->method);zero((void*)object,32);put((void*)object,site->table);put((void*)(object+type.owner_offset),owner);put((void*)site->bank,bank);put((void*)(row+4),object);put((void*)(row+24),8);put((void*)(row+28),0x8ad08ad0);put((void*)(frame+28),site->parent_word==7?0xffffffffu:node);
  u32 stack[10]={0};stack[0]=site->address;stack[1]=frame;stack[site->parent_word]=0x5006b2;stack[site->parent_word-2]=row;
  for(u32 tag=0;tag<64;++tag)if(site->tags&(1ull<<tag)){++pairs;
#define SPRITE_CHECK(value) do{++controls;if(!(value)){u32 diagnostic[4]={controls,site->address,tag,shorts};path(name,"world-sprite-guard-failure-",0,".bin");HANDLE diagnostic_file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(diagnostic_file!=(HANDLE)-1){write(diagnostic_file,diagnostic,16);CloseHandle(diagnostic_file);}ExitProcess(202);}}while(0)
   SPRITE_CHECK(raster_batch_caller(site->address,tag,stack));SPRITE_CHECK(!raster_caller(site->address,tag,stack));
   batch_guard_open=0;SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));batch_guard_open=1;
   ++batch_owner;SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));--batch_owner;
   put((void*)0x5e1404,0);SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));put((void*)0x5e1404,1);
   put((void*)0x6e1f88,1);SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));put((void*)0x6e1f88,0);
   put((void*)(frame+28),site->parent_word==7?node:0xffffffffu);SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));put((void*)(frame+28),site->parent_word==7?0xffffffffu:node);
   for(u32 n=0;n<(site->parent_word+1)*4;++n){SPRITE_CHECK(!raster_batch_caller(site->address,tag,(const u32*)(pages+4096-n)));++shorts;}
   for(u32 n=0;n<type.root_bytes;++n){u8* p=pages+4096-n;zero(p,n);if(n>=4)put(p,site->table);put((void*)(row+4),(u32)p);SPRITE_CHECK(!raster_batch_caller(site->address,tag,stack));++shorts;}put((void*)(row+4),object);
   SPRITE_CHECK(raster_batch_caller(site->address,tag,stack));
#undef SPRITE_CHECK
  }
 }
 sprite_fixture_require(pairs==10&&shorts==628,203);kind8_sprite_guard_fixture=0;
 u32 request[16];batch_header(request,dst);path(name,"world-kind8-sprite-batch-",0,".bin");HANDLE descriptor=CreateFileA(name,0x40000000,0,0,1,0x80,0);sprite_fixture_require(descriptor!=(HANDLE)-1&&write(descriptor,request,64)&&write(descriptor,batch_rows,request[11])&&CloseHandle(descriptor),204);
 sprite_fixture_require(batch_count==15&&batch_restore(),205);for(u32 i=0;i<480000;++i)if(((u16*)dst->pixels)[i]!=0x4321)ExitProcess(206);
 sprite_fixture_require(journal_flush()&&CloseHandle(file),207);u32 proof[10]={0};copy(proof,"MNMSPR01",8);proof[2]=5;proof[3]=10;proof[4]=15;proof[5]=0;proof[6]=pairs;proof[7]=shorts;proof[8]=controls;proof[9]=15;
 path(name,"world-kind8-sprite-proof-",0,".bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);sprite_fixture_require(f!=(HANDLE)-1&&write(f,proof,40)&&CloseHandle(f),208);
 surfaces[0]=saved;file=saved_file;limit=saved_limit;sequence=saved_sequence;bytes=saved_bytes;failures=saved_failures;stopped=saved_stop;enabled=saved_enabled;batch_enabled=0;raster_first=raster_last=0;
 put((void*)0x658174,saved.pixels);put((void*)0x6a49b8,saved.width);put((void*)0x656618,saved.height);
}
