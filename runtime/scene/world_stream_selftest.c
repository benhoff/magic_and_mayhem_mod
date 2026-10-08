/* Exercise the actual PE32 producer without original game inputs. */
#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/world_channel_v1.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
extern int world_stream_init(void);
extern u8* world_stream_begin(u32*);
extern void world_stream_publish(u32,u32,u32,u32,u32,u32);
extern void world_stream_close(void);
static void put(u8* p,u32 offset,u32 value){for(u32 i=0;i<4;++i)p[offset+i]=(u8)(value>>(i*8));}
static void envelope(u8* p,u32 sequence,u32 failure){
    for(u32 i=0;i<144;++i)p[i]=0;
    const char* magic=MNM_WORLD_MAGIC;for(u32 i=0;i<8;++i)p[i]=(u8)magic[i];
    put(p,8,1);put(p,12,80);put(p,16,144);put(p,20,sequence);put(p,24,8);put(p,28,4);put(p,32,8);
    put(p,36,1);put(p,40,failure);put(p,44,MNM_WORLD_BUILD);
}
void start(void){
    char path[260];u32 n=GetEnvironmentVariableA("MNM_WORLD_CHANNEL",path,sizeof(path));if(!n||n>=sizeof(path))ExitProcess(1);
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)ExitProcess(2);
    HANDLE map=CreateFileMappingA(file,0,4,0,MNM_WCH_SIZE,0);CloseHandle(file);if(!map)ExitProcess(3);
    u8* mapping=MapViewOfFile(map,2,0,0,MNM_WCH_SIZE);CloseHandle(map);if(!mapping||!world_stream_init())ExitProcess(4);
    u32* header=(u32*)mapping,*slot=(u32*)(mapping+MNM_WCH_HEADER);u32 sequence=0;
    u8* p=world_stream_begin(&sequence);if(!p||sequence!=1)ExitProcess(5);
    envelope(p,sequence,MNM_WORLD_UNSUPPORTED_KIND);
    // Unreadable oracle address proves ordinary refusal publication never
    // samples original pixels. The real verify producer refuses before access.
    world_stream_publish(144,MNM_WORLD_UNSUPPORTED_KIND,1,8,4,8);
    if(header[11]&MNM_WCH_VERIFY){if(header[5]!=MNM_WCH_FAILED||header[10]!=8||slot[0]||header[7])ExitProcess(6);world_stream_close();ExitProcess(0);}
    if(header[5]!=MNM_WCH_ACTIVE||slot[0]!=2||slot[1]!=1||slot[2]!=80||slot[3]||((u32*)(mapping+MNM_WCH_HEADER+16))[9]||((u32*)(mapping+MNM_WCH_HEADER+16))[4]!=80)ExitProcess(7);
    mnm_wch_store(slot,0);
    p=world_stream_begin(&sequence);if(!p||sequence!=2)ExitProcess(8);envelope(p,sequence,MNM_WORLD_UNSUPPORTED_WAVE);
    world_stream_publish(144,MNM_WORLD_UNSUPPORTED_WAVE,1,8,4,8);if(header[5]!=MNM_WCH_ACTIVE||slot[0]!=2||slot[2]!=80||slot[3])ExitProcess(9);
    mnm_wch_store(slot,0);
    p=world_stream_begin(&sequence);if(!p||sequence!=3)ExitProcess(10);envelope(p,sequence,0);
    world_stream_publish(144,0,1,8,4,8);if(header[5]!=MNM_WCH_ACTIVE||slot[0]!=2||slot[1]!=3||slot[2]!=144||slot[3])ExitProcess(11);
    mnm_wch_store(slot,0);
    p=world_stream_begin(&sequence);if(!p||sequence!=4)ExitProcess(12);envelope(p,sequence,2);
    world_stream_publish(144,2,1,8,4,8);if(header[5]!=MNM_WCH_FAILED||header[10]!=2||slot[0])ExitProcess(13);
    world_stream_close();ExitProcess(0);
}
