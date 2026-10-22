/* Admission of the actual supplied 1998 runtime; no installer is executed and
 * no Wine builtin fallback is allowed by the runner. Failure is evidence. */
#define start unused_restore_probe_entry
#include "surface-driver-restore-reference.c"
#undef start
API u32 WIN GetModuleFileNameA(HANDLE,char*,u32);
/* Read-only first-chance observation. Do not handle the exception or change
 * page protections. PE32 CONTEXT has EIP/CS/ESP at184/188/196 respectively. */
typedef struct {u32 code,flags;void* nested;void* address;u32 count;u32 info[15];} LegacyException;
typedef struct {LegacyException* record;u32* context;} LegacyExceptionPointers;
typedef struct {u32 base,allocation,allocation_protect,size,state,protect,type;} LegacyRegion;
static u32(WIN*legacy_query)(const void*,void*,u32);
static u32 observed_faults;
static i32 WIN legacy_observe(LegacyExceptionPointers* p){
    LegacyException* e=p->record;u32* c=p->context;
    if(e->code==0xc0000005u&&e->count>=2&&c[46]>=0x10000&&observed_faults++<4){
        LegacyRegion region;zero(&region,sizeof(region));
        emit(88,e->code,c[46],e->info[0],e->info[1],c[47]);
        u32 n=legacy_query((void*)e->info[1],&region,sizeof(region));
        emit(89,n,region.allocation,region.state,region.protect,region.type);
        zero(&region,sizeof(region));n=legacy_query((void*)c[49],&region,sizeof(region));
        if(n==sizeof(region)&&region.state==0x1000&&!(region.protect&0x101)&&
           c[49]>=region.base&&c[49]-region.base<=region.size&&region.size-(c[49]-region.base)>=16){
            u32* stack=(void*)c[49];emit(87,c[49],stack[0],stack[1],stack[2],stack[3]);
        }
    }
    return 0;
}
static void legacy_finish(u32 stage){emit(99,0,0,0,stage,0);CloseHandle(output);ExitProcess(0);}
void start(void){
    output=CreateFileA("legacy-admission.bin",0x40000000,0,0,1,0,0);check(output!=(HANDLE)-1,80);
    HANDLE kernel=GetModuleHandleA("kernel32.dll");
    typedef void*(WIN*AddObserver)(u32,i32(WIN*)(LegacyExceptionPointers*));
    AddObserver add=(AddObserver)GetProcAddress(kernel,"AddVectoredExceptionHandler");
    legacy_query=(void*)GetProcAddress(kernel,"VirtualQuery");
    check(add&&legacy_query&&add(1,legacy_observe),83);
    SetLastError(0);HANDLE module=LoadLibraryA("DDRAW.DLL");u32 error=GetLastError();emit(90,0,0,error,(u32)module,0);if(!module)legacy_finish(1);
    char path[1024];u32 length=GetModuleFileNameA(module,path,sizeof(path)),written;
    HANDLE names=CreateFileA("legacy-module-path.bin",0x40000000,0,0,1,0,0);check(names!=(HANDLE)-1&&length&&length<sizeof(path),81);check(WriteFile(names,path,length,&written,0)&&written==length,82);CloseHandle(names);
    typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(module,"DirectDrawCreate");emit(91,0,0,create?0:GetLastError(),create!=0,0);if(!create)legacy_finish(2);
    void* draw=0;u32 result=create(0,&draw,0);emit(92,0,0,result,draw!=0,0);if(result||!draw)legacy_finish(3);
    result=((u32(WIN*)(void*,void*,u32))vt(draw)[20])(draw,0,8);emit(93,0,0,result,0,8);
    u32 desc[27];zero(desc,sizeof(desc));desc[0]=108;desc[1]=0x1007;desc[2]=6;desc[3]=8;desc[18]=32;desc[19]=0x40;desc[21]=16;desc[22]=0xf800;desc[23]=0x7e0;desc[24]=31;desc[26]=0x840;
    void* surface=0;result=((u32(WIN*)(void*,void*,void**,void*))vt(draw)[6])(draw,desc,&surface,0);emit(94,0,0,result,surface!=0,0);
    if(!result&&surface){fill(surface,0,0,0x1234);inspect(surface,0,0);release(surface);}
    release(draw);legacy_finish(result?4:5);
}
