#include "../../runtime/shadow/win32_min.h"
#include "../profile-api/installed_wire.h"
API HANDLE WIN CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API int WIN WriteFile(HANDLE,const void*,u32,u32*,void*);
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void WIN ExitProcess(u32);
typedef u32 (WIN *Query)(u32,const char*,const char*,const char*,char*,u32,const char*);
typedef u32 (WIN *Install)(u32);
static char dllpath[512];static struct ProfileQuery query;static struct ProfileRecord record;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void read(HANDLE f,void* out,u32 bytes){u32 done;require(ReadFile(f,out,bytes,&done,0)&&done==bytes,20);}
static void write(HANDLE f,const void* input,u32 bytes){u32 done;require(WriteFile(f,input,bytes,&done,0)&&done==bytes,21);}
void start(void){
    u32 n=GetEnvironmentVariableA("MNM_PROFILE_DLL",dllpath,sizeof(dllpath));require(n && n<sizeof(dllpath),1);
    void* dll=LoadLibraryA(dllpath);require(dll!=0,2);Query fn=(Query)GetProcAddress(dll,"AudioProfileForTest");Install install=(Install)GetProcAddress(dll,"AudioProfileInstallForTest");require(fn && install,3);
    HANDLE input=CreateFileA("queries.bin",0x80000000,1,0,3,0x80,0);require(input!=(HANDLE)-1,4);
    u32 header[3];read(input,header,sizeof(header));require(header[0]==PROFILE_MAGIC && header[1]==PROFILE_VERSION && header[2]<=PROFILE_QUERY_LIMIT,5);
    HANDLE output=CreateFileA("transport-results.bin",0x40000000,0,0,2,0x80,0);require(output!=(HANDLE)-1,6);
    for(u32 i=0;i<header[2];++i){
        read(input,&query,sizeof(query));for(u32 j=0;j<sizeof(record);++j)((u8*)&record)[j]=0;
        SetLastError(0xabc123);record.returned=fn(query.op+22,query.section,query.key,"",record.bytes,query.op==3?query.fallback:query.capacity,"Sounds\\Sounds.ini");
        require(GetLastError()==0xabc123,7);write(output,&record,sizeof(record));
    }
    CloseHandle(input);CloseHandle(output);
    static const u32 sites[]={0x16e03a,0x16e20b,0x16ea77,0x16e41f,0x16ebd9,0x16f6cb,0x16e589};
    static const u8 expected[][6]={{0xff,0x15,0xf0,0x50,0x5c,0},{0xff,0x15,0xf0,0x50,0x5c,0},{0xff,0x15,0xf0,0x50,0x5c,0},
        {0x8b,0x1d,0x7c,0x50,0x5c,0},{0xff,0x15,0x7c,0x50,0x5c,0},{0xff,0x15,0x7c,0x50,0x5c,0},{0xff,0x15,0xec,0x50,0x5c,0}};
    u8* image=VirtualAlloc(0,0x200000,0x3000,4);require(image!=0,8);
    for(u32 failed=0;failed<7;++failed){
        for(u32 i=0;i<7;++i)for(u32 j=0;j<6;++j)image[sites[i]+j]=expected[i][j];
        image[sites[failed]]^=1;require(!install((u32)image),9);
        for(u32 i=0;i<7;++i)for(u32 j=0;j<6;++j)require(image[sites[i]+j]==(u8)(expected[i][j]^((i==failed&&!j)?1:0)),10);
    }
    for(u32 i=0;i<7;++i)for(u32 j=0;j<6;++j)image[sites[i]+j]=expected[i][j];
    require(install((u32)image),11);
    for(u32 i=0;i<7;++i)require(image[sites[i]]==(i==3?0xbb:0xe8)&&image[sites[i]+5]==0x90,12);
    require(!install((u32)image),13);
    HANDLE result=CreateFileA("producer-success.bin",0x40000000,0,0,2,0x80,0);require(result!=(HANDLE)-1,14);u32 success=1;write(result,&success,4);CloseHandle(result);ExitProcess(0);
}
