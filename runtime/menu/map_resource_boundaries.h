/* Optional bounded read-only World resource snapshots. No extra original
 * methods, QI/AddRef/Release, allocator calls or simulation writes. */
static HANDLE map_resource_file;
static u32 map_resource_sequence,map_resource_epoch,map_resource_fault;
static const u32 map_resource_globals[33]={
 0x5d9e38,0x6de6d9,0x689498,0x68949c,0x6def58,0x6def60,
 0x6def64,0x6b1248,0x6b124c,0x6e1f94,0x6e1f98,0x6e266c,
 0x6de558,0x6de55c,0x6de560,0x6de564,0x6de568,0x6de56c,0x6de570,0x6de574,0x6de578,
 0x6a28f8,0x6a28fc,0x6a2900,0x6a2904,0x6a2908,0x6a290c,0x6a2910,0x6a2914,0x6a2918,
 0x6903e8,0x6e1f9c,0x656624};
static const u32 map_resource_offsets[12]={
 0xfc2a,0xfc2e,0xfc32,0xfc36,0xfc3a,0xfc3e,0xfc42,0xfc46,0xfc4a,0xfc4e,0xfc52,0x100df};
static u32 map_resource_read(u32 address,u32 bytes){
 if(!readable((void*)address,bytes)){map_resource_fault=1;return 0xffffffff;}
 return bytes==1?*(u8*)address:get((void*)address);
}
static void map_resource_snapshot(u32 phase,u32 result){
 if(!map_resource_file)return;
 u32 error=GetLastError(),wrote=0;
 if(map_resource_sequence>=48){map_resource_fault=2;SetLastError(error);return;}
 u32 row[80];volatile u32* clear=row;for(u32 i=0;i<80;++i)clear[i]=0;
 row[0]=++map_resource_sequence;row[1]=phase;
 if(phase==1)++map_resource_epoch;
 row[2]=map_resource_epoch;row[3]=GetCurrentThreadId();row[4]=GetTickCount();row[5]=sequence;
 row[10]=menu_generation;row[12]=55;row[13]=0x6cbb78;row[14]=result;
 if(phase!=9){
  row[6]=map_resource_read(0x6cbb78,4);row[7]=map_resource_read(0x6cbb80,4);
  row[8]=map_resource_read(0x6f34e0,4);row[9]=map_resource_read(0x6f349c,4);
  for(u32 i=0;i<33;++i)row[16+i]=map_resource_read(map_resource_globals[i],i==31?1:4);
  for(u32 i=0;i<12;++i)row[49+i]=map_resource_read(0x6cbb78+map_resource_offsets[i],4);
  row[61]=map_resource_read(0x6cbb78+0x100ac,1);row[62]=map_resource_read(0x6cbb78+0x100a4,1);
  for(u32 i=0;i<4;++i){
   row[63+i]=map_resource_read((i<2?0x6a49e0:0x6a4fe0)+4+(i%2)*4,4);
   row[67+i]=row[63+i]?map_resource_read(row[63+i],4):0;
  }
 }
 row[11]=map_resource_fault;
 if(!WriteFile(map_resource_file,row,sizeof(row),&wrote,0)||wrote!=sizeof(row)){
  map_resource_fault=3;CloseHandle(map_resource_file);map_resource_file=0;
 }
 SetLastError(error);
}
static int map_resource_open(void){
 char path[1024];u32 error=GetLastError(),length=GetEnvironmentVariableA("MNM_MENU_MAP_RESOURCE_OBSERVE",path,sizeof(path));
 if(!length){SetLastError(error);return 1;}
 if(length>=sizeof(path)){SetLastError(error);return 0;}
 map_resource_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
 if(map_resource_file==(HANDLE)-1){map_resource_file=0;SetLastError(error);return 0;}
 const u32 header[16]={0x4d4d4e4d,0x31534552,1,64,320,48,0x40209ca7,33,12,4,0,0,0,0,0,0};u32 wrote=0;
 int ok=WriteFile(map_resource_file,header,64,&wrote,0)&&wrote==64;
 if(!ok){CloseHandle(map_resource_file);map_resource_file=0;}
 SetLastError(error);return ok;
}
static void map_resource_close(void){
 u32 error=GetLastError();if(map_resource_file){map_resource_snapshot(9,0);CloseHandle(map_resource_file);map_resource_file=0;}SetLastError(error);
}
