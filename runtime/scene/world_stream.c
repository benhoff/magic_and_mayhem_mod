#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/world_channel_v1.h"
__declspec(dllimport) HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
__declspec(dllimport) void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
__declspec(dllimport) u32 WIN GetFileSize(HANDLE,u32*);
__declspec(dllimport) int WIN UnmapViewOfFile(void*);
static u8* mapping;
static u32 session,sequence;
static u8* slot;
static u32* word(u32 offset){return (u32*)(mapping+offset);}
static int identity(void){
    const char* magic=MNM_WCH_MAGIC;
    for(u32 i=0;i<8;++i)if(mapping[i]!=(u8)magic[i])return 0;
    if(*word(8)!=1||*word(12)!=MNM_WCH_SIZE||*word(16)!=session||
       *word(44)>MNM_WCH_VERIFY||*word(52)!=2||*word(56)!=MNM_WORLD_MAX_BYTES||
       *word(60)!=MNM_WCH_ORACLE||*word(64)!=MNM_WORLD_BUILD)return 0;
    for(u32 i=68;i<MNM_WCH_HEADER;i+=4)if(*word(i))return 0;
    return 1;
}
void world_stream_fail(u32 reason){if(!mapping)return;*word(40)=reason;mnm_wch_store(word(20),MNM_WCH_FAILED);if(slot){mnm_wch_store((u32*)slot,0);slot=0;}}
int world_stream_init(void){
    char path[260];u32 n=GetEnvironmentVariableA("MNM_WORLD_CHANNEL",path,sizeof(path));if(!n||n>=sizeof(path))return 0;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return 0;
    u32 high=0,size=GetFileSize(file,&high);if(high||size!=MNM_WCH_SIZE){CloseHandle(file);return 0;}
    HANDLE map=CreateFileMappingA(file,0,4,0,MNM_WCH_SIZE,0);CloseHandle(file);if(!map)return 0;
    mapping=MapViewOfFile(map,2,0,0,MNM_WCH_SIZE);CloseHandle(map);if(!mapping)return 0;
    session=*word(16);
    if(!session||!identity()||mnm_wch_load(word(20))!=MNM_WCH_WAITING||
       mnm_wch_load(word(24))||mnm_wch_load(word(28))||mnm_wch_load(word(32))||
       mnm_wch_load((u32*)(mapping+MNM_WCH_HEADER))||
       mnm_wch_load((u32*)(mapping+MNM_WCH_HEADER+MNM_WCH_SLOT))){UnmapViewOfFile(mapping);mapping=0;return 0;}
    mnm_wch_store(word(20),MNM_WCH_ACTIVE);return 1;
}
u8* world_stream_begin(u32* number){
    if(!mapping)return 0;
    if(!identity()){world_stream_fail(100);return 0;}
    if(mnm_wch_load(word(24))){mnm_wch_store(word(20),MNM_WCH_ENDED);return 0;}
    if(mnm_wch_load(word(20))!=MNM_WCH_ACTIVE)return 0;
    for(u32 i=0;i<2;++i){u8* candidate=mapping+MNM_WCH_HEADER+i*MNM_WCH_SLOT;
        if(mnm_wch_cas((u32*)candidate,0,1)){slot=candidate;
            if(sequence==0xffffffff){world_stream_fail(101);return 0;}
            *number=++sequence;return slot+16;}}
    ++*word(36);return 0;
}
void world_stream_publish(u32 size,u32 failure,u32 canvas,u32 width,u32 height,u32 stride){
    if(!slot)return;
    if(!identity()){world_stream_fail(100);return;}
    if(mnm_wch_load(word(24))){mnm_wch_store((u32*)slot,0);slot=0;mnm_wch_store(word(20),MNM_WCH_ENDED);return;}
    if(failure){
        if((*word(44)&MNM_WCH_VERIFY)||(failure!=MNM_WORLD_UNSUPPORTED_WAVE&&failure!=MNM_WORLD_UNSUPPORTED_KIND)){world_stream_fail(failure);return;}
        // Normal shadow mode reports a whole-frame capability refusal and keeps
        // original drawing active. Strip all partial requests from the packet.
        u32* input=(u32*)(slot+16);size=MNM_WORLD_HEADER;input[4]=size;input[9]=0;
    }
    u32 oracle=0;
    if(*word(44)&MNM_WCH_VERIFY){oracle=width*height*2;if(oracle>MNM_WCH_ORACLE){world_stream_fail(102);return;}
        u8* out=slot+16+MNM_WORLD_MAX_BYTES;
        for(u32 y=0;y<height;++y)for(u32 x=0;x<width*2;++x)out[y*width*2+x]=((u8*)canvas)[y*stride*2+x];}
    ((u32*)slot)[1]=sequence;((u32*)slot)[2]=size;((u32*)slot)[3]=oracle;
    mnm_wch_store((u32*)slot,2);mnm_wch_store(word(28),sequence);slot=0;
}
void world_stream_close(void){if(mapping){if(mnm_wch_load(word(20))==MNM_WCH_ACTIVE)mnm_wch_store(word(20),MNM_WCH_ENDED);UnmapViewOfFile(mapping);mapping=0;}}
