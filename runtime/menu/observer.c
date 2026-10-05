/* Pinned PE32 forwarding observation, with an independently opt-in menu adapter. */
#include "../shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
#define THIS __attribute__((thiscall))
typedef u32 (THIS *ActionFn)(void*,u32);
typedef u32 (THIS *TickFn)(void*);
static ActionFn original_main,original_quick;
static TickFn original_tick;
static HANDLE log_file;
static volatile u32 log_busy;
static u32 sequence;
static u32 last_state[4][9];
static int have_state[4];
static u32 get(const void* p){const u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(const void* p,u32 n){
    struct {void* base;void* allocation;u32 protection,size,state,protect,type;} info;
    u32 at=(u32)p,end=at+n;if(end<at)return 0;
    while(at<end){
        if(!VirtualQuery((void*)at,&info,sizeof(info))||info.state!=0x1000||(info.protect&0x101))return 0;
        u32 next=(u32)info.base+info.size;if(next<=at)return 0;at=next;
    }return 1;
}
static void record(u32 event,void* object,u32 argument,u32 result){
    u32 error=GetLastError();
    if(!log_file||!__sync_bool_compare_and_swap(&log_busy,0,1)){SetLastError(error);return;}
    if(sequence<256 && readable(object,0x47)){
        u8 data[64];u8* p=object;
        u32 values[16]={++sequence,event,GetCurrentThreadId(),get(p+4),(u32)p,get(p),get(p+8),
                       get(p+0x33),get(p+0x43),argument,result,0,0,error,p[0xc],get(p+0xd)};
        if(readable((void*)0x6f349c,0x48)){values[11]=get((void*)0x6f34e0);values[12]=get((void*)0x6f349c);}
        // Idle ticks must not exhaust the bounded log before an action occurs.
        if(event==1||event==4){
            const u32 indices[9]={3,4,5,6,7,8,11,12,14};u32 key[9];
            for(u32 i=0;i<9;++i)key[i]=values[indices[i]];
            u32 slot=values[3]==3?0:values[3]==22?1:values[3]==14?2:3;
            if(have_state[slot]&&equal(key,last_state[slot],sizeof(key))){--sequence;__sync_lock_release(&log_busy);SetLastError(error);return;}
            copy(last_state[slot],key,sizeof(key));have_state[slot]=1;
        }
        for(u32 i=0;i<16;++i)put(data+4*i,values[i]);
        u32 wrote=0;if(!WriteFile(log_file,data,64,&wrote,0)||wrote!=64){CloseHandle(log_file);log_file=0;}
    }
    __sync_lock_release(&log_busy);SetLastError(error);
}
static u32 THIS main_action(void* object,u32 action){
    record(2,object,action,0);u32 result=original_main(object,action);record(3,object,action,result);return result;
}
static u32 THIS quick_action(void* object,u32 action){
    record(2,object,action,0);u32 result=original_quick(object,action);record(3,object,action,result);return result;
}
static u32 THIS main_action(void*,u32);
static u32 THIS quick_action(void*,u32);
#include "channel.h"
static u32 THIS spell_tick(void* object){
    record(1,object,0,0);menu_poll(object,1);u32 result=((TickFn)0x576830)(object);menu_poll(object,0);record(4,object,0,result);return result;
}
static u32 THIS tick(void* object){
    record(1,object,0,0);menu_poll(object,1);u32 result=original_tick(object);menu_poll(object,0);record(4,object,0,result);return result;
}
static void* trampoline(u32 site,u32 length,int relative_call){
    u8* out=VirtualAlloc(0,length+5,0x3000,0x40);if(!out)return 0;
    copy(out,(void*)site,length);
    if(relative_call){u32 target=site+8+get((void*)(site+4));put(out+4,target-(u32)out-8);}
    out[length]=0xe9;put(out+length+1,site+length-(u32)out-length-5);
    FlushInstructionCache(GetCurrentProcess(),out,length+5);return out;
}
static void jump(u32 site,u32 target,u32 length){
    u8* p=(u8*)site;p[0]=0xe9;put(p+1,target-site-5);for(u32 i=5;i<length;++i)p[i]=0x90;
}
static int install_battle(void){
    // Additional slots are touched only for the separate V2 contract.
    const u8 setup_bytes[8]={0x56,0x8b,0xf1,0xe8,0x68,0x9e,0x0a,0x00};
    const u8 map_bytes[9]={0x56,0x57,0x8b,0xf1,0xe8,0x17,0xb9,0x09,0x00};
    if(!readable((void*)0x5c6544,4)||!readable((void*)0x5c6950,4)||
       get((void*)0x5c6544)!=0x5595d0||get((void*)0x5c6950)!=0x5595d0||
       !readable((void*)0x4ad6a0,8)||!equal((void*)0x4ad6a0,setup_bytes,8)||
       !readable((void*)0x4bbbf0,9)||!equal((void*)0x4bbbf0,map_bytes,9))return 0;
    u32 a,b,unused;
    if(!VirtualProtect((void*)0x5c6544,4,0x40,&a))return 0;
    if(!VirtualProtect((void*)0x5c6950,4,0x40,&b)){VirtualProtect((void*)0x5c6544,4,a,&unused);return 0;}
    put((void*)0x5c6544,(u32)&tick);put((void*)0x5c6950,(u32)&tick);
    VirtualProtect((void*)0x5c6950,4,b,&unused);VirtualProtect((void*)0x5c6544,4,a,&unused);return 1;
}
static int install(void){
    const u8 main_bytes[7]={0x51,0x53,0x55,0x56,0x57,0x8b,0xf9};
    const u8 quick_bytes[8]={0x56,0x8b,0xf1,0xe8,0x68,0xf1,0x0a,0x00};
    char path[1024];u32 length=GetEnvironmentVariableA("MNM_MENU_OBSERVE",path,sizeof(path));
    if(!length||length>=sizeof(path)||log_file)return 0;
#ifndef MNM_MENU_SELFTEST
    if((u32)GetModuleHandleA(0)!=0x400000)return 0;
#endif
    if(!readable((void*)0x4a75c0,7)||!readable((void*)0x4a83a0,8)||
       !readable((void*)0x5c63e4,0x4c)||!equal((void*)0x4a75c0,main_bytes,7)||
       !equal((void*)0x4a83a0,quick_bytes,8)||get((void*)0x5c63f4)!=0x5595d0||get((void*)0x5c642c)!=0x5595d0)return 0;
    original_main=trampoline(0x4a75c0,7,0);original_quick=trampoline(0x4a83a0,8,1);
    if(!original_main||!original_quick)return 0;
    // Acquire all permissions before any instruction/table write.
    u32 main_protect,quick_protect,table_protect,unused;
    if(!VirtualProtect((void*)0x4a75c0,7,0x40,&main_protect))return 0;
    if(!VirtualProtect((void*)0x4a83a0,8,0x40,&quick_protect)){
        VirtualProtect((void*)0x4a75c0,7,main_protect,&unused);return 0;
    }
    if(!VirtualProtect((void*)0x5c63e4,0x4c,0x40,&table_protect)){
        VirtualProtect((void*)0x4a83a0,8,quick_protect,&unused);VirtualProtect((void*)0x4a75c0,7,main_protect,&unused);return 0;
    }
    log_file=CreateFileA(path,0x40000000,1,0,1,0x80,0); // CREATE_NEW; allow read-only observation.
    if(log_file==(HANDLE)-1)log_file=0;
    if(log_file){
        const u8 header[16]={'M','N','M','M','E','N','U','1',1,0,0,0,64,0,0,0};u32 wrote;
        if(!WriteFile(log_file,header,16,&wrote,0)||wrote!=16){CloseHandle(log_file);log_file=0;}
    }
    if(log_file){
        original_tick=(TickFn)0x5595d0;
        menu_init();
        if(menu_version>=2&&!install_battle())menu_retired=1;
        if(menu_version>=3){
            u32 old,unused;
            if(!readable((void*)0x5c776c,4)||get((void*)0x5c776c)!=0x576830||!equal((void*)0x576830,(u8[]){0x81,0xec,0,2},4)||
               !VirtualProtect((void*)0x5c776c,4,0x40,&old))menu_retired=1;
            else{put((void*)0x5c776c,(u32)&spell_tick);VirtualProtect((void*)0x5c776c,4,old,&unused);}
        }
        if(menu_version==4&&MNM_MENU_MINI_EXPERIMENTAL){
            u32 old,restore;
            const u8 callback_bytes[]={0x64,0xa1,0,0,0,0};
            if(!readable((void*)0x5c6654,4)||get((void*)0x5c6654)!=0x5595d0||
               !readable((void*)0x4b23f0,6)||!equal((void*)0x4b23f0,callback_bytes,6)||
               !VirtualProtect((void*)0x5c6654,4,0x40,&old))menu_retired=1;
            else {put((void*)0x5c6654,(u32)&tick);VirtualProtect((void*)0x5c6654,4,old,&restore);}
        }
        jump(0x4a75c0,(u32)&main_action,7);jump(0x4a83a0,(u32)&quick_action,8);
        put((void*)0x5c63f4,(u32)&tick);put((void*)0x5c642c,(u32)&tick);
        FlushInstructionCache(GetCurrentProcess(),(void*)0x4a75c0,7);
        FlushInstructionCache(GetCurrentProcess(),(void*)0x4a83a0,8);
    }
    VirtualProtect((void*)0x5c63e4,0x4c,table_protect,&unused);
    VirtualProtect((void*)0x4a83a0,8,quick_protect,&unused);VirtualProtect((void*)0x4a75c0,7,main_protect,&unused);
    return log_file!=0;
}
__declspec(dllexport) void MenuAnchor(void){}
#ifdef MNM_MENU_SELFTEST
__declspec(dllexport) int WIN MenuInstallForTest(void){u32 error=GetLastError();int ok=install();SetLastError(error);return ok;}
#endif
int WIN DllMain(void* dll,u32 reason,void* reserved){
    (void)dll;(void)reserved;
#ifndef MNM_MENU_SELFTEST
    if(reason==1){u32 error=GetLastError();install();SetLastError(error);}
#endif
    if(reason==0){if(log_file)CloseHandle(log_file);if(menu_words)UnmapViewOfFile(menu_words);}return 1;
}
