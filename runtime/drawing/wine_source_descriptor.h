/* Extra public metadata queries only, explicitly separate from original calls. */
static u32 wine_source_descriptor_enabled,wine_source_descriptor_records,wine_source_descriptor_samples;
static HANDLE wine_source_descriptor_file;
static u32(__attribute__((cdecl))*wine_source_descriptor_get)(void*,u32,u32*);
static void wine_source_descriptor_emit(u32* row){
 row[0]=++wine_source_descriptor_records;
 if(wine_source_descriptor_records>16384||!write(wine_source_descriptor_file,row,160))ExitProcess(99);
}
static void wine_source_descriptor(const u32* submission,void* texture){
 if(!wine_source_descriptor_enabled)return;
 u32 row[40]={0};row[1]=1;row[2]=submission[2];row[3]=submission[5];row[4]=GetCurrentThreadId();row[5]=GetTickCount();row[6]=submission[11];row[7]=submission[28];row[8]=submission[13];
 EnterCriticalSection(journal_lock);row[13]=wine_lifetime_records;
 u32 live=texture&&row[7]&&wine_lifetime_find(texture,1)==row[7];LeaveCriticalSection(journal_lock);
 row[10]=1;
 if(live){
  void* resource=((void*(__attribute__((cdecl))*)(void*))wine_resource_original[0])(texture);row[26]=1;
  if(!resource)row[10]=2;
  else{
   EnterCriticalSection(journal_lock);row[9]=wine_resource_id(resource,1);LeaveCriticalSection(journal_lock);
   u32 descriptor[10]={0};row[11]=wine_source_descriptor_get(resource,row[8],descriptor);row[12]=GetLastError();row[26]=2;
   row[10]=(i32)row[11]<0?3:0;if(!row[10])copy(row+16,descriptor,40);
  }
 }
 EnterCriticalSection(journal_lock);row[14]=wine_lifetime_records;row[15]=wine_lifetime_find(texture,1);
 if(live&&row[15]!=row[7])row[10]=4;
 ++wine_source_descriptor_samples;wine_source_descriptor_emit(row);LeaveCriticalSection(journal_lock);
}
static int wine_source_descriptor_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SOURCE_DESCRIPTOR",flag,2);if(!n)return 1;
 if(n!=1||flag[0]!='1'||!wine_resource_enabled||!wine_texture_entry_enabled)return 0;
 char name[600];path(name,"wine-source-descriptor.bin");wine_source_descriptor_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_source_descriptor_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWSD01",8);h[2]=1;h[3]=160;h[4]=0x40209ca7;h[5]=16384;h[6]=2048;h[7]=2;
 if(!write(wine_source_descriptor_file,h,64))return 0;wine_source_descriptor_enabled=1;return 1;
}
static int wine_source_descriptor_install(void* module){
 if(!wine_source_descriptor_enabled)return 1;
 void* target=GetProcAddress(module,"wined3d_resource_get_sub_resource_desc");
 if(!wine_resource_original[0]||target!=(void*)((u32)module+0x204340))return 0;
 wine_source_descriptor_get=(u32(__attribute__((cdecl))*)(void*,u32,u32*))target;return 1;
}
static void wine_source_descriptor_close(u32 reason){
 if(!wine_source_descriptor_file)return;u32 row[40]={0};row[1]=4;row[2]=reason;row[3]=wine_source_descriptor_samples;wine_source_descriptor_emit(row);
 if(!CloseHandle(wine_source_descriptor_file))ExitProcess(99);wine_source_descriptor_file=0;
}
