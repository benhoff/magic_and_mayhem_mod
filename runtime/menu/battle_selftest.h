#include "../../protocols/include/mnm/menu_v2.h"
API u32 WIN GetFileSize(HANDLE,u32*);
static int battle_fixture_enabled(void){
    char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return 0;
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)return 0;
    int enabled=GetFileSize(f,0)==MNM_MENU_V2_SIZE;CloseHandle(f);return enabled;
}
static u8 sliders[17][0x80],display[0x30],list[0x80],list_rows[3*0x101],label[16];
static u32 slider_table[17],labels[40],callbacks[17][3];
static void THIS fixture_callback(void* p,u32 index){
    u8* cb=p;((void (THIS *)(void*,u32))get(cb+4))((void*)get(cb+8),index);
}
static void battle_fixture_init(void){
    copy((void*)0x4ad6a0,setup_button,sizeof(setup_button));copy((void*)0x4bbbf0,map_button,sizeof(map_button));
    copy((void*)0x4cdf40,slider_set,sizeof(slider_set));copy((void*)0x4ad860,rule_change,sizeof(rule_change));
    copy((void*)0x4d1060,list_select,sizeof(list_select));copy((void*)0x4d1df0,list_get,sizeof(list_get));copy((void*)0x4d1ed0,list_index,sizeof(list_index));
    put((void*)0x5c6544,0x5595d0);put((void*)0x5c6950,0x5595d0);
    put((void*)0x5c6594,(u32)&fixture_callback);
    // Stub display/text services and Start loading only. Rules/list callbacks and
    // selected button bytecode remain original, executed at their pinned VAs.
    const u8 noop[]={0xc2,4,0};copy((void*)0x4d29a0,noop,3);copy((void*)0x4d28c0,noop,3);
    const u8 start_stub[]={0xc7,0x41,0x33,0xb8,0x03,0x69,0x00,0xc3};copy((void*)0x4ae670,start_stub,8);
    const u8 rect_stub[]={0xc2,4,0};copy((void*)0x4a0000,rect_stub,3);put((void*)0x5c0000,0);put((void*)0x5c0030,0x4a0000);
    const u8 ftol[]={0x83,0xec,8,0xdf,0x3c,0x24,0x8b,4,0x24,0x8b,0x54,0x24,4,0x83,0xc4,8,0xc3};copy((void*)0x59bee0,ftol,sizeof(ftol));
}
static void battle_fixture_reset(u32 id){
    u8* setup=(u8*)0x658970;u8* map=(u8*)0x690448;
    for(u32 i=0;i<0x600;++i)setup[i]=0;
    for(u32 i=0;i<0x160;++i)map[i]=0;
    put(setup,0x5c6534);put(setup+4,14);put(setup+8,1);put(setup+0x53,(u32)labels);put(setup+0x5f,(u32)slider_table);
    put(map,0x5c6940);put(map+4,25);put(map+8,1);put(map+0x57,(u32)list);put(map+0x15b,14);
    put(display,0x5c0000);put(display+0x19,100);
    put(label+8,(u32)"First map");labels[13]=(u32)label;
    put((void*)0x6c3758,1);copy((void*)0x6ead98,"No Player",10);
    for(u32 i=0;i<4;++i){u8* p=(u8*)(0x658f10+i*0x133);for(u32 j=0;j<0x133;++j)p[j]=0;
        copy(p,i<2?"Player":"No Player",i<2?7:10);put(p+0x15,i<2?i:0xffffffff);put(p+0x1d,i<2?i:0xffffffff);
    }
    for(u32 i=0;i<17;++i){
        slider_table[i]=(u32)sliders[i];put(sliders[i]+0x41,(u32)display);put(sliders[i]+0x65,0);put(sliders[i]+0x69,200);put(sliders[i]+0x6d,0);put(sliders[i]+0x71,100);
        u32 value=i<13?50:0;put(sliders[i]+0x61,value);
        if(i<13)put((void*)(0x6c375c+i*4),value);
        callbacks[i][0]=0x5c6594;callbacks[i][1]=0x4ad860;callbacks[i][2]=(u32)setup;
        put(sliders[i]+0x25,(u32)callbacks[i]);put(sliders[i]+0x2d,i);
    }
    for(u32 i=0;i<sizeof(list);++i)list[i]=0;
    for(u32 i=0;i<sizeof(list_rows);++i)list_rows[i]=0;
    copy(list_rows,"First map",10);copy(list_rows+0x101,"Second map",11);copy(list_rows+0x202,"Third map",10);
    put(list+0x45,(u32)list_rows);put(list+0x49,3);put(list+0x5b,0xffffffff);
    put((void*)0x6f34e0,id==14?(u32)setup:(u32)map);helper_count=0;
}
static void battle_fixture_tick(TickFn fn,u32 id){
    SetLastError(0xabc123);require(fn((void*)(id==14?0x658970:0x690448))==0x13579bdf&&GetLastError()==0xabc123,70);
}
static void battle_fixture_host(u32 action,u32 generation,u32 argument){
    channel[4]=++host_seq;channel[5]=1;channel[6]=++beat;channel[7]=++command_id;channel[8]=action;channel[9]=generation;channel[10]=argument;
    for(u32 i=0;i<17;++i)channel[11+i]=i<13?50:0;
    __atomic_thread_fence(__ATOMIC_RELEASE);channel[4]=++host_seq;
}
static void battle_fixture_tests(TickFn fn){
    char path[1024];GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);HANDLE m=CreateFileMappingA(f,0,4,0,0,0);CloseHandle(f);
    channel=MapViewOfFile(m,2,0,0,MNM_MENU_V2_SIZE);CloseHandle(m);require(channel!=0,71);host_seq=channel[4];beat=channel[6];
    reset(3,0x5c63e4);channel_tick(fn);u32 main_generation=channel[33];
    battle_fixture_host(MNM_MENU_QUIT,main_generation,0);channel_tick(fn);
    require(channel[37]==MNM_MENU_OK&&get(object+0x43)==1,83);
    reset(22,0x5c641c);channel_tick(fn);main_generation=channel[33];
    battle_fixture_host(MNM_MENU_OPEN_SINGLE,main_generation,0);channel_tick(fn);
    require(channel[37]==MNM_MENU_OK&&get(object+0x33)==0x658970,84);
    battle_fixture_reset(14);battle_fixture_tick(fn,14);require(channel[34]==14&&channel[35]==1&&channel[41]==3,72);
    u32 generation=channel[33];battle_fixture_host(MNM_MENU_SETUP_APPLY,generation,0);
    channel[11]=75;channel[27]=999; // Invalid last value must reject the entire transaction.
    battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_INVALID&&get((void*)0x6c375c)==50,73);
    battle_fixture_host(MNM_MENU_SETUP_APPLY,generation,0);channel[11]=75;channel[19]=3;channel[24]=10;
    battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_OK&&get((void*)0x6c375c)==75&&get((void*)0x6c377c)==3&&get((void*)0x658f3e)==3&&get((void*)0x658f29)==10,74);
    generation=channel[33];battle_fixture_host(MNM_MENU_SETUP_PLAYER,generation,99);battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_INVALID,75);
    battle_fixture_host(MNM_MENU_SETUP_MAP,generation,0);battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_OK&&get((void*)0x6589a3)==0x690448&&*(u8*)0x6905a3==14,76);
    battle_fixture_reset(25);battle_fixture_tick(fn,25);generation=channel[33];
    battle_fixture_host(MNM_MENU_MAP_OK,generation,4);battle_fixture_tick(fn,25);require(channel[37]==MNM_MENU_INVALID&&helper_count==0,77);
    battle_fixture_host(MNM_MENU_MAP_OK,generation,2);battle_fixture_tick(fn,25);require(channel[37]==MNM_MENU_OK&&get((void*)0x6c3758)==2&&get((void*)0x69048b)==1,78);
    battle_fixture_reset(14);battle_fixture_tick(fn,14);generation=channel[33];
    battle_fixture_host(MNM_MENU_SETUP_CANCEL,generation,0);battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_OK&&get((void*)0x6589b3)==1,79);
    battle_fixture_reset(14);battle_fixture_tick(fn,14);generation=channel[33];
    slider_table[16]=0; // Removing an opponent frees its handicap control.
    battle_fixture_host(MNM_MENU_SETUP_APPLY,generation,0);battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_OK,81);
    generation=channel[33];battle_fixture_host(MNM_MENU_SETUP_APPLY,generation,0);channel[27]=1;
    battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_INVALID,82);
    battle_fixture_host(MNM_MENU_SETUP_START,generation,0);battle_fixture_tick(fn,14);require(channel[37]==MNM_MENU_OK&&channel[39]==2&&get((void*)0x6589a3)==0x6903b8,80);
}
