#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
#define THIS __attribute__((thiscall))
static u32 get(const void* p){return *(const u32*)p;}
static void put(void* p,u32 value){*(u32*)p=value;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(const void* p,u32 n){u32 m[7];return VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(u32)p+n>=(u32)p&&(u32)p+n<=m[0]+m[3];}
static u8* owned;
static u8* owned_code;
#define MNM_REGION_INPUT_VTABLE ((u32)owned+0x200)
#define MNM_REGION_INPUT_CALLBACK ((u32)owned_code+0x100)
#define MNM_REGION_INPUT_ACTOR ((u32)owned+0x100)
#define MNM_REGION_INPUT_INDEX ((u32)owned+0x104)
#define MNM_REGION_INPUT_CLASS ((u32)owned+0x108)
#define MNM_REGION_INPUT_POOL ((u32)owned+0x110)
#define MNM_REGION_INPUT_CAMERA ((u32)owned+0x400)
#define MNM_REGION_INPUT_MODE ((u32)owned+0x120)
#define MNM_REGION_INPUT_CURSOR ((u32)owned+0x124)
#define MNM_REGION_INPUT_RING ((u32)owned+0x128)
#define MNM_REGION_INPUT_LATCH_COUNT ((u32)owned+0x12c)
#include "../runtime/menu/drawing_region_input.h"
static u8 fp_input[512] __attribute__((aligned(16))),fp_output[512] __attribute__((aligned(16))),fp_actual[512] __attribute__((aligned(16)));
static u8 before[65536];
static u32 expected_case,original_calls;
static RegionMouseRecord expected_mouse;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void zero_fp(void){for(u32 i=0;i<512;++i)fp_input[i]=fp_output[i]=fp_actual[i]=0;}
static u32 THIS original(void* object,u32 mask,RegionMouseRecord mouse){
 region_input_fp_save(fp_actual);
 require(equal(fp_input,fp_actual,512),211);
 require(object==(void*)MNM_REGION_INPUT_CAMERA&&mask==(expected_case|0x80000000)&&equal(mouse.words,expected_mouse.words,40),212);
 require(GetLastError()==(0xabc00000|expected_case),213);++original_calls;
 put((void*)((u32)owned+0x1000+0xe4),get((void*)((u32)owned+0x1000+0xe4))+1);
 u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));region_input_fp_save(fp_output);
 SetLastError(0xdef00000|expected_case);return 0x12340000|expected_case;
}
static void invoke(u32 value){
 expected_case=value;for(u32 i=0;i<10;++i)expected_mouse.words[i]=value*17+i;
 zero_fp();u16 control=0x027f;__asm__ volatile("fldcw %0"::"m"(control));region_input_fp_save(fp_input);
 copy(before,owned,65536);u32 old=original_calls;SetLastError(0xabc00000|value);
 u32 result=((RegionMouseFn)get((void*)MNM_REGION_INPUT_VTABLE))((void*)MNM_REGION_INPUT_CAMERA,value|0x80000000,expected_mouse);
 region_input_fp_save(fp_actual);
 require(equal(fp_output,fp_actual,512)&&result==(0x12340000|value)&&GetLastError()==(0xdef00000|value)&&original_calls==old+1,214);
 *(u32*)(before+0x1000+0xe4)+=1;require(equal(before,owned,65536),215);
}
static u32 WIN foreign(void* unused){(void)unused;invoke(35);return 0;}
void start(void){
 owned=VirtualAlloc(0,65536,0x3000,4);require(owned!=0,201);
 owned_code=VirtualAlloc(0,65536,0x3000,0x40);require(owned_code!=0,202);
 const u8 prefix[]={0x81,0xec,0x30,0x01,0,0,0x53,0x55,0x56,0x8b,0xe9,0x5e,0x5d,0x5b,0x81,0xc4,0x30,0x01,0,0,0xe9,0,0,0,0};
 copy((void*)MNM_REGION_INPUT_CALLBACK,prefix,sizeof(prefix));put((void*)(MNM_REGION_INPUT_CALLBACK+21),(u32)&original-(MNM_REGION_INPUT_CALLBACK+25));
 put((void*)MNM_REGION_INPUT_VTABLE,(MNM_REGION_INPUT_CALLBACK+1));require(!region_input_open()&&!region_input_file&&get((void*)MNM_REGION_INPUT_VTABLE)==(MNM_REGION_INPUT_CALLBACK+1),204);
 put((void*)MNM_REGION_INPUT_VTABLE,MNM_REGION_INPUT_CALLBACK);*(u8*)MNM_REGION_INPUT_CALLBACK=0x90;require(!region_input_open()&&!region_input_file,205);*(u8*)MNM_REGION_INPUT_CALLBACK=prefix[0];
 require(region_input_open()&&get((void*)MNM_REGION_INPUT_VTABLE)==(u32)&region_input_callback&&!region_input_open(),206);
 u32 actor=(u32)owned+0x1000;put((void*)MNM_REGION_INPUT_ACTOR,actor);put((void*)MNM_REGION_INPUT_POOL,actor);put((void*)(MNM_REGION_INPUT_POOL+4),1);
 put((void*)actor,0);put((void*)(actor+4),1);put((void*)(actor+0xa8),27);put((void*)(actor+0x174),0);put((void*)(actor+0xe4),100);
 put((void*)MNM_REGION_INPUT_CLASS,2);put((void*)MNM_REGION_INPUT_INDEX,7);put((void*)(MNM_REGION_INPUT_CAMERA+4),3);put((void*)(MNM_REGION_INPUT_CAMERA+8),1);
 put((void*)(MNM_REGION_INPUT_CAMERA+0x31),2);for(u32 i=0;i<6;++i)put((void*)(MNM_REGION_INPUT_CAMERA+0x39+i*4),10+i);
 for(u32 i=0;i<32;++i){
  if(region_input_track_enabled){
   if(i==3)put((void*)MNM_REGION_INPUT_ACTOR,0);
   if(i==4){put((void*)(actor+0x68),1);put((void*)MNM_REGION_INPUT_LATCH_COUNT,1);}
   if(i==5)put((void*)actor,1);
   if(i==6){put((void*)actor,0);put((void*)MNM_REGION_INPUT_ACTOR,actor);}
  }
  invoke(i);u32* entry=region_input_rows[i*2];u32* returned=region_input_rows[i*2+1];
  int hit=!region_input_track_enabled||i<3||i>5;
  require(entry[31]==(hit?100+i:0)&&returned[31]==(hit?101+i:0)&&entry[27]==(hit?1u:0u),216);
  if(hit)require(entry[28]==27&&entry[29]==0&&entry[30]==1,222);
  if(region_input_track_enabled){
   require(entry[45]==(i==5?5u:7u)&&entry[46]==1&&entry[47]==1&&!entry[55],223);
   require(entry[48]==(i==5?0u:1u)&&entry[52]==(i==5?0u:100+i)&&returned[52]==(i==5?0u:101+i),224);
   if(i==4)require(!entry[24]&&entry[53]==1&&returned[53]==1,225);
  }
 }
 require(region_input_count==64&&region_input_phase==64&&region_input_calls==32&&!region_input_pending&&!region_input_dropped&&!region_input_foreign,217);
 region_input_close();invoke(32);require(region_input_count==64,218);
 region_input_file=(HANDLE)1;region_input_count=8192;region_input_calls=4096;region_input_phase=8192;invoke(33);
 require(region_input_count==8192&&region_input_dropped==1&&region_input_phase==8192&&!region_input_pending,219);
 u32 owner=region_input_thread;HANDLE thread=CreateThread(0,0,&foreign,0,0,0);require(thread!=0&&WaitForSingleObject(thread,5000)==0&&CloseHandle(thread),220);
 require(region_input_foreign==1&&region_input_thread==owner&&region_input_count==8192&&region_input_dropped==1&&original_calls==35,221);
 region_input_file=0;ExitProcess(0);
}
