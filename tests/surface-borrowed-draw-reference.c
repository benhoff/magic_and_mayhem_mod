/* Real Surface2 draw admission with owned lock/DC operands. */
#define start unused_access_probe_start
#include "surface-access-reference.c"
#undef start
static void pixels(void* s,u32* words,int initialize,int source){u32 d[27];require(lock(s,d)==0,30);for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u32 i=y*8+x;u16* p=(u16*)((u8*)d[9]+y*d[4])+x;if(initialize)*p=(u16)(source?(i%3==0?0x8000:0x1234+i*73):0xa000+i);words[i]=*p;}require(unlock(s)==0,31);}
void start(void){
 void* lib=LoadLibraryA("ddraw.dll");typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");void* dd=0;require(create&&create(0,&dd,0)==0&&dd,10);require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,11);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(input!=(HANDLE)-1&&output!=(HANDLE)-1,12);u32 count;io(input,&count,4,0);require(count==180,13);
 for(u32 i=0;i<count;++i){u32 c[4];io(input,c,sizeof(c),0);require(c[0]==i&&c[1]<5&&c[2]<6&&c[3]<6,14);void* s=surface(dd,16,0x840);void* d=surface(dd,16,0x840);u32 beforeS[48],beforeD[48],afterS[48],afterD[48];pixels(s,beforeS,1,1);pixels(d,beforeD,1,0);u32 key[2]={0x8000,0x8000};require(((u32(WIN*)(void*,u32,void*))vt(s)[29])(s,8,key)==0,15);void* clip=0;
  if(c[3]){require(((u32(WIN*)(void*,u32,void**,void*))vt(dd)[4])(dd,0,&clip,0)==0,16);if(c[3]!=3){u32 region[16]={32,1,1,16,2,1,6,5,2,1,6,5,0,0,0,0};if(c[3]==2){u32 two[16]={32,1,2,32,0,0,8,6,0,0,3,2,5,3,8,6};for(u32 j=0;j<16;++j)region[j]=two[j];}if(c[3]==4){for(u32 j=0;j<16;++j)region[j]=0;region[0]=32;region[1]=1;}if(c[3]==5){u32 two[16]={32,1,2,32,0,0,8,6,0,0,8,3,0,4,8,6};for(u32 j=0;j<16;++j)region[j]=two[j];}require(((u32(WIN*)(void*,void*,u32))vt(clip)[7])(clip,region,0)==0,17);}require(((u32(WIN*)(void*,void*))vt(d)[28])(d,clip)==0,18);}
  u32 desc[27];void* dc=0;if(c[2]==1)require(lock(s,desc)==0,19);if(c[2]==2||c[2]==5)require(lock(d,desc)==0,20);if(c[2]==3)require(getdc(s,&dc)==0,21);if(c[2]==4||c[2]==5)require(getdc(d,&dc)==0,22);
  i32 r[4]={0,0,8,6};u32 result,flags=0;u32 effect[25];for(u32 j=0;j<25;++j)effect[j]=0;effect[0]=100;effect[20]=0xffff;
  if(c[1]==0){flags=0x400;result=((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(d)[5])(d,r,0,0,flags,effect);}
  else if(c[1]<3){flags=c[1]==2?0x8000:0;result=((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(d)[5])(d,r,s,r,flags,0);}
  else{flags=c[1]==4?1:0;result=((u32(WIN*)(void*,u32,u32,void*,void*,u32))vt(d)[7])(d,0,0,s,r,flags);}
  if(c[2]==3)require(releasedc(s,dc)==0,23);if(c[2]==4||c[2]==5)require(releasedc(d,dc)==0,24);if(c[2]==1)require(unlock(s)==0,25);if(c[2]==2||c[2]==5)require(unlock(d)==0,26);
  pixels(s,afterS,0,1);pixels(d,afterD,0,0);u32 row[6]={c[0],c[1],c[2],c[3],flags,result};io(output,row,sizeof(row),1);io(output,beforeS,sizeof(beforeS),1);io(output,beforeD,sizeof(beforeD),1);io(output,afterS,sizeof(afterS),1);io(output,afterD,sizeof(afterD),1);if(clip)release(clip);release(s);release(d);
 }
 release(dd);u32 end=0x42524431;io(output,&end,4,1);require(CloseHandle(input)&&CloseHandle(output),27);ExitProcess(0);
}
