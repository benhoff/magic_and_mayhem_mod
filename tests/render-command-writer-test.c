#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef uint8_t u8;typedef int32_t i32;typedef void* HANDLE;
#define API
#define WIN
static u32 storage[16+67108864/4];
static u32 env=1,unmaps,mapped_size=sizeof(storage),last_error,fail_alloc;
static HANDLE GetProcessHeap(void){return storage;}
static void* HeapAlloc(HANDLE h,u32 flags,u32 size){(void)h;(void)flags;return fail_alloc?0:malloc(size);}
static int HeapFree(HANDLE h,u32 flags,void* memory){(void)h;(void)flags;free(memory);return 1;}
static u32 GetLastError(void){return last_error;}
static void SetLastError(u32 value){last_error=value;}
static u32 GetEnvironmentVariableA(const char* key,char* path,u32 n){(void)key;(void)n;if(!env)return 0;memcpy(path,"test",5);return 4;}
static HANDLE CreateFileA(const char* p,u32 a,u32 b,void* c,u32 d,u32 e,void* f){(void)p;(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return storage;}
static u32 GetFileSize(HANDLE h,u32* high){(void)h;(void)high;return mapped_size;}
static i32 CloseHandle(HANDLE h){(void)h;return 1;}
static HANDLE CreateFileMappingA(HANDLE h,void* p,u32 a,u32 b,u32 c,const char* d){(void)h;(void)p;(void)a;(void)b;(void)c;(void)d;return storage;}
static void* MapViewOfFile(HANDLE h,u32 a,u32 b,u32 c,u32 d){(void)h;(void)a;(void)b;(void)c;(void)d;return storage;}
i32 UnmapViewOfFile(const void* p){(void)p;++unmaps;return 1;}
static void copy(void* to,const void* from,u32 n){memcpy(to,from,n);}
static int same(const void* a,const void* b,u32 n){return !memcmp(a,b,n);}
#include "../runtime/render/command_channel.h"
static void check(int ok){if(!ok)exit(1);}
static void reset(void){memset(storage,0,64);memcpy(storage,"MNMRDC01",8);storage[2]=1;storage[3]=sizeof(storage);storage[4]=123;command_channel=0;command_channel_bytes=0;command_channel_session=0;}
int main(void){
    reset();command_channel_init();check(command_channel==storage && storage[6]==1);
    u32 fields[2]={1,2};check(command_channel_record(1,11,fields,8,0,0));
    check(storage[5]==20 && storage[16]==11 && storage[17]==1 && storage[18]==8 && storage[19]==1 && storage[20]==2);
    command_channel_end();check(storage[6]==2);check(!command_channel_record(2,8,0,0,0,0));command_channel_close();check(storage[6]==2 && !command_channel);
    reset();command_channel_init();command_channel_bytes=MNM_RENDER_COMMANDS_V1_CAPACITY-4;storage[5]=command_channel_bytes;
    check(!command_channel_append(fields,4,fields,4,0,0));check(storage[5]==MNM_RENDER_COMMANDS_V1_CAPACITY-4 && storage[6]==3 && storage[7]==1);
    reset();command_channel_init();storage[8]=1;check(!command_channel_record(1,8,0,0,0,0));check(storage[5]==0 && storage[7]==3);
    reset();command_channel_init();command_channel_close();check(storage[6]==3 && storage[7]==4);
    reset();storage[9]=1;command_channel_init();check(!command_channel && storage[6]==0);
    reset();storage[4]=0;command_channel_init();check(!command_channel);
    reset();storage[6]=2;command_channel_init();check(!command_channel && storage[6]==2);
    reset();storage[5]=1;command_channel_init();check(!command_channel);
    reset();storage[6]=1;command_channel_init();check(!command_channel && storage[6]==1);
    reset();command_channel_init();storage[4]=999;check(!command_channel_record(1,8,0,0,0,0));check(storage[5]==0);command_channel_close();
    reset();env=0;command_channel_init();check(command_channel_append(0,0,0,0,0,0));check(unmaps>=6);
    command_channel_pump();
    puts("{\"success\":true,\"cases\":11}");return 0;
}
