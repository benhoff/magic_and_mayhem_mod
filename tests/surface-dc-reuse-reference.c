/* Real Surface2 GetDC/GDI/ReleaseDC raster and bounded admission observations. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
typedef i32(WIN*Stretch)(void*,i32,i32,i32,i32,i32,i32,i32,i32,const void*,const void*,u32,u32);
typedef int(WIN*Flush)(void);
typedef void*(WIN*CurrentObject)(void*,u32);
typedef int(WIN*GetObject)(void*,i32,void*);
typedef u32(WIN*GetPixel)(void*,i32,i32);
static u8 info[1064],pixels[2*1024*1024];
static u32 native[48],dcRgb[48],colors[256];
static void** vt(void* p){return *(void***)p;}
static void zero(void* p,u32 n){u8* q=p;while(n--)*q++=0;}
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void read(HANDLE f,void* p,u32 n){u32 got=0;require(ReadFile(f,p,n,&got,0)&&got==n,90);}
static void write(HANDLE f,const void* p,u32 n){u32 got=0;require(WriteFile(f,p,n,&got,0)&&got==n,91);}
static void release(void* p){((u32(WIN*)(void*))vt(p)[2])(p);}
static u32 lock(void* s,u32* d){zero(d,108);d[0]=108;return ((u32(WIN*)(void*,void*,void*,u32,void*))vt(s)[25])(s,0,d,1,0);}
static u32 unlock(void* s){return ((u32(WIN*)(void*,void*))vt(s)[32])(s,0);}
static u32 getdc(void* s,void** dc){return ((u32(WIN*)(void*,void**))vt(s)[17])(s,dc);}
static u32 releasedc(void* s,void* dc){return ((u32(WIN*)(void*,void*))vt(s)[26])(s,dc);}
static u32 output_state(void* p){return p==(void*)0xabababab?0:p?2:1;}
typedef void*(WIN*Region)(i32,i32,i32,i32);
typedef int(WIN*SelectRegion)(void*,void*);
typedef int(WIN*Combine)(void*,void*,void*,i32);
typedef int(WIN*Delete)(void*);
typedef int(WIN*ClipBox)(void*,void*);
typedef u32(WIN*ColorTable)(void*,u32,u32,void*);
static u32 palette_color(u32 i,u32 style){
    if(style==1)return i|(i<<8)|(i<<16);
    if(style==2){i=255-i;return i|(i<<8)|(i<<16);}
    u32 r=((i>>5)&7)*255/7,g=((i>>2)&7)*255/7,b=(i&3)*85;
    if(i==255){r=0;g=0;b=0;}
    return r|(g<<8)|(b<<16);
}
typedef u32(WIN*SetTable)(void*,u32,u32,const void*);
static void* make(void* dd,u32 bits){
 u32 d[27];zero(d,sizeof(d));d[0]=108;d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=bits==8?0x60:0x40;d[21]=bits;d[22]=bits==16?0xf800:bits==32?0xff0000:0;d[23]=bits==16?0x7e0:bits==32?0xff00:0;d[24]=bits==16?0x1f:bits==32?0xff:0;d[26]=0x840;void* s=0;
 require(((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&s,0)==0&&s,40);
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;require(((u32(WIN*)(void*,const void*,void**))vt(s)[0])(s,iid,&second)==0&&second,41);release(s);return second;
}
static u32 bind(void* s,void* palette){return ((u32(WIN*)(void*,void*))vt(s)[31])(s,palette);}
static u32 update(void* palette){u32 entries[32];for(u32 i=0;i<32;++i)entries[i]=palette_color(i+64,2);return ((u32(WIN*)(void*,u32,u32,u32,void*))vt(palette)[6])(palette,0,64,32,entries);}
void start(void){
 void* lib=LoadLibraryA("ddraw.dll"),*gdi=LoadLibraryA("gdi32.dll");require(lib&&gdi,10);
 typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");
 Stretch stretch=(Stretch)GetProcAddress(gdi,"StretchDIBits");Flush flush=(Flush)GetProcAddress(gdi,"GdiFlush");GetPixel pixel=(GetPixel)GetProcAddress(gdi,"GetPixel");ColorTable table=(ColorTable)GetProcAddress(gdi,"GetDIBColorTable");
 Region region=(Region)GetProcAddress(gdi,"CreateRectRgn");SelectRegion select=(SelectRegion)GetProcAddress(gdi,"SelectClipRgn");Delete del=(Delete)GetProcAddress(gdi,"DeleteObject");ClipBox box=(ClipBox)GetProcAddress(gdi,"GetClipBox");SetTable settable=(SetTable)GetProcAddress(gdi,"SetDIBColorTable");
 require(create&&stretch&&flush&&pixel&&table&&region&&select&&del&&box&&settable,11);
 void* dd=0;require(create(0,&dd,0)==0&&dd,12);require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(input!=(HANDLE)-1&&output!=(HANDLE)-1,14);
 // Separate preflight read: environment color-table input, no raster operation.
 void* preflight=make(dd,8),*dc=0;require(getdc(preflight,&dc)==0&&dc,42);require(table(dc,0,256,colors)==256,43);write(output,colors,sizeof(colors));require(releasedc(preflight,dc)==0,44);release(preflight);
 u32 count=0;read(input,&count,4);require(count>0&&count<=256,15);
 for(u32 i=0;i<count;++i){
  u32 c[12];read(input,c,48);require(c[1]==8&&c[2]==6&&(c[8]==8||c[8]==16||c[8]==32)&&c[5]>=40&&c[5]<=1064&&c[6]<=sizeof(pixels)&&c[9]<=1&&c[10]<=6&&c[11]<=1,16);read(input,info,c[5]);read(input,pixels,c[6]);
  void* s=make(dd,c[8]),*palettes[2]={0,0};
  if(c[8]==8){for(u32 j=0;j<2;++j){u32 entries[256];for(u32 k=0;k<256;++k)entries[k]=palette_color(k,j?3:1);require(((u32(WIN*)(void*,u32,void*,void**,void*))vt(dd)[5])(dd,0x44,entries,&palettes[j],0)==0&&palettes[j],45);}if(c[9])require(bind(s,palettes[0])==0,46);}
  u32 d[27];require(lock(s,d)==0&&d[21]==c[8]&&d[3]==8&&d[2]==6&&(i32)d[4]>=(i32)(8*(c[8]/8)),47);
  const u32 initial=c[8]==8?19:c[8]==16?0x2bab:0x556677;
  for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u8* q=(u8*)d[9]+y*d[4]+x*(c[8]/8);for(u32 b=0;b<c[8]/8;++b)q[b]=(u8)(initial>>(b*8));}require(unlock(s)==0,48);
  void* first=0;
  for(u32 phase=0;phase<3;++phase){
   u32 r[24];for(u32 j=0;j<24;++j)r[j]=0xffffffff;r[0]=c[0];r[1]=phase;
   if(phase==1){if(c[10]==1)r[9]=update(palettes[0]);if(c[10]==2)r[9]=bind(s,palettes[1]);if(c[10]==3)r[9]=bind(s,0);}
   dc=(void*)0xabababab;r[2]=getdc(s,&dc);r[3]=output_state(dc);require(!r[2]&&r[3]==2,49);if(!phase)first=dc;r[4]=dc==first;
   zero(colors,sizeof(colors));r[5]=table(dc,0,256,colors);write(output,colors,sizeof(colors));r[6]=(u32)box(dc,&r[20]);
   if(phase==0){
    if(c[10]==4){u32 changed[32];for(u32 j=0;j<32;++j){u32 k=j+64;changed[j]=((255-k)<<16)|(((k*5)&255)<<8)|((k*7)&255);}r[9]=settable(dc,64,32,changed);require(r[9]==32,50);}
    if(c[10]==5)r[9]=bind(s,palettes[1]);if(c[10]==6)r[9]=update(palettes[0]);
    if(c[11]){void* clip=region(2,1,6,5);require(clip!=0,51);r[7]=(u32)select(dc,clip);require(r[7]&&del(clip),52);}
   }
   if(phase==2){r[7]=(u32)select(dc,0);require(r[7],53);}
   zero(colors,sizeof(colors));r[8]=table(dc,0,256,colors);write(output,colors,sizeof(colors));
   r[10]=(u32)stretch(dc,0,0,c[3],c[4],0,0,c[3],c[4],pixels,info,c[7],0xcc0020);r[11]=(u32)flush();require(r[11],54);
   for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)dcRgb[y*8+x]=pixel(dc,x,y);
   r[12]=releasedc(s,dc);require(!r[12],55);r[13]=lock(s,d);require(!r[13]&&d[21]==c[8]&&d[3]==8&&d[2]==6&&(i32)d[4]>=(i32)(8*(c[8]/8)),56);r[14]=d[21];r[15]=d[4];r[16]=d[22];r[17]=d[23];r[18]=d[24];
   for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){const u8* q=(const u8*)d[9]+y*d[4]+x*(c[8]/8);u32 v=0;for(u32 b=0;b<c[8]/8;++b)v|=(u32)q[b]<<(b*8);native[y*8+x]=v;}
   r[19]=unlock(s);require(!r[19],57);write(output,r,sizeof(r));write(output,dcRgb,sizeof(dcRgb));write(output,native,sizeof(native));
  }
  release(s);if(palettes[0])release(palettes[0]);if(palettes[1])release(palettes[1]);
 }
 release(dd);u32 end=0x52554331;write(output,&end,4);require(CloseHandle(input)&&CloseHandle(output),58);ExitProcess(0);
}
