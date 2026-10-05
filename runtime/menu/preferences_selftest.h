#include "../../protocols/include/mnm/menu_v6.h"
static u8 pref_sliders[2*0x86],pref_radios[12*0x7e],pref_buttons[2*0x59];
static u32 pref_groups[5][4],pref_lists[5][3],pref_slider_cb[3],pref_button_cb[3],pref_writes,pref_audio,pref_samples;
static int preferences_fixture_enabled(void){
    char path[1024];if(!GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path)))return 0;
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)return 0;
    int ok=GetFileSize(f,0)==MNM_MENU_V6_SIZE;CloseHandle(f);return ok;
}
static void THIS pref_write(void* self){require(self==(void*)0x6de6c8,141);++pref_writes;}
static void THIS pref_sound(void* self,u32 value){require(self==(void*)0x6b0198,142);put((void*)0x6de70d,value);++pref_audio;}
static void THIS pref_sample(void* self,u32 id,u32 value,u32 a,u32 b,u32 c,u32 d,u32 e){
    require(self==(void*)0x6b0198&&id==509&&value==get((void*)0x6de70d)&&!a&&!b&&!c&&d==0xffffffff&&e==0xffffffff,143);++pref_samples;
}
static void pref_jump(u32 at,void* target){u8 jump[5]={0xe9,0,0,0,0};put(jump+1,(u32)target-at-5);copy((void*)at,jump,5);}
static void preferences_fixture_init(void){
    copy((void*)0x4a9840,pref_button_bytes,sizeof(pref_button_bytes));copy((void*)0x4a9700,pref_slider_bytes,sizeof(pref_slider_bytes));
    copy((void*)0x4ce730,pref_group_bytes,sizeof(pref_group_bytes));put((void*)0x5c649c,0x5595d0);put((void*)0x5c64bc,(u32)&fixture_callback);
    pref_jump(0x54c890,&pref_write);pref_jump(0x56fd30,&pref_sound);pref_jump(0x56f000,&pref_sample);
    const u8 noop[]={0xc2,4,0},sample_music[]={0xc2,4,0};copy((void*)0x4f7970,noop,3);copy((void*)0x482190,sample_music,3);
}
static void preferences_fixture_reset(void){
    reset(3,0x5c63e4);u8* p=(u8*)0x6a4948;for(u32 i=0;i<0x70;++i)p[i]=0;
    put(p,0x5c648c);put(p+4,10);put(p+8,1);put(p+0x53,(u32)pref_sliders);put(p+0x57,(u32)pref_radios);
    put(p+0x5b,(u32)pref_buttons);put(p+0x63,(u32)pref_groups);put(p+0x68,-250);put(p+0x6c,0);
    pref_slider_cb[0]=pref_button_cb[0]=0x5c64bc;pref_slider_cb[1]=0x4a9700;pref_button_cb[1]=0x4a9840;
    pref_slider_cb[2]=pref_button_cb[2]=(u32)p;put(p+0x4b,(u32)pref_slider_cb);put(p+0x5f,(u32)pref_button_cb);
    for(u32 i=0;i<sizeof(pref_sliders);++i)pref_sliders[i]=0;
    for(u32 i=0;i<sizeof(pref_radios);++i)pref_radios[i]=0;
    for(u32 i=0;i<sizeof(pref_buttons);++i)pref_buttons[i]=0;
    put(display,0x5c0000);put(display+0x19,100);
    for(u32 i=0;i<2;++i){u8* s=pref_sliders+i*0x86,*b=pref_buttons+i*0x59;
        put(s,0x5c6d10);put(s+8,1);put(s+0x2d,i);put(s+0x25,(u32)pref_slider_cb);put(s+0x41,(u32)display);
        put(s+0x61,i?4750:0);put(s+0x65,i?2500:0);put(s+0x69,i?5000:15);put(s+0x71,100);
        put(b,0x5c6c6c);put(b+8,1);put(b+0x2d,i);put(b+0x25,(u32)pref_button_cb);
    }
    const u32 first[]={0,2,4,7,10},count[]={2,2,3,3,2};
    for(u32 i=0;i<12;++i){u8* r=pref_radios+i*0x7e;put(r,0x5c6d40);put(r+8,1);put(r+0x2d,i);}
    for(u32 i=0;i<5;++i){pref_groups[i][0]=(u32)pref_lists[i];pref_groups[i][1]=i==2?1:0;pref_groups[i][2]=pref_groups[i][3]=count[i];
        for(u32 j=0;j<count[i];++j)pref_lists[i][j]=(u32)(pref_radios+(first[i]+j)*0x7e);}
    put((void*)0x6f349c,1);put((void*)0x6f34a0,(u32)object);put((void*)0x6f34a4,(u32)p);put((void*)0x6f34e0,(u32)p);
    put((void*)0x657c74,0);put((void*)0x6b01b4,1);put((void*)0x6de709,0);put((void*)0x6de70d,-250);put((void*)0x6de6d5,1);put((void*)0x6de6fd,17);
    pref_writes=pref_audio=pref_samples=0;
}
static void preferences_fixture_tick(TickFn fn){SetLastError(0xabc123);require(fn((void*)0x6a4948)==0x13579bdf&&GetLastError()==0xabc123,144);}
static void preferences_fixture_host(u32 action,u32 generation,u32 argument){
    channel[4]=++host_seq;channel[5]=1;channel[6]=++beat;channel[7]=++command_id;channel[8]=action;channel[9]=generation;channel[10]=argument;
    const u32 values[]={0,(u32)-500,0,0,2,0,1};for(u32 i=0;i<7;++i)channel[MNM_MENU_V6_HOST_PREFERENCES/4+i]=values[i];
    __atomic_thread_fence(__ATOMIC_RELEASE);channel[4]=++host_seq;
}
static void preferences_fixture_tests(TickFn main_tick){
    char path[1024];GetEnvironmentVariableA("MNM_MENU_CHANNEL",path,sizeof(path));
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0),m=CreateFileMappingA(f,0,4,0,0,0);CloseHandle(f);
    channel=MapViewOfFile(m,2,0,0,MNM_MENU_V6_SIZE);CloseHandle(m);require(channel!=0,145);host_seq=channel[4];beat=channel[6];
    reset(3,0x5c63e4);channel_tick(main_tick);preferences_fixture_host(MNM_MENU_OPEN_PREFERENCES,channel[33],0);channel_tick(main_tick);
    require(channel[37]==MNM_MENU_OK&&get(object+0x33)==0x6a4948,146);
    TickFn fn=(TickFn)get((void*)0x5c649c);require((u32)fn!=0x5595d0,147);
    preferences_fixture_reset();preferences_fixture_tick(fn);require(channel[34]==10&&channel[35]==1&&channel[MNM_MENU_V6_PREFERENCES/4+1]==16383,148);
    preferences_fixture_host(MNM_MENU_PREFERENCES_PREVIEW,channel[33],1);preferences_fixture_tick(fn);
    require(channel[37]==MNM_MENU_OK,181);require(get(pref_sliders+0xe7)==4500,182);require(get((void*)0x6de70d)==(u32)-500,183);require(pref_audio==1,184);require(pref_samples==1,185);require(pref_writes==0&&pref_groups[2][1]==1,186);
    preferences_fixture_tick(fn);require(pref_audio==1&&pref_samples==1,150);
    preferences_fixture_host(MNM_MENU_PREFERENCES_CANCEL,channel[33],0);preferences_fixture_tick(fn);
    require(channel[37]==MNM_MENU_OK&&get((void*)0x6de70d)==(u32)-250&&pref_writes==0&&get((void*)0x6a498b)==1&&helper_count==1,151);
    preferences_fixture_reset();preferences_fixture_tick(fn);preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);preferences_fixture_tick(fn);
    require(channel[37]==MNM_MENU_OK&&pref_groups[2][1]==2&&get((void*)0x6de6f5)==2&&pref_writes==1&&helper_count==1,152);
    preferences_fixture_tick(fn);require(pref_writes==1,153);
    preferences_fixture_reset();preferences_fixture_tick(fn);preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33]-1,0);preferences_fixture_tick(fn);require(channel[37]==MNM_MENU_STALE&&!pref_writes&&!pref_audio,154);
    preferences_fixture_reset();preferences_fixture_tick(fn);preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);
    channel[MNM_MENU_V6_HOST_PREFERENCES/4+6]=2;preferences_fixture_tick(fn);require(channel[37]==MNM_MENU_INVALID&&!pref_audio&&!pref_writes&&pref_groups[2][1]==1,155);
    for(u32 kind=0;kind<12;++kind){
        preferences_fixture_reset();
        if(kind==0)pref_slider_cb[2]=0;
        if(kind==1)pref_button_cb[2]=0;
        if(kind==2)put(object+4,22);
        if(kind==3)pref_groups[4][2]=0;
        if(kind==4)pref_groups[4][0]=1;
        if(kind==5)put((void*)0x6a4983,1); // modal +0x3b
        if(kind==6)*(u8*)0x6a4954=1; // fade +0xc
        if(kind==7)put((void*)0x6a497b,1); // pending +0x33
        if(kind==8)put((void*)0x6a498b,1); // return +0x43
        if(kind==9)put(pref_sliders+0x65,1);
        if(kind==10)pref_lists[4][1]=0;
        if(kind==11)put((void*)0x6f34e0,0);
        preferences_fixture_tick(fn);require(!channel[35],156);
        preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);preferences_fixture_tick(fn);
        require(channel[37]==MNM_MENU_UNAVAILABLE&&!pref_audio&&!pref_writes&&helper_count==0,157);
    }
    for(u32 kind=0;kind<2;++kind){preferences_fixture_reset();if(!kind)put(pref_sliders+0x86+0x3d,2);else put(pref_radios+6*0x7e+0x3d,2);
        preferences_fixture_tick(fn);preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);preferences_fixture_tick(fn);
        require(channel[37]==MNM_MENU_UNAVAILABLE&&!pref_writes&&!pref_audio&&pref_groups[2][1]==1,158);}
    preferences_fixture_reset();preferences_fixture_tick(fn);u32 ack=channel[36];
    channel[4]=host_seq+1;channel[7]=command_id+1;channel[8]=MNM_MENU_PREFERENCES_OK;channel[9]=channel[33];preferences_fixture_tick(fn);
    require(channel[36]==ack&&!pref_writes&&!pref_audio,159);host(0,channel[33],1);preferences_fixture_tick(fn);
    host(0,channel[33],0);preferences_fixture_tick(fn);require(channel[37]==MNM_MENU_RETIRED&&!channel[35],160);
    preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);preferences_fixture_tick(fn);require(channel[37]==MNM_MENU_RETIRED&&!pref_writes,161);
}
