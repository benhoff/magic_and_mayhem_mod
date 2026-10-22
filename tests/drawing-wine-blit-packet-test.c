#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static void __attribute__((cdecl)) callback(void* context,const void* packet){(void)context;(void)packet;SetLastError(0xcafe);}
void start(void){
 require(initialize(),120);u8* code=VirtualAlloc(0,64,0x3000,0x40);require(code!=0,121);
 const u8 bytes[]={0x8b,0x03,0x89,0x74,0x24,0x04,0xc3};copy(code,bytes,sizeof(bytes));code[0]=0x90;require(!wine_blit_packet_install_site(code)&&code[0]==0x90,122);code[0]=0x8b;require(wine_blit_packet_install_site(code),123);
 u32 command[25]={42,2,0,0,0,0,0,0,0,3};u32 registers[9]={0};registers[7]=(u32)command;
 char mode[2]={0};GetEnvironmentVariableA("MNM_PACKET_FIXTURE_CASE",mode,2);
 if(mode[0]){wine_blit_packet_worker(mode[0]=='n'?0:command,mode[0]=='w'?0:1,mode[0]=='p'?1:2);ExitProcess(129);}
 u32 slot=wine_submission_slot();wine_submission_parents[slot].submission_depth=1;wine_submission_parents[slot].submission[0]=17;wine_blit_packet_observe(registers);
 require(wine_blit_packet_commands==1,124);wine_blit_worker_original=(void*)&callback;SetLastError(0xbeef);wine_blit_worker((void*)1,command);require(GetLastError()==0xcafe&&!wine_blit_packets[0].pointer,125);
 /* The real assembly shim is also exercised on a second packet; retainedflags
  * and caller registers stay identical apart from the original sixinstructions. */
 u32 values[3]={0};void* table=(void*)0x12345678;
 extern void packet_fixture_call(void*,void*,void*,u32*);
 SetLastError(0xbeef);packet_fixture_call(code,command,&table,values);
 require(values[0]==0x12345678&&(values[1]&1)&&values[2]==0xabcdef01&&GetLastError()==0xbeef&&wine_blit_packet_commands==2,126);
 wine_blit_worker((void*)1,command);wine_blit_packet_close(1);require(!wine_blit_packet_enabled,130);u32 prior=wine_blit_packet_records;wine_blit_worker((void*)1,command);require(wine_blit_packet_records==prior&&GetLastError()==0xcafe,131);wine_blit_worker_close(1);wine_lifetime_close(1);ExitProcess(0);
}
