static u32 mini_pair[2],mini_callback[3];
static u8 mini_dialog[0x61];
static int mini_fixture_enabled(void){
    char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return 0;
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)return 0;
    int ok=GetFileSize(f,0)==MNM_MENU_V4_SIZE;CloseHandle(f);return ok;
}
static void mini_fixture_init(void){
    copy((void*)0x4b23f0,mini_button,sizeof(mini_button));put((void*)0x5c6654,0x5595d0);
    // Original branch/SEH/receiver bytecode; stub audio/allocation/dialog services.
    const u8 ret[]={0xc3},ret1[]={0xc2,4,0},ret3[]={0xc2,12,0},receiver[]={0x8b,0xc1,0xc3};
    copy((void*)0x4a39a0,ret,1);copy((void*)0x4cef40,receiver,3);
    copy((void*)0x4cfbd0,ret1,3);copy((void*)0x4cfbe0,ret1,3);copy((void*)0x4cf010,ret3,3);
    u8 allocate[]={0xb8,0,0,0,0,0xc3};put(allocate+1,(u32)mini_dialog);copy((void*)0x597850,allocate,6);
}
static void mini_fixture_reset(void){
    reset(17,0x5c6644);put(object+0x37,0);put(object+0x3b,0);put(object+0x53,2);put(object+0x57,3);
    mini_callback[0]=0x5c6674;mini_callback[1]=0x4b23f0;mini_callback[2]=(u32)object;
    mini_pair[0]=(u32)mini_callback;mini_pair[1]=0;put(object+0x4f,(u32)mini_pair);
    put((void*)0x68991c,1);put((void*)0x689920,3);
    put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);
    put((void*)0x6f34a0,0x6cbb78);put((void*)0x6f34a4,(u32)object);
}
static void mini_fixture_tests(TickFn fn){
    char path[1024];GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0),m=CreateFileMappingA(f,0,4,0,0,0);CloseHandle(f);
    channel=MapViewOfFile(m,2,0,0,MNM_MENU_V4_SIZE);CloseHandle(m);require(channel!=0,110);
    host_seq=channel[4];beat=channel[6];
    if(!MNM_MENU_MINI_EXPERIMENTAL){
        mini_fixture_reset();channel_tick(fn);
        require(get((void*)0x5c6654)==0x5595d0&&channel[37]==MNM_MENU_RETIRED&&channel[35]==0,122);return;
    }
    require(get((void*)0x5c6654)!=(u32)0x5595d0,111);
    // Compare original callbacks with runtime dispatch, including ABI preservation.
    for(u32 pass=0;pass<2;++pass)for(u32 action=0;action<3;++action){
        mini_fixture_reset();SetLastError(0xabc123);abi_failure=0;
        if(!pass)require(invoke_action(0x4b23f0,object,action)==0&&!abi_failure,112);
        else {
            channel_tick(fn);require(channel[34]==17&&channel[35]==1&&channel[MNM_MENU_V4_MINI/4+3]==2,113);
            ++command_id;host(action==0?MNM_MENU_MINI_PREFERENCES:action==1?MNM_MENU_MINI_QUIT:MNM_MENU_MINI_CANCEL,channel[33],1);
            channel_tick(fn);require(channel[36]==command_id&&channel[37]==MNM_MENU_OK,114);
            u32 pending=get(object+0x33),modal=get(object+0x3b),returning=get(object+0x43);
            channel_tick(fn);require(get(object+0x33)==pending&&get(object+0x3b)==modal&&get(object+0x43)==returning,115);
        }
        require(GetLastError()==0xabc123&&get(object+0x33)==(action==0?0x6a4948:0)&&get(object+0x43)==(action!=1)&&get(object+0x3b)==(action==1?(u32)mini_dialog:0),116);
        require(object[0x32]==0xa5&&object[0x47]==0xa5,117);
    }
    mini_fixture_reset();channel_tick(fn);u32 generation=channel[33];
    ++command_id;host(MNM_MENU_MINI_CANCEL,generation-1,1);channel_tick(fn);require(channel[37]==MNM_MENU_STALE&&!get(object+0x43),118);
    // Unsupported parent, network, special exit, callback owner and modal state.
    for(u32 kind=0;kind<5;++kind){
        mini_fixture_reset();
        if(kind==0)put((void*)0x6cbb7c,3);
        if(kind==1)put((void*)0x689920,5);
        if(kind==2)put(object+0x53,4);
        if(kind==3)mini_callback[2]=0;
        if(kind==4)put(object+0x3b,(u32)mini_dialog);
        channel_tick(fn);require(channel[35]==0,119);
        ++command_id;host(MNM_MENU_MINI_CANCEL,channel[33],1);channel_tick(fn);require(channel[37]==MNM_MENU_UNAVAILABLE&&!get(object+0x43),120);
    }
    mini_fixture_reset();channel_tick(fn);host(0,channel[33],0);channel_tick(fn);
    require(channel[37]==MNM_MENU_RETIRED&&channel[35]==0,121);
}
