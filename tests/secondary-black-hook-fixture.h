/* Actual installed producer hooks reached through the unchanged secondary
 * queue fragment. Its destination stays PAGE_NOACCESS for all five draws. */
extern void secondary_black_call(void*,int,int,void*,u32*);
static void secondary_black_hook_fixture(void){
 char name[260];if(!journal_flush())ExitProcess(230);
 path(name,"world-secondary-black-fixture-",0,".bin");file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)ExitProcess(231);
 u32 header[16]={0};copy(header,MNM_PRODUCER_V3_MAGIC,8);header[2]=3;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=MNM_PRODUCER_V3_MAX_RECORDS;header[6]=32;if(!write(file,header,64))ExitProcess(232);
 bytes=64;sequence=0;limit=16;stopped=0;enabled=1;depth=0;zero(source_packets,sizeof(source_packets));source_packet_count=source_packet_bytes=0;
 const u32 frame=0x700200,palette=0x710000,row=0x700600;zero((void*)palette,4096);put((void*)palette,palette);put((void*)(palette+8),127);sprite_source(frame,palette,0);
 struct Surface* dst=surfaces;dst->object=0x700100;dst->id=99;dst->width=dst->stride=800;dst->height=600;dst->pixels=(u32)VirtualAlloc(0,960000,0x3000,4);if(!dst->pixels)ExitProcess(233);
 for(u32 i=0;i<480000;++i)((u16*)dst->pixels)[i]=0x4321;
 put((void*)0x658174,dst->pixels);put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,800);put((void*)0x656618,600);put((void*)0x6a2dc8,800);put((void*)0x6e1f88,0);
 queue=raster_first=raster_last=1;raster_world_open=1;batch_enabled=1;batch_guard_open=0;bypass_completed=0;continuous_map=0;kind8_sprite_guard_fixture=1;
 if(!batch_start(dst))ExitProcess(234);
 copy((void*)0x500a0e,secondary_black_branch,sizeof(secondary_black_branch));copy((void*)0x5009d2,secondary_black_store,sizeof(secondary_black_store));*(u8*)0x500d05=0xc3;
 const int xs[]={5,-1,799};const u32 counts[]={1,3,5};
 for(u32 i=0;i<3;++i){u32 q[6];zero((void*)row,64);for(u32 j=0;j<64;++j)*(u8*)(row+j)=0x5a;SetLastError(1234);secondary_black_call((void*)frame,xs[i],2,(void*)row,q);
  if(GetLastError()!=1234||(q[0]&65535)||q[1]!=2||q[2]!=row||q[3]!=frame||q[4]!=(u32)xs[i]||q[5]||(*(u16*)(row+30))||batch_count!=counts[i]||bypass_completed!=counts[i])ExitProcess(235);
  for(u32 j=0;j<64;++j)if(j!=30&&j!=31&&*(u8*)(row+j)!=0x5a)ExitProcess(236);
 }
 if(!bypass_equal((void*)0x500a0e,secondary_black_branch,sizeof(secondary_black_branch))||!bypass_equal((void*)0x5009d2,secondary_black_store,sizeof(secondary_black_store)))ExitProcess(237);
 u32 request[16];batch_header(request,dst);path(name,"world-secondary-black-batch-",0,".bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f==(HANDLE)-1||!write(f,request,64)||!write(f,batch_rows,request[11])||!CloseHandle(f))ExitProcess(238);
 if(!batch_restore())ExitProcess(239);for(u32 i=0;i<480000;++i)if(((u16*)dst->pixels)[i]!=0x4321)ExitProcess(240);
 if(!journal_flush()||!CloseHandle(file))ExitProcess(241);file=0;u32 proof[8]={0};copy(proof,"MNMSEC01",8);proof[2]=3;proof[3]=5;proof[4]=2;proof[5]=1;proof[6]=1;proof[7]=1;
 path(name,"world-secondary-black-proof-",0,".bin");f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f==(HANDLE)-1||!write(f,proof,sizeof(proof))||!CloseHandle(f))ExitProcess(242);
}
