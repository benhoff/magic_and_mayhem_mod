#include "../../protocols/include/mnm/menu_v1.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API void WIN Sleep(u32);
static u32* channel;
static u32 command_id,host_seq,beat;
static void host(u32 action,u32 generation,u32 alive){
    channel[4]=++host_seq;
    channel[5]=alive;channel[6]=++beat;channel[7]=command_id;channel[8]=action;channel[9]=generation;
    __atomic_thread_fence(__ATOMIC_RELEASE);channel[4]=++host_seq;
}
static void channel_tick(TickFn fn){SetLastError(0xabc123);require(fn(object)==0x13579bdf&&GetLastError()==0xabc123,41);}
static void idle(TickFn fn,u32 id){reset(id,id==3?0x5c63e4:0x5c641c);channel_tick(fn);require(channel[18]==id&&channel[19]==1,42);}
static void channel_tests(TickFn fn){
    char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);require(file!=(HANDLE)-1,43);
    HANDLE mapping=CreateFileMappingA(file,0,4,0,0,0);CloseHandle(file);require(mapping!=0,44);
    channel=MapViewOfFile(mapping,2,0,0,MNM_MENU_V1_SIZE);CloseHandle(mapping);require(channel!=0,45);
    host_seq=channel[4];beat=channel[6];
    idle(fn,3);u32 generation=channel[17];
    command_id=1;host(MNM_MENU_OPEN_QUICK,generation,1);channel_tick(fn);
    require(channel[20]==1&&channel[21]==MNM_MENU_OK&&helper_count==1&&get(object+0x33)==0x6e0020,46);
    channel_tick(fn);require(helper_count==1,47); // Same request cannot run twice.
    idle(fn,3);generation=channel[17];
    ++command_id;host(MNM_MENU_OPEN_QUICK,generation-1,1);channel_tick(fn);
    require(channel[21]==MNM_MENU_STALE&&helper_count==0,48);
    ++command_id;host(99,generation,1);channel_tick(fn);require(channel[21]==MNM_MENU_UNSUPPORTED&&helper_count==0,49);
    ++command_id;host(MNM_MENU_BACK,generation,1);channel_tick(fn);require(channel[21]==MNM_MENU_UNSUPPORTED&&helper_count==0,50);
    object[0xc]=1;channel_tick(fn);generation=channel[17];
    ++command_id;host(MNM_MENU_OPEN_QUICK,generation,1);channel_tick(fn);require(channel[21]==MNM_MENU_UNAVAILABLE&&helper_count==0,51);
    idle(fn,22);generation=channel[17];
    ++command_id;host(MNM_MENU_BACK,generation,1);channel_tick(fn);
    require(channel[20]==command_id&&channel[21]==MNM_MENU_OK&&helper_count==1&&get(object+0x43)==1,52);
    idle(fn,3);generation=channel[17];*(u8*)0x6e2030=0;
    ++command_id;host(MNM_MENU_QUIT,generation,1);channel_tick(fn);
    require(channel[21]==MNM_MENU_OK&&helper_count==1&&get(object+0x43)==1,57);
    idle(fn,22);generation=channel[17];
    ++command_id;host(MNM_MENU_QUIT,generation,1);channel_tick(fn);
    require(channel[21]==MNM_MENU_UNSUPPORTED&&helper_count==0,58);
    idle(fn,3);put((void*)0x6f34e0,0);channel_tick(fn);generation=channel[17];
    ++command_id;host(MNM_MENU_OPEN_QUICK,generation,1);channel_tick(fn);require(channel[21]==MNM_MENU_UNAVAILABLE&&helper_count==0,53);
    idle(fn,3);generation=channel[17];
    // An unstable host lane cannot authorize a callback.
    channel[4]=host_seq+1;channel[7]=command_id+1;channel[8]=MNM_MENU_OPEN_QUICK;channel[9]=generation;
    channel_tick(fn);require(helper_count==0&&channel[20]==command_id,54);
    host(0,generation,1);channel_tick(fn);
    Sleep(MNM_MENU_V1_LEASE_MS+100);channel_tick(fn);require(channel[21]==MNM_MENU_RETIRED&&channel[19]==0,55);
    ++command_id;host(MNM_MENU_OPEN_QUICK,generation,1);channel_tick(fn);
    require(channel[21]==MNM_MENU_RETIRED&&helper_count==0,56); // Heartbeat never revives a retired adapter.
}
