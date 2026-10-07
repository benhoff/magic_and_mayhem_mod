/* Bounded real Surface2 lock/DC transitions; no original game instructions. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
static void** vt(void* p){return *(void***)p;}
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void io(HANDLE f,void* p,u32 n,int out){u32 got=0;require((out?WriteFile(f,p,n,&got,0):ReadFile(f,p,n,&got,0))&&got==n,90);}
static void release(void* p){((u32(WIN*)(void*))vt(p)[2])(p);}
static u32 lock(void* s,u32* d){for(u32 i=0;i<27;++i)d[i]=0xabababab;d[0]=108;return ((u32(WIN*)(void*,void*,void*,u32,void*))vt(s)[25])(s,0,d,1,0);}
static u32 unlock(void* s){return ((u32(WIN*)(void*,void*))vt(s)[32])(s,0);}
static u32 getdc(void* s,void** dc){return ((u32(WIN*)(void*,void**))vt(s)[17])(s,dc);}
static u32 releasedc(void* s,void* dc){return ((u32(WIN*)(void*,void*))vt(s)[26])(s,dc);}
static void* surface(void* dd,u32 bits,u32 caps){
 u32 d[27];for(u32 i=0;i<27;++i)d[i]=0;d[0]=108;d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=bits==8?0x60:0x40;d[21]=bits;d[22]=bits==16?0xf800:bits==32?0xff0000:0;d[23]=bits==16?0x7e0:bits==32?0xff00:0;d[24]=bits==16?0x1f:bits==32?0xff:0;d[26]=caps;void* s=0;
 require(((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&s,0)==0&&s,17);
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;require(((u32(WIN*)(void*,const void*,void**))vt(s)[0])(s,iid,&second)==0&&second,18);release(s);return second;
}
void start(void){
 void* lib=LoadLibraryA("ddraw.dll");require(lib!=0,10);typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");require(create!=0,11);void* dd=0;require(create(0,&dd,0)==0&&dd,12);require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(input!=(HANDLE)-1&&output!=(HANDLE)-1,14);u32 count;io(input,&count,4,0);require(count<=256,15);
 for(u32 i=0;i<count;++i){
  u32 c[4],ops[64],d[27];io(input,c,16,0);require(c[0]==i&&c[3]<=64,16);io(input,ops,c[3]*4,0);void* s=surface(dd,c[1],c[2]);void* other=surface(dd,c[1],c[2]);void* foreign=0;require(getdc(other,&foreign)==0&&foreign,19);void* saved=0;
  require(lock(s,d)==0,20);for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u32 value=(y*8+x)*17+3;u8* p=(u8*)d[9]+y*d[4]+x*(c[1]/8);for(u32 b=0;b<c[1]/8;++b)p[b]=(u8)(value>>(8*b));}require(unlock(s)==0,21);
  for(u32 step=0;step<c[3];++step){u32 row[32];for(u32 j=0;j<32;++j)row[j]=0xffffffff;row[0]=i;row[1]=step;row[2]=ops[step];void* dc=(void*)0xabababab;
   if(ops[step]==0){row[3]=lock(s,d);for(u32 j=0;j<27;++j)row[j+5]=d[j];row[14]=d[9]==0xabababab?0:d[9]?2:1;}
   else if(ops[step]==1)row[3]=unlock(s);
   else if(ops[step]==2){row[3]=getdc(s,&dc);row[4]=dc==(void*)0xabababab?0:dc?2:1;if(!row[3])saved=dc;}
   else if(ops[step]<=5)row[3]=releasedc(s,ops[step]==3?saved:ops[step]==4?0:foreign);
   else require(0,22);
   io(output,row,sizeof(row),1);
  }
  u32 final[28];final[0]=lock(s,d);for(u32 j=0;j<27;++j)final[j+1]=d[j];final[10]=d[9]==0xabababab?0:d[9]?2:1;io(output,final,sizeof(final),1);if(!final[0]){u32 words[48];for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){const u8* p=(const u8*)d[9]+y*d[4]+x*(c[1]/8);u32 value=0;for(u32 b=0;b<c[1]/8;++b)value|=(u32)p[b]<<(8*b);words[y*8+x]=value;}require(unlock(s)==0,24);io(output,words,sizeof(words),1);}require(releasedc(other,foreign)==0,25);release(other);release(s);
 }
 release(dd);u32 end=0x41434331;io(output,&end,4,1);require(CloseHandle(input)&&CloseHandle(output),26);ExitProcess(0);
}
