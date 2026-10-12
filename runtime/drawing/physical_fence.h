/* Reference-only physical witness after an observed successful primary API call.
 * Other original threads continue. No source ownership/global route claim. */
API void WIN Sleep(u32);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
static u32 physical_enabled,physical_byte_limit=1024u*1024u*1024u;
static int physical_budget(void){
 char text[8];u32 n=GetEnvironmentVariableA("MNM_DRAW_PHYSICAL_MIB",text,sizeof(text));
 if(!n)return 1;if(!physical_enabled||n>4)return 0;u32 mib=0;
 for(u32 i=0;i<n;++i){if(text[i]<'0'||text[i]>'9')return 0;mib=mib*10+(u32)(text[i]-'0');}
 if(!mib||mib>3072)return 0;physical_byte_limit=mib*1024u*1024u;return 1;
}
static void physical_name(char* out,const char* prefix,u32 ordinal){
 char suffix[80];u32 n=0;while(*prefix)suffix[n++]=*prefix++;
 n+=6;for(u32 i=0;i<6;++i){suffix[n-1-i]=(char)('0'+ordinal%10);ordinal/=10;}
 suffix[n++]='.';suffix[n++]='b';suffix[n++]='i';suffix[n++]='n';suffix[n]=0;path(out,suffix);
}
static void physical_fence(struct Context* c){
 if(!physical_enabled||!c->r[12])return;
 void* gdi=GetModuleHandleA("gdi32.dll");void* flush=gdi?GetProcAddress(gdi,"GdiFlush"):0;
 if(!flush||!((i32(WIN*)(void))flush)())ExitProcess(96);
 char name[600];physical_name(name,"api-physical-request-",c->r[12]);
 HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(file==(HANDLE)-1||!write(file,c->r,128)||!CloseHandle(file))ExitProcess(96);
 physical_name(name,"api-physical-ack-",c->r[12]);u32 start=GetTickCount();
 for(;;){
  file=CreateFileA(name,0x80000000,3,0,3,0x80,0);
  if(file!=(HANDLE)-1){u32 ack[4],got=0,size=GetFileSize(file,0);int okay=size==16&&ReadFile(file,ack,16,&got,0)&&got==16;CloseHandle(file);
   if(size>16)ExitProcess(96);
   if(okay){if(ack[0]!=0x414d4e4d||ack[1]!=0x31304950||ack[2]!=c->r[12]||ack[3]!=c->r[0])ExitProcess(96);return;}}
  if(GetTickCount()-start>=5000)ExitProcess(96);Sleep(1);
 }
}
