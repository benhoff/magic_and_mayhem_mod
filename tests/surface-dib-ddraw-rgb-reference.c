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
static u32 native[2048*2048],dcRgb[2048*2048];
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
void start(void){
    void* lib=LoadLibraryA("ddraw.dll"),*gdi=LoadLibraryA("gdi32.dll");require(lib&&gdi,10);
    typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");
    Stretch stretch=(Stretch)GetProcAddress(gdi,"StretchDIBits");Flush flush=(Flush)GetProcAddress(gdi,"GdiFlush");
    CurrentObject object=(CurrentObject)GetProcAddress(gdi,"GetCurrentObject");GetObject inspect=(GetObject)GetProcAddress(gdi,"GetObjectA");GetPixel pixel=(GetPixel)GetProcAddress(gdi,"GetPixel");require(create&&stretch&&flush&&object&&inspect&&pixel,11);
    void* dd=0;require(create(0,&dd,0)==0&&dd,12);require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,0,8)==0,13);
    HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);require(input!=(HANDLE)-1&&output!=(HANDLE)-1,14);u32 count=0;read(input,&count,4);require(count>0&&count<=128,15);
    for(u32 i=0;i<count;++i){
        /* id,width,height,source width/height,info length,pixels length,usage,bits,caps */
        u32 c[10];read(input,c,40);require(c[1]>0&&c[1]<=2048&&c[2]>0&&c[2]<=2048&&(c[8]==16||c[8]==32)&&c[5]>=40&&c[5]<=1064&&c[6]<=sizeof(pixels),16);read(input,info,c[5]);read(input,pixels,c[6]);
        u32 record[32];zero(record,sizeof(record));record[0]=c[0];u32 d[27];zero(d,sizeof(d));d[0]=108;d[1]=0x1007;d[2]=c[2];d[3]=c[1];d[18]=32;d[19]=0x40;d[21]=c[8];d[22]=c[8]==16?0xf800:0xff0000;d[23]=c[8]==16?0x7e0:0xff00;d[24]=c[8]==16?0x1f:0xff;d[26]=c[9];void* s=0;
        record[1]=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&s,0);require(record[1]==0&&s,17);
        static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;require(((u32(WIN*)(void*,const void*,void**))vt(s)[0])(s,iid,&second)==0&&second,18);release(s);s=second;
        record[3]=lock(s,d);require(record[3]==0&&d[21]==c[8]&&d[3]==c[1]&&d[2]==c[2]&&(i32)d[4]>=(i32)(c[1]*(c[8]/8)),19);record[2]=d[26];
        for(u32 y=0;y<c[2];++y)for(u32 x=0;x<c[1];++x){u8* p=(u8*)d[9]+y*d[4]+x*(c[8]/8);u32 value=c[8]==16?0x2bab:0x556677;for(u32 b=0;b<c[8]/8;++b)p[b]=(u8)(value>>(b*8));}
        void* locked=(void*)0xabababab;record[4]=getdc(s,&locked);record[5]=output_state(locked);if(!record[4])require(releasedc(s,locked)==0,20);record[6]=unlock(s);require(record[6]==0,21);
        void* dc=(void*)0xabababab;record[7]=getdc(s,&dc);record[8]=output_state(dc);record[12]=0xffffffff;record[9]=record[11]=record[18]=record[19]=0xffffffff;
        if(!record[7]){
            require(dc&&dc!=(void*)0xabababab,22);void* repeated=(void*)0xabababab;record[9]=getdc(s,&repeated);record[10]=output_state(repeated);if(!record[9]&&repeated!=dc)require(releasedc(s,repeated)==0,23);
            u32 busy[27];record[11]=lock(s,busy);if(!record[11])require(unlock(s)==0,24);
            record[12]=(u32)stretch(dc,0,0,c[3],c[4],0,0,c[3],c[4],pixels,info,c[7],0xcc0020);record[13]=(u32)flush();require(record[13],25);
            u32 bitmap[6];zero(bitmap,sizeof(bitmap));record[14]=(u32)inspect(object(dc,7),24,bitmap);record[15]=bitmap[1];record[16]=bitmap[2];record[17]=bitmap[4]>>16;
            record[29]=pixel(dc,0,0);record[30]=pixel(dc,c[1]/2,c[2]/2);record[31]=pixel(dc,c[1]-1,c[2]-1);
            for(u32 y=0;y<c[2];++y)for(u32 x=0;x<c[1];++x){dcRgb[y*c[1]+x]=pixel(dc,x,y);require(dcRgb[y*c[1]+x]!=0xffffffff,30);}
            record[18]=releasedc(s,dc);require(record[18]==0,26);record[19]=releasedc(s,dc);
        }
        record[20]=lock(s,d);require(record[20]==0&&d[21]==c[8]&&d[3]==c[1]&&d[2]==c[2]&&(i32)d[4]>=(i32)(c[1]*(c[8]/8)),27);record[21]=d[21];record[22]=d[3];record[23]=d[2];record[24]=d[4];record[25]=d[22];record[26]=d[23];record[27]=d[24];
        for(u32 y=0;y<c[2];++y)for(u32 x=0;x<c[1];++x){const u8* p=(const u8*)d[9]+y*d[4]+x*(c[8]/8);u32 value=0;for(u32 b=0;b<c[8]/8;++b)value|=(u32)p[b]<<(b*8);native[y*c[1]+x]=value;}
        record[28]=unlock(s);require(record[28]==0,28);write(output,record,sizeof(record));write(output,native,c[1]*c[2]*4);write(output,dcRgb,c[1]*c[2]*4);release(s);
    }
    release(dd);u32 end=0x44444331;write(output,&end,4);require(CloseHandle(input)&&CloseHandle(output),29);ExitProcess(0);
}
