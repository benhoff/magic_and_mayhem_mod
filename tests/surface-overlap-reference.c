/* Original opaque rectangle wrapper against one real Surface2 backing store.
 * Separate Surface1 source aliases are confirmed using COM IUnknown identity.
 * The destination observer forwards all arguments/results to real Surface2.
 */
#define start unused_key_probe_start
#include "surface-key-reference.c"
#undef start
struct ClipRecord {u32 size,result,data[16];};
struct OverlapOutput {struct Output draw;u32 pointer_equal,identity_equal,source_kind,destination_kind;struct ClipRecord before,after;};
void* memcpy(void* destination,const void* source,u32 length){u8* d=destination;const u8* s=source;for(u32 i=0;i<length;++i)d[i]=s[i];return destination;}
static struct ClipRecord clip_state(void* c){struct ClipRecord r;clear(&r,sizeof(r));if(c){r.size=64;r.result=((i32(WIN*)(void*,void*,void*,u32*))table(c)[3])(c,0,r.data,&r.size);require(r.size<=64,35);}return r;}
static u32 same_identity(void* a,void* b){static const u8 iid[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};void* ia=0;void* ib=0;
 require(((i32(WIN*)(void*,const void*,void**))table(a)[0])(a,iid,&ia)==0 && ia,36);require(((i32(WIN*)(void*,const void*,void**))table(b)[0])(b,iid,&ib)==0 && ib,37);u32 equal=ia==ib;release(ia);release(ib);return equal;}
void start(void){map_original();require(*(u8*)0x58c360==0x83 && *(u8*)0x58c361==0xec,50);
 void* dd=0;HANDLE dll=LoadLibraryA("ddraw.dll");require(dll!=0,10);typedef i32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");require(create && create(0,&dd,0)==0,11);require(((i32(WIN*)(void*,void*,u32))table(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);require(input!=(HANDLE)-1 && output!=(HANDLE)-1,14);u32 n;struct Input c;
 void* proxy_table[8]={0,0,0,0,0,(void*)blt,0,(void*)fast};void** proxy=proxy_table;
 for(;;){require(ReadFile(input,&c,sizeof(c),&n,0),15);if(!n)break;require(n==sizeof(c) && c.entry==0x58c360 && c.mode<=1 && c.phase<=5,16);
  void* d=surface(dd);void* s=d;void* clipper=0;actual_destination=d;clear(&out,sizeof(out));out.input=c;
  pixels(d,out.dest_before,1,0,&c,out.dest_desc);
  if(c.mode){static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};require(((i32(WIN*)(void*,const void*,void**))table(d)[0])(d,iid,&s)==0 && s,38);}
  struct OverlapOutput result;clear(&result,sizeof(result));result.pointer_equal=s==d;result.identity_equal=same_identity(s,d);result.source_kind=c.mode?1:2;result.destination_kind=2;require(result.identity_equal,39);
  pixels(s,out.source_before,0,0,&c,out.source_desc);
  if(c.phase){require(((i32(WIN*)(void*,u32,void**,void*))table(dd)[4])(dd,0,&clipper,0)==0,30);
   if(c.phase!=3){u32 region[16]={32,1,1,16,2,1,6,5,2,1,6,5,0,0,0,0};
    if(c.phase==4){clear(region,64);region[0]=32;region[1]=1;}
    if(c.phase==2){u32 two[16]={32,1,2,32,0,0,8,6,0,0,3,2,5,3,8,6};for(u32 i=0;i<16;++i)region[i]=two[i];}
    if(c.phase==5){u32 two[16]={32,1,2,32,0,0,8,6,0,0,8,3,0,4,8,6};for(u32 i=0;i<16;++i)region[i]=two[i];}
    require(((i32(WIN*)(void*,void*,u32))table(clipper)[7])(clipper,region,0)==0,31);
   }require(((i32(WIN*)(void*,void*))table(d)[28])(d,clipper)==0,34);
  }
  result.before=clip_state(clipper);key(s,out.key_before);
  u32 source[16],target[16];clear(source,sizeof(source));clear(target,sizeof(target));source[2]=(u32)s;target[2]=(u32)&proxy;source[4]=target[4]=8;source[5]=target[5]=6;
  *(u32*)0x6f68d8=c.fast;*(u32*)0x6f98d8=c.no_wait;*(u32*)0x6f98c0=0;
  typedef void(__attribute__((thiscall))*Copy)(void*,void*,struct Rect*,i32,i32);((Copy)c.entry)(source,target,&out.input.src,c.dst.l,c.dst.t);
  pixels(s,out.source_after,0,0,&c,out.source_desc);pixels(d,out.dest_after,0,0,&c,out.dest_desc);key(s,out.key_after);result.after=clip_state(clipper);result.draw=out;
  require(out.trace[0]==1 && WriteFile(output,&result,sizeof(result),&n,0) && n==sizeof(result),17);
  if(c.mode)release(s);release(d);release(clipper);
 }CloseHandle(input);CloseHandle(output);release(dd);ExitProcess(0);
}
