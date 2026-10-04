#include "../../protocols/include/mnm/input_v1.h"
/* Optional Qt-owned client coordinates and Windows virtual-key polling state. */
API void* WIN GetProcAddress(void*,const char*);
typedef short (WIN *InputKey)(i32);
typedef i32 (WIN *InputCursor)(i32*);
typedef i32 (WIN *InputClient)(void*,i32*);
static u32* input_words;static void* input_window;
static InputKey input_original_async,input_original_key;
static InputCursor input_original_cursor;static InputClient input_client;
static u32 input_pressed[MNM_INPUT_V1_KEY_COUNT],input_sequence,input_seen;
static int input_supported_key(i32 code){
    return code==1 || code==2 || code==4 || code==8 || code==9 || code==13 || (code>=16 && code<=19) || code==27 ||
        (code>=32 && code<=40) || code==45 || code==46 || (code>=48 && code<=57) || (code>=65 && code<=90) ||
        (code>=96 && code<=105) || (code>=112 && code<=135) || (code>=0xba && code<=0xc0) || (code>=0xdb && code<=0xde);
}
static int input_snapshot(i32 code,u32* key,i32* point){
    if(!input_words)return 0;
    u32 sequence=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);if(sequence&1)return 0;
    u32 active=__atomic_load_n(input_words+MNM_INPUT_V1_ACTIVE_OFFSET/4,__ATOMIC_RELAXED),x=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_X_OFFSET/4,__ATOMIC_RELAXED),y=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_Y_OFFSET/4,__ATOMIC_RELAXED);
    u32 width=__atomic_load_n(input_words+MNM_INPUT_V1_WIDTH_OFFSET/4,__ATOMIC_RELAXED),height=__atomic_load_n(input_words+MNM_INPUT_V1_HEIGHT_OFFSET/4,__ATOMIC_RELAXED);
    u32 value=code>=0 && (u32)code<MNM_INPUT_V1_KEY_COUNT?__atomic_load_n(input_words+MNM_INPUT_V1_KEYS_OFFSET/4+code,__ATOMIC_RELAXED):0;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    if(sequence!=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE))return 0;
    u32 now=GetTickCount();if(sequence!=__atomic_exchange_n(&input_sequence,sequence,__ATOMIC_RELAXED))__atomic_store_n(&input_seen,now,__ATOMIC_RELAXED);
    if(!active || now-__atomic_load_n(&input_seen,__ATOMIC_RELAXED)>MNM_INPUT_V1_LEASE_MS || !width || !height || width>MNM_INPUT_V1_MAX_WIDTH || height>MNM_INPUT_V1_MAX_HEIGHT || x>=width || y>=height)return 0;
    *key=value;point[0]=(i32)x;point[1]=(i32)y;return 1;
}
static short WIN input_async(i32 code){
    u32 error=GetLastError(),value;i32 point[2];
    if(!input_supported_key(code) || !input_snapshot(code,&value,point)){SetLastError(error);return input_original_async(code);}
    u32 generation=value&MNM_INPUT_V1_KEY_GENERATION_MASK,previous=__atomic_exchange_n(input_pressed+code,generation,__ATOMIC_RELAXED);
    SetLastError(error);return (short)((value&MNM_INPUT_V1_KEY_DOWN_MASK?0x8000:0)|(generation!=previous));
}
static short WIN input_key(i32 code){
    u32 error=GetLastError(),value;i32 point[2];
    if(!input_supported_key(code) || !input_snapshot(code,&value,point)){SetLastError(error);return input_original_key(code);}
    SetLastError(error);return (short)(value&MNM_INPUT_V1_KEY_DOWN_MASK?0x8000:0);
}
static i32 WIN input_cursor(i32* output){
    u32 error=GetLastError(),value;i32 point[2];
    if(!input_window || !readable(output,8) || !input_snapshot(-1,&value,point) || !input_client(input_window,point)){
        SetLastError(error);return input_original_cursor(output);
    }
    copy(output,point,8);SetLastError(error);return 1;
}
static void input_init(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_RENDER_INPUT",path,sizeof(path));if(!length || length>=sizeof(path))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return;
    if(GetFileSize(file,0)!=MNM_INPUT_V1_SIZE){CloseHandle(file);return;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_INPUT_V1_SIZE,0);CloseHandle(file);if(!mapping)return;
    u32* words=MapViewOfFile(mapping,2,0,0,MNM_INPUT_V1_SIZE);CloseHandle(mapping);
    if(!words || !same(words,MNM_INPUT_V1_MAGIC,MNM_INPUT_V1_MAGIC_SIZE) || words[MNM_INPUT_V1_VERSION_OFFSET/4]!=MNM_INPUT_V1_VERSION || words[MNM_INPUT_V1_DECLARED_SIZE_OFFSET/4]!=MNM_INPUT_V1_SIZE)return;
    input_words=words;
}
static int input_patch(void** iat){
    u32 protection,ignored;
    if(!input_original_async || !input_original_key || !input_original_cursor || !input_client || !readable(iat,172) ||
       iat[3]!=(void*)input_original_async || iat[6]!=(void*)input_original_cursor || iat[38]!=(void*)input_original_key ||
       !VirtualProtect(iat,172,4,&protection))return 0;
    __atomic_store_n(iat+3,(void*)&input_async,__ATOMIC_RELEASE);__atomic_store_n(iat+6,(void*)&input_cursor,__ATOMIC_RELEASE);
    __atomic_store_n(iat+38,(void*)&input_key,__ATOMIC_RELEASE);VirtualProtect(iat,172,protection,&ignored);return 1;
}
#ifndef MNM_RENDER_SELFTEST
static void input_install(u32 base){
    if(!input_words)return;
    void* user=GetModuleHandleA("user32.dll");if(!user){input_words=0;return;}
    input_original_async=(InputKey)GetProcAddress(user,"GetAsyncKeyState");input_original_key=(InputKey)GetProcAddress(user,"GetKeyState");
    input_original_cursor=(InputCursor)GetProcAddress(user,"GetCursorPos");input_client=(InputClient)GetProcAddress(user,"ClientToScreen");
    if(!input_patch((void**)(base+0x1c51f8)))input_words=0;
}
#endif
#ifdef MNM_RENDER_SELFTEST
static short WIN input_test_key(i32 code){if(GetLastError()!=0x77 || code<0)ExitProcess(290);SetLastError(0x88);return 0x1234;}
static i32 WIN input_test_cursor(i32* point){if(GetLastError()!=0x77)ExitProcess(291);if(point){point[0]=10;point[1]=20;}SetLastError(0x88);return 7;}
static i32 WIN input_test_client(void* window,i32* point){if(window!=(void*)0x1234)ExitProcess(292);point[0]+=100;point[1]+=200;SetLastError(0x99);return 1;}
__declspec(dllexport) i32 WIN RenderInputForTest(u32 action,i32 code,i32* point){
    if(action==3){input_original_async=input_original_key=&input_test_key;input_original_cursor=&input_test_cursor;input_client=&input_test_client;input_window=0;return input_words!=0;}
    if(action==4){input_sequence=input_words[MNM_INPUT_V1_SEQUENCE_OFFSET/4];input_seen=GetTickCount()-(MNM_INPUT_V1_LEASE_MS+1);return 0;}
    if(action==5){input_sequence=0;return 0;}
    if(action==6){static void* iat[43];iat[0]=(void*)0x5678;iat[3]=(void*)input_original_async;iat[6]=0;iat[38]=(void*)input_original_key;
        if(input_patch(iat) || iat[3]!=(void*)input_original_async || iat[38]!=(void*)input_original_key)return 0;
        iat[6]=(void*)input_original_cursor;
        return input_patch(iat) && iat[0]==(void*)0x5678 && iat[3]==(void*)&input_async && iat[6]==(void*)&input_cursor && iat[38]==(void*)&input_key;
    }
    return action==0?input_async(code):action==1?input_key(code):input_cursor(point);
}
#endif
