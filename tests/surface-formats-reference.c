/* Standalone Surface2 native words and caller-owned signed/padded rows. */
#define start unused_access_start
#include "surface-access-reference.c"
#undef start
static u32 source_memory[1024],destination_memory[1024];
static const u32 depths[5]={8,16,16,24,32};
static const u32 masks[5][3]={{0,0,0},{0x7c00,0x3e0,31},{0xf800,0x7e0,31},{0xff0000,0xff00,255},{0xff0000,0xff00,255}};
static u32 maximum(u32 f){return f==4?0xffffffff:(1u<<depths[f])-1;}
static u32 basekey(u32 f){return f==0?0x81:f==1?0x9234:f==2?0x9234:f==3?0x901234:0x7f901234;}
static i32 pitch_for(u32 f,u32 layout){u32 tight=7*(depths[f]/8),aligned=(tight+3)&~3u;return layout==0?(i32)aligned:layout==1?(i32)(aligned+4):layout==2?-(i32)(aligned+4):layout==3?(i32)tight:-(i32)tight;}
static u32 value(u32 f,u32 i,int destination){u32 key=basekey(f),v=(0xa0b0c0d0+i*73)&maximum(f);if(!destination){v=(key^(0x1234+i*73))&maximum(f);if(i%4==0)v=key;else if(i%4==1)v=f==0?0x82:f==1?key^0x8000:f==4?key^0xff000000:key^1;}return v;}
static u32 make(void* dd,u32 f,u32 layout,u32* memory,void** result,u32* direct,u32* setting){
 u32 d[27];for(u32 i=0;i<27;++i)d[i]=0;d[0]=108;d[1]=0x1007;d[2]=5;d[3]=7;d[18]=32;d[19]=f==0?0x60:0x40;d[21]=depths[f];for(u32 i=0;i<3;++i)d[22+i]=masks[f][i];d[26]=0x840;
 if(layout){i32 pitch=pitch_for(f,layout);d[1]|=0x808;d[4]=(u32)pitch;d[9]=(u32)((u8*)memory+16+(pitch<0?4*(u32)-pitch:0));}
 void* first=0;u32 h=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&first,0);*direct=h;*setting=0xffffffff;
 if(h && layout){d[1]&=~0x808u;d[4]=d[9]=0;h=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&first,0);if(h)return h;
  static const u8 iid3[16]={0,0x4e,4,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb};void* third=0;require(((u32(WIN*)(void*,const void*,void**))vt(first)[0])(first,iid3,&third)==0&&third,19);
  i32 pitch=pitch_for(f,layout);d[1]=0x80e;d[4]=(u32)pitch;d[9]=(u32)((u8*)memory+16+(pitch<0?4*(u32)-pitch:0));d[26]=0;
  *setting=((u32(WIN*)(void*,void*,u32))vt(third)[39])(third,d,0);release(third);if(*setting){release(first);return *setting;}
 }
 if(h)return h;
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};require(((u32(WIN*)(void*,const void*,void**))vt(first)[0])(first,iid,result)==0&&*result,18);release(first);return 0;
}
static void sample(void* s,u32 f,u32 layout,u32* memory,int initialize,int destination,HANDLE out){
 u32 d[27];require(lock(s,d)==0,30);require(d[2]==5&&d[3]==7&&d[21]==depths[f],31);u32 words[35];i32 pitch=(i32)d[4];require(pitch>=-64&&pitch<=64&&pitch!=0,32);
 for(u32 y=0;y<5;++y)for(u32 x=0;x<7;++x){u8* p=(u8*)d[9]+(i32)y*pitch+x*(depths[f]/8);u32 v=value(f,y*7+x,destination);if(initialize)for(u32 byte=0;byte<depths[f]/8;++byte)p[byte]=(u8)(v>>(byte*8));v=0;for(u32 byte=0;byte<depths[f]/8;++byte)v|=(u32)p[byte]<<(byte*8);words[y*7+x]=v;}
 u32 pointer=d[9];d[9]=0;io(out,d,sizeof(d),1);io(out,words,sizeof(words),1);
 u32 pointer_matches=!layout || pointer==(u32)((u8*)memory+16+(pitch<0?4*(u32)-pitch:0));io(out,&pointer_matches,4,1);
 require(unlock(s)==0,33);
}
static void* clipper(void* dd,void* d,u32 mode){if(!mode)return 0;void* c=0;require(((u32(WIN*)(void*,u32,void**,void*))vt(dd)[4])(dd,0,&c,0)==0,35);if(mode!=4){u32 r[16]={32,1,1,16,1,1,6,4,1,1,6,4,0,0,0,0};if(mode==2){u32 two[16]={32,1,2,32,0,0,7,5,0,0,3,2,4,3,7,5};for(u32 i=0;i<16;++i)r[i]=two[i];}if(mode==3){for(u32 i=0;i<16;++i)r[i]=0;r[0]=32;r[1]=1;}require(((u32(WIN*)(void*,void*,u32))vt(c)[7])(c,r,0)==0,36);}require(((u32(WIN*)(void*,void*))vt(d)[28])(d,c)==0,37);return c;}
void start(void){
 void* lib=LoadLibraryA("ddraw.dll");typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");void* dd=0;require(create&&create(0,&dd,0)==0&&dd,10);void* dd2=0;static const u8 iid_dd2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};require(((u32(WIN*)(void*,const void*,void**))vt(dd)[0])(dd,iid_dd2,&dd2)==0&&dd2,11);release(dd);dd=dd2;require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,11);
 HANDLE in=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),out=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(in!=(HANDLE)-1&&out!=(HANDLE)-1,12);u32 count;io(in,&count,4,0);require(count<=4096,13);
 for(u32 n=0;n<count;++n){u32 c[6];io(in,c,sizeof(c),0);require(c[0]==n&&c[1]<5&&c[2]<5&&c[3]<7&&c[4]<5&&c[5]<4,14);io(out,c,sizeof(c),1);u32 f=c[1],layout=c[2],kind=c[3],mode=c[5];for(u32 i=0;i<1024;++i)source_memory[i]=destination_memory[i]=0xcdcdcdcd;
  void* s=0;void* d=0;u32 creation[2],direct[2],setting[2];creation[0]=make(dd,f,layout,source_memory,&s,&direct[0],&setting[0]);if(kind>=5){creation[1]=creation[0];direct[1]=direct[0];setting[1]=setting[0];}else creation[1]=make(dd,f,layout,destination_memory,&d,&direct[1],&setting[1]);io(out,direct,sizeof(direct),1);io(out,setting,sizeof(setting),1);io(out,creation,sizeof(creation),1);if(creation[0]||creation[1]){if(s)release(s);if(d)release(d);continue;}if(kind>=5)d=s;
  sample(s,f,layout,source_memory,1,0,out);if(d!=s)sample(d,f,layout,destination_memory,1,1,out);else sample(s,f,layout,source_memory,0,0,out);
  void* palette=0;if(f==0){u8 entries[1024];for(u32 i=0;i<256;++i){u32 v=i==0x82?0x81:i;entries[i*4]=(u8)(v*17);entries[i*4+1]=(u8)(v*29+3);entries[i*4+2]=(u8)(v*43+9);entries[i*4+3]=0;}require(((u32(WIN*)(void*,u32,void*,void**,void*))vt(dd)[5])(dd,0x44,entries,&palette,0)==0,40);require(((u32(WIN*)(void*,void*))vt(s)[31])(s,palette)==0,41);if(d!=s)require(((u32(WIN*)(void*,void*))vt(d)[31])(d,palette)==0,42);}
  u32 key=basekey(f);if(mode==3)key&=f==0?255:masks[f][0]|masks[f][1]|masks[f][2];u32 key_results[3]={0xffffffff,0xffffffff,0xffffffff};if(mode!=2){u32 range[2]={key,key};key_results[0]=((u32(WIN*)(void*,u32,void*))vt(s)[29])(s,8,range);if(mode==1){key^=1;range[0]=range[1]=key;key_results[1]=((u32(WIN*)(void*,u32,void*))vt(s)[29])(s,8,range);}}
  u32 range[2]={0xabababab,0xabababab};key_results[2]=((u32(WIN*)(void*,u32,void*))vt(s)[16])(s,8,range);io(out,key_results,sizeof(key_results),1);io(out,range,sizeof(range),1);
  void* clip=clipper(dd,d,c[4]);i32 src[4]={0,0,7,5},dest[4]={0,0,7,5};if(kind>=5){src[2]=6;src[3]=4;dest[0]=dest[1]=1;}u32 flags=kind==0?0x400:(kind==2||kind==6)?0x8000:kind==4?1:0;u32 result;
  if(kind==0){u32 fx[25];for(u32 i=0;i<25;++i)fx[i]=0;fx[0]=100;fx[20]=0xd3e2f197;result=((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(d)[5])(d,dest,0,0,flags,fx);}
  else if(kind==3||kind==4)result=((u32(WIN*)(void*,u32,u32,void*,void*,u32))vt(d)[7])(d,0,0,s,src,flags);
  else result=((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(d)[5])(d,dest,s,src,flags,0);
  u32 draw[2]={flags,result};io(out,draw,sizeof(draw),1);sample(s,f,layout,source_memory,0,0,out);sample(d,f,layout,d==s?source_memory:destination_memory,0,1,out);
  if(layout){io(out,source_memory,256,1);io(out,d==s?source_memory:destination_memory,256,1);}if(clip)release(clip);if(palette)release(palette);if(d!=s)release(d);release(s);
 }
 release(dd);u32 end=0x464d5431;io(out,&end,4,1);require(CloseHandle(in)&&CloseHandle(out),49);ExitProcess(0);
}
