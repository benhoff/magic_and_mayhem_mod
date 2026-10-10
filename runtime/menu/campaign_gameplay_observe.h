/* Opt-in passive, build-specific creature snapshots and original melee returns.
 * Never issues orders, changes health/mana, or replaces simulation. */
static HANDLE gameplay_file;
static volatile u32 gameplay_busy;
static u32 gameplay_sequence,gameplay_next_sample;
static u8 gameplay_was_active[256];
static TickFn gameplay_original_melee;
static void gameplay_write(u32* words){
    if(!gameplay_file||gameplay_sequence>=65536||!__sync_bool_compare_and_swap(&gameplay_busy,0,1))return;
    words[0]=++gameplay_sequence;words[2]=GetCurrentThreadId();words[3]=GetTickCount();
    u8 data[64];for(u32 i=0;i<16;++i)put(data+4*i,words[i]);u32 wrote;
    if(!WriteFile(gameplay_file,data,64,&wrote,0)||wrote!=64){CloseHandle(gameplay_file);gameplay_file=0;}
    __sync_lock_release(&gameplay_busy);
}
static u8* gameplay_creature(u32 slot){
    if(!readable((void*)0x6def58,8))return 0;
    u32 base=get((void*)0x6def58),capacity=get((void*)0x6def5c);
    if(!base||!capacity||capacity>256||slot>=capacity)return 0;
    u32 at=base+slot*0xe4b;if(at<base)return 0;
    u8* p=(u8*)at;
    return readable(p,0xe4b)&&get(p)==slot?p:0;
}
static u32 THIS gameplay_melee(void* object){
    u32 error=GetLastError();u8* source=object;u8* target=0;
    u32 row[16]={0,2,0,0,0xffffffff,0,0xffffffff,0,0xffffffff,0,0xffffffff,0,0,0,0,0};
    if(readable(source,0xe4b)&&gameplay_creature(get(source))==source){
        row[4]=get(source);row[5]=get(source+0xa8);row[6]=get(source+0x174);
        row[7]=get(source+0xe4);row[8]=get(source+0x64c);target=gameplay_creature(row[8]);
        if(target){row[9]=get(target+0xa8);row[10]=get(target+0x174);row[11]=get(target+0xe4);row[12]=row[11];row[13]=get(target+4);}
    }
    SetLastError(error);u32 result=gameplay_original_melee(object);error=GetLastError();
    if(target&&gameplay_creature(row[8])==target&&get(target+0xa8)==row[9]&&get(target+0x174)==row[10]){row[12]=get(target+0xe4);row[14]=get(target+4);}
    row[15]=result;gameplay_write(row);SetLastError(error);return result;
}
static void gameplay_sample(void){
    if(!gameplay_file)return;
    u32 error=GetLastError(),now=GetTickCount();
    if((int)(now-gameplay_next_sample)<0){SetLastError(error);return;}
    gameplay_next_sample=now+250;
    if(readable((void*)0x6def5c,4)){
        u32 capacity=get((void*)0x6def5c);
        if(capacity<=256)for(u32 slot=0;slot<capacity;++slot){
            u8* p=gameplay_creature(slot);if(!p)continue;
            u32 active=get(p+4);if(!active&&!gameplay_was_active[slot])continue;
            gameplay_was_active[slot]=active!=0;
            u32 row[16]={0,1,0,0,slot,get(p+0xa8),get(p+0x174),active,get(p+0xe4),
                get(p+8),get(p+0xc),get(p+0x10),get(p+0x5ec),get(p+0x5fc),get(p+0x64c),get(p+0xe8)};
            gameplay_write(row);
        }
    }
    SetLastError(error);
}
#include "campaign_spell_observe.h"
static int install_gameplay_observe(void){
    char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_GAMEPLAY_OBSERVE",path,sizeof(path));
    if(!n)return 1;
    const u8 expected[6]={0x81,0xec,0x1c,0x01,0,0};
    if(n>=sizeof(path)||!readable((void*)0x50c0d0,6)||!equal((void*)0x50c0d0,expected,6))return 0;
    gameplay_original_melee=(TickFn)trampoline(0x50c0d0,6,0);if(!gameplay_original_melee)return 0;
    u32 old,restore;if(!VirtualProtect((void*)0x50c0d0,6,0x40,&old))return 0;
    gameplay_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(gameplay_file==(HANDLE)-1)gameplay_file=0;
    if(!gameplay_file){VirtualProtect((void*)0x50c0d0,6,old,&restore);return 0;}
    const u8 header[16]={'M','N','M','G','P','0','0','1',1,0,0,0,64,0,0,0};u32 wrote;
    if(!WriteFile(gameplay_file,header,16,&wrote,0)||wrote!=16){CloseHandle(gameplay_file);gameplay_file=0;VirtualProtect((void*)0x50c0d0,6,old,&restore);return 0;}
    if(!install_spell_observe()){CloseHandle(gameplay_file);gameplay_file=0;VirtualProtect((void*)0x50c0d0,6,old,&restore);return 0;}
    jump(0x50c0d0,(u32)&gameplay_melee,6);VirtualProtect((void*)0x50c0d0,6,old,&restore);
    FlushInstructionCache(GetCurrentProcess(),(void*)0x50c0d0,6);return 1;
}
