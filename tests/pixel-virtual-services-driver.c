/* Unchanged original callers with real Wine Surface1/Surface2 aliases.
 * Shadow only the owned Surface1 vtables; forward Unlock unchanged. */
#include "../runtime/shadow/win32_min.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
API HANDLE WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(HANDLE,const char*);
API u32 WIN GetModuleFileNameA(HANDLE,char*,u32);
__asm__(".section .text$a,\"xr\"\n.globl _original_reservation\n_original_reservation:\n.space 33554432\n");
#pragma code_seg(".text$z")
struct Event {u32 pc,owner,pointer_match,result;};
struct Output {u32 kind,format,width,height,early,result,event_count,target_rva,target_module_is_ddraw,distinct_interfaces,pitches[2];struct Event events[4];u16 before[2][128],after[2][128];};
static struct Output out;
static void *first[2],*second[2],**saved[2],*shadow[2][40],*locked[2];
static void* unlock_target;
static void clear(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static void require(int ok,u32 stage){if(!ok)ExitProcess(stage);}
static void** table(void* p){return *(void***)p;}
static void release(void* p){if(p)((u32(WIN*)(void*))table(p)[2])(p);}
static u32 WIN unlock(void* receiver,void* pointer){
 u32 id=receiver==first[0]?0:1;require(receiver==first[id] && out.event_count<4,51);
 struct Event* e=&out.events[out.event_count++];e->pc=(u32)__builtin_return_address(0);e->owner=id;e->pointer_match=pointer==locked[id];
 e->result=((u32(WIN*)(void*,void*))unlock_target)(receiver,pointer);return e->result;
}
static void map_original(void){
 HANDLE file=CreateFileA("original.exe",0x80000000,1,0,3,0,0);require(file!=(HANDLE)-1,40);u32 size=GetFileSize(file,0),n;
 require(size>=4096 && size<=16*1024*1024,41);u8* raw=VirtualAlloc(0,size,0x3000,4);require(raw && ReadFile(file,raw,size,&n,0) && n==size,42);CloseHandle(file);
 u32 pe=*(u32*)(raw+60),opt=pe+24;require(pe<size-256 && *(u32*)(raw+pe)==0x4550,43);u32 length=*(u32*)(raw+opt+56),protection;
 require(*(u32*)(raw+opt+28)==0x400000 && length<=32*1024*1024 && VirtualProtect((void*)0x400000,length,0x40,&protection),44);
 u32 count=*(u16*)(raw+pe+6),section=opt+*(u16*)(raw+pe+20);require(section+count*40<=size,46);
 for(u32 i=0;i<count;++i){u32* a=(u32*)(raw+section+i*40);u32 rva=a[3],bytes=a[4],offset=a[5];require(offset<=size && bytes<=size-offset && rva<=length && bytes<=length-rva,47);for(u32 j=0;j<bytes;++j)((u8*)0x400000)[rva+j]=raw[offset+j];}
}
static void pixels(void* surface,u16* data,u32 w,u32 h,int write,u32 seed){
 u32 d[27];clear(d,108);d[0]=108;require(!((u32(WIN*)(void*,void*,void*,u32,void*))table(surface)[25])(surface,0,d,1,0),60);
 require(d[2]==h && d[3]==w && d[21]==16 && d[4]>=w*2 && w*h<=128,61);
 locked[surface==second[0]?0:1]=(void*)d[9];
 out.pitches[surface==second[0]?0:1]=d[4];
 for(u32 y=0;y<h;++y)for(u32 x=0;x<w;++x){u16* p=(u16*)((u8*)d[9]+y*d[4])+x;if(write)*p=(u16)(seed+(y*w+x)*317);data[y*w+x]=*p;}
 require(!((u32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)d[9]),62);
}
void start(void){
 map_original();HANDLE lib=LoadLibraryA("ddraw.dll");require(lib!=0,10);void* dd=0;
 typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");require(create && !create(0,&dd,0),11);
 static const u8 iid[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};void* dd2=0;
 require(!((u32(WIN*)(void*,const void*,void**))table(dd)[0])(dd,iid,&dd2) && dd2,12);release(dd);dd=dd2;
 require(!((u32(WIN*)(void*,void*,u32))table(dd)[20])(dd,0,8),13);
 HANDLE output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);require(output!=(HANDLE)-1,14);
 for(u32 kind=0;kind<2;++kind)for(u32 format=0;format<2;++format)for(u32 wi=0;wi<4;++wi)for(u32 hi=0;hi<2;++hi)for(u32 early=0;early<2;++early){
  const u32 widths[4]={1,3,7,16};u32 w=widths[wi],h=hi?5:1;clear(&out,sizeof(out));out.kind=kind;out.format=format;out.width=w;out.height=h;out.early=early;
  u32 wrappers[2][16];clear(wrappers,sizeof(wrappers));
  for(u32 i=0;i<2;++i){
   u32 d[27];clear(d,108);d[0]=108;d[1]=0x1007;d[2]=h;d[3]=w;d[18]=32;d[19]=0x40;d[21]=16;d[22]=format?0x7c00:0xf800;d[23]=format?0x3e0:0x7e0;d[24]=31;d[26]=0x840;
   *(u32*)0x6f98c8=(u32)dd;*(u32*)0x6f8154=0;*(u32*)0x6e1f1c=0;*(u32*)0x6de6dd=0;
   typedef void(__attribute__((thiscall))*Factory)(void*,void*,u32,u32);((Factory)0x58ad90)(wrappers[i],d,0,0);
   first[i]=(void*)wrappers[i][1];second[i]=(void*)wrappers[i][2];require(first[i] && second[i],15);
   out.distinct_interfaces+=first[i]!=second[i];saved[i]=table(first[i]);
   if(!i)unlock_target=saved[i][32];else require(unlock_target==saved[i][32],16);
   pixels(second[i],out.before[i],w,h,1,0x1234+i*593);
  }
  u32 info[7];char module[260];clear(module,260);require(VirtualQuery(unlock_target,info,28)==28 && GetModuleFileNameA((HANDLE)info[1],module,260),17);
  out.target_rva=(u32)unlock_target-info[1];out.target_module_is_ddraw=info[1]==(u32)lib;require(out.target_module_is_ddraw,18);
  for(u32 i=0;i<2;++i){for(u32 j=0;j<33;++j)shadow[i][j]=saved[i][j];shadow[i][32]=(void*)unlock;*(void***)first[i]=shadow[i];}
  *(u32*)0x6e1f1c=early;*(u32*)0x6e1f88=format;
  if(!kind){u32 rect[4]={0,0,w,1};typedef u32(__attribute__((thiscall))*Copy)(void*,void*,void*,u32);out.result=((Copy)0x58d3b0)(wrappers[0],wrappers[1],rect,0);}
  else {typedef u32(__attribute__((thiscall))*Brighten)(void*);out.result=((Brighten)0x58d770)(wrappers[0]);}
  for(u32 i=0;i<2;++i)*(void***)first[i]=saved[i];
  for(u32 i=0;i<2;++i)pixels(second[i],out.after[i],w,h,0,0);
  u32 n;require(WriteFile(output,&out,sizeof(out),&n,0) && n==sizeof(out),19);
  for(u32 i=0;i<2;++i){release(second[i]);release(first[i]);}
 }
 CloseHandle(output);release(dd);ExitProcess(0);
}
