/* One bounded rejected progressed-Region snapshot. This diagnostic neither
 * admits a request nor writes original campaign or menu state. */
static u32 region_diagnostic_line,region_diagnostic_samples,region_diagnostic_done;
static void region_snapshot_diagnostic(void* object){
 u32 error=GetLastError();u8* p=object;
 if(region_diagnostic_done||!region_campaign_enabled||!region_snapshot_failure_line||
    p!=(u8*)0x6578c0||!readable(p,0x81)||get(p)!=0x5c667c||get(p+4)!=18||get(p+8)!=1||
    get(p+0x6f)<2||get(p+0x6f)>19||!readable((void*)0x6f349c,0x48)||
    !readable((void*)0x659408,0x1e25)||get((void*)0x6f34e0)!=(u32)p)return;
 if(region_diagnostic_line!=region_snapshot_failure_line){region_diagnostic_line=region_snapshot_failure_line;region_diagnostic_samples=0;}
 if(++region_diagnostic_samples<4)return;
 region_diagnostic_done=1;
 u32 blocks[][4]={
  {(u32)p,0x81,0,0},{0x659408,0x1e25,0,0},{0x6f349c,0x48,0,0},{0x68991c,8,0,0},
  {0x657ce0,8,0,0},{0x65ae1d,320,0,0},{0x65af5d,320,0,0},{0x65b1d9,80,0,0},{0x65ab9d,320,0,0},
  {get(p+0x5b),16,0,0},{get(p+0x53),4*0x7e,0,0},{get(p+0x5f),16,0,0},
  {get(p+0x4b),2*0x59,0,0},{get(p+0x4f),3*0x7a,0,0},
  {get(p+0x73),128,0,0},{get(p+0x78),128,0,0},{get((void*)(0x659408+0x59)),800*600,0,0}};
 const u32 count=sizeof(blocks)/sizeof(blocks[0]);u32 bytes=64+sizeof(blocks),done=0;
 for(u32 i=0;i<count;++i){blocks[i][2]=readable((void*)blocks[i][0],blocks[i][1]);blocks[i][3]=bytes;if(blocks[i][2])bytes+=blocks[i][1];}
 u32 header[16]={0x524d4e4d,0x31304745,1,64,bytes,count,region_diagnostic_line,GetCurrentThreadId(),get(p+0x6f),(u32)p,
  get((void*)0x6f34e0),menu_request,menu_generation,menu_version,region_diagnostic_samples,0};
 HANDLE file=CreateFileA("native-region-snapshot-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){
  if(WriteFile(file,header,sizeof(header),&done,0)&&done==sizeof(header)&&
     WriteFile(file,blocks,sizeof(blocks),&done,0)&&done==sizeof(blocks))
   for(u32 i=0;i<count;++i)if(blocks[i][2]&&(!WriteFile(file,(void*)blocks[i][0],blocks[i][1],&done,0)||done!=blocks[i][1]))break;
  CloseHandle(file);
 }
 SetLastError(error);
}
