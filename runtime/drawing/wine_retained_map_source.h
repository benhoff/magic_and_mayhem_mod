/* Last OBSERVED descriptor address. Closure never implies retained ownership. */
static HANDLE wine_retained_file;
static u32 wine_resource_records;
static u32 wine_retained_enabled,wine_retained_records,wine_retained_queries;
static struct {void* resource;void* memory;u32 texture,generation,sub,resource_id,call,interval,data,row,slice,open;} wine_retained_maps[2048];
static void wine_retained_emit(u32* r){r[0]=++wine_retained_records;if(wine_retained_records>131072||!write(wine_retained_file,r,192))ExitProcess(99);}
/* These two transitions run under journal_lock after the original result. */
static void wine_retained_map(const u32* map,void* resource,const u32* descriptor){
 if(!wine_retained_enabled)return;u32 at=2048;
 for(u32 i=0;i<2048;++i)if(wine_retained_maps[i].resource==resource&&wine_retained_maps[i].sub==map[12]){at=i;break;}
 if(at==2048)for(u32 i=0;i<2048;++i)if(!wine_retained_maps[i].resource){at=i;break;}if(at==2048)ExitProcess(99);
 wine_retained_maps[at].resource=resource;wine_retained_maps[at].memory=(void*)descriptor[2];wine_retained_maps[at].texture=map[28];wine_retained_maps[at].generation=map[29];wine_retained_maps[at].sub=map[12];wine_retained_maps[at].resource_id=map[10];wine_retained_maps[at].call=map[2];wine_retained_maps[at].interval=map[27];wine_retained_maps[at].data=map[26];wine_retained_maps[at].row=map[24];wine_retained_maps[at].slice=map[25];wine_retained_maps[at].open=1;
}
static void wine_retained_unmap(const u32* unmap){if(!wine_retained_enabled||!unmap[27])return;for(u32 i=0;i<2048;++i)if(wine_retained_maps[i].interval==unmap[27]){wine_retained_maps[i].open=0;return;}}
static void wine_retained_source(const u32* submission,void* texture){
 if(!wine_retained_enabled)return;u32 r[48]={0},at=2048,matches=0;void* memory=0;
 EnterCriticalSection(journal_lock);r[1]=3;r[2]=submission[2];r[3]=GetCurrentThreadId();r[4]=submission[5];r[5]=submission[11];r[6]=submission[28];r[7]=submission[13];r[16]=wine_resource_records;r[18]=wine_lifetime_records;r[20]=GetTickCount();
 if(r[6])for(u32 i=0;i<2048;++i)if(wine_retained_maps[i].resource&&wine_retained_maps[i].texture==r[5]&&wine_retained_maps[i].generation==r[6]&&wine_retained_maps[i].sub==r[7]){at=i;++matches;}
 if(matches==1){memory=wine_retained_maps[at].memory;r[8]=wine_retained_maps[at].resource_id;r[9]=wine_retained_maps[at].call;r[10]=wine_retained_maps[at].interval;r[11]=wine_retained_maps[at].data;r[12]=wine_retained_maps[at].row;r[13]=wine_retained_maps[at].slice;r[14]=wine_retained_maps[at].open;}else{at=2048;if(matches>1)r[15]=3;}r[22]=matches;
 LeaveCriticalSection(journal_lock);
 if(at!=2048){
  if(!wine_map_storage_getter){void* scene=GetModuleHandleA("MnmScene.dll");wine_map_storage_getter=scene?(StoragePoint)GetProcAddress(scene,storage_point_export_name(0)):0;if(!wine_map_storage_getter)ExitProcess(99);}
  r[15]=wine_map_storage_getter(memory,r[12],r[13],r+24,64);if(!r[15])memset(r+24,0,64);
 }
 EnterCriticalSection(journal_lock);r[17]=wine_resource_records;r[19]=wine_lifetime_records;r[21]=GetTickCount();
 if(at!=2048&&(wine_retained_maps[at].call!=r[9]||wine_retained_maps[at].interval!=r[10]||wine_retained_maps[at].texture!=r[5]||wine_retained_maps[at].generation!=r[6]||wine_retained_maps[at].open!=r[14])){r[15]=2;memset(r+24,0,64);}
 if(wine_lifetime_find(texture,1)!=r[6]){r[15]=4;memset(r+24,0,64);}
 ++wine_retained_queries;wine_retained_emit(r);LeaveCriticalSection(journal_lock);
}
static int wine_retained_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_RETAINED_MAP",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_points_enabled)return 0;
 char name[600];path(name,"wine-retained-map-source.bin");wine_retained_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_retained_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMWQS01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;if(!write(wine_retained_file,h,64))return 0;wine_retained_enabled=1;return 1;
}
static void wine_retained_close(u32 reason){if(!wine_retained_file)return;u32 r[48]={0};r[1]=4;r[2]=reason;r[3]=GetCurrentThreadId();r[4]=wine_retained_queries;wine_retained_emit(r);if(!CloseHandle(wine_retained_file))ExitProcess(99);wine_retained_file=0;wine_retained_enabled=0;}
