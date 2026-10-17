/* Diagnostic public read mapping only; never reuse an unmapped data pointer. */
static u32 wine_source_map_enabled,wine_source_map_records,wine_source_map_samples;
static u32 wine_source_map_payloads,wine_source_map_attempts[2],wine_source_map_busy;
static HANDLE wine_source_map_file;
static int(WIN *wine_source_map_counter)(void*);
static u32 wine_source_map_bounded,wine_source_map_total_attempts,wine_source_map_total_bytes;
static u32 wine_source_map_attempt_limit=8192,wine_source_map_byte_limit=0x80000000u;
static int wine_source_map_limit(const char* name,u32 maximum,u32* value){
 char text[12];u32 n=GetEnvironmentVariableA(name,text,sizeof(text));if(!n)return 1;
 if(n>=sizeof(text)||text[0]=='0')return 0;u32 result=0;
 for(u32 i=0;i<n;++i){if(text[i]<'0'||text[i]>'9')return 0;u32 digit=text[i]-'0';if(result>maximum/10||(result==maximum/10&&digit>maximum%10))return 0;result=result*10+digit;}
 if(!result)return 0;*value=result;return 1;
}
static u32 wine_source_map_crc(const u8* data,u32 n){
 u32 crc=0xffffffffu;while(n--){crc^=*data++;for(u32 bit=0;bit<8;++bit)crc=(crc>>1)^(0xedb88320u&(0u-(crc&1)));}return crc^0xffffffffu;
}
static void wine_source_map_record(u32* r,void* texture){
 (void)texture;EnterCriticalSection(journal_lock);
 r[0]=++wine_source_map_records;++wine_source_map_samples;
 if(wine_source_map_records>16384||!write(wine_source_map_file,r,256))ExitProcess(99);
 LeaveCriticalSection(journal_lock);
}
static void wine_source_map(const u32* submission,void* texture){
 if(!wine_source_map_enabled)return;
 u32 r[64]={0};r[1]=1;r[2]=submission[2];r[3]=submission[5];r[4]=GetCurrentThreadId();r[5]=GetTickCount();r[6]=submission[11];r[7]=submission[28];r[8]=submission[13];
 EnterCriticalSection(journal_lock);r[16]=wine_lifetime_records;u32 live=texture&&r[7]&&wine_lifetime_find(texture,1)==r[7];LeaveCriticalSection(journal_lock);
 if(!__sync_bool_compare_and_swap(&wine_source_map_busy,0,1)){r[10]=10;r[17]=r[16];r[18]=live?r[7]:0;wine_source_map_record(r,texture);ExitProcess(99);}
 if(wine_source_map_bounded&&!wine_source_map_counter(r+39))ExitProcess(99);
 r[10]=1;r[38]=1;void* resource=0;void* owned=0;u32 size=0,version_epoch=0;
 if(live){
  resource=((void*(__attribute__((cdecl))*)(void*))wine_resource_original[0])(texture);r[34]=1;r[38]=2;
  if(resource){
   EnterCriticalSection(journal_lock);r[9]=wine_resource_id(resource,1);LeaveCriticalSection(journal_lock);
   r[11]=wine_source_descriptor_get(resource,r[8],r+19);r[34]=2;r[38]=3;
   if((i32)r[11]>=0){
    r[38]=0;r[10]=2;
    if(r[19]==116&&!r[20]&&!r[21]&&r[27]==1){
     if(wine_source_map_bounded){if((r[24]&4)&&r[25]&&r[25]<=800&&r[26]&&r[26]<=600&&r[28]>=r[25]*r[26]*2&&r[28]<=r[26]*32768)r[35]=1;}
     else if((r[24]&5)==5){if(r[25]==40&&r[26]==40)r[35]=1;else if(r[25]==400&&r[26]==280)r[35]=2;}
    }
    if(r[35]){
     EnterCriticalSection(journal_lock);u32 active=0;
     for(u32 i=0;i<2048;++i)if(wine_resource_active[i].resource==resource&&wine_resource_active[i].sub==r[8])active=1;
     u32 current=wine_lifetime_find(texture,1);LeaveCriticalSection(journal_lock);
     if(current!=r[7])r[10]=7;
     else if(active)r[10]=4;
     else if(wine_source_map_bounded&&(wine_source_map_total_attempts>=wine_source_map_attempt_limit||r[25]*r[26]*2>wine_source_map_byte_limit-wine_source_map_total_bytes)){r[10]=3;r[31]=wine_source_map_total_attempts;}
     else if(!wine_source_map_bounded&&wine_source_map_attempts[r[35]-1]>=16){r[10]=3;r[31]=16;}
     else{
      r[31]=wine_source_map_bounded?++wine_source_map_total_attempts:++wine_source_map_attempts[r[35]-1];size=r[25]*r[26]*2;owned=VirtualAlloc(0,size,0x3000,4);
      if(!owned)r[10]=8;
      else{
       wine_reserve_begin(r,texture,resource);
       u32 mapped[3]={0};r[37]=0x80000000u;
       r[12]=((u32(__attribute__((cdecl))*)(void*,u32,u32*,const u32*,u32))wine_resource_original[1])(resource,r[8],mapped,0,r[37]);r[13]=GetLastError();++r[34];
       r[10]=5;
       if((i32)r[12]>=0){
        r[29]=mapped[0];r[30]=mapped[1];r[10]=6;
        if(mapped[0]>=r[25]*2&&mapped[0]<=32768&&readable((void*)mapped[2],(r[26]-1)*mapped[0]+r[25]*2)){
         version_epoch=wine_source_version_read(resource);
         for(u32 y=0;y<r[26];++y)copy((u8*)owned+y*r[25]*2,(void*)(mapped[2]+y*mapped[0]),r[25]*2);
         r[10]=0;
        }
        r[14]=((u32(__attribute__((cdecl))*)(void*,u32))wine_resource_original[2])(resource,r[8]);r[15]=GetLastError();++r[34];
        if((i32)r[14]<0)r[10]=9;
       }
      }
     }
    }
   }else for(u32 i=19;i<29;++i)r[i]=0;
  }
 }
 if(wine_source_map_bounded&&!wine_source_map_counter(r+41))ExitProcess(99);
 EnterCriticalSection(journal_lock);r[17]=wine_lifetime_records;u32 after=r[18]=wine_lifetime_find(texture,1);LeaveCriticalSection(journal_lock);
 if(r[7]&&after!=r[7]&&r[10]<8&&r[10]!=1)r[10]=7;
 if(r[10]==0){
  r[32]=size;r[33]=wine_source_map_payloads+1;r[36]=wine_source_map_crc(owned,size);
  char name[600],suffix[]="wine-source-map-000000.565";u32 ordinal=r[33];
  for(i32 i=21;i>=16;--i){suffix[i]=(char)('0'+ordinal%10);ordinal/=10;}path(name,suffix);
  HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);u32 okay=file!=(HANDLE)-1;
  if(okay){okay=write(file,owned,size);if(!CloseHandle(file))okay=0;}
  if(!okay){r[10]=8;r[32]=r[33]=r[36]=0;}else{++wine_source_map_payloads;wine_source_map_total_bytes+=size;}
 }
 if(owned&&!VirtualFree(owned,0,0x8000))ExitProcess(99);
 if(r[10]==0)wine_source_version_snapshot(r,texture,resource,version_epoch);
 wine_source_map_record(r,texture);wine_reserve_unsupported(r[10]);
 if(r[10]>=8)ExitProcess(99);
 __sync_lock_release(&wine_source_map_busy);
}
static int wine_source_map_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SOURCE_MAP",flag,2);if(!n)return 1;
 if(n!=1||(flag[0]!='1'&&flag[0]!='2')||!wine_source_descriptor_enabled||!wine_resource_enabled||!wine_texture_entry_enabled)return 0;
 wine_source_map_bounded=flag[0]=='2';
 if(wine_source_map_bounded&&(!wine_source_map_limit("MNM_DRAW_WINE_SOURCE_MAP_ATTEMPTS",8192,&wine_source_map_attempt_limit)||!wine_source_map_limit("MNM_DRAW_WINE_SOURCE_MAP_BYTES",0x80000000u,&wine_source_map_byte_limit)))return 0;
 if(wine_source_map_bounded){wine_source_map_counter=GetProcAddress(GetModuleHandleA("kernel32.dll"),"QueryPerformanceCounter");if(!wine_source_map_counter)return 0;}
 char name[600];path(name,"wine-source-map.bin");wine_source_map_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_source_map_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWMS01",8);h[2]=1;h[3]=256;h[4]=0x40209ca7;h[5]=16384;h[6]=16;h[7]=2;h[8]=4;h[9]=0x80000000u;h[10]=116;h[11]=3635200;h[12]=h[13]=40;h[14]=400;h[15]=280;
 if(wine_source_map_bounded){copy(h,"MNMWMS04",8);h[2]=4;h[6]=wine_source_map_attempt_limit;h[7]=1;h[11]=wine_source_map_byte_limit;h[12]=800;h[13]=600;h[14]=h[15]=0;}
 if(!write(wine_source_map_file,h,64))return 0;wine_source_map_enabled=1;return 1;
}
static int wine_source_map_install(void){return !wine_source_map_enabled||(wine_resource_original[1]&&wine_resource_original[2]&&wine_source_descriptor_get);}
static void wine_source_map_close(u32 reason){
 if(!wine_source_map_file)return;if(wine_source_map_busy)ExitProcess(99);
 u32 r[64]={0};r[0]=++wine_source_map_records;r[1]=4;r[2]=reason;r[3]=wine_source_map_samples;r[4]=wine_source_map_payloads;r[5]=wine_source_map_attempts[0];r[6]=wine_source_map_attempts[1];
 if(wine_source_map_bounded){r[5]=wine_source_map_total_attempts;r[6]=wine_source_map_total_bytes;}
 if(wine_source_map_records>16384||!write(wine_source_map_file,r,256)||!CloseHandle(wine_source_map_file))ExitProcess(99);wine_source_map_file=0;wine_source_map_enabled=0;
}
