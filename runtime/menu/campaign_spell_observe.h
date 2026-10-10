/* Separate opt-in, bounded diagnostic wire; original cast/impact forwarding. */
static HANDLE spell_file;
static volatile u32 spell_busy;
static u32 spell_sequence;
static TickFn spell_original_cast;
static ActionFn spell_original_impact;
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
static int install_spell_observe(void){
    char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_SPELL_OBSERVE",path,sizeof(path));
    if(!n)return 1;
    const u8 cast_bytes[7]={0x6a,0xff,0x68,0xc8,0x2d,0x5c,0};
    const u8 impact_bytes[5]={0x8b,0xd1,0x53,0x55,0x56};
    if(n>=sizeof(path)||!readable((void*)0x57b710,7)||!equal((void*)0x57b710,cast_bytes,7)||
       !readable((void*)0x48ecf0,5)||!equal((void*)0x48ecf0,impact_bytes,5))return 0;
    spell_original_cast=(TickFn)trampoline(0x57b710,7,0);
    spell_original_impact=(ActionFn)trampoline(0x48ecf0,5,0);
    if(!spell_original_cast||!spell_original_impact)return 0;
    u32 old_cast,old_impact,restore;
    if(!VirtualProtect((void*)0x57b710,7,0x40,&old_cast))return 0;
    if(!VirtualProtect((void*)0x48ecf0,5,0x40,&old_impact)){VirtualProtect((void*)0x57b710,7,old_cast,&restore);return 0;}
    spell_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(spell_file==(HANDLE)-1)spell_file=0;
    const u8 header[16]={'M','N','M','C','A','0','0','1',1,0,0,0,64,0,0,0};u32 wrote=0;
    int ok=spell_file&&WriteFile(spell_file,header,16,&wrote,0)&&wrote==16;
    if(ok){jump(0x57b710,(u32)&spell_cast,7);jump(0x48ecf0,(u32)&spell_impact,5);}
    else if(spell_file){CloseHandle(spell_file);spell_file=0;}
    VirtualProtect((void*)0x48ecf0,5,old_impact,&restore);VirtualProtect((void*)0x57b710,7,old_cast,&restore);
    if(ok){FlushInstructionCache(GetCurrentProcess(),(void*)0x57b710,7);FlushInstructionCache(GetCurrentProcess(),(void*)0x48ecf0,5);}
    return ok;
}
