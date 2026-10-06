/* Native lifecycle policy; resolved PE32 main-image imports only. No worker
 * creation or join from DllMain. Original ExitProcess code/error pass through. */
__declspec(dllexport) u32 WIN RenderShutdown(u32);
typedef void (WIN *CommandExit)(u32);
static CommandExit command_original_exit;
static u32 command_exit_installed;
static void WIN command_exit(u32 code){
    u32 error=GetLastError();
    u32 complete=RenderShutdown(3000);
    lock_diagnostic("command_lifecycle_shutdown",0,0,complete,code,0,0,0);
    SetLastError(error);command_original_exit(code);
}
static int command_image_range(u8* base,u32 size,u32 rva,u32 bytes){
    return rva && rva<=size && bytes<=size-rva && readable(base+rva,bytes);
}
/* Identify exactly one resolved ExitProcess import, without build-specific
 * offsets or trusting unbound import names. Validate all traversed bounds
 * before changing one pointer. Malformed/duplicate/unresolved layouts refuse. */
static int command_exit_install(u8* base){
    if(command_exit_installed)return 1;
    if(!readable(base,64) || *(u16*)base!=0x5a4d)return 0;
    u32 nt=*(u32*)(base+60);
    if((u32)base>0xffffffffu-4352 || nt>4096 || !readable(base+nt,248) || *(u32*)(base+nt)!=0x4550 ||
       *(u16*)(base+nt+4)!=0x14c || *(u16*)(base+nt+20)<224 || *(u16*)(base+nt+24)!=0x10b)return 0;
    u32 size=*(u32*)(base+nt+80),rva=*(u32*)(base+nt+128),bytes=*(u32*)(base+nt+132);
    if(size<nt+248 || size>64*1024*1024 || (u32)base>0xffffffffu-size ||
       *(u32*)(base+nt+116)<2 || bytes<20 || bytes>20*1024 || !command_image_range(base,size,rva,bytes))return 0;
    CommandExit expected=(CommandExit)GetProcAddress(GetModuleHandleA("kernel32.dll"),"ExitProcess");
    if(!expected)return 0;
    void** slot=0;u32 traversed=0,terminated=0;
    for(u32 at=0;at+20<=bytes;at+=20){
        u32* desc=(u32*)(base+rva+at);
        if(!(desc[0]|desc[1]|desc[2]|desc[3]|desc[4])){terminated=1;break;}
        u32 thunk=desc[4],ended=0;
        for(u32 i=0;i<4096;++i){
            if(++traversed>4096 || thunk>0xffffffffu-i*4 || !command_image_range(base,size,thunk+i*4,4))return 0;
            void** candidate=(void**)(base+thunk+i*4);
            if(!*candidate){ended=1;break;}
            if(*candidate==(void*)expected){if(slot)return 0;slot=candidate;}
        }
        if(!ended)return 0;
    }
    if(!terminated || !slot)return 0;
    u32 protection,ignored;
    if(!VirtualProtect(slot,4,4,&protection))return 0;
    command_original_exit=expected;
    int installed=__sync_bool_compare_and_swap(slot,(void*)expected,(void*)&command_exit);
    VirtualProtect(slot,4,protection,&ignored);
    if(installed)command_exit_installed=1;
    return installed;
}
static int command_auto_shutdown(void){
    char value[8];u32 n=GetEnvironmentVariableA("MNM_RENDER_AUTO_SHUTDOWN",value,sizeof(value));
    return game_session_continuous && !(n==1 && value[0]=='0');
}
__declspec(dllexport) u32 WIN RenderStartup(void){
    u32 error=GetLastError();
    struct CommandLifecycleLease lease __attribute__((cleanup(command_lifecycle_leave)))=command_lifecycle_enter();
    if(!lease.held || !game_tracker_acquire()){SetLastError(error);return 0;}
    int ready=stream && !command_channel_refused &&
        (!game_session_continuous || (game_session_enabled && lock_capture_path_length &&
         (!command_auto_shutdown() || command_exit_installed)));
    game_tracker_release();
    if(ready)ready=command_scheduler_start_locked();
    if(!command_control_start())ready=0;
    SetLastError(error);return ready;
}
#ifdef MNM_RENDER_SELFTEST
__declspec(dllexport) u32 WIN RenderExitInstallForTest(void* image){
    u32 error=GetLastError();int installed=command_exit_install(image);SetLastError(error);return installed;
}
#endif
