/* Original Unlock poisons pixels: snapshots must already own their bytes. */
static char lifecycle_mode[32];
struct LifecycleSurface {void** table;u32 live,held;};
static u16 lifecycle_pixels[6]={0xf800,0x7e0,0xeeee,0x001f,0xffff,0xeeee};
static u32 lifecycle_locks,lifecycle_unlocks,lifecycle_kind;
static i32 WIN lifecycle_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct LifecycleSurface* s=object;
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
    struct LifecycleSurface* s=object;
    if(!s->held || GetLastError()!=0x77)ExitProcess(92);
    ++lifecycle_unlocks;SetLastError(0x88);
    if(lifecycle_mode[0]=='u' && lifecycle_unlocks==1)return (i32)0x887601ae;
    void* expected=lifecycle_kind>=14?0:lifecycle_pixels+3;
    if(argument!=expected)ExitProcess(93);
    s->held=0;for(u32 i=0;i<6;++i)lifecycle_pixels[i]=0;
    return 19;
}
static u32 WIN lifecycle_release(void* object){struct LifecycleSurface* s=object;s->live=0;SetLastError(0x88);return 0;}
static void test_lock_lifecycle(void){
    static void* table[33];table[25]=(void*)&lifecycle_lock;table[32]=(void*)&lifecycle_unlock;table[2]=(void*)&lifecycle_release;
    struct LifecycleSurface surface={table,1,0};lifecycle_kind=lifecycle_mode[0]=='n'?12:14;
    RenderInstallForTest(&surface,lifecycle_kind);
    typedef i32 (WIN *LifecycleLock)(void*,void*,u32*,u32,HANDLE);
    typedef i32 (WIN *LifecycleUnlock)(void*,void*);
    u32 d[31]={0};d[0]=lifecycle_kind>=14?124:108;u32 rect[4]={0,0,2,2};
    SetLastError(0x77);i32 result=((LifecycleLock)table[25])(&surface,lifecycle_mode[0]=='p'?rect:0,d,lifecycle_mode[0]=='r'?0x11:1,0);
    if(GetLastError()!=0x88 || lifecycle_locks!=1 || result!=(lifecycle_mode[0]=='f'?(i32)0x887601ae:13))ExitProcess(94);
    if(result<0)ExitProcess(0);
    SetLastError(0x77);result=((LifecycleUnlock)table[32])(&surface,lifecycle_kind>=14?0:(void*)d[9]);
    if(lifecycle_mode[0]=='u'){
        if(result!=(i32)0x887601ae || GetLastError()!=0x88 || !surface.held)ExitProcess(95);
        SetLastError(0x77);result=((LifecycleUnlock)table[32])(&surface,0);
    }
    if(result!=19 || GetLastError()!=0x88 || surface.held || lifecycle_unlocks!=(lifecycle_mode[0]=='u'?2u:1u))ExitProcess(96);
    if(lifecycle_locks!=1)ExitProcess(97); /* No observer Lock is allowed. */
    SetLastError(0x77);if(((u32 (WIN *)(void*))table[2])(&surface)!=0 || GetLastError()!=0x88)ExitProcess(98);
    ExitProcess(0);
}
