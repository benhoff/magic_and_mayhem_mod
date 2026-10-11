#include "../runtime/shadow/win32_min.h"
API u32 WIN SceneProductionInstallForTest(u32);
API u32 WIN SceneProductionStatusForTest(u32);
extern void fixture_invoke(void*,u32,void*),fixture_draw(void);
/* Fixed fixture image owns the game's authored address range. Observer storage
 * lives in its separate DLL, outside these independently initialized bytes. */
static volatile u8 fixture_address_space[0x300000];
static u8 baseline[576] __attribute__((aligned(16))),observed[576] __attribute__((aligned(16)));
static u8 creatures[3*0xe4b],effects[2*0x22e],objects[2*0x7e];
static u32 draws,test_case;
static void put(u32 at,u32 v){*(u32*)at=v;}
static void require(int okay,u32 code){if(!okay)ExitProcess(code);}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static void branch(u32 at,u32 target){*(u8*)at=0xe9;put(at+1,target-at-5);}
static void bytes(u32 at,const u8* source,u32 n){for(u32 i=0;i<n;++i)*(u8*)(at+i)=source[i];}
void fixture_work(void){
 ++draws;
 if(draws==2){
  if(test_case==1)++*(u32*)0x6c4830;
  if(test_case==2)creatures[0xe4]^=1;
  if(test_case==3)++*(u32*)(0x6df188+0x28);
  if(test_case==4)++*(u32*)(0x6e1ac8+251*4);
  if(test_case==5)put(0x6def5c,3);
  if(test_case==7)++*(u32*)(0x6cbb78+0x100);
  if(test_case==8)++*(u32*)(0x6c5490+0x100);
 }
 SetLastError(0xabc123);
}
static void reset(void){
 draws=0;put(0x6c4830,128);put(0x6cbb78+0xfd5a,0);put(0x6f34e0,0x6cbb78);
 put(0x6def58,(u32)creatures);put(0x6def5c,2);put(0x6df180,1);
 put(0x689498,(u32)effects);put(0x68949c,2);put(0x6898dc,1);
 put(0x6b1248,(u32)objects);put(0x6b124c,2);put(0x6b1260,1);
 put(0x689930+0x99,3);put(0x689930+0x25,0x689930);put(0x689930+0x229,0);
 put(0x689b59,0x689c00);put(0x689b61,1);put(0x689b65,1);
}
void start(void){
 fixture_address_space[0]=1;u32 protection;
 require(VirtualProtect((void*)0x460000,0x2a0000,0x40,&protection),1);
 const u8 outer[]={0x53,0x55,0x56,0x8b,0xf1,0x33,0xdb,0x57};bytes(0x4fd280,outer,sizeof(outer));branch(0x4fd288,0x4fd315);
 const u8 start[]={0x8b,0x46,0x25,0x8b,0xce};bytes(0x4fd315,start,5);branch(0x4fd31a,(u32)fixture_draw);
 const u8 end[]={0x33,0xdb,0x8b,0x96,0x29,0x02,0,0};bytes(0x4fd5ad,end,8);
 const u8 finish[]={0x5f,0x5e,0x5d,0x5b,0xf9,0xb8,0xdf,0x9b,0x57,0x13,0xb9,0xcd,0xab,0x68,0x24,0xc3};bytes(0x4fd5b5,finish,sizeof(finish));
 const u8 q1[]={0xe8,0xf0,0x2d,0,0},q2[]={0xe8,0x47,0x2d,0,0};bytes(0x4fd4ab,q1,5);bytes(0x4fd554,q2,5);*(u8*)0x4fd559=0xc3;*(u8*)0x5002a0=0xc3;
 char option[3];u32 n=GetEnvironmentVariableA("MNM_PRODUCTION_TEST_CASE",option,3);require(n==2&&option[0]>='0'&&option[0]<='8'&&(option[1]=='1'||option[1]=='2'),2);
 test_case=option[0]-'0';u32 candidate=option[1]-'0';reset();
 SetLastError(0x77);fixture_invoke((void*)0x4fd280,0x689930,baseline);require(GetLastError()==0xabc123&&draws==1,3);reset();
 /* Every anchor byte mutation must refuse before any hook is installed. */
 const u32 sites[]={0x4fd280,0x4fd315,0x4fd5ad,0x4fd4ab,0x4fd554},lengths[]={5,5,8,5,5};
 for(u32 i=0;i<5;++i)for(u32 j=0;j<lengths[i];++j){*(u8*)(sites[i]+j)^=1;SetLastError(0x77);require(!SceneProductionInstallForTest(candidate)&&GetLastError()==0x77,4);*(u8*)(sites[i]+j)^=1;}
 require(!SceneProductionInstallForTest(3),5);SetLastError(0x77);require(SceneProductionInstallForTest(candidate)&&GetLastError()==0x77,6);
 if(test_case==6)put(0x6def58,0xffffffff);
 SetLastError(0x77);fixture_invoke((void*)0x4fd280,0x689930,observed);
 require(GetLastError()==0xabc123,70);
 require(same(baseline,observed,12)&&same(baseline+16,observed+16,20),71);
 require(*(u32*)(observed+12)==*(u32*)(observed+36),72);
 require(same(baseline+48,observed+48,512),73);
 require(SceneProductionStatusForTest(0)==1,8);
 if(!test_case)require(draws==2&&!SceneProductionStatusForTest(1)&&!SceneProductionStatusForTest(2)&&SceneProductionStatusForTest(5)==1,9);
 else if(test_case<=4||test_case>=7)require(draws==2&&SceneProductionStatusForTest(1)==1&&SceneProductionStatusForTest(4)>0,10);
 else require(SceneProductionStatusForTest(2)>=1&&draws==(test_case==6?1u:2u),11);
 require(!SceneProductionInstallForTest(candidate),12);ExitProcess(0);
}
