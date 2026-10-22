/* Opt-in original-driver observation. No native pixel input or replacement.
 * Added QI/Lock/Unlock calls are explicit diagnostic intervention. Only the
 * first drawing-canvas clear is read, inside a successful real driver lease. */
API u32 WIN GetCurrentThreadId(void);
extern void canvas_driver_blt_return(void);
u32 canvas_driver_blt_continue=0x58bc5c;
static HANDLE driver_file;
static u32 driver_enabled,driver_sequence,driver_generation,driver_thread;
static u32 driver_blt_count,driver_blt_result,driver_leases,driver_fault;
static char driver_directory[260];

static HANDLE driver_open(const char* suffix){
    char path[260];u32 n=0;
    while(driver_directory[n]&&n<220){path[n]=driver_directory[n];++n;}
    while(*suffix&&n<259)path[n++]=*suffix++;
    if(*suffix)return 0;path[n]=0;
    HANDLE f=CreateFileA(path,0x40000000,0,0,1,0x80,0);
    return f==(HANDLE)-1?0:f;
}
static void driver_emit(u32* row){
    if(!driver_file)return;
    row[0]=++driver_sequence;row[4]=GetCurrentThreadId();
    row[27]=driver_generation;row[28]=get((void*)0x6e1f68);
    row[29]=driver_fault;row[30]=driver_blt_count;
    u32 wrote;
    if(driver_sequence>4096||!WriteFile(driver_file,row,128,&wrote,0)||wrote!=128){
        driver_fault=1;CloseHandle(driver_file);driver_file=0;
    }
}
static void driver_common(u32* row,u32 event,u32 wrapper,u32 caller){
    row[1]=event;row[2]=wrapper;row[3]=caller;row[11]=driver_blt_result;
    if(readable(wrapper,12)){row[5]=get((void*)(wrapper+4));row[6]=get((void*)(wrapper+8));}
}
void canvas_driver_blt_observe(u32* registers){
    u32 error=GetLastError();
    if(driver_enabled&&driver_file&&registers[1]==0x6a49e0){
        u32 row[32]={0};
        if(driver_thread!=GetCurrentThreadId())driver_fault=2;
        driver_blt_result=registers[7];++driver_blt_count;
        driver_common(row,20,registers[1],0x58bc5a);driver_emit(row);
    }
    SetLastError(error);
}
static void driver_created(u32 wrapper,u32 caller){
    static const u8 iid[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    u32 row[32]={0};driver_common(row,10,wrapper,caller);
    ++driver_generation;
    for(u32 i=0;i<2;++i){
        void* surface=(void*)row[5+i];void* identity=0;
        if(!readable((u32)surface,4)||!readable(get(surface),12)){driver_fault=3;break;}
        void** table=*(void***)surface;
        row[9+i]=((u32(WIN*)(void*,const void*,void**))table[0])(surface,iid,&identity);
        row[7+i]=(u32)identity;
        if(!row[9+i]&&identity)((u32(WIN*)(void*))(*(void***)identity)[2])(identity);
    }
    driver_emit(row);
}
static void driver_cleared(u32 wrapper,u32 caller,u32 colour){
    u32 row[32]={0},desc[27]={0};driver_common(row,30,wrapper,caller);
    if(driver_thread!=GetCurrentThreadId())driver_fault=2;
    void* surface=(void*)row[6];
    row[13]=row[14]=0xffffffff;
    if(!readable((u32)surface,4)||!readable(get(surface),132)){driver_fault=3;driver_emit(row);return;}
    void** table=*(void***)surface;
    row[12]=((u32(WIN*)(void*,u32))table[13])(surface,2); /* DDGBS_ISBLTDONE */
    desc[0]=108;
    row[13]=((u32(WIN*)(void*,void*,void*,u32,HANDLE))table[25])(surface,0,desc,0x11,0);
    row[15]=desc[3];row[16]=desc[2];row[17]=desc[4];
    row[18]=desc[21];row[19]=desc[22];row[20]=desc[23];row[21]=desc[24];
    if(!row[13]){
        row[26]|=1;++driver_leases;
        i32 pitch=(i32)desc[4];u32 width=desc[3],height=desc[2],pixels=desc[9];
        int shape=width&&width<=2048&&height&&height<=2048&&pitch>=0&&
            (u32)pitch>=width*2&&(u32)pitch<=8192&&desc[21]==16&&
            desc[22]==0xf800&&desc[23]==0x7e0&&desc[24]==31;
        if(shape&&readable(pixels,(u32)pitch*height)){
            HANDLE raw=driver_open("\\canvas-driver-clear.565");
            u32 hash=2166136261u;
            for(u32 y=0;y<height;++y){
                const u8* p=(const u8*)pixels+y*(u32)pitch;
                for(u32 x=0;x<width;++x){u16 word=(u16)p[2*x]|(u16)p[2*x+1]<<8;
                    if(word)++row[22];if(word!=(u16)colour)++row[23];
                    hash=(hash^p[2*x])*16777619u;hash=(hash^p[2*x+1])*16777619u;
                }
                u32 wrote;
                if(!raw||!WriteFile(raw,p,width*2,&wrote,0)||wrote!=width*2)driver_fault=4;
                else row[25]+=wrote;
            }
            row[24]=hash;if(raw)CloseHandle(raw);row[26]|=2;
        }else driver_fault=5;
        row[14]=((u32(WIN*)(void*,void*))table[32])(surface,0);
        if(row[14])driver_fault=6;else row[26]|=4;
    }
    driver_emit(row);
}
static void driver_after(u32 tag,u32 wrapper,u32 caller,u32 colour){
    if(!driver_enabled||!driver_file||wrapper!=0x6a49e0)return;
    if(tag==0)driver_created(wrapper,caller);
    if(tag==5&&caller==0x4e41fd)driver_cleared(wrapper,caller,colour);
    if(tag==1){u32 row[32]={0};driver_common(row,40,wrapper,caller);driver_emit(row);}
}
static void driver_finish(void){
    if(driver_file){u32 row[32]={0};row[1]=99;row[25]=driver_leases;driver_emit(row);
        CloseHandle(driver_file);driver_file=0;}
}
static int driver_prepare(void){
    char flag[2];
    u32 flag_length=GetEnvironmentVariableA("MNM_CANVAS_DRIVER_OBSERVE",flag,2);
    if(!flag_length)return 1;
    if(flag_length!=1||flag[0]!='1')return 0;
    static const u8 expected[5]={0xff,0x51,0x14,0x8b,0xf8};
    if(!readable(0x58bc57,5))return 0;
    for(u32 i=0;i<5;++i)if(((const u8*)0x58bc57)[i]!=expected[i])return 0;
    u32 n=GetEnvironmentVariableA("MNM_SCENE_DIR",driver_directory,220);
    if(!n||n>=220)return 0;
    driver_thread=GetCurrentThreadId();driver_file=driver_open("\\canvas-driver.bin");
    if(!driver_file)return 0;
    u32 header[16]={0},wrote;copy(header,"MNMCDB01",8);
    header[2]=1;header[3]=128;header[4]=0x40209ca7;header[5]=4096;
    if(!WriteFile(driver_file,header,64,&wrote,0)||wrote!=64)return 0;
    driver_enabled=1;return 1;
}
static int driver_install(void){
    if(!driver_enabled)return 1;
    u8 patch[5]={0xe9};u32 old,unused;
    put(patch+1,(u32)&canvas_driver_blt_return-0x58bc57-5);
    if(!VirtualProtect((void*)0x58bc57,5,0x40,&old))return 0;
    copy((void*)0x58bc57,patch,5);FlushInstructionCache(GetCurrentProcess(),(void*)0x58bc57,5);
    return VirtualProtect((void*)0x58bc57,5,old,&unused)!=0;
}
