#include "../../protocols/include/mnm/input_v1.h"
#include "../../protocols/include/mnm/input_v2.h"
/* Optional Qt-owned client coordinates and Windows virtual-key polling state. */
API void* WIN GetProcAddress(void*,const char*);
typedef short (WIN *InputKey)(i32);
typedef i32 (WIN *InputCursor)(i32*);
typedef i32 (WIN *InputClient)(void*,i32*);
static u32* input_words;static void* input_window;
static int input_desktop_coordinates;
static InputKey input_original_async,input_original_key;
static InputCursor input_original_cursor;static InputClient input_client;
static u32 input_pressed[MNM_INPUT_V1_KEY_COUNT],input_sequence,input_seen;
static int input_supported_key(i32 code){
    return code==1 || code==2 || code==4 || code==8 || code==9 || code==13 || (code>=16 && code<=19) || code==27 ||
        (code>=32 && code<=40) || code==45 || code==46 || (code>=48 && code<=57) || (code>=65 && code<=90) ||
        (code>=96 && code<=105) || (code>=112 && code<=135) || (code>=0xba && code<=0xc0) || (code>=0xdb && code<=0xde);
}
/* Opt-in transition-only acknowledgement of actual original Right-key polls.
 * No callback result, LastError or input ownership is changed. The complete
 * private process lifetime supplies the terminal boundary for this tiny journal. */
