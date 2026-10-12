#include "../../protocols/include/mnm/api_census_v1.h"
#include "nocd-imports.h"
typedef unsigned int u32;
typedef unsigned char u8;
#define WIN __attribute__((stdcall))
#define API __declspec(dllimport)
API void* WIN GetModuleHandleA(const char*);
API u32 WIN GetModuleHandleExA(u32,const char*,void**);
API u32 WIN GetModuleFileNameA(void*,char*,u32);
API void* WIN VirtualAlloc(void*,u32,u32,u32);
API u32 WIN VirtualProtect(void*,u32,u32,u32*);
API u32 WIN VirtualQuery(const void*,void*,u32);
API u32 WIN FlushInstructionCache(void*,const void*,u32);
API void* WIN GetCurrentProcess(void);
API u32 WIN GetEnvironmentVariableA(const char*,char*,u32);
API void* WIN CreateFileA(const char*,u32,u32,void*,u32,u32,void*);
API u32 WIN GetFileSize(void*,u32*);
API void* WIN CreateFileMappingA(void*,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(void*,u32,u32,u32,u32);
API u32 WIN CloseHandle(void*);
API u32 WIN GetLastError(void);
API void WIN SetLastError(u32);
static u32* trace;
static void* targets[MNM_CENSUS_IMPORT_COUNT];
static void copy(void* to,const void* from,u32 size){u8* out=to;const u8* in=from;for(u32 i=0;i<size;++i)out[i]=in[i];}
static int same(const void* a,const void* b,u32 size){const u8* x=a;const u8* y=b;for(u32 i=0;i<size;++i)if(x[i]!=y[i])return 0;return 1;}
static int readable(const void* address,u32 size){
    struct {u32 base,allocation,allocation_protection,extent,state,protection,type;} region;
    u32 at=(u32)address;
    if(VirtualQuery(address,&region,sizeof(region))!=sizeof(region)||region.state!=0x1000u||(region.protection&0x101u)||at<region.base)return 0;
    u32 distance=at-region.base;return distance<=region.extent&&size<=region.extent-distance;
}
static void label(u8* out,u32 size,const char* value){u32 i=0;while(i+1<size&&value[i]){u8 c=(u8)value[i];out[i]=c>=32&&c<127?c:'?';++i;}out[i]=0;}
static u8 lower(u8 c){return c>='A'&&c<='Z'?c+32:c;}
static void emit(u8* code,u32* low,u32* high,void** target){
    /* PUSHFD; LOCK INC low; JNZ over LOCK INC high; POPFD; JMP [target]. */
    static const u8 bytes[24]={0x9c,0xf0,0xff,0x05,0,0,0,0,0x75,7,0xf0,0xff,0x05,0,0,0,0,0x9d,0xff,0x25,0,0,0,0};
    u32 address=(u32)low;copy(code,bytes,sizeof(bytes));copy(code+4,&address,4);
    address=(u32)high;copy(code+13,&address,4);address=(u32)target;copy(code+20,&address,4);
}
static u32 classify(void* target,u8* output){
    void* module=0;char path[512];
    if(!GetModuleHandleExA(6,(const char*)target,&module))return MNM_API_CENSUS_V1_TARGET_KIND_UNKNOWN;
    u32 length=GetModuleFileNameA(module,path,sizeof(path));if(!length||length>=sizeof(path))return MNM_API_CENSUS_V1_TARGET_KIND_UNKNOWN;
    const char* name=path;for(u32 i=0;i<length;++i)if(path[i]=='\\'||path[i]=='/')name=path+i+1;
    label(output,MNM_API_CENSUS_V1_ROW_TARGET_MODULE_SIZE,name);
    if(lower((u8)name[0])=='m'&&lower((u8)name[1])=='n'&&lower((u8)name[2])=='m')return MNM_API_CENSUS_V1_TARGET_KIND_NATIVE_SHIM;
    if(module==GetModuleHandleA(0)|| (lower((u8)name[0])=='j'&&lower((u8)name[1])=='p'&&lower((u8)name[2])=='e'&&lower((u8)name[3])=='g'))return MNM_API_CENSUS_V1_TARGET_KIND_LEGACY_CODEC;
    return MNM_API_CENSUS_V1_TARGET_KIND_WINE;
}
__declspec(dllexport) void ApiCensusAnchor(void){}
static void refuse(u32 reason){trace[MNM_API_CENSUS_V1_REFUSAL_OFFSET/4]=reason;__atomic_store_n(trace+MNM_API_CENSUS_V1_STATE_OFFSET/4,MNM_API_CENSUS_V1_STATE_REFUSED,__ATOMIC_RELEASE);}
static void install(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_WINE_IMPORT_CENSUS_FILE",path,sizeof(path));if(!length||length>=sizeof(path))return;
    void* file=CreateFileA(path,0xc0000000u,3,0,3,0x80,0);if(file==(void*)-1)return;
    if(GetFileSize(file,0)!=MNM_API_CENSUS_V1_SIZE){CloseHandle(file);return;}
    void* mapping=CreateFileMappingA(file,0,4,0,MNM_API_CENSUS_V1_SIZE,0);CloseHandle(file);if(!mapping)return;
    trace=MapViewOfFile(mapping,2,0,0,MNM_API_CENSUS_V1_SIZE);CloseHandle(mapping);if(!trace)return;
    if(!same(trace,MNM_API_CENSUS_V1_MAGIC,8)||trace[2]!=MNM_API_CENSUS_V1_VERSION||trace[3]!=MNM_API_CENSUS_V1_SIZE){trace=0;return;}
    for(u32 i=16;i<MNM_API_CENSUS_V1_SIZE;++i)if(((u8*)trace)[i]){refuse(1);return;}
    u32 base=(u32)GetModuleHandleA(0);void* pinned=0;
    static const u8 thunk[6]={0xff,0x25,0x1c,0x50,0x5c,0};
    if(base!=0x400000||!readable((void*)(base+0x197566),6)||!same((void*)(base+0x197566),thunk,6)||!GetModuleHandleExA(5,(const char*)&ApiCensusAnchor,&pinned)){refuse(2);return;}
    u8* code=VirtualAlloc(0,MNM_CENSUS_IMPORT_COUNT*32u,0x3000,4);if(!code){refuse(3);return;}
    u32 first=0xffffffffu,last=0;
    for(u32 i=0;i<MNM_CENSUS_IMPORT_COUNT;++i){
        const struct CensusImport* entry=census_imports+i;
        if(entry->rva<first)first=entry->rva;if(entry->rva+4>last)last=entry->rva+4;
        if(!readable((void*)(base+entry->rva),4)){refuse(4);return;}
        targets[i]=*(void**)(base+entry->rva);if(!targets[i]||!readable(targets[i],1)){refuse(4);return;}
        u8* row=(u8*)trace+MNM_API_CENSUS_V1_ROWS_OFFSET+i*MNM_API_CENSUS_V1_ROW_STRIDE;
        u32 kind=classify(targets[i],row+MNM_API_CENSUS_V1_ROW_TARGET_MODULE);
        copy(row+MNM_API_CENSUS_V1_ROW_IAT_RVA,&entry->rva,4);copy(row+MNM_API_CENSUS_V1_ROW_TARGET_KIND,&kind,4);
        label(row+MNM_API_CENSUS_V1_ROW_IMPORT_MODULE,MNM_API_CENSUS_V1_ROW_IMPORT_MODULE_SIZE,entry->module);
        label(row+MNM_API_CENSUS_V1_ROW_SYMBOL,MNM_API_CENSUS_V1_ROW_SYMBOL_SIZE,entry->symbol);
        emit(code+i*32,(u32*)(row+MNM_API_CENSUS_V1_ROW_COUNT_LOW),(u32*)(row+MNM_API_CENSUS_V1_ROW_COUNT_HIGH),targets+i);
    }
    u32 previous,ignored;
    if(!VirtualProtect(code,MNM_CENSUS_IMPORT_COUNT*32u,0x20,&previous)||!FlushInstructionCache(GetCurrentProcess(),code,MNM_CENSUS_IMPORT_COUNT*32u)||!VirtualProtect((void*)(base+first),last-first,4,&previous)){refuse(5);return;}
    for(u32 i=0;i<MNM_CENSUS_IMPORT_COUNT;++i)__atomic_store_n((void**)(base+census_imports[i].rva),(void*)(code+i*32),__ATOMIC_RELEASE);
    VirtualProtect((void*)(base+first),last-first,previous,&ignored);
    trace[MNM_API_CENSUS_V1_INSTALLED_OFFSET/4]=MNM_CENSUS_IMPORT_COUNT;
    trace[MNM_API_CENSUS_V1_ROW_COUNT_OFFSET/4]=MNM_CENSUS_IMPORT_COUNT;
    trace[MNM_API_CENSUS_V1_ROW_STRIDE_OFFSET/4]=MNM_API_CENSUS_V1_ROW_STRIDE;
    __atomic_store_n(trace+MNM_API_CENSUS_V1_STATE_OFFSET/4,MNM_API_CENSUS_V1_STATE_ACTIVE,__ATOMIC_RELEASE);
}
#ifdef MNM_API_CENSUS_SELFTEST
static u32 test_counts[4][2];static void* test_targets[4];
__declspec(dllexport) void* WIN CensusProxyForTest(u32 index,void* target){
    if(index>=4||!target)return 0;
    u8* code=VirtualAlloc(0,32,0x3000,4);u32 previous;if(!code)return 0;test_targets[index]=target;
    emit(code,test_counts[index],test_counts[index]+1,test_targets+index);
    if(!VirtualProtect(code,32,0x20,&previous)||!FlushInstructionCache(GetCurrentProcess(),code,32))return 0;
    return code;
}
__declspec(dllexport) u32 WIN CensusCountForTest(u32 index,u32 high){return index<4&&high<2?test_counts[index][high]:0;}
__declspec(dllexport) void WIN CensusSeedForTest(u32 index,u32 low,u32 high){if(index<4){test_counts[index][0]=low;test_counts[index][1]=high;}}
#endif
int WIN DllMain(void* instance,u32 reason,void* reserved){
    (void)instance;(void)reserved;
    if(reason==1){u32 error=GetLastError();install();SetLastError(error);}
    if(reason==0&&trace&&trace[MNM_API_CENSUS_V1_STATE_OFFSET/4]==MNM_API_CENSUS_V1_STATE_ACTIVE)
        __atomic_store_n(trace+MNM_API_CENSUS_V1_STATE_OFFSET/4,MNM_API_CENSUS_V1_STATE_DETACHED,__ATOMIC_RELEASE);
    /* Keep code/map/targets valid for pinned process-lifetime callbacks. */
    return 1;
}
