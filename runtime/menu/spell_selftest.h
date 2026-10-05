static u8 spell_cells[3][7][0x86],spell_shelves[25][0x82];
static u32 spell_callbacks[2][4],spell_table[8];
static char* THIS fixture_spell_text(void* object,u32 index){(void)object;(void)index;return "Fixture name";}
static void spell_fixture_init(void){
    copy((void*)0x576380,spell_slot_button,sizeof(spell_slot_button));copy((void*)0x5765d0,spell_shelf_button,sizeof(spell_shelf_button));
    copy((void*)0x579710,spell_slot_set,sizeof(spell_slot_set));copy((void*)0x576810,spell_ok_button,sizeof(spell_ok_button));
    copy((void*)0x576830,spell_tick_bytes,6);put((void*)0x5c776c,0x576830);
    u8* p=(u8*)0x6f2aa0;for(u32 i=0;i<0x26a;++i)p[i]=0;
    put(p,0x5c775c);put(p+4,7);put(p+8,1);put(p+0x250,1);put(p+0x90,0x65b250);put(p+0x94,0xffffffff);
    put((void*)0x6f5284,30);put((void*)0x6f5288,0);
    const u8 no3[]={0xc2,12,0},no1[]={0xc2,4,0},no7[]={0xc2,28,0},no0[]={0xc3};
    copy((void*)0x4cb7d0,no3,3);copy((void*)0x4cb7f0,no3,3);copy((void*)0x579500,no1,3);copy((void*)0x5799c0,no1,3);copy((void*)0x56f000,no7,3);
    copy((void*)0x4a0010,no0,1);spell_table[4]=0x4a0010;
    // Read-only name catalog service stub; control callback bytecode and original
    // talisman setter are executed at pinned VAs, while drawing/tooltips/audio are stubbed.
    u8 jump_text[]={0xe9,0,0,0,0};put(jump_text+1,(u32)&fixture_spell_text-0x58a685);copy((void*)0x58a680,jump_text,5);
    for(u32 k=0;k<2;++k){spell_callbacks[k][0]=0x5c77b4;spell_callbacks[k][1]=k?0x5765d0:0x576380;spell_callbacks[k][2]=0;spell_callbacks[k][3]=(u32)p;}
    for(u32 a=0;a<3;++a){put(p+0x84+4*a,2);put(p+0x218+4*a,(u32)spell_cells[a]);
        for(u32 i=0;i<7;++i){u8* c=spell_cells[a][i];put(c,(u32)spell_table);put(c+0x25,(u32)spell_callbacks[0]);put(c+0x2d,0x7fe+a*7+i);put(c+0x49,1);put(c+0x7a,a);put(c+0x7e,0xffffffff);}
    }
    put(p+0x214,(u32)spell_shelves);put(p+0x98,0x6f4d48);
    for(u32 i=0;i<25;++i){u8* c=spell_shelves[i];put(c,(u32)spell_table);put(c+0x25,(u32)spell_callbacks[1]);put(c+0x2d,0x7d3+i);put(c+0x7a,i<3?i:0xffffffff);}
    put(spell_shelves[2]+0x7a,0xffffffff);put(spell_cells[2][1]+0x7e,2);
    for(u32 i=0;i<63;++i)put((void*)(0x6f4c08+4*i),i);
}
static void spell_fixture_request(u32 generation){
    channel[4]=++host_seq;channel[5]=1;channel[6]=++beat;channel[7]=++command_id;channel[8]=MNM_MENU_SPELL_FINISH;channel[9]=generation;
    for(u32 i=0;i<63;++i)channel[MNM_MENU_V3_HOST_SLOTS/4+i]=0xffffffff;
    __atomic_thread_fence(__ATOMIC_RELEASE);channel[4]=++host_seq;
}
static void spell_fixture_tick(TickFn fn){put((void*)0x6f34e0,0x6f2aa0);SetLastError(0xabc123);require(fn((void*)0x6f2aa0)==0x13579bdf&&GetLastError()==0xabc123,91);}
static void spell_fixture_tests(TickFn fn){
    char path[1024];GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    HANDLE m=CreateFileMappingA(f,0,4,0,0,0);CloseHandle(f);channel=MapViewOfFile(m,2,0,0,MNM_MENU_V3_SIZE);CloseHandle(m);
    require(channel!=0,92);host_seq=channel[4];beat=channel[6];spell_fixture_tick(fn);
    require(channel[34]==7&&channel[35]==1&&channel[(MNM_MENU_V3_SPELL+8)/4]==7,93);
    u32 generation=channel[33];spell_fixture_request(generation);channel[MNM_MENU_V3_HOST_SLOTS/4]=0;channel[MNM_MENU_V3_HOST_SLOTS/4+21]=0;
    spell_fixture_tick(fn);require(channel[37]==MNM_MENU_INVALID&&get(spell_shelves[0]+0x7a)==0&&!get((void*)0x6f2ade),94);
    spell_fixture_request(generation);channel[MNM_MENU_V3_HOST_SLOTS/4+2]=0;
    spell_fixture_tick(fn);require(channel[37]==MNM_MENU_INVALID&&get(spell_shelves[0]+0x7a)==0,95);
    spell_fixture_request(generation-1);spell_fixture_tick(fn);require(channel[37]==MNM_MENU_STALE,96);
    put((void*)0x6f5288,1);spell_fixture_request(generation);spell_fixture_tick(fn);require(channel[37]==MNM_MENU_UNAVAILABLE&&get(spell_cells[2][1]+0x7e)==2,100);put((void*)0x6f5288,0);
    spell_fixture_request(generation);channel[MNM_MENU_V3_HOST_SLOTS/4]=0;channel[MNM_MENU_V3_HOST_SLOTS/4+21]=1;
    spell_fixture_tick(fn);require(channel[37]==MNM_MENU_OK&&get(spell_cells[0][0]+0x7e)==0&&get(spell_cells[1][0]+0x7e)==1&&get(spell_shelves[0]+0x7a)==0xffffffff&&get(spell_shelves[1]+0x7a)==0xffffffff&&channel[39]==2,97);
    require(get(spell_shelves[2]+0x7a)==2&&get(spell_cells[2][1]+0x7e)==0xffffffff,101);
    require(get((void*)0x6f5288)==0&&get((void*)0x6f2b34)==0xffffffff&&get((void*)0x6f2ae2)==0&&get((void*)0x6f2ade)==1,98);
    u32 actions=helper_count;spell_fixture_tick(fn);require(helper_count==actions,99);
}
static int spell_fixture_enabled(void){char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return 0;HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)return 0;int ok=GetFileSize(f,0)==MNM_MENU_V3_SIZE;CloseHandle(f);return ok;}
