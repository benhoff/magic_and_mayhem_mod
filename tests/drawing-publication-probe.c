/* Actual production publication callback with scripted COM Lock/Unlock only.
 * No original instructions, driver surface or trampoline ABI claim. */
#define MNM_SCENE_SELFTEST 1
#include "../runtime/scene/canvas_producers.c"
static volatile u8 reserve[0x310000];
static u32 mode,lock_calls,unlock_calls,table[40],iface,wrapper[4],source_wrapper[4];
static u16 fixture_pixels[8]={1,2,3,0xeeee,4,5,6,0xffff};
static i32 WIN fixture_lock(void* self,void* rect,u32* desc,u32 flags,HANDLE event){
 if(self!=&iface||rect||flags!=0x4810||event)ExitProcess(3);
 ++lock_calls;SetLastError(0xbad);
 if(mode==3)return (i32)0x80004005u;
 desc[2]=2;desc[3]=3;desc[4]=8;desc[9]=(u32)fixture_pixels;
 desc[21]=mode==5?15:16;desc[22]=0xf800;desc[23]=0x7e0;desc[24]=31;return 0;
}
static i32 WIN fixture_unlock(void* self,void* pixels){
 if(self!=&iface||pixels!=fixture_pixels)ExitProcess(4);
 ++unlock_calls;SetLastError(0xbad);return mode==4?(i32)0x80004005u:0;
}
void start(void){
 reserve[sizeof(reserve)-1]=1;
 char value[4]={0};GetEnvironmentVariableA("MNM_PUBLICATION_CASE",value,sizeof(value));
 if(value[0]<'0'||value[0]>'8'||value[1])ExitProcess(1);mode=(u32)(value[0]-'0');
 if(!GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory)))ExitProcess(2);
 iface=(u32)table;table[25]=(u32)fixture_lock;table[32]=(u32)fixture_unlock;
 wrapper[2]=mode==8?0:(u32)&iface;
 surfaces[0].object=(u32)wrapper;surfaces[0].id=1;
 surfaces[1].object=(u32)source_wrapper;surfaces[1].id=2;
 put((void*)0x6f98c0,(u32)wrapper);queue=limit=1;sequence=5;
 oracle_byte_limit=mode==6?0:64;publication_bytes=mode==7?65:0;
 publication_file=CreateFileA("publication.bin",0x40000000,0,0,1,0x80,0);
 if(publication_file==(HANDLE)-1)ExitProcess(5);publication_enabled=1;
 u32 regs[8]={0};regs[0]=(u32)source_wrapper;regs[4]=(u32)wrapper;regs[7]=mode==2?0x80004001u:17;
 SetLastError(0x51a7);drawing_publication_observe(regs,mode==1?2:1);
 u32 callback_error=GetLastError();if(callback_error!=0x51a7||!CloseHandle(publication_file))ExitProcess(6);
 u32 expected_lock=mode==2||mode==8?0:1,expected_unlock=expected_lock&&mode!=3;
 u32 expected_frames=mode==0||mode==1||mode==4,expected_drops=mode>=3;
 if(lock_calls!=expected_lock||unlock_calls!=expected_unlock||publication_frames!=expected_frames||publication_drops!=expected_drops||publication_attempts!=1)ExitProcess(7);
 u32 metrics[8]={mode,lock_calls,unlock_calls,publication_frames,publication_drops,publication_bytes,callback_error,regs[7]},written=0;
 HANDLE out=CreateFileA("publication-controls.bin",0x40000000,0,0,1,0x80,0);
 if(out==(HANDLE)-1||!WriteFile(out,metrics,sizeof(metrics),&written,0)||written!=sizeof(metrics)||!CloseHandle(out))ExitProcess(8);
 ExitProcess(0);
}
