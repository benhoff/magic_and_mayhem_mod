#include "../../protocols/include/mnm/media_v1.h"
/* Opt-in Qt media broker. No DirectSound buffer interception in this chunk. */
API void WIN Sleep(u32);
typedef i32 (__fastcall *MediaMovie)(const char*,void*,void*,i32,i32,i32);
typedef i32 (WIN *MediaSound)(const char*,void*,u32);
static u32* media_words;
static MediaMovie media_original_movie;
static MediaSound media_original_sound;
static volatile u32 media_busy;
static u32 media_next;static volatile u32 media_native_sound,media_legacy_sound;
static void media_init(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_RENDER_MEDIA",path,sizeof(path));
    if(!length || length>=sizeof(path))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return;
    if(GetFileSize(file,0)!=MNM_MEDIA_V1_SIZE){CloseHandle(file);return;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_MEDIA_V1_SIZE,0);CloseHandle(file);if(!mapping)return;
    u32* words=MapViewOfFile(mapping,2,0,0,MNM_MEDIA_V1_SIZE);CloseHandle(mapping);
    if(!words || !same(words,MNM_MEDIA_V1_MAGIC,MNM_MEDIA_V1_MAGIC_SIZE) || words[MNM_MEDIA_V1_VERSION_OFFSET/4]!=MNM_MEDIA_V1_VERSION || words[MNM_MEDIA_V1_DECLARED_SIZE_OFFSET/4]!=MNM_MEDIA_V1_SIZE)return;
    media_words=words;
}
/* -1 means safe legacy fallback; -2 means accepted playback then failed. */
static i32 media_request(u32 operation,const char* path,u32 flags){
    if(!media_words || __atomic_exchange_n(&media_busy,1,__ATOMIC_ACQUIRE))return -1;
    u32 length=0;
    if(path){
        while(length<MNM_MEDIA_V1_PATH_SIZE){if(!readable(path+length,1))goto fallback;
            u8 c=(u8)path[length];if(!c)break;if(c<32 || c>126)goto fallback;++length;}
        if(!length || length==MNM_MEDIA_V1_PATH_SIZE)goto fallback;
    }
    if(operation!=MNM_MEDIA_V1_OPERATION_STOP_FILE_SOUND && !path)goto fallback;
    u32 id=++media_next;if(!id)id=++media_next;
    u32 seq=__atomic_load_n(media_words+MNM_MEDIA_V1_REQUEST_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
    __atomic_store_n(media_words+MNM_MEDIA_V1_REQUEST_SEQUENCE_OFFSET/4,seq+1,__ATOMIC_SEQ_CST);
    media_words[MNM_MEDIA_V1_REQUEST_ID_OFFSET/4]=id;media_words[MNM_MEDIA_V1_OPERATION_OFFSET/4]=operation;media_words[MNM_MEDIA_V1_FLAGS_OFFSET/4]=flags;media_words[MNM_MEDIA_V1_PATH_LENGTH_OFFSET/4]=length;
    if(length)copy((u8*)media_words+MNM_MEDIA_V1_PATH_OFFSET,path,length);
    __atomic_store_n(media_words+MNM_MEDIA_V1_REQUEST_SEQUENCE_OFFSET/4,seq+2,__ATOMIC_RELEASE);
    u32 start=GetTickCount(),seen=start,heartbeat=__atomic_load_n(media_words+MNM_MEDIA_V1_HEARTBEAT_OFFSET/4,__ATOMIC_ACQUIRE),accepted=0;
    for(;;){
        u32 now=GetTickCount(),beat=__atomic_load_n(media_words+MNM_MEDIA_V1_HEARTBEAT_OFFSET/4,__ATOMIC_ACQUIRE);
        if(beat!=heartbeat){heartbeat=beat;seen=now;}
        u32 response=__atomic_load_n(media_words+MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);
        if(!(response&1)){
            u32 response_id=__atomic_load_n(media_words+MNM_MEDIA_V1_RESPONSE_ID_OFFSET/4,__ATOMIC_RELAXED),status=__atomic_load_n(media_words+MNM_MEDIA_V1_RESPONSE_STATUS_OFFSET/4,__ATOMIC_RELAXED),accepted_id=__atomic_load_n(media_words+MNM_MEDIA_V1_ACCEPTED_ID_OFFSET/4,__ATOMIC_RELAXED);
            __atomic_thread_fence(__ATOMIC_ACQUIRE);
            if(response==__atomic_load_n(media_words+MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE) && response_id==id){
                if(accepted_id==id)accepted=1;
                if(status==MNM_MEDIA_V1_STATUS_ACCEPTED){accepted=1;if(operation==MNM_MEDIA_V1_OPERATION_FILE_SOUND && (flags&MNM_MEDIA_V1_SOUND_FLAG_ASYNC)){__atomic_store_n(&media_busy,0,__ATOMIC_RELEASE);return 1;}}
                if(status==MNM_MEDIA_V1_STATUS_COMPLETE || status==MNM_MEDIA_V1_STATUS_CANCELLED || status==MNM_MEDIA_V1_STATUS_SOUND_BUSY){__atomic_store_n(&media_busy,0,__ATOMIC_RELEASE);return status==MNM_MEDIA_V1_STATUS_COMPLETE?1:0;}
                if(status==MNM_MEDIA_V1_STATUS_DECODER_ERROR || status==MNM_MEDIA_V1_STATUS_UNSUPPORTED){__atomic_store_n(&media_busy,0,__ATOMIC_RELEASE);return accepted?-2:-1;}
            }
        }
        if(now-seen>MNM_MEDIA_V1_STALE_HOST_MS || (!accepted && now-start>MNM_MEDIA_V1_ACCEPTANCE_TIMEOUT_MS) || now-start>MNM_MEDIA_V1_PLAYBACK_TIMEOUT_MS){
            __atomic_store_n(media_words+MNM_MEDIA_V1_CANCELLED_REQUEST_ID_OFFSET/4,id,__ATOMIC_RELEASE);
            __atomic_store_n(&media_busy,0,__ATOMIC_RELEASE);return accepted?-2:-1;
        }
        Sleep(MNM_MEDIA_V1_POLL_MS);
    }
fallback:
    __atomic_store_n(&media_busy,0,__ATOMIC_RELEASE);return -1;
}
static i32 __fastcall media_movie(const char* path,void* surface,void* window,i32 x,i32 y,i32 control){
    u32 error=GetLastError();
    i32 result=(u8)control==0?media_request(MNM_MEDIA_V1_OPERATION_MOVIE,path,0):-1;
    SetLastError(error);
    if(result==-1)return media_original_movie(path,surface,window,x,y,control);
    return result>0;
}
static i32 WIN media_sound(const char* path,void* module,u32 flags){
    u32 error=GetLastError();i32 result=-1;
    if(!path && !flags)result=media_request(MNM_MEDIA_V1_OPERATION_STOP_FILE_SOUND,0,0);
    else if(!module && (flags&MNM_MEDIA_V1_SOUND_FLAG_FILENAME) && !(flags&~MNM_MEDIA_V1_SOUND_ALLOWED_FLAGS) && (!(flags&MNM_MEDIA_V1_SOUND_FLAG_LOOP) || (flags&MNM_MEDIA_V1_SOUND_FLAG_ASYNC)) &&
            !((flags&MNM_MEDIA_V1_SOUND_FLAG_NO_STOP) && __atomic_load_n(&media_legacy_sound,__ATOMIC_RELAXED))){
        if(__atomic_exchange_n(&media_legacy_sound,0,__ATOMIC_RELAXED)){SetLastError(error);media_original_sound(0,0,0);}
        result=media_request(MNM_MEDIA_V1_OPERATION_FILE_SOUND,path,flags);
    }
    if(result==-1){
        // Unsupported WinMM calls must replace the Qt file-sound channel too.
        if(media_words && __atomic_exchange_n(&media_native_sound,0,__ATOMIC_RELAXED))__atomic_add_fetch(media_words+MNM_MEDIA_V1_SOUND_CANCEL_GENERATION_OFFSET/4,1,__ATOMIC_RELEASE);
        SetLastError(error);i32 legacy=media_original_sound(path,module,flags);
        if(legacy)__atomic_store_n(&media_legacy_sound,path && (flags&MNM_MEDIA_V1_SOUND_FLAG_ASYNC),__ATOMIC_RELAXED);
        return legacy;
    }
    if(result>0){
        SetLastError(error);media_original_sound(0,0,0); // Retire any legacy async file sound.
        __atomic_store_n(&media_legacy_sound,0,__ATOMIC_RELAXED);
        __atomic_store_n(&media_native_sound,path && (flags&MNM_MEDIA_V1_SOUND_FLAG_ASYNC),__ATOMIC_RELAXED);
    }
    SetLastError(error);return result>0;
}
/* Whole instructions; no relative operands in the pinned five-byte prologue. */
static int media_patch_movie(u8* entry){
    static const u8 expected[5]={0x51,0x8d,0x44,0x24,0x00};
    if(!readable(entry,5) || !same(entry,expected,5))return 0;
    u8* trampoline=VirtualAlloc(0,10,0x3000,4);if(!trampoline)return 0;
    copy(trampoline,entry,5);trampoline[5]=0xe9;u32 back=(u32)(entry+5)-(u32)(trampoline+10);copy(trampoline+6,&back,4);
    u32 protection,ignored;
    if(!VirtualProtect(trampoline,10,0x20,&ignored) || !FlushInstructionCache(GetCurrentProcess(),trampoline,10))return 0;
    if(!VirtualProtect(entry,5,0x40,&protection))return 0;
    media_original_movie=(MediaMovie)trampoline;
    u32 offset=(u32)&media_movie-(u32)(entry+5);entry[0]=0xe9;copy(entry+1,&offset,4);
    FlushInstructionCache(GetCurrentProcess(),entry,5);VirtualProtect(entry,5,protection,&ignored);return 1;
}
#ifndef MNM_RENDER_SELFTEST
static void media_install(u32 base){
    if(!media_words)return;
    void* winmm=GetModuleHandleA("winmm.dll");
    media_original_sound=winmm?(MediaSound)GetProcAddress(winmm,"PlaySoundA"):0;
    void** iat=(void**)(base+0x1c52d8);u32 protection,ignored;
    if(!media_original_sound || !readable(iat,4) || *iat!=(void*)media_original_sound ||
       !readable((void*)(base+0x69a00),5) || !VirtualProtect(iat,4,4,&protection))return;
    if(media_patch_movie((u8*)(base+0x69a00)))__atomic_store_n(iat,(void*)&media_sound,__ATOMIC_RELEASE);
    VirtualProtect(iat,4,protection,&ignored);
}
#else
static i32 __fastcall media_test_movie(const char* path,void* surface,void* window,i32 x,i32 y,i32 control){
    (void)path;if(surface!=(void*)0x1234 || window!=(void*)0x5678 || x!=11 || y!=22 || control!=0 || GetLastError()!=0x77)ExitProcess(310);
    SetLastError(0x88);return 17;
}
static i32 WIN media_test_sound(const char* path,void* module,u32 flags){
    (void)module;(void)flags;if(GetLastError()!=0x77)ExitProcess(311);if(!path)return 1;SetLastError(0x88);return 23;
}
__declspec(dllexport) i32 WIN RenderMediaForTest(u32 operation,const char* path,u32 flags){
    media_original_movie=&media_test_movie;media_original_sound=&media_test_sound;
    if(operation==4){
        static const u8 fixture[14]={0x51,0x8d,0x44,0x24,0,0x59,0xb8,23,0,0,0,0xc2,16,0};
        u8* code=VirtualAlloc(0,sizeof(fixture),0x3000,0x40);if(!code)return 0;copy(code,fixture,sizeof(fixture));
        code[0]=0x90;if(media_patch_movie(code) || code[0]!=0x90)return 0;code[0]=0x51;
        if(!media_patch_movie(code))return 0;
        return ((MediaMovie)code)(0,0,0,0,0,0)==23;
    }
    return operation==MNM_MEDIA_V1_OPERATION_MOVIE?media_movie(path,(void*)0x1234,(void*)0x5678,11,22,0):media_sound(path,0,flags);
}
#endif
