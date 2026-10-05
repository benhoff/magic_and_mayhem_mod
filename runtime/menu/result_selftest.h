static u8 result_controls[3][0x50],result_labels[26][12];
static u32 result_buttons[3],result_texts[26],result_callback[3];
static int result_fixture_enabled(void){
    char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return 0;
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)return 0;
    int ok=GetFileSize(f,0)==MNM_MENU_V5_SIZE;CloseHandle(f);return ok;
}
static void result_fixture_init(void){
    copy((void*)0x475860,result_button,sizeof(result_button));put((void*)0x5c5f04,0x5595d0);
    const u8 tick[]={0xb8,0xdf,0x9b,0x57,0x13,0xc3},noop[]={0xc3};
    copy((void*)0x4757c0,tick,sizeof(tick));copy((void*)0x4a39a0,noop,1);
}
static void result_fixture_reset(void){
    u8* p=(u8*)0x6deec8;for(u32 i=0;i<0x85;++i)p[i]=0;
    put(p,0x5c5ef4);put(p+4,26);put(p+8,1);put(p+0x57,(u32)result_buttons);put(p+0x4f,(u32)result_texts);
    result_callback[0]=0x5c5f58;result_callback[1]=0x475860;result_callback[2]=(u32)p;
    result_buttons[0]=0;
    for(u32 i=1;i<3;++i){result_buttons[i]=(u32)result_controls[i];put(result_controls[i]+0x25,(u32)result_callback);put(result_controls[i]+0x2d,i);}
    for(u32 i=0;i<26;++i){result_texts[i]=(u32)result_labels[i];put(result_labels[i]+8,(u32)(i==6?"Fixture player":i==10?"12":""));}
    put((void*)0x689920,1);put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);
    put((void*)0x6f349c,1);put((void*)0x6f34e0,(u32)p);put((void*)0x6f34a0,0x6cbb78);put((void*)0x6f34a4,(u32)p);
    *(u8*)0x6dbc1a=0;
}
static void result_fixture_tick(TickFn fn){SetLastError(0xabc123);require(fn((void*)0x6deec8)==0x13579bdf&&GetLastError()==0xabc123,130);}
static void result_fixture_tests(void){
    char path[1024];GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0),m=CreateFileMappingA(f,0,4,0,0,0);CloseHandle(f);
    channel=MapViewOfFile(m,2,0,0,MNM_MENU_V5_SIZE);CloseHandle(m);require(channel!=0,131);
    host_seq=channel[4];beat=channel[6];TickFn fn=(TickFn)get((void*)0x5c5f04);require((u32)fn!=0x5595d0,132);
    for(u32 pass=0;pass<2;++pass)for(u32 action=1;action<3;++action){
        result_fixture_reset();SetLastError(0xabc123);abi_failure=0;
        if(!pass)require(invoke_action(0x475860,(void*)0x6deec8,action)==0&&!abi_failure,133);
        else {
            result_fixture_tick(fn);require(channel[34]==26&&channel[35]==1&&channel[MNM_MENU_V5_RESULTS/4]==3&&channel[(MNM_MENU_V5_RESULTS+16)/4]==1,134);
            ++command_id;host(action==1?MNM_MENU_RESULT_CONTINUE:MNM_MENU_RESULT_QUIT,channel[33],1);result_fixture_tick(fn);
            require(channel[36]==command_id&&channel[37]==MNM_MENU_OK,135);result_fixture_tick(fn);
        }
        require(get((void*)0x6def0b)==1&&*(u8*)0x6dbc1a==(action==2)&&GetLastError()==0xabc123,136);
    }
    result_fixture_reset();result_fixture_tick(fn);u32 gen=channel[33];
    ++command_id;host(MNM_MENU_RESULT_QUIT,gen-1,1);result_fixture_tick(fn);require(channel[37]==MNM_MENU_STALE&&!*(u8*)0x6dbc1a,137);
    for(u32 kind=0;kind<5;++kind){
        result_fixture_reset();
        if(kind==0)put((void*)0x689920,2);
        if(kind==1)put((void*)0x6cbb7c,3);
        if(kind==2)result_callback[2]=0;
        if(kind==3)result_buttons[0]=(u32)result_controls[0];
        if(kind==4)put((void*)0x6def03,1); // confirmation at object +0x3b
        result_fixture_tick(fn);require(!channel[35],138);
        ++command_id;host(MNM_MENU_RESULT_QUIT,channel[33],1);result_fixture_tick(fn);require(channel[37]==MNM_MENU_UNAVAILABLE&&!*(u8*)0x6dbc1a,139);
    }
    result_fixture_reset();result_fixture_tick(fn);host(0,channel[33],0);result_fixture_tick(fn);require(channel[37]==MNM_MENU_RETIRED,140);
}
