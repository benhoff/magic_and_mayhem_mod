/* Sourcebytewitnesses are externaloracles,never nativeinput. */
typedef u32(WIN* SourceBytes)(void*,u32*,u32*,u8*,u32);
static SourceBytes source_bytes_getter;
static HANDLE source_bytes_file;
static u32 source_bytes_enabled,source_bytes_records,source_bytes_planes,source_bytes_total;
static int source_bytes_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_BYTES",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_points_enabled)return 0;
 char name[600];path(name,"api-source-bytes.bin");source_bytes_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(source_bytes_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMSBY01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=4096;h[6]=1024;h[7]=1024u*1024u*1024u;if(!write(source_bytes_file,h,64))return 0;source_bytes_enabled=1;return 1;
}
static void source_bytes_capture(struct Context* c,u32 phase){
 if(!source_bytes_enabled||!c->observed||c->r[1]!=1||c->r[2]!=7||c->args[4]!=16)return;
 if(c->cursor_gate_requested&&!c->cursor_gate_token)return;
 if(!source_bytes_getter){void* scene=GetModuleHandleA("MnmScene.dll");source_bytes_getter=scene?(SourceBytes)GetProcAddress(scene,"SceneDrawingSourceBytesV1"):0;if(!source_bytes_getter)ExitProcess(99);}
 u32 r[48]={0};r[1]=phase;r[2]=c->r[29];r[3]=c->r[6];r[4]=GetTickCount();r[5]=c->r[4];r[6]=c->r[3];
 u8* pixels=VirtualAlloc(0,800*600*2,0x3000,4);if(!pixels)ExitProcess(99);
 r[7]=source_bytes_getter((void*)c->args[2],r+16,r+32,pixels,800*600*2);
 if(!r[7]&&(r[23]!=800||r[24]!=600||c->args[0]||c->args[1]||!readable((void*)c->args[3],16)||((u32*)c->args[3])[0]||((u32*)c->args[3])[1]||((u32*)c->args[3])[2]!=800||((u32*)c->args[3])[3]!=600))r[7]=5;
 if(!r[7]){EnterCriticalSection(journal_lock);if(source_bytes_planes>=1024||source_bytes_total>1024u*1024u*1024u-800*600*2)ExitProcess(99);r[8]=++source_bytes_planes;source_bytes_total+=800*600*2;LeaveCriticalSection(journal_lock);
  char name[600],suffix[40];static const char prefix[]="api-source-oracle-000000.565";copy(suffix,prefix,sizeof(prefix));u32 ordinal=r[8];for(i32 i=23;i>=18;--i){suffix[i]=(char)('0'+ordinal%10);ordinal/=10;}path(name,suffix);HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1||!write(file,pixels,800*600*2)||!CloseHandle(file))ExitProcess(99);r[9]=800*600*2;
 }
 VirtualFree(pixels,0,0x8000);EnterCriticalSection(journal_lock);r[0]=++source_bytes_records;if(source_bytes_records>4096||!write(source_bytes_file,r,192))ExitProcess(99);LeaveCriticalSection(journal_lock);
}
static void source_bytes_close(u32 reason){if(!source_bytes_file)return;u32 r[48]={0};r[0]=++source_bytes_records;r[1]=4;r[2]=reason;r[8]=source_bytes_planes;r[9]=source_bytes_total;if(!write(source_bytes_file,r,192)||!CloseHandle(source_bytes_file))ExitProcess(99);source_bytes_file=0;source_bytes_enabled=0;}
