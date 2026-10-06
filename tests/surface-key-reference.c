/* Execute unchanged selected original code against real Wine Surface2 objects.
 * Only destination COM slots are observed; they forward all arguments/results.
 * No original instruction bytes or files are patched. No native renderer here. */
#include "../runtime/shadow/win32_min.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
API HANDLE WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(HANDLE,const char*);
/* Reserve the original VA range at load time so Wine heaps/stacks cannot occupy it.
 * All harness code follows this padding; map_original writes only the reservation. */
__asm__(".section .text$a,\"xr\"\n.globl _original_reservation\n_original_reservation:\n.space 33554432\n");
#pragma code_seg(".text$z")
struct Rect {i32 l,t,r,b;};
struct Input {u32 id,entry,fast,no_wait,pattern,key,mode,phase;struct Rect src,dst;};
struct Output {struct Input input;u32 trace[16],source_desc[27],dest_desc[27];u16 source_before[48],dest_before[48],source_after[48],dest_after[48];u32 key_before[3],key_after[3];};
static struct Output out;
static void* actual_destination;
static void clear(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static void require(int ok,u32 stage){if(!ok)ExitProcess(stage);}
static void** table(void* p){return *(void***)p;}
static void release(void* p){if(p)((u32(WIN*)(void*))table(p)[2])(p);}
static void rect_copy(u32* to,const struct Rect* from){if(from){to[0]=from->l;to[1]=from->t;to[2]=from->r;to[3]=from->b;}}
static i32 WIN blt(void* receiver,struct Rect* d,void* s,struct Rect* r,u32 flags,void* effects){
 (void)receiver;++out.trace[0];out.trace[1]=1;out.trace[2]=flags;rect_copy(out.trace+4,r);rect_copy(out.trace+8,d);out.trace[14]=effects!=0;out.trace[15]=s!=0;
 if(effects){out.trace[12]=((u32*)effects)[0];out.trace[13]=((u32*)effects)[20];}
 i32 h=((i32(WIN*)(void*,void*,void*,void*,u32,void*))table(actual_destination)[5])(actual_destination,d,s,r,flags,effects);out.trace[3]=h;return h;
}
static i32 WIN fast(void* receiver,u32 x,u32 y,void* s,struct Rect* r,u32 flags){
 (void)receiver;++out.trace[0];out.trace[1]=2;out.trace[2]=flags;rect_copy(out.trace+4,r);out.trace[8]=x;out.trace[9]=y;out.trace[15]=s!=0;
 i32 h=((i32(WIN*)(void*,u32,u32,void*,void*,u32))table(actual_destination)[7])(actual_destination,x,y,s,r,flags);out.trace[3]=h;return h;
}
static void key(void* s,u32* k){k[1]=k[2]=0;k[0]=((i32(WIN*)(void*,u32,void*))table(s)[16])(s,8,k+1);}
static void set_key(void* s,u32 value){u32 k[2]={value,value};require(((i32(WIN*)(void*,u32,void*))table(s)[29])(s,8,k)==0,25);}
static void pixels(void* p,u16* data,int write,int dest,const struct Input* c,u32* desc){
 u32 d[27];clear(d,108);d[0]=108;require(((i32(WIN*)(void*,void*,void*,u32,HANDLE))table(p)[25])(p,0,d,1,0)==0,20);
 require(d[2]==6 && d[3]==8 && d[21]==16 && d[22]==0xf800 && d[23]==0x7e0 && d[24]==31 && d[4]>=16,21);
 for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u16* q=(u16*)((u8*)d[9]+y*d[4])+x;u32 i=y*8+x;
  if(write){u16 value=(u16)(c->key^(0x1234+i*73));if(value==c->key || value==(c->key^0xffff))value=(u16)(c->key^1);
   if(c->pattern==1 || (c->pattern==0 && i%3==0))value=(u16)c->key;
   if(c->pattern==0 && i%3==1)value=(u16)(c->key^0xffff);
   *q=dest?(u16)(0xa000+i):value;
  }data[i]=*q;
 }
 for(u32 i=0;i<27;++i)desc[i]=i==9?0:d[i];
 require(((i32(WIN*)(void*,void*))table(p)[32])(p,0)==0,22);
}
static void* surface(void* dd){u32 d[27];void* p=0;clear(d,108);d[0]=108;d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
 require(((i32(WIN*)(void*,void*,void**,void*))table(dd)[6])(dd,d,&p,0)==0,12);
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;
 require(((i32(WIN*)(void*,const void*,void**))table(p)[0])(p,iid,&second)==0 && second,18);release(p);return second;
}
static void map_original(void){HANDLE file=CreateFileA("original.exe",0x80000000,1,0,3,0,0);require(file!=(HANDLE)-1,40);u32 size=GetFileSize(file,0),n;
 require(size>=4096 && size<=16*1024*1024,41);u8* raw=VirtualAlloc((void*)0x28000000,size,0x3000,4);require(raw!=0 && ReadFile(file,raw,size,&n,0) && n==size,42);CloseHandle(file);
 u32 pe=*(u32*)(raw+60);require(pe<size-256 && *(u32*)(raw+pe)==0x4550,43);u32 opt=pe+24;u32 length=*(u32*)(raw+opt+56);
 require(*(u32*)(raw+opt+28)==0x400000 && length<=32*1024*1024,44);u8* mapped=(u8*)0x400000;u32 protection;require(VirtualProtect(mapped,length,0x40,&protection),45);
 u32 count=*(u16*)(raw+pe+6),section=opt+*(u16*)(raw+pe+20);require(section+count*40<=size,46);
 for(u32 i=0;i<count;++i){u32* a=(u32*)(raw+section+i*40);u32 rva=a[3],bytes=a[4],offset=a[5];require(offset<=size && bytes<=size-offset && rva<=length && bytes<=length-rva,47);for(u32 j=0;j<bytes;++j)mapped[rva+j]=raw[offset+j];}
 require(*(u8*)0x58ca90==0x83 && *(u8*)0x58ca91==0xec,48);
}
void start(void){map_original();void* dd=0;HANDLE dll=LoadLibraryA("ddraw.dll");require(dll!=0,10);
 typedef i32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");require(create && create(0,&dd,0)==0,11);require(((i32(WIN*)(void*,void*,u32))table(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);require(input!=(HANDLE)-1 && output!=(HANDLE)-1,14);u32 n;struct Input c;
 void* proxy_table[8]={0,0,0,0,0,(void*)blt,0,(void*)fast};void** proxy=proxy_table;
 for(;;){require(ReadFile(input,&c,sizeof(c),&n,0),15);if(!n)break;require(n==sizeof(c) && c.entry==0x58ca90 && c.mode<=2 && c.phase==0,16);
  void* s=surface(dd);void* d=surface(dd);actual_destination=d;
  clear(&out,sizeof(out));pixels(s,out.source_before,1,0,&c,out.source_desc);pixels(d,out.dest_before,1,1,&c,out.dest_desc);if(c.mode!=1)set_key(s,c.key);
  for(u32 phase=0;phase<(c.mode==2?2u:1u);++phase){clear(&out,sizeof(out));out.input=c;out.input.phase=phase;if(phase)set_key(s,c.key^0xffff);
   pixels(s,out.source_before,0,0,&c,out.source_desc);pixels(d,out.dest_before,0,1,&c,out.dest_desc);key(s,out.key_before);
   u32 source[16],target[16];clear(source,sizeof(source));clear(target,sizeof(target));source[2]=(u32)s;target[2]=(u32)&proxy;source[4]=target[4]=8;source[5]=target[5]=6;
   *(u32*)0x6f68d8=c.fast;*(u32*)0x6f98d8=c.no_wait;*(u32*)0x6f98c0=0;
   typedef void(__attribute__((thiscall))*Copy)(void*,void*,struct Rect*,i32,i32);((Copy)c.entry)(source,target,&out.input.src,c.dst.l,c.dst.t);
   pixels(s,out.source_after,0,0,&c,out.source_desc);pixels(d,out.dest_after,0,1,&c,out.dest_desc);key(s,out.key_after);
   require(out.trace[0]==1 && WriteFile(output,&out,sizeof(out),&n,0) && n==sizeof(out),17);
  }release(d);release(s);
 }CloseHandle(input);CloseHandle(output);release(dd);ExitProcess(0);
}
