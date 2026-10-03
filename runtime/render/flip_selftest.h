/* Application-facing double-buffer fixture; reuse native lock/alias assertions. */
static struct HState hf;static char f_mode[32];static u32 f_calls,f_seeded;
static i32 WIN f_desc(void* object,u32* d){
    i32 status=h_desc(object,d);struct HState* s=((struct HSurface*)object)->state;
    if(s==&hf){d[1]|=0x20;d[5]=f_mode[0]=='c'?2:1;d[26]=0x238;}
    else if(s==&hd){d[26]=0x1c;if(f_mode[0]=='m' && f_seeded)d[22]=0x7c00;}
    return status;
}
static i32 WIN f_lock(void* object,void* rect,u32* desc,u32 flags,HANDLE event){
    if(f_mode[0]=='p' && f_calls && flags==0x4810){SetLastError(0x88);return (i32)0x8876021c;}
    return h_lock(object,rect,desc,flags,event);
}
static i32 WIN f_attached(void* object,const u32* caps,void** out){
    if(((struct HSurface*)object)->state!=&hf || caps[0]!=4)ExitProcess(71);
    ++hd.refs;*out=&hd.base;SetLastError(0x99);return 0;
}
static i32 WIN f_flip(void* object,void* target,u32 flags){
    if(((struct HSurface*)object)->state!=&hf || hf.locked || hd.locked || GetLastError()!=0x77 ||
       target!=(f_mode[0]=='t'?(void*)&hd.alias:0) || flags!=(f_mode[0]=='s'?0x10u:1u))ExitProcess(72);
    ++f_calls;SetLastError(0x88);
    if(f_mode[0]=='f' && f_calls==1)return (i32)0x8876021c;
    if(f_mode[0]!='b' && f_mode[0]!='p'){u16* pixels=hf.pixels;hf.pixels=hd.pixels;hd.pixels=pixels;}
    return 23;
}
static void f_do_flip(void){
    SetLastError(0x77);i32 status=((i32 (WIN *)(void*,void*,u32))hf.base.table[11])(&hf.base,
        f_mode[0]=='t'?&hd.alias:0,f_mode[0]=='s'?0x10:1);
    if(status!=(f_mode[0]=='f' && f_calls==1?(i32)0x8876021c:23) || GetLastError()!=0x88)ExitProcess(73);
    if(hs.refs!=1 || hd.refs!=(f_mode[0]=='t'?2u:1u) || hf.refs!=1 || hs.locked || hd.locked || hf.locked)ExitProcess(74);
}
static void test_flip_history(void){
    static void *old[33],*modern[33];h_install(old);h_install(modern);
    old[25]=modern[25]=(void*)&f_lock;old[22]=modern[22]=(void*)&f_desc;old[12]=modern[12]=(void*)&f_attached;old[11]=modern[11]=(void*)&f_flip;
    GetEnvironmentVariableA("MNM_FLIP_SELFTEST",f_mode,sizeof(f_mode));
    static u16 src[6]={0xffff,0,0xf800,0x7e0,0,0xffff},back[12],front[12];
    for(u32 i=0;i<12;++i){back[i]=0x1f;front[i]=0xf800;}
    u32 kind=f_mode[0]=='o'?12:14;void** table=kind==12?old:modern;
    hs.base.table=table;hs.base.state=&hs;hs.base.modern=kind>=14;hs.alias=hs.base;
    hd.base=hs.base;hd.base.state=&hd;hd.alias=hd.base;hf.base=hs.base;hf.base.state=&hf;hf.alias=hf.base;
    hs.width=3;hs.height=2;hs.pitch=6;hs.pixels=src;hs.refs=1;
    hd.width=hf.width=4;hd.height=hf.height=3;hd.pitch=hf.pitch=8;hd.pixels=back;hf.pixels=front;hd.refs=hf.refs=1;
    RenderInstallForTest(&hs.base,kind);RenderInstallForTest(&hd.base,kind);RenderInstallForTest(&hf.base,kind);
    h_copy(&hs.base);f_seeded=1; /* Seed history with an eligible draw to the back buffer. */
    if(f_mode[0]=='t'){
        static const u8 unknown[16]={0};void* out=0;
        if(h_query(&hd.alias,unknown,&out) || out!=&hd.base)ExitProcess(75);
        /* Alias identity is observed through original QI at Flip. */
    }
    f_do_flip();if(f_mode[0]=='f')f_do_flip();
    if(f_mode[0]!='c' && f_mode[0]!='s' && f_mode[0]!='m' && f_mode[0]!='b' && f_mode[0]!='p'){
        h_put(&hd.base,0xf81f,0);f_do_flip();
        h_put(&hd.base,0xffe0,0);f_do_flip();
    }
    if(f_mode[0]=='t')h_drop(&hd.alias,1);
    h_drop(&hs.base,0);h_drop(&hd.base,0);h_drop(&hf.base,0);
}
