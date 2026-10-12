#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "storage_bridge_bytes.h"
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static u32 saves,deletes,removals,fades;
static void THIS fade(void* p){++fades;((u8*)p)[0xc]=1;}
static void THIS disable(void* p){(void)p;}
static u32 THIS save_request(void* p,u32 index){require(!index,10);++saves;((u8*)p)[0x63]=((u8*)p)[0x64]=1;return 0;}
static int WIN delete_file(const char* path){require(equal(path,"Save\\second.sav",16),11);++deletes;return 1;}
static void THIS remove_row(void* p,const char* stem){(void)p;require(equal(stem,"second",7),12);++removals;}
static void stub(u32 address,void* target){((u8*)address)[0]=0xe9;put((void*)(address+1),(u32)target-address-5);}
static void host(u32 id,u32 action,u32 argument,const char* name){u32* h=menu_words+4;u32 seq=menu_load(h);menu_store(h,seq+1);menu_store(h+3,id);menu_store(h+4,action);menu_store(h+5,menu_generation);menu_store(h+6,argument);zero((u8*)menu_words+MNM_MENU_V13_HOST_NAME,128);if(name){u32 n=0;while(name[n])++n;copy((u8*)menu_words+MNM_MENU_V13_HOST_NAME,name,n);}menu_store(h,seq+2);}
static u8* setup(int save,int campaign){
    u8* p=(u8*)(save?0x6de7c8:0x6de4e8);zero(p,0x80);put(p,save?0x5c64fc:0x5c64c4);put(p+4,save?13:MNM_MENU_LOAD_SCREEN);put(p+8,1);put(p+0x4f,0x6e3700);put(p+(save?0x57:0x5b),0x6e3800);put(p+0x53,0x6e3c00);if(!save)put(p+0x60,campaign?17:3);
    zero((void*)0x6e3700,0x700);put((void*)0x6e3700,0x5c6ca0);put((void*)0x6e3745,0x6e3a00);put((void*)0x6e3760,256);put((void*)0x6e3800,0x5c6db8);put((void*)0x6e3845,0x6e3900);put((void*)0x6e3849,2);put((void*)0x6e385b,0xffffffff);*(u8*)0x6e385a=1;
    copy((void*)0x6e3900,"first",6);copy((void*)0x6e3a01,"second",7);
    // Edit storage is separate from original list row storage.
    put((void*)0x6e3745,0x6e3b10);copy((void*)0x6e3b10,"first",6);
    for(u32 i=0;i<2;++i){u32 cb=0x6e3c20+i*12;put((void*)(0x6e3c00+i*4),cb);put((void*)cb,save?0x5c652c:0x5c64f4);put((void*)(cb+4),save?0x4ab750:0x4aa800);put((void*)(cb+8),(u32)p);}
    put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);
    u32 depth=campaign?5:3;put((void*)0x6f349c,depth);put((void*)0x6f34a8,0x657ce0);put((void*)0x6f34ac,campaign?0x659408:(u32)p);if(campaign){put((void*)0x6f34b0,0x6cbb78);put((void*)0x6f34b4,(u32)p);}put((void*)0x6f34e0,(u32)p);put((void*)0x68991c,0);put((void*)0x689920,5);return p;
}
void start(void){
    fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
    copy((void*)0x4aa800,load_button,sizeof(load_button));copy((void*)0x4ab750,save_button,sizeof(save_button));copy((void*)0x4cd0e0,edit_set,sizeof(edit_set));copy((void*)0x4d1060,list_select,sizeof(list_select));copy((void*)0x4d1df0,list_get,sizeof(list_get));
    copy((void*)0x5e0778,"Save\\",6);copy((void*)0x5e0780,".sav",5);stub(0x557510,(void*)&fade);stub(0x4ca950,(void*)&disable);stub(0x4ab870,(void*)&save_request);stub(0x4d1d10,(void*)&remove_row);put((void*)0x5c508c,(u32)&delete_file);
    menu_init();require(menu_words&&menu_version==13&&!menu_retired,2);static u8 snapshot[MNM_MENU_V13_STORAGE_SIZE];u8* p=setup(0,0);
    require(storage_snapshot(p,snapshot)&&get(snapshot)==MNM_MENU_LOAD_SCREEN&&get(snapshot+4)==3&&get(snapshot+24)==2,3);
    const u32 fields[]={0,4,0x4f,0x53,0x5b,0x60};for(u32 i=0;i<6;++i){u32 before=get(p+fields[i]);put(p+fields[i],0xffffffff);require(!storage_snapshot(p,snapshot),20+i);put(p+fields[i],before);}
    const u32 globals[]={0x6f349c,0x6f34a8,0x68991c,0x689920,0x6e3700,0x6e3760,0x6e3800,0x6e3849,0x6e3c28};for(u32 i=0;i<9;++i){u32 before=get((void*)globals[i]);put((void*)globals[i],0xffffffff);require(!storage_snapshot(p,snapshot),30+i);put((void*)globals[i],before);}
    menu_poll(p,0);require(menu_ready&&menu_screen==MNM_MENU_LOAD_SCREEN,40);host(1,MNM_MENU_STORAGE_ACCEPT,3,0);menu_poll(p,1);require(menu_status==MNM_MENU_INVALID&&!p[0x64],41);
    host(2,MNM_MENU_STORAGE_ACCEPT,2,0);menu_store(menu_words+9,menu_generation-1);menu_poll(p,1);require(menu_status==MNM_MENU_STALE&&!p[0x64],42);
    host(3,MNM_MENU_STORAGE_ACCEPT,2,0);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&p[0x64]&&get((void*)0x6e385b)==1&&equal((void*)0x6e3b10,"second",7),43);
    p[0x64]=0;menu_poll(p,1);require(!p[0x64]&&menu_request==3,44);
    p=setup(1,1);menu_poll(p,0);require(menu_ready&&menu_screen==13,45);
    for(u32 i=0;i<3;++i){const char* bad=i==0?"../escape":i==1?"CON":"trailing.";host(4+i,MNM_MENU_STORAGE_ACCEPT,0,bad);menu_poll(p,1);require(menu_status==MNM_MENU_INVALID&&!saves,46);}
    host(7,MNM_MENU_STORAGE_ACCEPT,0,"new-save");menu_poll(p,1);require(menu_status==MNM_MENU_OK&&saves==1&&p[0x64]&&equal((void*)0x6e3b10,"new-save",9),47);p[0x64]=0;menu_poll(p,1);require(saves==1,48);
    p=setup(1,1);menu_poll(p,0);host(8,MNM_MENU_STORAGE_DELETE,2,0);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&deletes==1&&removals==1,49);
    put(p+0x3b,0x6e3d00);menu_poll(p,0);host(9,MNM_MENU_STORAGE_CANCEL,0,0);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&!fades,50);
    put(p+0x3b,0);menu_poll(p,0);menu_retired=1;host(10,MNM_MENU_STORAGE_CANCEL,0,0);menu_poll(p,1);require(menu_status==MNM_MENU_RETIRED&&!fades,51);ExitProcess(0);
}
