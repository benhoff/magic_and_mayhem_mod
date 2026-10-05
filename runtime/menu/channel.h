#include "battle.h"
#include "spells.h"
#include "mini.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(const void*);
API u32 WIN GetFileSize(HANDLE,u32*);
API u32 WIN GetTickCount(void);
static u32* menu_words;
static u32 menu_version=1,menu_size=MNM_MENU_V1_SIZE;
static u32 menu_generation,menu_screen,menu_ready,menu_owner,menu_thread;
static u32 menu_request,menu_status,menu_heartbeat,menu_seen;
static int menu_retired;
static u32 menu_handoff;
static u32 menu_load(u32* p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);}
static void menu_store(u32* p,u32 v){__atomic_store_n(p,v,__ATOMIC_RELEASE);}
static int menu_header(void){return equal(menu_words,menu_version==4?MNM_MENU_V4_MAGIC:menu_version==3?MNM_MENU_V3_MAGIC:menu_version==2?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8)&&menu_load(menu_words+2)==menu_version&&menu_load(menu_words+3)==menu_size;}
static int menu_host(u32* out){
    u32* p=menu_words+MNM_MENU_V1_HOST_WORD;u32 seq=menu_load(p);if(seq&1)return 0;
    for(u32 i=0;i<(menu_version>=2?23u:5u);++i)out[i]=menu_load(p+1+i);
    if(menu_version>=3)for(u32 i=0;i<63;++i)out[23+i]=menu_load(menu_words+MNM_MENU_V3_HOST_SLOTS/4+i);
    __atomic_thread_fence(__ATOMIC_ACQUIRE);return seq==menu_load(p);
}
static void menu_publish(void){
    u32* p=menu_words+(menu_version>=2?MNM_MENU_V2_ENGINE_WORD:MNM_MENU_V1_ENGINE_WORD);u32 seq=menu_load(p);
    menu_store(p,seq+1);
    u32 fields[6]={menu_generation,menu_screen,menu_retired?0:menu_ready,menu_request,menu_status,menu_thread};
    for(u32 i=0;i<6;++i)menu_store(p+1+i,fields[i]);
    if(menu_version>=2)menu_store(p+7,menu_handoff);
    if(menu_version>=2)copy((u8*)menu_words+MNM_MENU_V2_MAP,battle_payload,sizeof(battle_payload));
    if(menu_version>=3)copy((u8*)menu_words+MNM_MENU_V3_SPELL,spell_payload,sizeof(spell_payload));
    if(menu_version==4)copy((u8*)menu_words+MNM_MENU_V4_MINI,mini_payload,sizeof(mini_payload));
    menu_store(p,seq+2);
}
static void menu_init(void){
    char path[1024];u32 length=GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));if(!length||length>=sizeof(path))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return;
    menu_size=GetFileSize(file,0);
    if(menu_size!=MNM_MENU_V1_SIZE&&menu_size!=MNM_MENU_V2_SIZE&&menu_size!=MNM_MENU_V3_SIZE&&menu_size!=MNM_MENU_V4_SIZE){CloseHandle(file);return;}
    menu_version=menu_size==MNM_MENU_V4_SIZE?4:menu_size==MNM_MENU_V3_SIZE?3:menu_size==MNM_MENU_V2_SIZE?2:1;
    HANDLE mapping=CreateFileMappingA(file,0,4,0,0,0);CloseHandle(file);if(!mapping)return;
    menu_words=MapViewOfFile(mapping,2,0,0,menu_size);CloseHandle(mapping);if(!menu_words)return;
    u32 host[86];
    if(!menu_header()||!menu_host(host)||!host[0]||host[2]||menu_load(menu_words+(menu_version>=2?32:16))){
        UnmapViewOfFile(menu_words);menu_words=0;return;
    }
    if(menu_version==4&&!MNM_MENU_MINI_EXPERIMENTAL)menu_retired=1;
    menu_heartbeat=host[1];menu_seen=GetTickCount();
}
static void menu_state(void* object){
    u32 owner=get((void*)0x6f34e0),screen=0,ready=0;
    // Only the currently ticking initialized object can authorize a command.
    if(owner==(u32)object&&readable(object,0x47)){
        u8* p=object;u32 table=get(p),id=get(p+4);
        if((id==3&&table==0x5c63e4)||(id==22&&table==0x5c641c)||(menu_version>=2&&((id==14&&table==0x5c6534)||(id==25&&table==0x5c6940)))){
            screen=id;ready=get(p+8)==1&&!p[0xc]&&!get(p+0x33)&&!get(p+0x43);
        }
    }
    if(menu_version>=3&&owner==(u32)object&&readable(object,0x26a)&&get(object)==0x5c775c&&get((u8*)object+4)==7){
        u8* p=object;screen=7;ready=get(p+8)==1&&!p[0xc]&&!get(p+0x12)&&!get(p+0x3e);
        if(ready){static u8 next[MNM_MENU_V3_SPELL_SIZE];if(!spell_snapshot(object,next))ready=0;
            else {if(!spell_valid||!equal(next,spell_payload,4)||!equal(next+8,spell_payload+8,sizeof(next)-8))++menu_generation;copy(spell_payload,next,sizeof(next));spell_valid=1;}}
    }
    if(menu_version==4&&MNM_MENU_MINI_EXPERIMENTAL&&owner==(u32)object&&readable(object,0x5b)&&get(object)==0x5c6644&&get((u8*)object+4)==MNM_MENU_MINI_SCREEN){
        u8 next[MNM_MENU_V4_MINI_SIZE];
        if(mini_snapshot(object,next)){
            screen=MNM_MENU_MINI_SCREEN;
            if(!equal(next,mini_payload,sizeof(next)))++menu_generation;
            copy(mini_payload,next,sizeof(next));u8* p=object;
            ready=get(p+8)==1&&!p[0xc]&&!get(p+0x33)&&!get(p+0x37)&&!get(p+0x43)&&!get(next+4);
        }
    }
    if(menu_version>=3&&menu_screen==7&&screen==0&&owner!=menu_owner)menu_handoff=2;
    if(menu_version>=2&&ready&&(screen==14||screen==25)){
        static u8 next[MNM_MENU_V2_PAYLOAD_SIZE];
        if(!battle_snapshot(next))ready=0;
        else if(!battle_valid||!equal(next,battle_payload,sizeof(next))){copy(battle_payload,next,sizeof(next));battle_valid=1;++menu_generation;}
    }
    if(owner!=menu_owner||screen!=menu_screen||ready!=menu_ready){
        ++menu_generation;menu_owner=owner;menu_screen=screen;menu_ready=ready;
    }
}
static void menu_poll(void* object,int execute){
    if(!menu_words)return;
    u32 error=GetLastError(),thread=GetCurrentThreadId();
    if(!menu_thread)menu_thread=thread;
    if(thread!=menu_thread){SetLastError(error);return;}
    if(!menu_header()){menu_retired=1;SetLastError(error);return;}
    menu_state(object);
    if(menu_version>=2&&menu_handoff&&menu_ready&&(menu_screen==3||menu_screen==22))menu_handoff=0;
    u32 host[86],now=GetTickCount();
    if(menu_host(host)){
        if(host[1]!=menu_heartbeat){menu_heartbeat=host[1];menu_seen=now;}
        if(!host[0]||now-menu_seen>MNM_MENU_V1_LEASE_MS)menu_retired=1;
        if(execute&&host[2]>menu_request){
            // Consume rejected requests too: never dispatch an ID twice.
            menu_request=host[2];menu_status=MNM_MENU_OK;
            if(menu_retired)menu_status=MNM_MENU_RETIRED;
            else if(host[4]!=menu_generation)menu_status=MNM_MENU_STALE;
            else if(!menu_ready)menu_status=MNM_MENU_UNAVAILABLE;
            else if(host[3]==MNM_MENU_OPEN_QUICK&&menu_screen==3)main_action(object,2);
            else if(host[3]==MNM_MENU_BACK&&menu_screen==22)quick_action(object,3);
            else if(host[3]==MNM_MENU_QUIT&&menu_screen==3)main_action(object,4);
            else if(menu_version>=3&&menu_screen==7&&host[3]==MNM_MENU_SPELL_FINISH)menu_status=spell_finish(object,host+23);
            else if(menu_version==4&&menu_screen==MNM_MENU_MINI_SCREEN)menu_status=mini_dispatch(object,host[3]);
            else if(menu_version>=2)menu_status=battle_dispatch(object,host,menu_screen);
            else menu_status=MNM_MENU_UNSUPPORTED;
            if(menu_version>=2&&host[3]==MNM_MENU_SETUP_START&&menu_status==MNM_MENU_OK){
                u32 next=get((u8*)object+0x33);menu_handoff=next==0x6f2aa0?1:next==0x6903b8?2:0;
            }
            if(menu_version>=3&&host[3]==MNM_MENU_SPELL_FINISH&&menu_status==MNM_MENU_OK)menu_handoff=2;
            menu_state(object);
        }
    }else if(now-menu_seen>MNM_MENU_V1_LEASE_MS)menu_retired=1;
    if(menu_retired)menu_status=MNM_MENU_RETIRED;
    menu_publish();SetLastError(error);
}
