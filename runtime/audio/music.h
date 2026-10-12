#include "../../protocols/include/mnm/music_v1.h"
/* Selected CD compatibility. Process callbacks/objects stay in this PE. */
API void* WIN GetProcAddress(void*,const char*);
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
typedef u32 (WIN *MusicMci)(u32,u32,u32,u32*);
typedef int (WIN *MusicPost)(void*,u32,u32,u32);
typedef i32 (__thiscall *MusicVolume)(void*,i32);
static u32* music_words;static volatile u32 music_busy,music_dead;
static u32 music_next,music_opened,music_notify_id,music_notify_window,music_event_seen;
static MusicMci music_original_mci;static MusicPost music_post;static MusicVolume music_original_volume;
#define MUSIC_DEVICE 0x7321u
static int music_readable(const void* pointer,u32 bytes,int write){
    u32 start=(u32)pointer;if(!start || bytes>0xffffffffu-start)return 0;
    while(bytes){u32 info[7];if(!VirtualQuery((void*)start,info,sizeof(info)) || info[4]!=0x1000 || (info[5]&0x101))return 0;
        if(write && !(info[5]&0xcc))return 0;
        u32 end=info[0]+info[3];if(end<=start)return 0;u32 count=end-start;if(count>bytes)count=bytes;start+=count;bytes-=count;}
    return 1;
}
static int music_type(const char* value){
    static const char name[]="cdaudio";
    for(u32 i=0;i<sizeof(name);++i){if(!music_readable(value+i,1,0))return 0;u8 c=value[i];if(c>='A' && c<='Z')c+=32;if(c!=(u8)name[i])return 0;}
    return 1;
}
static u32 music_unsupported(void){if(music_words)__atomic_add_fetch(music_words+MNM_MUSIC_V1_UNSUPPORTED_CD_CALLS_OFFSET/4,1,__ATOMIC_RELAXED);return 274;}
static void music_notify(void){
    if(!music_words || !music_notify_id)return;
    u32 sequence=__atomic_load_n(music_words+MNM_MUSIC_V1_EVENT_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);if((sequence&1) || sequence==music_event_seen)return;
    u32 id=music_words[MNM_MUSIC_V1_EVENT_ID_OFFSET/4],kind=music_words[MNM_MUSIC_V1_EVENT_KIND_OFFSET/4];
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(sequence!=__atomic_load_n(music_words+MNM_MUSIC_V1_EVENT_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE))return;
    music_event_seen=sequence;
    if(id==music_notify_id && kind){u32 window=music_notify_window;music_notify_id=music_notify_window=0;
        if(window && music_post)music_post((void*)window,0x3b9,kind==MNM_MUSIC_V1_EVENT_COMPLETE?1u:kind==MNM_MUSIC_V1_EVENT_CANCELLED?4u:8u,MUSIC_DEVICE);}
}
static u32 WIN music_notify_worker(void* unused){
    (void)unused;
    while(music_words && __atomic_load_n(music_words+MNM_MUSIC_V1_READY_OFFSET/4,__ATOMIC_ACQUIRE)){
        if(!__atomic_exchange_n(&music_busy,1,__ATOMIC_ACQUIRE)){music_notify();__atomic_store_n(&music_busy,0,__ATOMIC_RELEASE);}
        Sleep(MNM_MUSIC_V1_HEARTBEAT_MS);
    }return 0;
}
static u32 music_request(u32 operation,u32 track,u32 from,u32 flags,u32* request){
    if(!music_words || music_dead || !__atomic_load_n(music_words+MNM_MUSIC_V1_READY_OFFSET/4,__ATOMIC_ACQUIRE))return 276;
    u32 id=++music_next;if(!id){music_dead=1;return 276;}
    u32 sequence=__atomic_load_n(music_words+MNM_MUSIC_V1_REQUEST_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
    __atomic_store_n(music_words+MNM_MUSIC_V1_REQUEST_SEQUENCE_OFFSET/4,sequence+1,__ATOMIC_SEQ_CST);
    music_words[MNM_MUSIC_V1_REQUEST_ID_OFFSET/4]=id;music_words[MNM_MUSIC_V1_OPERATION_OFFSET/4]=operation;
    music_words[MNM_MUSIC_V1_TRACK_OFFSET/4]=track;music_words[MNM_MUSIC_V1_FROM_FRAME_OFFSET/4]=from;music_words[MNM_MUSIC_V1_TO_FRAME_OFFSET/4]=0;music_words[MNM_MUSIC_V1_FLAGS_OFFSET/4]=flags;
    __atomic_store_n(music_words+MNM_MUSIC_V1_REQUEST_SEQUENCE_OFFSET/4,sequence+2,__ATOMIC_RELEASE);
    u32 start=GetTickCount();
    for(;;){u32 response=__atomic_load_n(music_words+MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);
        if(!(response&1) && music_words[MNM_MUSIC_V1_RESPONSE_ID_OFFSET/4]==id){u32 status=music_words[MNM_MUSIC_V1_RESPONSE_STATUS_OFFSET/4];
            __atomic_thread_fence(__ATOMIC_ACQUIRE);if(response==__atomic_load_n(music_words+MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE)){
                if(!status && request)*request=id;return status?status==MNM_MUSIC_V1_STATUS_INVALID?282u:276u:0u;}}
        if(!__atomic_load_n(music_words+MNM_MUSIC_V1_READY_OFFSET/4,__ATOMIC_ACQUIRE) || GetTickCount()-start>MNM_MUSIC_V1_REQUEST_TIMEOUT_MS){
            __atomic_store_n(music_words+MNM_MUSIC_V1_CANCELLED_REQUEST_ID_OFFSET/4,id,__ATOMIC_RELEASE);music_dead=1;return 276;}
        Sleep(1);
    }
}
static u32 music_msf(u32 frames){return ((frames/4500)&255u)|(((frames/75)%60)<<8)|((frames%75)<<16);}
static u32 music_status(u32 flags,u32* parameters){
    if((flags&~0x312u) || !(flags&0x100) || !music_readable(parameters,16,1))return music_unsupported();
    u32 phase,position,current,sequence,started=GetTickCount();
    for(;;){if(GetTickCount()-started>MNM_MUSIC_V1_REQUEST_TIMEOUT_MS || !music_words[MNM_MUSIC_V1_READY_OFFSET/4])return 276;
        sequence=__atomic_load_n(music_words+MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);if(sequence&1){Sleep(1);continue;}
        phase=music_words[MNM_MUSIC_V1_PHASE_OFFSET/4];position=music_words[MNM_MUSIC_V1_POSITION_CD_FRAMES_OFFSET/4];current=music_words[MNM_MUSIC_V1_CURRENT_TRACK_OFFSET/4];
        __atomic_thread_fence(__ATOMIC_ACQUIRE);if(sequence==__atomic_load_n(music_words+MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE))break;}
    u32 item=parameters[2],track=(flags&0x10)?parameters[3]:0,count=music_words[MNM_MUSIC_V1_TRACK_COUNT_OFFSET/4],result=0;
    if(track>count)return 282;
    u32 start=track?music_words[MNM_MUSIC_V1_TOC_OFFSET/4+(track-1)*2]:0;
    u32 end=track?music_words[MNM_MUSIC_V1_TOC_OFFSET/4+(track-1)*2+1]:music_words[MNM_MUSIC_V1_LEADOUT_SECTOR_OFFSET/4];
    switch(item){
    case 1:result=music_msf(end-start);break;
    case 2:result=(current?current:1)|(music_msf(position)<<8);break;
    case 3:result=count;break;
    case 4:result=phase==MNM_MUSIC_V1_PHASE_PLAYING||phase==MNM_MUSIC_V1_PHASE_LOADING?0x20e:phase==MNM_MUSIC_V1_PHASE_PAUSED?0x211:phase==MNM_MUSIC_V1_PHASE_UNAVAILABLE||phase==MNM_MUSIC_V1_PHASE_FAILED?0x20c:0x20d;break;
    case 5:case 7:result=1;break;case 6:result=10;break;
    case 8:result=current;break;
    case 0x4001:if(!track)return 282;result=track==1?1089:1088;break;
    default:return music_unsupported();
    }
    parameters[1]=result;return 0;
}
static u32 WIN music_mci(u32 device,u32 message,u32 flags,u32* parameters){
    u32 error=GetLastError(),result=0;
    if(message==0x803 && music_readable(parameters,20,0) && (flags&0x2000) && !(flags&0x1000) && music_type((const char*)parameters[2])){
        if(!music_words)return music_original_mci?music_original_mci(device,message,flags,parameters):276;
        if(device || (flags&~0x2302u) || !music_readable(parameters+1,4,1)){u32 invalid=music_unsupported();SetLastError(error);return invalid;}
    }else if(device!=MUSIC_DEVICE){
        if(music_words)__atomic_add_fetch(music_words+MNM_MUSIC_V1_LEGACY_OTHER_MCI_CALLS_OFFSET/4,1,__ATOMIC_RELAXED);
        SetLastError(error);return music_original_mci?music_original_mci(device,message,flags,parameters):257;
    }
    if(__atomic_exchange_n(&music_busy,1,__ATOMIC_ACQUIRE)){SetLastError(error);return 276;}
    music_notify();
    if(message!=0x803 && !music_opened){result=257;goto done;}
    if(message==0x803){result=music_request(MNM_MUSIC_V1_OPERATION_OPEN,0,0,0,0);if(!result){music_opened=1;parameters[1]=MUSIC_DEVICE;__atomic_or_fetch(music_words+MNM_MUSIC_V1_PRODUCER_FEATURES_OFFSET/4,MNM_MUSIC_V1_FEATURE_NATIVE_SELECTED,__ATOMIC_RELEASE);}goto done;}
    if(message==0x814){result=music_status(flags,parameters);goto done;}
    if(message==0x80d){result=(flags==0x402 && music_readable(parameters,12,0) && parameters[1]==10)?0:music_unsupported();goto done;}
    if(message==0x806){
        if((flags&~0xfu) || !(flags&4) || !music_readable(parameters,flags&8?12:8,0)){result=music_unsupported();goto done;}
        u32 track=parameters[1]&255u;if(parameters[1]!=track || track<2 || track>music_words[MNM_MUSIC_V1_TRACK_COUNT_OFFSET/4] || ((flags&8)&&parameters[2]!=track+1)){result=282;goto done;}
        u32 request=0;result=music_request(MNM_MUSIC_V1_OPERATION_PLAY,track,0,0,&request);
        if(!result){if(music_notify_id && music_notify_window && music_post)music_post((void*)music_notify_window,0x3b9,2,MUSIC_DEVICE);
            music_notify_id=flags&1?request:0;music_notify_window=flags&1?parameters[0]:0;}
        goto done;
    }
    if(message==0x804 || message==0x808 || message==0x809 || message==0x855){
        if(flags!=2){result=music_unsupported();goto done;}
        result=music_request(message==0x804?MNM_MUSIC_V1_OPERATION_CLOSE:message==0x808?MNM_MUSIC_V1_OPERATION_STOP:message==0x809?MNM_MUSIC_V1_OPERATION_PAUSE:MNM_MUSIC_V1_OPERATION_RESUME,0,0,0,0);
        if(!result && (message==0x804 || message==0x808)){if(music_notify_id && music_notify_window && music_post)music_post((void*)music_notify_window,0x3b9,4,MUSIC_DEVICE);music_notify_id=music_notify_window=0;if(message==0x804)music_opened=0;}
        goto done;
    }
    result=music_unsupported();
done:
    __atomic_store_n(&music_busy,0,__ATOMIC_RELEASE);SetLastError(error);return result;
}
static u32 WIN music_zero_mixers(void){return 0;}
static i32 __fastcall music_volume(void* object,void* unused,i32 level){
    (void)unused;i32 result=music_original_volume(object,level);if(result || !music_words)return result;
    if(!music_readable((u8*)object+0x32c,4,0) || !*(u32*)((u8*)object+0x32c))return result;
    u32 volume=level<=0?0u:level>=15?65535u:(u32)level*4369u;
    if(__atomic_exchange_n(&music_busy,1,__ATOMIC_ACQUIRE))return result;
    u32 status=music_request(MNM_MUSIC_V1_OPERATION_VOLUME,0,volume,0,0);__atomic_store_n(&music_busy,0,__ATOMIC_RELEASE);return status?-6:result;
}
static void music_init(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_MUSIC_CHANNEL",path,sizeof(path));if(!length || length>=sizeof(path))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return;
    if(GetFileSize(file,0)!=MNM_MUSIC_V1_SIZE){CloseHandle(file);return;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_MUSIC_V1_SIZE,0);CloseHandle(file);if(!mapping)return;
    u32* words=MapViewOfFile(mapping,2,0,0,MNM_MUSIC_V1_SIZE);CloseHandle(mapping);
    if(!words || !same(words,MNM_MUSIC_V1_MAGIC,8) || words[2]!=MNM_MUSIC_V1_VERSION || words[3]!=MNM_MUSIC_V1_SIZE)return;
    music_words=words;
    void* user=GetModuleHandleA("user32.dll");music_post=user?(MusicPost)GetProcAddress(user,"PostMessageA"):0;
}
static int music_install(u32 base){
    if(!music_words)return 1;
    static const u8 mixer[6]={0xff,0x15,0xf8,0x52,0x5c,0},volume[6]={0x53,0x8b,0xd9,0x55,0x33,0xed};
    u8* mixer_call=(u8*)(base+0x8157b);u8* entry=(u8*)(base+0x82190);void** iat=(void**)(base+0x1c52dc);
    void* winmm=GetModuleHandleA("winmm.dll");music_original_mci=winmm?(MusicMci)GetProcAddress(winmm,"mciSendCommandA"):0;
    if(!music_original_mci || !music_readable(iat,4,0) || *iat!=(void*)music_original_mci || !music_readable(mixer_call,6,0) || !same(mixer_call,mixer,6) || !music_readable(entry,6,0) || !same(entry,volume,6))return 0;
    u8* trampoline=VirtualAlloc(0,11,0x3000,4);if(!trampoline)return 0;copy(trampoline,entry,6);trampoline[6]=0xe9;u32 back=(u32)(entry+6)-(u32)(trampoline+11);copy(trampoline+7,&back,4);
    u32 old_iat,old_mixer,old_volume,ignored;
    if(!VirtualProtect(trampoline,11,0x20,&ignored) || !FlushInstructionCache(GetCurrentProcess(),trampoline,11))return 0;
    if(!VirtualProtect(iat,4,4,&old_iat))return 0;
    if(!VirtualProtect(mixer_call,6,0x40,&old_mixer)){VirtualProtect(iat,4,old_iat,&ignored);return 0;}
    if(!VirtualProtect(entry,6,0x40,&old_volume)){VirtualProtect(iat,4,old_iat,&ignored);VirtualProtect(mixer_call,6,old_mixer,&ignored);return 0;}
    music_original_volume=(MusicVolume)trampoline;
    u32 offset=(u32)&music_zero_mixers-(u32)(mixer_call+5);mixer_call[0]=0xe8;copy(mixer_call+1,&offset,4);mixer_call[5]=0x90;
    offset=(u32)&music_volume-(u32)(entry+5);entry[0]=0xe9;copy(entry+1,&offset,4);entry[5]=0x90;
    __atomic_store_n(iat,(void*)&music_mci,__ATOMIC_RELEASE);
    FlushInstructionCache(GetCurrentProcess(),entry,6);FlushInstructionCache(GetCurrentProcess(),mixer_call,6);
    VirtualProtect(iat,4,old_iat,&ignored);VirtualProtect(entry,6,old_volume,&ignored);VirtualProtect(mixer_call,6,old_mixer,&ignored);
    __atomic_or_fetch(music_words+MNM_MUSIC_V1_PRODUCER_FEATURES_OFFSET/4,MNM_MUSIC_V1_FEATURE_MCI|MNM_MUSIC_V1_FEATURE_ZERO_MIXERS|MNM_MUSIC_V1_FEATURE_MUSIC_VOLUME,__ATOMIC_RELEASE);
    HANDLE worker=CreateThread(0,0,(void*)&music_notify_worker,0,0,0);if(worker)CloseHandle(worker);else return 0;
    return 1;
}
#ifdef MNM_AUDIO_SELFTEST
static u32 WIN music_test_legacy(u32 device,u32 message,u32 flags,u32* parameters){(void)device;(void)message;(void)flags;(void)parameters;SetLastError(0x88);return 23;}
static volatile u32 music_test_notifications,music_test_kind,music_test_device;
static int WIN music_test_post(void* window,u32 message,u32 kind,u32 device){
    if(window!=(void*)0x11223344 || message!=0x3b9)return 0;
    music_test_kind=kind;music_test_device=device;__atomic_add_fetch(&music_test_notifications,1,__ATOMIC_RELEASE);return 1;
}
__declspec(dllexport) u32 WIN AudioMusicForTest(u32 device,u32 message,u32 flags,u32* parameters){
    music_original_mci=&music_test_legacy;music_post=&music_test_post;return music_mci(device,message,flags,parameters);
}
__declspec(dllexport) u32 WIN AudioMusicNotifyForTest(u32* kind,u32* device){
    music_notify();if(kind)*kind=music_test_kind;if(device)*device=music_test_device;return __atomic_load_n(&music_test_notifications,__ATOMIC_ACQUIRE);
}
__declspec(dllexport) u32 WIN AudioMusicInstallForTest(u32 base){return music_install(base);}
#endif
