/* Diagnostic observation only. This file is never a native raster input. */
#include "../shadow/win32_min.h"
static HANDLE file;
static u32 calls,tokens[16],used;
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
void world_lifetime_close(void){if(file){CloseHandle(file);file=0;}}
void world_lifetime_observe(u32* registers){
    char path[260],enabled[2];u32 error=GetLastError(),written;
    if(calls>=256||!GetEnvironmentVariableA("MNM_SCENE_LIFETIME",enabled,2))goto done;
    ++calls;
    if((u32)GetModuleHandleA(0)!=0x400000)goto done;
    if(!file){u32 n=GetEnvironmentVariableA("MNM_SCENE_DIR",path,220);if(!n||n>=220)goto done;
        const char* suffix="\\canvas-lifetime.bin";while(*suffix)path[n++]=*suffix++;path[n]=0;
        file=CreateFileA(path,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1){file=0;goto done;}
        u32 header[16]={0};header[0]=0x434d4e4d;header[1]=0x3146494c;header[2]=1;header[3]=64;header[4]=64;header[5]=256;
        if(!WriteFile(file,header,64,&written,0)||written!=64){world_lifetime_close();goto done;}
    }
    u32 record[16]={0},queue=registers[6];record[0]=calls;
    if(readable(queue,24)){record[1]=get((void*)(queue+20));record[2]=get((void*)(queue+8));record[15]=get((void*)(queue+12));}
    u32 canvas=get((void*)0x658174),width=get((void*)0x6a49b8),height=get((void*)0x656618),stride=get((void*)0x6a2dc8);
    record[4]=width;record[5]=height;record[6]=stride;record[8]=canvas;
    record[9]=get((void*)0x689b7c);record[10]=get((void*)0x6c4e3c);record[11]=get((void*)0x6e1f88);
    record[13]=0xffffffff;record[12]=2166136261u;
    if(width&&height&&width<=2048&&height<=2048&&stride>=width&&stride<=4096&&readable(canvas,stride*height*2)){
        for(u32 i=0;i<used;++i)if(tokens[i]==canvas){record[3]=i+1;break;}
        if(!record[3]&&used<16){tokens[used++]=canvas;record[3]=used;}
        for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x){u8* p=(u8*)canvas+(y*stride+x)*2;u32 value=p[0]|(u32)p[1]<<8;
            record[12]=(record[12]^p[0])*16777619u;record[12]=(record[12]^p[1])*16777619u;
            if(value){++record[7];if(record[13]==0xffffffff){record[13]=y*width+x;record[14]=value;}}}
    }else record[7]=0xffffffff;
    if(!WriteFile(file,record,64,&written,0)||written!=64)world_lifetime_close();
done:SetLastError(error);
}
