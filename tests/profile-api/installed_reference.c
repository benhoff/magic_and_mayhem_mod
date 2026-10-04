/* PE32 console reference, reads staged installed bytes; no game imports. */
#include "installed_wire.h"
typedef unsigned int U32;
#define API __declspec(dllimport)
#define CALL __stdcall
API U32 CALL GetPrivateProfileStringA(const char*,const char*,const char*,char*,U32,const char*);
API U32 CALL GetPrivateProfileSectionA(const char*,char*,U32,const char*);
API U32 CALL GetPrivateProfileIntA(const char*,const char*,int,const char*);
API U32 CALL GetFullPathNameA(const char*,U32,char*,char**);
API void* CALL CreateFileA(const char*,U32,U32,void*,U32,U32,void*);
API int CALL ReadFile(void*,void*,U32,U32*,void*);
API int CALL WriteFile(void*,const void*,U32,U32*,void*);
API int CALL CloseHandle(void*);
API void CALL ExitProcess(U32);
static struct {struct ProfileRecord result;U32 guard;} record;
static struct ProfileQuery query;
static char path[1024];
static int terminated(const char* text){U32 i;for(i=0;i<128;++i)if(!text[i])return 1;return 0;}
void start(void){
    U32 header[3],n,i,j;void *input,*output;
    input=CreateFileA("queries.bin",0x80000000,1,0,3,0x80,0);
    output=CreateFileA("wine-results.bin",0x40000000,0,0,2,0x80,0);
    if(input==(void*)-1 || output==(void*)-1 || !GetFullPathNameA("fixture.ini",1024,path,0))ExitProcess(2);
    if(!ReadFile(input,header,12,&n,0) || n!=12 || header[0]!=PROFILE_MAGIC || header[1]!=PROFILE_VERSION || header[2]>PROFILE_QUERY_LIMIT)ExitProcess(3);
    for(i=0;i<header[2];++i){
        if(!ReadFile(input,&query,sizeof(query),&n,0) || n!=sizeof(query) || query.op<1 || query.op>3 || !query.capacity || query.capacity>PROFILE_CAPACITY || !terminated(query.section) || !terminated(query.key))ExitProcess(4);
        record.result.returned=0;for(j=0;j<PROFILE_CAPACITY;++j)record.result.bytes[j]=0;record.guard=0x12345678;
        if(query.op==1)record.result.returned=GetPrivateProfileStringA(query.section,query.key,"",record.result.bytes,query.capacity,path);
        if(query.op==2)record.result.returned=GetPrivateProfileSectionA(query.section,record.result.bytes,query.capacity,path);
        if(query.op==3)record.result.returned=GetPrivateProfileIntA(query.section,query.key,(int)query.fallback,path);
        if(record.guard!=0x12345678 || !WriteFile(output,&record.result,sizeof(record.result),&n,0) || n!=sizeof(record.result))ExitProcess(5);
    }
    CloseHandle(input);CloseHandle(output);ExitProcess(0);
}
