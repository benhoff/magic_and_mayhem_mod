#include "../../runtime/shadow/win32_min.h"
__declspec(dllimport) void* WIN LoadLibraryA(const char*);
__declspec(dllimport) void* WIN GetProcAddress(void*,const char*);
__declspec(dllimport) int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
__declspec(dllimport) int WIN VirtualFree(void*,u32,u32);
__declspec(dllimport) int WIN WriteFile(HANDLE,const void*,u32,u32*,void*);
typedef i32 (WIN *Query)(const char*,void*);
typedef u32 (WIN *Install)(u32);
struct Row {char filename[512];u32 bytes,success;};
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;for(u32 i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static void fill(void* a,u8 v,u32 n){u8* p=a;for(u32 i=0;i<n;++i)p[i]=v;}
static int guards(Install install){
    u8* image=VirtualAlloc(0,2u*1024u*1024u,0x3000,0x40);if(!image)return 0;
    const u8 call[5]={0xe8,0x5c,0xf5,0x02,0};const u8 consume[4]={0x8b,0x54,0x24,0x50};
    u8* site=image+0x16ed0f;u8* read=image+0x16ed1b;
    for(u32 i=0;i<5;++i)site[i]=call[i];for(u32 i=0;i<4;++i)read[i]=consume[i];
    for(u32 field=0;field<2;++field){u8* at=field?read:site;at[0]^=1;
        if(install((u32)image))return 0;
        if(field && !equal(site,call,5))return 0;
        at[0]^=1;
    }
    if(!install((u32)image) || site[0]!=0xe8 || equal(site,call,5) || !equal(read,consume,4) || install((u32)image))return 0;
    /* Exercise the patched call with the original cdecl caller cleanup. */
    u8* entry=image+0x1000;const u8 arguments[8]={0xff,0x74,0x24,0x08,0xff,0x74,0x24,0x08};
    for(u32 i=0;i<8;++i)entry[i]=arguments[i];entry[8]=0xe9;
    const i32 relative=(i32)(site-(entry+13));for(u32 i=0;i<4;++i)entry[9+i]=(u8)((u32)relative>>(i*8));
    site[5]=0x83;site[6]=0xc4;site[7]=0x08;site[8]=0xc3;
    FlushInstructionCache(GetCurrentProcess(),image,2u*1024u*1024u);
    u8 output[80],expected[80];fill(output,0xa5,sizeof(output));fill(expected,0xa5,sizeof(expected));
    for(u32 i=0;i<4;++i)expected[36+i]=0;
    typedef i32 (*CdeclQuery)(const char*,void*);SetLastError(0x13572468);
    if(((CdeclQuery)entry)("Dummy.wav",output+16) || GetLastError()!=0x13572468 || !equal(output,expected,sizeof(output)))return 0;
    return VirtualFree(image,0,0x8000)!=0;
}
void start(void){
    char path[512];if(!GetEnvironmentVariableA("MNM_FILE_SIZE_DLL",path,sizeof(path)))ExitProcess(2);
    void* module=LoadLibraryA(path);if(!module)ExitProcess(3);
    Query query=(Query)GetProcAddress(module,"AudioFileSizeForTest");
    Install install=(Install)GetProcAddress(module,"AudioFileSizesInstallForTest");
    if(!query || !install || !guards(install))ExitProcess(4);
    HANDLE input=CreateFileA("queries.bin",0x80000000,1,0,3,0,0);u32 count=0,read=0;
    if(input==(HANDLE)-1 || !ReadFile(input,&count,4,&read,0) || read!=4 || !count || count>4096)ExitProcess(5);
    for(u32 i=0;i<count;++i){
        struct Row row;if(!ReadFile(input,&row,sizeof(row),&read,0) || read!=sizeof(row))ExitProcess(6);
        u8 output[80],expected[80];fill(output,0xa5,sizeof(output));fill(expected,0xa5,sizeof(expected));
        if(row.success)for(u32 j=0;j<4;++j)expected[16+20+j]=(u8)(row.bytes>>(8*j));
        SetLastError(0x24681357);const i32 result=query(row.filename,output+16);
        if(result!=(row.success?0:-1) || GetLastError()!=0x24681357 || !equal(output,expected,sizeof(output)))ExitProcess(7);
    }
    CloseHandle(input);
    u8 output[80];fill(output,0xa5,sizeof(output));SetLastError(0x12345678);
    if(query(0,output+16)!=-1 || query("a.wav",0)!=-1 || GetLastError()!=0x12345678)ExitProcess(8);
    const u32 success=1;HANDLE file=CreateFileA("producer-success.bin",0x40000000,0,0,1,0,0);u32 wrote;
    if(file==(HANDLE)-1 || !WriteFile(file,&success,4,&wrote,0) || wrote!=4)ExitProcess(9);
    CloseHandle(file);ExitProcess(0);
}
