/* Original Unlock poisons pixels: snapshots must already own their bytes. */
static char lifecycle_mode[32];
struct LifecycleSurface {void** table;u32 live,held;struct LifecycleSurface* state;u32 kind;};
static struct LifecycleSurface *lifecycle_alias_one,*lifecycle_alias_two;
static u32 lifecycle_queries;
static struct LifecycleSurface* lifecycle_state(void* object){struct LifecycleSurface* s=object;return s->state?s->state:s;}
static u16 lifecycle_pixels[6]={0xf800,0x7e0,0xeeee,0x001f,0xffff,0xeeee};
static u32 lifecycle_locks,lifecycle_unlocks,lifecycle_kind;
static i32 WIN lifecycle_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct LifecycleSurface* s=lifecycle_state(object);
    if(!s->live || s->held || GetLastError()!=0x77 || event)ExitProcess(90);
    ++lifecycle_locks;SetLastError(0x88);
    if(lifecycle_mode[0]=='f')return (i32)0x887601ae;
    if((lifecycle_mode[0]=='p')!=(rect!=0) || flags!=(lifecycle_mode[0]=='r'?0x11u:1u))ExitProcess(91);
    s->held=1;d[1]=1;d[2]=2;d[3]=2;d[4]=lifecycle_mode[0]=='n'?(u32)-6:6;
    d[9]=(u32)(lifecycle_pixels+(lifecycle_mode[0]=='n'?3:0));d[19]=0x40;
    d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=lifecycle_mode[0]=='o'?0x40:0x200;
    if(lifecycle_mode[0]=='i'){d[19]=0x60;d[21]=8;d[22]=d[23]=d[24]=0;}
    if(lifecycle_mode[0]=='b')d[23]=0xf800;
    return 13;
}
static i32 WIN lifecycle_unlock(void* object,void* argument){
    struct LifecycleSurface* s=lifecycle_state(object);
    if(!s->held || GetLastError()!=0x77)ExitProcess(92);
    ++lifecycle_unlocks;SetLastError(0x88);
    if(lifecycle_mode[0]=='u' && lifecycle_unlocks==1)return (i32)0x887601ae;
    u32 kind=((struct LifecycleSurface*)object)->kind;
    void* expected=(kind?kind:lifecycle_kind)>=14?0:(lifecycle_mode[0]=='n'?lifecycle_pixels+3:lifecycle_pixels);
    if(argument!=expected)ExitProcess(93);
    s->held=0;for(u32 i=0;i<6;++i)lifecycle_pixels[i]=0;
    return 19;
}
static u32 WIN lifecycle_release(void* object){struct LifecycleSurface* s=lifecycle_state(object);s->live=0;SetLastError(0x88);return 0;}
static i32 WIN lifecycle_query(void* object,const u8* guid,void** result){
    (void)object;if(GetLastError()!=0x77)ExitProcess(100);
    ++lifecycle_queries;*result=guid[0]==0x81?lifecycle_alias_one:lifecycle_alias_two;
    SetLastError(0x88);return lifecycle_mode[0]=='q'?(i32)0x80004002:23;
}
static void test_lock_lifecycle(void){
    u32 alias_mode=lifecycle_mode[0]=='a'||lifecycle_mode[0]=='t'||lifecycle_mode[0]=='q'||lifecycle_mode[0]=='x'||lifecycle_mode[0]=='z';
    static void* table[33];if(alias_mode)table[0]=(void*)&lifecycle_query;table[25]=(void*)&lifecycle_lock;table[32]=(void*)&lifecycle_unlock;table[2]=(void*)&lifecycle_release;
    struct LifecycleSurface surface={table,1,0,0,0};lifecycle_kind=(alias_mode||lifecycle_mode[0]=='n')?12:14;
    RenderInstallForTest(&surface,lifecycle_kind);
    static void* alias_table[33];static void* modern_table[33];
    struct LifecycleSurface alias_one={alias_table,1,0,&surface,11},alias_two={modern_table,1,0,&surface,14};
    lifecycle_alias_one=&alias_one;lifecycle_alias_two=&alias_two;
    if(alias_mode){
        surface.kind=12;
        alias_table[0]=modern_table[0]=(void*)&lifecycle_query;
        alias_table[25]=modern_table[25]=(void*)&lifecycle_lock;
        alias_table[32]=modern_table[32]=(void*)&lifecycle_unlock;
        alias_table[2]=modern_table[2]=(void*)&lifecycle_release;
        /* Tables are installed before use, like interfaces returned by QI. */
        RenderInstallForTest(&alias_one,11);RenderInstallForTest(&alias_two,14);
        if(lifecycle_mode[0]!='x'){
            static const u8 surface1[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
            static const u8 surface4[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};
            typedef i32 (WIN *LifecycleQuery)(void*,const u8*,void**);void* result=0;
            SetLastError(0x77);i32 status=((LifecycleQuery)table[0])(&surface,surface1,&result);
            if(status!=(lifecycle_mode[0]=='q'?(i32)0x80004002:23)||GetLastError()!=0x88||result!=&alias_one)ExitProcess(101);
            if(lifecycle_mode[0]=='t'){
                SetLastError(0x77);if(((LifecycleQuery)alias_table[0])(&alias_one,surface4,&result)!=23||GetLastError()!=0x88||result!=&alias_two)ExitProcess(102);
            }
        }
    }
    typedef i32 (WIN *LifecycleLock)(void*,void*,u32*,u32,HANDLE);
    typedef i32 (WIN *LifecycleUnlock)(void*,void*);
    u32 d[31]={0};d[0]=lifecycle_kind>=14?124:108;u32 rect[4]={0,0,2,2};
    SetLastError(0x77);i32 result=((LifecycleLock)table[25])(&surface,lifecycle_mode[0]=='p'?rect:0,d,lifecycle_mode[0]=='r'?0x11:1,0);
    if(GetLastError()!=0x88 || lifecycle_locks!=1 || result!=(lifecycle_mode[0]=='f'?(i32)0x887601ae:13))ExitProcess(94);
    if(result<0)ExitProcess(0);
    struct LifecycleSurface* unlock_surface=alias_mode?(lifecycle_mode[0]=='t'?&alias_two:&alias_one):&surface;
    SetLastError(0x77);result=((LifecycleUnlock)unlock_surface->table[32])(unlock_surface,(alias_mode?unlock_surface->kind:lifecycle_kind)>=14?0:(void*)d[9]);
    if(lifecycle_mode[0]=='u'){
        if(result!=(i32)0x887601ae || GetLastError()!=0x88 || !surface.held)ExitProcess(95);
        SetLastError(0x77);result=((LifecycleUnlock)table[32])(&surface,0);
    }
    if(result!=19 || GetLastError()!=0x88 || surface.held || lifecycle_unlocks!=(lifecycle_mode[0]=='u'?2u:1u))ExitProcess(96);
    if(lifecycle_queries!=(alias_mode?(lifecycle_mode[0]=='x'?0u:lifecycle_mode[0]=='t'?2u:1u):0u))ExitProcess(103);
    if(lifecycle_mode[0]=='z'){
        SetLastError(0x77);if(((u32 (WIN *)(void*))alias_table[2])(&alias_one)!=0||GetLastError()!=0x88)ExitProcess(104);
        surface.live=1;d[0]=108;
        SetLastError(0x77);if(((LifecycleLock)table[25])(&surface,0,d,1,0)!=13||GetLastError()!=0x88)ExitProcess(105);
        SetLastError(0x77);if(((LifecycleUnlock)alias_table[32])(&alias_one,(void*)d[9])!=19||GetLastError()!=0x88)ExitProcess(106);
        if(lifecycle_locks!=2||lifecycle_unlocks!=2)ExitProcess(107);
    }else if(lifecycle_locks!=1)ExitProcess(97); /* No observer Lock is allowed. */
    SetLastError(0x77);if(((u32 (WIN *)(void*))table[2])(&surface)!=0 || GetLastError()!=0x88)ExitProcess(98);
    ExitProcess(0);
}
