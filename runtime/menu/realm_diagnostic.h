/* One rejected Realm Activate request: read-only source fields, no retry. */
static int realm_activation_diagnostic_done;
static void realm_activation_diagnostic(void* object,u32 screen,u32 generation,u32 argument){
 if(realm_activation_diagnostic_done||menu_version<19)return;
 realm_activation_diagnostic_done=1;u32 error=GetLastError(),done=0;
 static u32 blocks[][4]={
  {0x659408,0x1e25,0,0},{0x6f349c,0x48,0,0},{0x68991c,8,0,0},{0x657ce0,8,0,0},
  {0x65ae1d,320,0,0},{0x65af5d,320,0,0},{0x65b1d9,80,0,0},{0x65ab9d,320,0,0},
  {0x65a065,80*20,0,0},{0x6e0080,4,0,0},{0x6c54dc,4,0,0},{0x6f2d28,32,0,0},
  {0x641fbc,4,0,0},{0x641ab8,REALM_INPUT_RING_COUNT*40,0,0},{0x6de6d5,1,0,0}};
 const u32 count=sizeof(blocks)/sizeof(blocks[0]);u32 bytes=64+sizeof(blocks);
 for(u32 i=0;i<count;++i){blocks[i][2]=readable((void*)blocks[i][0],blocks[i][1]);blocks[i][3]=bytes;if(blocks[i][2])bytes+=blocks[i][1];}
 u32 header[16]={0x524d4e4d,0x31304152,1,64,bytes,count,screen,GetCurrentThreadId(),argument,(u32)object,
  menu_owner,menu_request,generation,menu_version,menu_ready,realm_input_failure_line};
 HANDLE file=CreateFileA("native-realm-activation-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){
  if(WriteFile(file,header,sizeof(header),&done,0)&&done==sizeof(header)&&
     WriteFile(file,blocks,sizeof(blocks),&done,0)&&done==sizeof(blocks))
   for(u32 i=0;i<count;++i)if(blocks[i][2]&&(!WriteFile(file,(void*)blocks[i][0],blocks[i][1],&done,0)||done!=blocks[i][1]))break;
  CloseHandle(file);
 }
 SetLastError(error);
}
