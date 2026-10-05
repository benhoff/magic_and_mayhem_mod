/* Test-only forwarding probe. Original leave/rebuild bodies remain untouched. */
struct MenuDisplayRect {i32 left,top,right,bottom;};
API int WIN GetClientRect(HANDLE,struct MenuDisplayRect*);
static void THIS preferences_display_leave(void* object){
    u32 error=GetLastError();u8* p=object;
    int observe=p==(u8*)0x6a4948&&readable(p,0x70)&&get(p)==0x5c648c;
    if(observe)record(6,p,p[0x67],get((void*)0x6de6d5));
    SetLastError(error);
    ((void (THIS *)(void*))0x4a8b60)(object);
    error=GetLastError();
    if(observe){
        struct MenuDisplayRect rect={0,0,0,0};
        if(GetClientRect((HANDLE)get((void*)0x656614),&rect))
            record(7,p,(u32)(rect.right-rect.left),(u32)(rect.bottom-rect.top));
        record(8,p,p[0x67],get((void*)0x6b0630)|(get((void*)0x6568b0)<<16));
    }
    SetLastError(error);
}
static int install_preferences_display(void){
    char flag[4];if(GetEnvironmentVariableA("MNM_MENU_DISPLAY_OBSERVE",flag,sizeof(flag))!=1||flag[0]!='1')return 1;
    const u8 bytes[]={0x53,0x56,0x8b,0xf1,0x8b,0x06,0xff,0x50};u32 old,restore;
    if(!readable((void*)0x5c6490,4)||get((void*)0x5c6490)!=0x4a8b60||
       !readable((void*)0x4a8b60,8)||!equal((void*)0x4a8b60,bytes,8)||
       !readable((void*)0x656614,4)||!readable((void*)0x6de6d5,4)||
       !readable((void*)0x6b0630,4)||!readable((void*)0x6568b0,4)||
       !VirtualProtect((void*)0x5c6490,4,0x40,&old))return 0;
    put((void*)0x5c6490,(u32)&preferences_display_leave);
    VirtualProtect((void*)0x5c6490,4,old,&restore);return 1;
}
