/* Execute unchanged hash-pinned original primary/method fragments with synthetic
 * RNG/raster tails. Production admission checks inspect the real call stack. */
#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
void* memset(void* p,int v,u32 n){volatile u8* b=p;while(n--)*b++=(u8)v;return p;}
#include "../runtime/scene/world_raster_callers.h"
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void zero(void* d,u32 n){u8* a=d;while(n--)*a++=0;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||!n||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
#include "../runtime/scene/kind8_sprite_admission.h"
#include "../runtime/scene/fading_particle_admission.h"
#include "../reconstruction/rendering/kind8_class.h"
static u32 batch_guard_open=1,batch_owner;
#include "sprite-batch-caller.inc"
#include "sprite-observer-caller.inc"
#include "kind8-sprite-original.inc"
extern u32 kind8_sprite_primary_call(u32*);
static u32 calls,randoms,randomValues[2],backendReturn,variant,hits,frameBase;
static volatile u8 reserve[0x380000];
static u32 draws[3][8];static HANDLE file;static u8 buffer[65520];static u32 used;
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static void require(int yes,u32 code){if(!yes)ExitProcess(code);}
static void flush(void){u32 n=0;require(WriteFile(file,buffer,used,&n,0)&&n==used,10);used=0;}
static void output(const void* bytes,u32 n){if(used+n>sizeof(buffer))flush();copy(buffer+used,bytes,n);used+=n;}
static u32 capture(u32 tag,const u32* stack,void* frame,int x,int y,u32 a,u32 b,u32 argc){
 require(raster_batch_caller(stack[0],tag,stack),11);require(!raster_caller(stack[0],tag,stack),12);++hits;
 require(calls<3&&(u32)frame>=frameBase&&((u32)frame-frameBase)%32==0&&(u32)frame-frameBase<256,13);
 u32 row[8]={tag==22||tag==36?0:tag==17?2:1,100+((u32)frame-frameBase)/32,(u32)x,(u32)y,a,b,argc,0};copy(draws[calls++],row,32);return backendReturn;
}
static u32 __attribute__((thiscall,noinline)) rng(void* object){require((u32)object==0x6574c8&&randoms<2,14);return randomValues[randoms++];}
static u32 __attribute__((noinline)) word(void* f,int x,int y){return capture(variant?36:22,(u32*)__builtin_frame_address(0)+1,f,x,y,0,0,3);}
static u32 __attribute__((noinline)) generic(void* f,int x,int y,u32 a,u32 b){return capture(variant?14:13,(u32*)__builtin_frame_address(0)+1,f,x,y,a,b,5);}
static u32 __attribute__((noinline)) clipped(void* f,int x,int y,u32 a,u32 b){return capture(17,(u32*)__builtin_frame_address(0)+1,f,x,y,a,b,5);}
static void bind(u32 at,void* to){*(u8*)at=0xe9;put((void*)(at+1),(u32)to-at-5);}
static u32 guard_controls;
static void guard_check(int yes){++guard_controls;require(yes,30);}
static void guard_matrix(void){
 u32 object[8]={0},owner[17]={0},bank[4]={0},frame[8]={0},row[9]={0},stack[10]={0};
 const u32 thread=GetCurrentThreadId();
 for(u32 k=0;k<9;++k){const struct SpriteRasterSite* site=sprite_raster_sites+k;const struct MnmKind8SpriteClass type=mnm_kind8_sprite_class(site->table,site->method);
  object[0]=site->table;object[type.owner_offset/4]=(u32)owner;row[1]=(u32)object;row[6]=8;row[7]=0x8ad08ad0;
  stack[0]=site->address;stack[1]=(u32)frame;stack[site->parent_word]=0x5006b2;stack[site->parent_word-2]=(u32)row;
  frame[7]=site->parent_word==7?0xffffffffu:0;bank[3]=1;put((void*)site->bank,(u32)bank);put((void*)0x644520,0);owner[9]=1;
  for(u32 tag=0;tag<64;++tag){const int allowed=(site->tags&(1ull<<tag))!=0;guard_check((raster_batch_caller(site->address,tag,stack)!=0)==allowed);if(!allowed)continue;
#define VALID() raster_batch_caller(site->address,tag,stack)
   guard_check(!raster_caller(site->address,tag,stack));guard_check(!raster_batch_caller(site->address,64,stack));guard_check(!raster_batch_caller(site->address,tag,0));
   const u32 starts[]={site->address,site->method,0x5006a2,0x5006b2},lengths[]={10,9,16,10};
   for(u32 r=0;r<4;++r)for(u32 i=0;i<lengths[r];++i){u8* b=(u8*)(starts[r]+i);const u8 saved=*b;*b^=1;guard_check(!VALID());*b=saved;}guard_check(VALID());
   batch_guard_open=0;guard_check(!VALID());batch_guard_open=1;batch_owner=thread+1;guard_check(!VALID());batch_owner=thread;
   for(u32 player=0;player<8;++player){zero(owner,sizeof(owner));owner[9+player]=1;put((void*)0x644520,player);guard_check(VALID());owner[9+player]=0;guard_check(!VALID());}owner[9]=1;put((void*)0x644520,0);
   for(u32 bad=8;bad<10;++bad){put((void*)0x644520,bad);guard_check(!VALID());}put((void*)0x644520,0);
   const u32 invalid_counts[]={0,4097,0x80000000u,0xffffffffu};for(u32 i=0;i<4;++i){bank[3]=invalid_counts[i];guard_check(!VALID());}bank[3]=4096;guard_check(VALID());bank[3]=1;
   frame[7]^=0xffffffffu;guard_check(!VALID());frame[7]^=0xffffffffu;
   object[0]=site->table^1;guard_check(!VALID());object[0]=site->table;put((void*)(site->table+12),site->method^1);guard_check(!VALID());put((void*)(site->table+12),site->method);
   const u32 positions[]={site->parent_word,site->parent_word-2,1};for(u32 i=0;i<3;++i){u32 saved=stack[positions[i]];stack[positions[i]]=1;guard_check(!VALID());stack[positions[i]]=saved;}
   object[type.owner_offset/4]=1;guard_check(!VALID());object[type.owner_offset/4]=(u32)owner;put((void*)site->bank,1);guard_check(!VALID());put((void*)site->bank,(u32)bank);
   row[6]=9;guard_check(!VALID());row[6]=8;row[7]=0;guard_check(!VALID());row[7]=0x8ad08ad0;
   put((void*)0x5e1404,0);guard_check(!VALID());put((void*)0x5e1404,1);put((void*)0x6e1f88,1);guard_check(!VALID());put((void*)0x6e1f88,0);guard_check(VALID());
#undef VALID
  }
 }
}
void start(void){
 char path[512];u32 pathBytes=GetEnvironmentVariableA("MNM_SPRITE_TRACE",path,sizeof(path)-7);require(pathBytes&&pathBytes<sizeof(path)-7,1);
 reserve[sizeof(reserve)-1]=1;u32 prior;require((u32)GetModuleHandleA(0)==0x400000&&VirtualProtect((void*)0x480000,0x2a0000,0x40,&prior),2);
 const u32 tables[]={0x5c74e4,0x5c74fc,0x5c7514},methods[]={0x5492b0,0x549730,0x549ba0},banks[]={0x6f2150,0x6f2268,0x6f21e8};
 copy((void*)methods[0],sprite_method_flies,sizeof(sprite_method_flies));copy((void*)methods[1],sprite_method_cure,sizeof(sprite_method_cure));copy((void*)methods[2],sprite_method_attached,sizeof(sprite_method_attached));
 copy((void*)0x5006a2,sprite_primary_prefix,sizeof(sprite_primary_prefix));copy((void*)0x5006b2,sprite_primary_continuation,sizeof(sprite_primary_continuation));copy((void*)0x500848,sprite_primary_store,sizeof(sprite_primary_store));*(u8*)0x50084c=0xc3;
 for(u32 i=0;i<3;++i)put((void*)(tables[i]+12),methods[i]);put((void*)0x5e1404,1);put((void*)0x6e1f88,0);batch_owner=GetCurrentThreadId();
 bind(0x54e200,rng);bind(0x57de00,clipped);put((void*)0x6903dc,(u32)word);put((void*)0x6c4e3c,(u32)generic);
 require(FlushInstructionCache(GetCurrentProcess(),(void*)0x400000,0x320000),3);
 file=CreateFileA(path,0x40000000,0,0,1,0x80,0);require(file!=(HANDLE)-1,4);
 const u32 pairs[][2]={{0,0},{1,1},{49,7},{50,8},{51,9},{99,0xffffffffu},{100,0x80000000u},{0xffffffffu,0x80000033u},{0x80000032u,50},{0x80000033u,51}};
 const u32 counts[]={0,1,2,5,8},masks[]={0,0x55,0xff},returns[]={0,1,2,0x1234,0x10001,0xffffffffu};u32 cases=0;
 for(u32 family=0;family<3;++family)for(u32 present=0;present<2;++present)for(int player=-1;player<=8;++player)
 for(u32 enabled=0;enabled<2;++enabled)for(u32 ci=0;ci<5;++ci)for(u32 pi=0;pi<10;++pi)for(u32 mi=0;mi<3;++mi)for(u32 ri=0;ri<6;++ri)for(variant=0;variant<2;++variant){
  u32 object[8],before[8],owner[17]={0},bank[8]={0},offsets[8],frames[8][8]={0};
  for(u32 i=0;i<8;++i){object[i]=0x12340000+i;owner[9+i]=enabled?0x80000000:0;offsets[i]=i*32;frames[i][7]=masks[mi]&(1u<<i)?0xffffffffu:0x80000000u;}
  object[0]=tables[family];object[family?4:7]=(u32)owner;bank[3]=counts[ci];frameBase=(u32)frames;calls=randoms=0;zero(draws,sizeof(draws));randomValues[0]=pairs[pi][0];randomValues[1]=pairs[pi][1];backendReturn=returns[ri];
  put((void*)0x644520,(u32)player);put((void*)banks[family],present?(u32)bank:0);put((void*)(banks[family]+8),(u32)offsets);put((void*)(banks[family]+12),(u32)frames);copy(before,object,sizeof(before));
  u32 row[9]={0,(u32)object,(u32)-123,456,0,0,8,0x8ad08ad0,0x12345678};
  /* The primary row accepts the exact class/root for positive and no-op gates. */
  require(kind8_supported((u32)row),15);u32 returned=kind8_sprite_primary_call(row)&65535;
  require(!returned&&row[7]==0x8ad0&&row[8]==0x12345678,16);for(u32 i=0;i<8;++i)require(object[i]==before[i],17);
  object[family?4:7]=777;u32 head[3]={returned,calls,randoms};output(head,12);output(draws,sizeof(draws));output(object,sizeof(object));++cases;
 }
 require(cases==216000,18);flush();require(CloseHandle(file),19);
 require(equal((void*)methods[0],sprite_method_flies,sizeof(sprite_method_flies))&&equal((void*)methods[1],sprite_method_cure,sizeof(sprite_method_cure))&&equal((void*)methods[2],sprite_method_attached,sizeof(sprite_method_attached)),20);
 guard_matrix();
 copy(path+pathBytes,".proof",7);file=CreateFileA(path,0x40000000,0,0,1,0x80,0);u32 proof[5]={0x53505233,cases,hits,1,guard_controls};u32 n=0;require(file!=(HANDLE)-1&&WriteFile(file,proof,20,&n,0)&&n==20&&CloseHandle(file),21);ExitProcess(0);
}
