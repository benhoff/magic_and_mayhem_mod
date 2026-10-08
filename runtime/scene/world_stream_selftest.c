/* Exercise the actual PE32 producer without original game inputs. */
#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/world_channel_v2.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
extern int world_stream_init(void);
extern int world_stream_queue(u32);
extern void world_stream_missing(void);
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
    HANDLE map=CreateFileMappingA(file,0,4,0,0,0);CloseHandle(file);if(!map)ExitProcess(3);
    u8* mapping=MapViewOfFile(map,2,0,0,0);CloseHandle(map);if(!mapping||!world_stream_init())ExitProcess(4);
    u32* header=(u32*)mapping,*slot=(u32*)(mapping+MNM_WCH_HEADER);u32 sequence=0;
    if(header[2]==2){
        char mode[32];GetEnvironmentVariableA("MNM_HISTORY_TEST",mode,sizeof(mode));
        if(mode[0]=='g'){if(world_stream_queue(2)||header[5]!=MNM_WCH_FAILED||header[10]!=103)ExitProcess(20);world_stream_close();ExitProcess(0);}
        if(!world_stream_queue(1))ExitProcess(21);
        if(mode[0]=='m'){world_stream_missing();if(header[5]!=MNM_WCH_FAILED||header[10]!=103)ExitProcess(22);world_stream_close();ExitProcess(0);}
        u16 canvas[32];for(u32 i=0;i<32;++i)canvas[i]=(u16)(i+1);
        for(u32 index=0;index<header[13];++index){
            if(index&&!world_stream_queue(index+1))ExitProcess(23);
            u8* input=world_stream_begin(&sequence);if(!input||sequence!=index+1)ExitProcess(24);
            world_stream_missing();envelope(input,sequence,0);
            u32 failure=mode[0]=='f'?MNM_WORLD_UNSUPPORTED_KIND:0;
            world_stream_publish(144,failure,(u32)canvas+(mode[0]=='c'&&index?2:0),8,4,8);
            if(failure||(mode[0]=='c'&&index)){
                if(header[5]!=MNM_WCH_FAILED||header[10]!=(failure?failure:104))ExitProcess(25);
                world_stream_close();ExitProcess(0);
            }
            u32* row=(u32*)(mapping+MNM_WCH_HEADER+index*MNM_WCH_HISTORY_SLOT);
            if(row[0]!=2||row[1]!=index+1||row[2]!=144||row[3]!=((header[11]&MNM_WCH_VERIFY)?64:0)||row[4]!=1||row[5]!=index+1||row[6]!=(index?0:1)||row[7])ExitProcess(26);
            if(header[11]&MNM_WCH_VERIFY){u16* oracle=(u16*)((u8*)row+32+MNM_WORLD_MAX_BYTES);for(u32 i=0;i<32;++i)if(oracle[i]!=canvas[i])ExitProcess(27);}
        }
        if(header[5]!=MNM_WCH_ENDED||header[7]!=header[13]||header[9]||world_stream_queue(header[13]+1))ExitProcess(28);
        world_stream_close();ExitProcess(0);
    }
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
