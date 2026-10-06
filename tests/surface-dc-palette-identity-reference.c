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
static u32 native[48],dcBefore[48],dcClipped[48],dcRgb[48],colors[256];
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
void start(void){
    void* lib=LoadLibraryA("ddraw.dll"),*gdi=LoadLibraryA("gdi32.dll");require(lib&&gdi,10);
    typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");
    Stretch stretch=(Stretch)GetProcAddress(gdi,"StretchDIBits");Flush flush=(Flush)GetProcAddress(gdi,"GdiFlush");
    GetPixel pixel=(GetPixel)GetProcAddress(gdi,"GetPixel");ColorTable table=(ColorTable)GetProcAddress(gdi,"GetDIBColorTable");
    Region region=(Region)GetProcAddress(gdi,"CreateRectRgn");SelectRegion select=(SelectRegion)GetProcAddress(gdi,"SelectClipRgn");
    Combine combine=(Combine)GetProcAddress(gdi,"CombineRgn");Delete del=(Delete)GetProcAddress(gdi,"DeleteObject");ClipBox box=(ClipBox)GetProcAddress(gdi,"GetClipBox");
    require(create&&stretch&&flush&&pixel&&table&&region&&select&&combine&&del&&box,11);
    void* dd=0;require(create(0,&dd,0)==0&&dd,12);require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,13);
    HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(input!=(HANDLE)-1&&output!=(HANDLE)-1,14);u32 count=0;read(input,&count,4);require(count>0&&count<=512,15);
    for(u32 i=0;i<count;++i){
        u32 c[12];read(input,c,48);require(c[1]==8&&c[2]==6&&(c[8]==8||c[8]==16||c[8]==32)&&c[5]>=40&&c[5]<=1064&&c[6]<=sizeof(pixels)&&c[9]<=4&&c[10]<=4&&c[11]<=4,16);read(input,info,c[5]);read(input,pixels,c[6]);
        u32 record[32];for(u32 j=0;j<32;++j)record[j]=0xffffffff;record[0]=c[0];u32 d[27];zero(d,sizeof(d));d[0]=108;d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=c[8]==8?0x60:0x40;d[21]=c[8];d[22]=c[8]==16?0xf800:c[8]==32?0xff0000:0;d[23]=c[8]==16?0x7e0:c[8]==32?0xff00:0;d[24]=c[8]==16?0x1f:c[8]==32?0xff:0;d[26]=0x840;void* s=0;
        record[1]=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&s,0);require(!record[1]&&s,17);
        static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;require(((u32(WIN*)(void*,const void*,void**))vt(s)[0])(s,iid,&second)==0&&second,18);release(s);s=second;
        void* palette=0,*clipper=0;
        if(c[8]==8&&c[9]){
            u32 entries[256];for(u32 j=0;j<256;++j){if(c[9]==4){require(c[5]==1064,36);const u8* q=info+40+j*4;entries[j]=(u32)q[2]|((u32)q[1]<<8)|((u32)q[0]<<16);}else entries[j]=palette_color(j,c[9]);}
            record[4]=((u32(WIN*)(void*,u32,void*,void**,void*))vt(dd)[5])(dd,0x44,entries,&palette,0);require(!record[4]&&palette,19);
            record[3]=((u32(WIN*)(void*,void*))vt(s)[31])(s,palette);require(!record[3],20);
        }
        if(c[10]){
            record[5]=((u32(WIN*)(void*,u32,void**,void*))vt(dd)[4])(dd,0,&clipper,0);require(!record[5]&&clipper,21);
            if(c[10]!=3){
                u32 r[16]={32,1,1,16,2,1,6,5,2,1,6,5,0,0,0,0};
                if(c[10]==2){u32 two[16]={32,1,2,32,0,0,8,6,0,0,3,2,5,3,8,6};for(u32 j=0;j<16;++j)r[j]=two[j];}
                if(c[10]==4){zero(r,sizeof(r));r[0]=32;r[1]=1;}
                record[6]=((u32(WIN*)(void*,void*,u32))vt(clipper)[7])(clipper,r,0);require(!record[6],22);
            }
            record[7]=((u32(WIN*)(void*,void*))vt(s)[28])(s,clipper);require(!record[7],23);
        }
        record[30]=lock(s,d);require(!record[30]&&d[21]==c[8]&&d[3]==8&&d[2]==6&&(i32)d[4]>=(i32)(8*(c[8]/8)),24);record[2]=d[26];
        u32 initial=c[8]==8?19:c[8]==16?0x2bab:0x556677;
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u8* p=(u8*)d[9]+y*d[4]+x*(c[8]/8);for(u32 b=0;b<c[8]/8;++b)p[b]=(u8)(initial>>(b*8));}
        record[31]=unlock(s);require(!record[31],25);
        void* dc=(void*)0xabababab;record[8]=getdc(s,&dc);record[9]=output_state(dc);require(!record[8]&&record[9]==2,26);
        zero(colors,sizeof(colors));record[10]=table(dc,0,256,colors);record[11]=(u32)box(dc,&record[12]);
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)dcBefore[y*8+x]=pixel(dc,x,y);
        if(c[11]){
            void* r=c[11]==3?region(0,0,0,0):c[11]==2?region(0,0,3,2):region(2,1,6,5);require(r!=0,27);
            if(c[11]==2||c[11]==4){void* t=c[11]==2?region(5,3,8,6):region(4,0,8,3);require(t&&combine(r,r,t,2)!=0&&del(t),28);}
            record[16]=(u32)select(dc,r);require(record[16]&&del(r),29);
        }
        record[17]=(u32)stretch(dc,0,0,c[3],c[4],0,0,c[3],c[4],pixels,info,c[7],0xcc0020);record[18]=(u32)flush();require(record[18],30);
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)dcClipped[y*8+x]=pixel(dc,x,y);
        record[19]=(u32)select(dc,0);require(record[19],31);
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)dcRgb[y*8+x]=pixel(dc,x,y);
        record[20]=releasedc(s,dc);require(!record[20],32);
        record[21]=lock(s,d);require(!record[21]&&d[21]==c[8]&&d[3]==8&&d[2]==6&&(i32)d[4]>=(i32)(8*(c[8]/8)),33);record[22]=d[21];record[23]=d[3];record[24]=d[2];record[25]=d[4];record[26]=d[22];record[27]=d[23];record[28]=d[24];
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){const u8* p=(const u8*)d[9]+y*d[4]+x*(c[8]/8);u32 v=0;for(u32 b=0;b<c[8]/8;++b)v|=(u32)p[b]<<(b*8);native[y*8+x]=v;}
        record[29]=unlock(s);require(!record[29],34);write(output,record,sizeof(record));write(output,colors,sizeof(colors));write(output,dcBefore,sizeof(dcBefore));write(output,dcClipped,sizeof(dcClipped));write(output,dcRgb,sizeof(dcRgb));write(output,native,sizeof(native));release(s);if(palette)release(palette);if(clipper)release(clipper);
    }
    release(dd);u32 end=0x50444331;write(output,&end,4);require(CloseHandle(input)&&CloseHandle(output),35);ExitProcess(0);
}
