/* Actual original manager and custom heap; owned virtual update/priority
 * callbacks exercise insertion during update. Real original deleting bodies
 * and actual original frees still execute. */
#include "original-pe32-image.h"
#include "original-pe32-imports.h"
#include "particle-pool-fixture.h"
static u32 pool[POOL_CAPACITY+2],table[6];
static HANDLE pool_output;
static u32 __attribute__((thiscall)) original_priority(u32* self){return (u32)fixture_priority(self[1]);}
static u32 __attribute__((thiscall)) original_update(u32* self){return fixture_update(self[1]);}
static u32 token_of(u32 address){
    if(!address)return 0;
    u32 token=((u32*)address)[1];fixture_require(token&&token<=next_node&&nodes[token].alive&&nodes[token].address==address,211);return token;
}
__attribute__((used)) static u32 free_continue=0x597876;
void pool_free_observe(u32* registers){u32* stack=registers+9;fixture_destroyed(token_of(stack[1]));}
__attribute__((naked)) static void forwarding_delete(void){
    __asm__("pushfl\n\tpushal\n\tmovl %esp,%eax\n\tpushl %eax\n\tcall _pool_free_observe\n\taddl $4,%esp\n\tpopal\n\tpopfl\n\tmovl 4(%esp),%eax\n\ttestl %eax,%eax\n\tjmp *_free_continue");
}
static void fixture_require(int ok,u32 code){original_require(ok,code);}
static void fixture_write(const void* data,u32 bytes){u32 written;fixture_require(WriteFile(pool_output,data,bytes,&written,0)&&written==bytes,212);}
static void pool_allocate(u32 token){
    typedef void*(*Allocate)(u32);u32* object=((Allocate)0x597850)(128);fixture_require(object!=0,213);
    nodes[token].address=(u32)object;for(u32 i=0;i<32;++i)object[i]=0xa5a5a5a5;
    object[0]=(u32)table;object[1]=token;object[2]=(u32)nodes[token].score;object[3]=0;
}
static void pool_validate_object(u32 token){
    u32* object=(u32*)nodes[token].address;fixture_require(nodes[token].alive&&object[0]==(u32)table&&object[1]==token&&object[2]==(u32)nodes[token].score&&object[3]==nodes[token].age,214);
    for(u32 i=4;i<32;++i)fixture_require(object[i]==0xa5a5a5a5,215);
}
static void pool_increment_age(u32 token){((u32*)nodes[token].address)[3]++;}
static u32 pool_insert(u32 token){typedef u32(__attribute__((thiscall))*Insert)(void*,void*);return ((Insert)0x5457e0)(pool,(void*)nodes[token].address);}
static void pool_update(void){typedef void(__attribute__((thiscall))*Update)(void*);((Update)0x545910)(pool);}
static void pool_clear(void){typedef void(__attribute__((thiscall))*Clear)(void*);((Clear)0x5458b0)(pool);}
static u32 pool_entry(u32 index){return token_of(pool[index]);}
static i32 pool_count(void){return (i32)pool[POOL_CAPACITY];}
static i32 pool_hint(void){return (i32)pool[POOL_CAPACITY+1];}
static void pool_initialize(u32 count,i32 hint){
    for(u32 i=0;i<POOL_CAPACITY+2;++i)pool[i]=0;
    for(u32 i=0;i<count;++i)pool[i]=nodes[i+1].address;
    pool[POOL_CAPACITY]=count;pool[POOL_CAPACITY+1]=(u32)hint;
}
static void pool_free_unowned(u32 token){typedef void(__attribute__((thiscall))*Delete)(void*,u32);((Delete)0x54aa10)((void*)nodes[token].address,1);}
void start(void){
    original_map();fixture_require(original_kernel_imports()>0,216);
    static const u8 expected[6]={0x8b,0x44,0x24,0x04,0x85,0xc0};
    u8* entry=(u8*)0x597870;for(u32 i=0;i<6;++i)fixture_require(entry[i]==expected[i],217);
    entry[0]=0xe9;*(u32*)(entry+1)=(u32)&forwarding_delete-0x597875;entry[5]=0x90;
    table[0]=0x54aa10;table[4]=(u32)&original_update;table[5]=(u32)&original_priority;
    pool_output=CreateFileA("particle-pool.bin",0x40000000,0,0,1,0,0);fixture_require(pool_output!=(HANDLE)-1,218);
    fixture_run();CloseHandle(pool_output);
    typedef u32(*Finish)(void);fixture_require(((Finish)0x597db0)()&&*(u32*)0x5f2094==0,219);
    ExitProcess(0);
}
