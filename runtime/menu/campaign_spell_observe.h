/* Separate opt-in, bounded diagnostic wire; original cast/impact forwarding. */
static HANDLE spell_file;
static volatile u32 spell_busy;
static u32 spell_sequence;
static TickFn spell_original_cast;
static ActionFn spell_original_impact;
typedef u32 (THIS *SpellDamageFn)(void*,u32,u32,u32);
static TickFn spell_original_dispatch;
static SpellDamageFn spell_original_damage;
typedef u32 (THIS *SpellHealthFn)(void*,u32,u32,u32,u32);
static SpellHealthFn spell_original_health;
static volatile u32 spell_context_thread;
static u32 spell_context_id=0xffffffff,spell_context_type=0xffffffff;
static u32 spell_context_source=0xffffffff,spell_context_owner=0xffffffff;
static void spell_write(u32* words){
    if(!spell_file||spell_sequence>=65536||!__sync_bool_compare_and_swap(&spell_busy,0,1))return;
    words[0]=++spell_sequence;words[2]=GetCurrentThreadId();words[3]=GetTickCount();
    u8 data[64];for(u32 i=0;i<16;++i)put(data+4*i,words[i]);u32 wrote;
    if(!WriteFile(spell_file,data,64,&wrote,0)||wrote!=64){CloseHandle(spell_file);spell_file=0;}
    __sync_lock_release(&spell_busy);
}
static u32 THIS spell_cast(void* descriptor){
    u32 error=GetLastError();u8* d=descriptor;u8* source=0;
    u32 row[16]={0,1,0,0,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0,0,0xffffffff,0,0,0,0,0};
    if(readable(d,0x30)){
        row[4]=get(d);row[5]=get(d+0x1c);row[10]=get(d+0x20);
        row[11]=get(d+0x10);row[12]=get(d+0x14);row[13]=get(d+0x18);
        source=gameplay_creature(row[5]);
        if(source){row[6]=get(source+0xa8);row[7]=get(source+0x174);row[8]=get(source+0xe8);row[9]=row[8];}
    }
    SetLastError(error);u32 result=spell_original_cast(descriptor);error=GetLastError();
    if(source&&gameplay_creature(row[5])==source&&get(source+0xa8)==row[6]&&get(source+0x174)==row[7]){
        row[9]=get(source+0xe8);row[15]=1;
    }
    row[14]=result;spell_write(row);SetLastError(error);return result;
}
static u32 THIS spell_impact(void* object,u32 amount){
    u32 error=GetLastError();u8* p=object;u8* target=0;
    u32 row[16]={0,2,0,0,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0,0,amount,0xffffffff,0,0};
    if(readable(p,0x22e)){
        row[4]=get(p);row[5]=get(p+0x28);row[6]=get(p+0x4c);row[13]=get(p+0x212);
        /* The original uses +0x198 only when +0x48 is not -1. Validate the
         * borrowed pointer against the current creature pool before reading. */
        if(get(p+0x48)!=0xffffffff){
            u8* candidate=(u8*)get(p+0x198);
            if(readable(candidate,0xe4b)&&gameplay_creature(get(candidate))==candidate){
                target=candidate;row[7]=get(target);row[8]=get(target+0xa8);row[9]=get(target+0x174);
                row[10]=get(target+0xe4);row[11]=row[10];row[14]=get(target+4);
            }
        }
    }
    SetLastError(error);u32 result=spell_original_impact(object,amount);error=GetLastError();
    if(target&&gameplay_creature(row[7])==target&&get(target+0xa8)==row[8]&&get(target+0x174)==row[9]){
        row[11]=get(target+0xe4);row[15]=get(target+4);
    }
    spell_write(row);SetLastError(error);return result;
}
static u32 THIS spell_dispatch(void* object){
    u32 error=GetLastError(),tid=GetCurrentThreadId();
    int acquired=__sync_bool_compare_and_swap(&spell_context_thread,0,tid);
    int owns=acquired||spell_context_thread==tid;
    u32 saved[4]={0xffffffff,0xffffffff,0xffffffff,0xffffffff};
    if(owns){
        saved[0]=spell_context_id;saved[1]=spell_context_type;saved[2]=spell_context_source;saved[3]=spell_context_owner;
        spell_context_id=spell_context_type=spell_context_source=spell_context_owner=0xffffffff;
        u8* p=object;
        if(readable(p,0x22e)){
            spell_context_id=get(p+0x4c);spell_context_type=get(p+0x28);
            u32 slot=get(p+0x44);u8* source=gameplay_creature(slot);
            if(source&&get(source+4)&&(int)get(source+0xe4)>0){spell_context_source=slot;spell_context_owner=get(source+0x174);}
        }
    }
    SetLastError(error);u32 result=spell_original_dispatch(object);error=GetLastError();
    if(owns){spell_context_id=saved[0];spell_context_type=saved[1];spell_context_source=saved[2];spell_context_owner=saved[3];}
    if(acquired)__sync_lock_release(&spell_context_thread);
    SetLastError(error);return result;
}
static u32 THIS spell_damage(void* object,u32 amount,u32 owner,u32 attribution){
    u32 error=GetLastError();u8* target=object;
    u32 row[16]={0,3,0,0,0xffffffff,0xffffffff,0xffffffff,0,0,0,amount,owner,0xffffffff,0xffffffff,0xffffffff,0xffffffff};
    if(readable(target,0xe4b)&&gameplay_creature(get(target))==target){
        row[4]=get(target);row[5]=get(target+0xa8);row[6]=get(target+0x174);
        row[7]=get(target+0xe4);row[8]=row[7];row[9]=get(target+4);
    }else target=0;
    if(spell_context_thread==GetCurrentThreadId()){
        row[12]=spell_context_id;row[14]=spell_context_source;row[15]=spell_context_owner;
    }
    row[13]=(u32)__builtin_return_address(0);
    SetLastError(error);u32 result=spell_original_damage(object,amount,owner,attribution);error=GetLastError();
    if(target&&gameplay_creature(row[4])==target&&get(target+0xa8)==row[5]&&get(target+0x174)==row[6])row[8]=get(target+0xe4);
    spell_write(row);SetLastError(error);return result;
}
static u32 THIS spell_health(void* object,u32 amount,u32 owner,u32 attribution,u32 feedback){
    u32 error=GetLastError();u8* target=object;
    u32 row[16]={0,4,0,0,0xffffffff,0xffffffff,0xffffffff,0,0,0,amount,owner,0xffffffff,0xffffffff,0xffffffff,0xffffffff};
    if(readable(target,0xe4b)&&gameplay_creature(get(target))==target){
        row[4]=get(target);row[5]=get(target+0xa8);row[6]=get(target+0x174);
        row[7]=get(target+0xe4);row[8]=row[7];row[9]=get(target+4);
    }else target=0;
    if(spell_context_thread==GetCurrentThreadId()){
        row[12]=spell_context_id;row[14]=spell_context_source;row[15]=spell_context_owner;
    }
    row[13]=(u32)__builtin_return_address(0);
    SetLastError(error);u32 result=spell_original_health(object,amount,owner,attribution,feedback);error=GetLastError();
    if(target&&gameplay_creature(row[4])==target&&get(target+0xa8)==row[5]&&get(target+0x174)==row[6])row[8]=get(target+0xe4);
    spell_write(row);SetLastError(error);return result;
}
static int install_spell_observe(void){
    char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_SPELL_OBSERVE",path,sizeof(path));
    if(!n)return 1;
    const u8 cast_bytes[7]={0x6a,0xff,0x68,0xc8,0x2d,0x5c,0};
    const u8 impact_bytes[5]={0x8b,0xd1,0x53,0x55,0x56};
    const u8 dispatch_bytes[7]={0x6a,0xff,0x68,0x78,0x02,0x5c,0};
    const u8 damage_bytes[5]={0x56,0x8b,0x74,0x24,0x08};
    const u8 health_bytes[6]={0x51,0xa0,0x58,0x98,0x6e,0};
    if(n>=sizeof(path)||!readable((void*)0x57b710,7)||!equal((void*)0x57b710,cast_bytes,7)||
       !readable((void*)0x48ecf0,5)||!equal((void*)0x48ecf0,impact_bytes,5)||
       !readable((void*)0x48b0f0,7)||!equal((void*)0x48b0f0,dispatch_bytes,7)||
       !readable((void*)0x514860,5)||!equal((void*)0x514860,damage_bytes,5)||
       !readable((void*)0x5076f0,6)||!equal((void*)0x5076f0,health_bytes,6))return 0;
    spell_original_cast=(TickFn)trampoline(0x57b710,7,0);
    spell_original_impact=(ActionFn)trampoline(0x48ecf0,5,0);
    spell_original_dispatch=(TickFn)trampoline(0x48b0f0,7,0);
    spell_original_damage=(SpellDamageFn)trampoline(0x514860,5,0);
    spell_original_health=(SpellHealthFn)trampoline(0x5076f0,6,0);
    if(!spell_original_cast||!spell_original_impact||!spell_original_dispatch||!spell_original_damage||!spell_original_health)return 0;
    const u32 sites[5]={0x57b710,0x48ecf0,0x48b0f0,0x514860,0x5076f0},lengths[5]={7,5,7,5,6};
    const u32 hooks[5]={(u32)&spell_cast,(u32)&spell_impact,(u32)&spell_dispatch,(u32)&spell_damage,(u32)&spell_health};
    u32 old[5],restore;
    for(u32 i=0;i<5;++i)if(!VirtualProtect((void*)sites[i],lengths[i],0x40,old+i)){
        for(u32 j=0;j<i;++j)VirtualProtect((void*)sites[j],lengths[j],old[j],&restore);
        return 0;
    }
    spell_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(spell_file==(HANDLE)-1)spell_file=0;
    const u8 header[16]={'M','N','M','C','A','0','0','3',3,0,0,0,64,0,0,0};u32 wrote=0;
    int ok=spell_file&&WriteFile(spell_file,header,16,&wrote,0)&&wrote==16;
    if(ok)for(u32 i=0;i<5;++i)jump(sites[i],hooks[i],lengths[i]);
    else if(spell_file){CloseHandle(spell_file);spell_file=0;}
    for(u32 i=0;i<5;++i){VirtualProtect((void*)sites[i],lengths[i],old[i],&restore);if(ok)FlushInstructionCache(GetCurrentProcess(),(void*)sites[i],lengths[i]);}
    return ok;
}