static char input_acceptance_path[512];
static u32 input_acceptance_data[16+8*12],input_acceptance_count,input_acceptance_down;
static volatile i32 input_acceptance_busy;
static void input_acceptance(i32 code,u32 value,u32 sequence,u32 owned,u32 caller){
    if(!input_acceptance_path[0] || code!=39 || __sync_lock_test_and_set(&input_acceptance_busy,1))return;
    u32 down=(value&0x8000)!=0;
    if(input_acceptance_count<8 && (input_acceptance_count?down!=input_acceptance_down:down)){
        u32* row=input_acceptance_data+16+input_acceptance_count*12;
        row[0]=++input_acceptance_count;row[1]=sequence;row[2]=(u32)code;row[3]=value&65535;row[4]=owned;
        row[5]=caller;row[6]=GetCurrentThreadId();row[7]=GetTickCount();input_acceptance_down=down;
        copy(input_acceptance_data,"MNMIAK01",8);input_acceptance_data[2]=1;input_acceptance_data[3]=48;input_acceptance_data[4]=input_acceptance_count;input_acceptance_data[5]=8;
        HANDLE file=CreateFileA(input_acceptance_path,0x40000000,3,0,2,0x80,0);u32 written=0;
        if(file!=(HANDLE)-1){WriteFile(file,input_acceptance_data,64+input_acceptance_count*48,&written,0);CloseHandle(file);}
    }
    __sync_lock_release(&input_acceptance_busy);
}
static int input_snapshot(i32 code,u32* key,i32* point,u32* accepted){
    if(!input_words)return 0;
    u32 sequence=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE);if(sequence&1)return 0;
    u32 active=__atomic_load_n(input_words+MNM_INPUT_V1_ACTIVE_OFFSET/4,__ATOMIC_RELAXED),x=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_X_OFFSET/4,__ATOMIC_RELAXED),y=__atomic_load_n(input_words+MNM_INPUT_V1_CURSOR_Y_OFFSET/4,__ATOMIC_RELAXED);
    u32 width=__atomic_load_n(input_words+MNM_INPUT_V1_WIDTH_OFFSET/4,__ATOMIC_RELAXED),height=__atomic_load_n(input_words+MNM_INPUT_V1_HEIGHT_OFFSET/4,__ATOMIC_RELAXED);
    u32 value=code>=0 && (u32)code<MNM_INPUT_V1_KEY_COUNT?__atomic_load_n(input_words+MNM_INPUT_V1_KEYS_OFFSET/4+code,__ATOMIC_RELAXED):0;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    if(sequence!=__atomic_load_n(input_words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE))return 0;
    u32 now=GetTickCount();if(sequence!=__atomic_exchange_n(&input_sequence,sequence,__ATOMIC_RELAXED))__atomic_store_n(&input_seen,now,__ATOMIC_RELAXED);
    if(!active || now-__atomic_load_n(&input_seen,__ATOMIC_RELAXED)>MNM_INPUT_V1_LEASE_MS || !width || !height || width>MNM_INPUT_V1_MAX_WIDTH || height>MNM_INPUT_V1_MAX_HEIGHT || x>=width || y>=height)return 0;
    *key=value;point[0]=(i32)x;point[1]=(i32)y;*accepted=sequence;return 1;
}
static short WIN input_async(i32 code){
    u32 error=GetLastError(),value,sequence;i32 point[2];
    if(!input_supported_key(code) || !input_snapshot(code,&value,point,&sequence)){SetLastError(error);short result=input_original_async(code);error=GetLastError();input_acceptance(code,(u32)(unsigned short)result,0,0,(u32)__builtin_return_address(0));SetLastError(error);return result;}
    u32 generation=value&MNM_INPUT_V1_KEY_GENERATION_MASK,previous=__atomic_exchange_n(input_pressed+code,generation,__ATOMIC_RELAXED);
    short result=(short)((value&MNM_INPUT_V1_KEY_DOWN_MASK?0x8000:0)|(generation!=previous));input_acceptance(code,(u32)(unsigned short)result,sequence,1,(u32)__builtin_return_address(0));SetLastError(error);return result;
}
static short WIN input_key(i32 code){
    u32 error=GetLastError(),value,sequence;i32 point[2];
    if(!input_supported_key(code) || !input_snapshot(code,&value,point,&sequence)){SetLastError(error);short result=input_original_key(code);error=GetLastError();input_acceptance(code,(u32)(unsigned short)result,0,0,(u32)__builtin_return_address(0));SetLastError(error);return result;}
    short result=(short)(value&MNM_INPUT_V1_KEY_DOWN_MASK?0x8000:0);input_acceptance(code,(u32)(unsigned short)result,sequence,1,(u32)__builtin_return_address(0));SetLastError(error);return result;
}
static i32 WIN input_cursor(i32* output){
    u32 error=GetLastError(),value,sequence;i32 point[2];
    if(!input_window || !readable(output,8) || !input_snapshot(-1,&value,point,&sequence) || (!input_desktop_coordinates && !input_client(input_window,point))){
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
    if(!words || words[MNM_INPUT_V1_DECLARED_SIZE_OFFSET/4]!=MNM_INPUT_V1_SIZE)return;
    int legacy=same(words,MNM_INPUT_V1_MAGIC,8) && words[MNM_INPUT_V1_VERSION_OFFSET/4]==MNM_INPUT_V1_VERSION;
    int desktop=same(words,MNM_INPUT_V2_MAGIC,8) && words[MNM_INPUT_V2_VERSION_OFFSET/4]==MNM_INPUT_V2_VERSION && words[MNM_INPUT_V2_COORDINATE_SPACE_OFFSET/4]==MNM_INPUT_V2_COORDINATE_SPACE_DESKTOP;
    if(!legacy && !desktop)return;
    if(desktop)for(u32 i=MNM_INPUT_V2_RESERVED_OFFSET/4;i<MNM_INPUT_V2_HEADER_SIZE/4;++i)if(words[i])return;
    input_desktop_coordinates=desktop;
    input_words=words;
    char flag[4];if(length+12<sizeof(input_acceptance_path) && GetEnvironmentVariableA("MNM_ACCEPTANCE_PILOT",flag,4)==1 && flag[0]=='1'){
        copy(input_acceptance_path,path,length);copy(input_acceptance_path+length,".acceptance",12);
    }
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
