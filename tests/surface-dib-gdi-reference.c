/* Independent real GDI raster endpoint; no game code or native decoder. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
typedef void* (WIN *Compatible)(void*);
typedef void* (WIN *DibSection)(void*,const void*,u32,void**,void*,u32);
typedef void* (WIN *Select)(void*,void*);
typedef int (WIN *Delete)(void*);
typedef int (WIN *Stretch)(void*,i32,i32,i32,i32,i32,i32,i32,i32,const void*,const void*,u32,u32);
typedef int (WIN *Flush)(void);
static u8 info[1064],pixels[2*1024*1024];
static void require(int ok){if(!ok)ExitProcess(2);}
static void read(HANDLE f,void* dst,u32 n){u32 got=0;require(ReadFile(f,dst,n,&got,0)&&got==n);}
static void write(HANDLE f,const void* src,u32 n){u32 got=0;require(WriteFile(f,src,n,&got,0)&&got==n);}
void start(void){
    void* library=LoadLibraryA("gdi32.dll");require(library!=0);
    Compatible compatible=(Compatible)GetProcAddress(library,"CreateCompatibleDC");
    DibSection section=(DibSection)GetProcAddress(library,"CreateDIBSection");
    Select select=(Select)GetProcAddress(library,"SelectObject");
    Delete deleteObject=(Delete)GetProcAddress(library,"DeleteObject"),deleteDc=(Delete)GetProcAddress(library,"DeleteDC");
    Stretch stretch=(Stretch)GetProcAddress(library,"StretchDIBits");
    Flush flush=(Flush)GetProcAddress(library,"GdiFlush");
    require(compatible&&section&&select&&deleteObject&&deleteDc&&stretch&&flush);
    HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,2,0,0);
    require(input!=(HANDLE)-1&&output!=(HANDLE)-1);u32 count=0;read(input,&count,4);require(count>0&&count<=128);
    for(u32 i=0;i<count;++i){
        /* id, target width/height, source width/height, info bytes, pixel bytes, usage */
        u32 c[8];read(input,c,32);require(c[1]>0&&c[1]<=2048&&c[2]>0&&c[2]<=2048&&c[5]>=40&&c[5]<=1064&&c[6]<=sizeof(pixels));
        read(input,info,c[5]);read(input,pixels,c[6]);
        i32 header[10]={40,(i32)c[1],-(i32)c[2],32<<16|1,0,0,0,0,0,0};
        void* dc=compatible(0);u32* data=0;require(dc!=0);
        void* bitmap=section(dc,header,0,(void**)&data,0,0);require(bitmap&&data);
        void* previous=select(dc,bitmap);require(previous&&previous!=(void*)-1);
        for(u32 p=0;p<c[1]*c[2];++p)data[p]=0x00556677;
        i32 result=stretch(dc,0,0,c[3],c[4],0,0,c[3],c[4],pixels,info,c[7],0xcc0020);
        require(flush());u32 record[4]={c[0],(u32)result,c[1],c[2]};write(output,record,16);write(output,data,c[1]*c[2]*4);
        require(select(dc,previous)==bitmap);require(deleteObject(bitmap));require(deleteDc(dc));
    }
    u32 end=0x44494231;write(output,&end,4);require(CloseHandle(input));require(CloseHandle(output));ExitProcess(0);
}
