/* Freestanding PE32: real Wine kernel32 profile calls; no game imports. */
typedef unsigned int U32;
#define API __declspec(dllimport)
#define CALL __stdcall
API U32 CALL GetPrivateProfileStringA(const char*,const char*,const char*,char*,U32,const char*);
API U32 CALL GetPrivateProfileSectionA(const char*,char*,U32,const char*);
API U32 CALL GetPrivateProfileIntA(const char*,const char*,int,const char*);
API U32 CALL GetFullPathNameA(const char*,U32,char*,char**);
API void* CALL CreateFileA(const char*,U32,U32,void*,U32,U32,void*);
API int CALL WriteFile(void*,const void*,U32,U32*,void*);
API int CALL CloseHandle(void*);
API void CALL ExitProcess(U32);
struct Query {U32 op;const char *section,*key;U32 capacity;const char* fallback;U32 number;};
#define Q(id,op,section,key,cap,fallback,number) {op,section,key,cap,fallback,number},
static const struct Query queries[]={
#include "queries.inc"
};
#undef Q
void start(void){
    char path[1024];U32 i,j,written;
    void* output=CreateFileA("wine-results.bin",0x40000000,0,0,2,0x80,0);
    if(output==(void*)-1 || !GetFullPathNameA("fixture.ini",1024,path,0))ExitProcess(2);
    for(i=0;i<sizeof(queries)/sizeof(queries[0]);++i){
        const struct Query* q=&queries[i];
        struct {U32 returned;char bytes[128];U32 guard;} record;
        record.returned=0;for(j=0;j<128;++j)record.bytes[j]=0;record.guard=0x12345678;
        if(q->op==1)record.returned=GetPrivateProfileStringA(q->section,q->key,q->fallback,record.bytes,q->capacity,path);
        if(q->op==2)record.returned=GetPrivateProfileSectionA(q->section,record.bytes,q->capacity,path);
        if(q->op==3)record.returned=GetPrivateProfileIntA(q->section,q->key,(int)q->number,path);
        if(record.guard!=0x12345678 || !WriteFile(output,&record,132,&written,0) || written!=132)ExitProcess(3);
    }
    CloseHandle(output);ExitProcess(0);
}
