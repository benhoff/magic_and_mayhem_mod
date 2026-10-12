/* Original callbacks execute in a private PE32 mapping; no game or save I/O. */
#include "../runtime/shadow/win32_min.h"
#include "save_load_fixture_bytes.h"
#define THIS __attribute__((thiscall))
API HANDLE WIN GetStdHandle(u32);
typedef u32 (THIS *Action)(void*,u32);
typedef u32 (THIS *Tick)(void*);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
u32 saved_esp,callback_result,abi_failure;
u32 invoke_action(u32,void*,u32);
static u32 fades,disabled,loads,writes,writer_result,overwrite,modal_count;
static u8 modal[0x100],allocated[0x100];
static char passed_name[128];
static u32 get(const void* p){const u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
void* memcpy(void* d,const void* s,unsigned n){copy(d,s,n);return d;}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static void require(int condition,u32 code){if(!condition)ExitProcess(code);}
static void THIS fade(void* p){require(p==(void*)0x6de4e8||p==(void*)0x6de7c8,10);++fades;}
static void THIS disable(void* p){require(p==(void*)0x680000,11);++disabled;}
static u32 THIS load(const char* name){++loads;copy(passed_name,name,12);return writer_result;}
static u32 __attribute__((fastcall)) save(const char* name,u32 admitted){++writes;overwrite=admitted;copy(passed_name,name,12);return writer_result;}
static void* allocate(u32 size){require(size==0x61,12);zero(allocated,sizeof(allocated));return allocated;}
static void* THIS construct(void* p){require(p==allocated,13);++modal_count;zero(modal,sizeof(modal));return modal;}
static void THIS destroy(void* p){require(p==modal,14);}
static void release(void* p){require(p==modal,15);}
static void THIS bind(void* p,void* value){require(p==modal&&(value==(void*)0x6a49e0||value==(void*)0x656680),16);}
static void THIS message(void* p,void* cb,u32 text,u32 kind){require(p==modal&&cb==(void*)0x680100&&text>=4&&text<=6&&kind>=1&&kind<=3,17);}
static void stub(u32 address,void* target){u8* p=(u8*)address;p[0]=0xe9;put(p+1,(u32)target-address-5);}
static void install(u32 address,const u8* bytes,u32 size){copy((void*)address,bytes,size);}
static void reset(void* object,u32 id){zero(object,0x80);put((u8*)object+4,id);put((u8*)object+0x4f,0x680200);put((void*)0x680245,0x680300);put((u8*)object+0x53,0x680400);put((void*)0x680410,0x680100);copy((void*)0x680300,"fixture-save",13);fades=disabled=loads=writes=overwrite=modal_count=0;writer_result=1;}
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
#define INSTALL(name) install(name##_address,name##_bytes,sizeof(name##_bytes))
 INSTALL(load_button);INSTALL(load_tick);INSTALL(save_button);INSTALL(save_tick);INSTALL(edit_set);INSTALL(list_select);INSTALL(list_get);
 stub(0x557510,(void*)&fade);stub(0x4ca950,(void*)&disable);stub(0x4ea280,(void*)&load);stub(0x4ea5a0,(void*)&save);
 stub(0x597850,(void*)&allocate);stub(0x4cef40,(void*)&construct);stub(0x4cefa0,(void*)&destroy);stub(0x597870,(void*)&release);
 stub(0x4cfbd0,(void*)&bind);stub(0x4cfbe0,(void*)&bind);stub(0x4cf010,(void*)&message);
 void* object=(void*)0x6de4e8;u8* p=object;Action button=(Action)load_button_address;Tick tick=(Tick)load_tick_address;
 for(u32 caller=3;caller<=17;caller+=14){reset(object,12);put(p+0x60,caller);put((void*)0x657d37,123);
   require(!invoke_action((u32)button,object,1)&&fades==1&&get(p+0x43)==1&&get((void*)0x657d37)==(caller==3?0:123),20);}
 reset(object,12);require(!invoke_action((u32)button,object,9)&&!fades&&!p[0x64],21);
 for(u32 list=0;list<=1;++list){reset(object,12);put(p+0x5b,list?0x680000:0);require(!invoke_action((u32)button,object,0)&&p[0x5f]==1&&p[0x64]==1&&disabled==list,22);
   tick(object);require(loads==1&&!p[0x64]&&fades==1&&get(p+0x43)==1&&get((void*)0x657d37)==1&&*(u8*)0x6db966==1,23);
   tick(object);require(loads==1,24);}
 reset(object,12);tick(object);require(!loads,25);
 object=(void*)0x6de7c8;p=object;button=(Action)save_button_address;tick=(Tick)save_tick_address;
 reset(object,13);require(!invoke_action((u32)button,object,1)&&fades==1&&get(p+0x43)==1&&!writes,26);
 reset(object,13);require(!invoke_action((u32)button,object,9)&&!fades&&!writes,27);
 for(u32 admitted=0;admitted<=1;++admitted){reset(object,13);copy((void*)0x680300,"  fixture-save",15);put(p+0x5f,admitted);tick(object);
   require(writes==1&&overwrite==admitted&&passed_name[0]=='f'&&!get(p+0x5f)&&get(p+0x43)==1&&fades==1,28);}
 reset(object,13);writer_result=2;tick(object);require(writes==1&&modal_count==1&&get(p+0x3b)==(u32)modal&&get(p+0x5f)==1&&!p[0x63]&&!fades,29);
 /* The filename setter copies a terminator; list selection is reset then selected,
  * preventing the original toggle-on-selected behavior from undoing a request. */
 void* edit=(void*)0x680200;invoke_action(edit_set_address,edit,(u32)"second-save");require(!((char*)0x680300)[11],30);
 void* list=(void*)0x680500;zero(list,0x80);put((u8*)list+0x45,0x680600);put((u8*)list+0x49,2);copy((void*)0x680600,"first",6);copy((void*)0x680701,"second",7);*((u8*)list+0x5a)=1;
 Action select=(Action)list_select_address;invoke_action((u32)select,list,1);invoke_action((u32)select,list,1);require(get((u8*)list+0x5b)==0xffffffff,31);
 invoke_action((u32)select,list,0xffffffff);invoke_action((u32)select,list,1);require(get((u8*)list+0x5b)==1&&((Tick)list_get_address)(list)==0x680701,32);
 require(!abi_failure,33);
 const char report[]="{\"success\":true,\"cases\":13,\"real_game_launched\":false,\"save_io_stubbed\":true}\n";u32 wrote;
 WriteFile(GetStdHandle(0xfffffff5),report,sizeof(report)-1,&wrote,0);ExitProcess(0);
}
