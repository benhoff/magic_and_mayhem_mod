/* Opt-in passive snapshots at original Save/Load completion flag transitions after Tick.
 * Uses the existing guarded creature pool reader; never writes game state. */
static HANDLE checkpoint_file;
static u32 checkpoint_sequence,checkpoint_boundaries;
static int checkpoint_open(void){
    char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_CHECKPOINT_OBSERVE",path,sizeof(path));
    if(!n)return 1;
    if(n>=sizeof(path))return 0;
    checkpoint_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(checkpoint_file==(HANDLE)-1)checkpoint_file=0;
    if(!checkpoint_file)return 0;
    const u8 header[16]={'M','N','M','C','P','S','0','1',1,0,0,0,64,0,0,0};u32 wrote;
    return WriteFile(checkpoint_file,header,16,&wrote,0)&&wrote==16;
}
static int checkpoint_write(u32* row){
    u8 bytes[64];u32 wrote;row[0]=++checkpoint_sequence;
    for(u32 i=0;i<16;++i)put(bytes+4*i,row[i]);
    return WriteFile(checkpoint_file,bytes,64,&wrote,0)&&wrote==64;
}
static void checkpoint_storage_sample(u32 screen,u32 completed){
    if(!checkpoint_file||completed!=1||checkpoint_boundaries>=8)return;
    u32 error=GetLastError(),capacity=0;
    if(readable((void*)0x6def5c,4))capacity=get((void*)0x6def5c);
    if(!capacity||capacity>256){SetLastError(error);return;}
    u32 thread=GetCurrentThreadId(),tick=GetTickCount();
    u32 boundary[16]={0,1,thread,tick,++checkpoint_boundaries,screen,capacity,completed,0,0,0,0,0,0,0,0};
    int ok=checkpoint_write(boundary);
    for(u32 slot=0;ok&&slot<capacity;++slot){
        u8* p=gameplay_creature(slot);
        if(!p){ok=0;break;}
        u32 row[16]={0,2,thread,tick,slot,0,0,0,0,0,0,0,0,0,0,0};
        if(get(p+4)){
            row[5]=get(p+0xa8);row[6]=get(p+0x174);row[7]=get(p+4);row[8]=get(p+0xe4);
            row[9]=get(p+8);row[10]=get(p+0xc);row[11]=get(p+0x10);
            row[12]=get(p+0x5ec);row[13]=get(p+0x5fc);row[14]=get(p+0x64c);row[15]=get(p+0xe8);
        }
        ok=checkpoint_write(row);
    }
    if(!ok){CloseHandle(checkpoint_file);checkpoint_file=0;}
    SetLastError(error);
}
