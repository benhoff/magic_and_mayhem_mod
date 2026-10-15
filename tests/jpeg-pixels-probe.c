#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetFileSize(HANDLE,u32*);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
typedef u32 (__attribute__((fastcall)) *Jpeg)(const char*,u16*,i32,i32,i32,i32);
typedef u32 (__attribute__((thiscall)) *Wrapper)(void*,const char*);
typedef u32 (WIN *Count)(void);
static volatile u8 reserved_game_globals[4*1024*1024];
static u32 locks,unlocks,wrong_unlock,expected_interface,expected_pixels;
static u32 __attribute__((thiscall)) lock(void* object){++locks;return ((u32*)object)[9];}
static u32 WIN unlock(void* object,void* pixels){++unlocks;if((u32)object!=expected_interface || (u32)pixels!=expected_pixels)++wrong_unlock;return 0x12345678;}
static u32 results[4096][4][8];
static u32 word(const u8* b){return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
void start(void){
 reserved_game_globals[sizeof(reserved_game_globals)-1]=1;
 char path[1024];u32 n=GetEnvironmentVariableA("MNM_JPEG_PIXELS",path,sizeof(path));if(!n || n>=sizeof(path))ExitProcess(150);
 HANDLE f=CreateFileA(path,0x80000000,1,0,3,0x80,0);if(f==(HANDLE)-1)ExitProcess(151);
 u32 high=0,bytes=GetFileSize(f,&high);if(high || bytes<32 || bytes>1024u*1024u*1024u)ExitProcess(152);
 HANDLE mapping=CreateFileMappingA(f,0,2,0,0,0);if(!mapping)ExitProcess(153);const u8* cache=MapViewOfFile(mapping,4,0,0,bytes);if(!cache)ExitProcess(154);
 const u32 count=word(cache+16);if(!count || count>4096)ExitProcess(155);
 void* old=LoadLibraryA("jpeg.dll");if(!old)ExitProcess(156);
 Jpeg reference=(Jpeg)GetProcAddress(old,"?jpeg_read@@YI_NPADPAXHHHH@Z");if(!reference)ExitProcess(157);
 u32 protect,done=0;HANDLE wrapper_file=CreateFileA("jpeg-wrapper.bin",0x80000000,1,0,3,0x80,0);
 if(wrapper_file==(HANDLE)-1 || !VirtualProtect((void*)0x58d000,4096,0x40,&protect) || !VirtualProtect((void*)0x58b000,4096,0x40,&protect) ||
  !ReadFile(wrapper_file,(void*)0x58d320,77,&done,0) || done!=77){ExitProcess(164);}
 CloseHandle(wrapper_file);
 u8* l=(u8*)0x58b660;l[0]=0xe9;*(u32*)(l+1)=(u32)lock-0x58b665;*(u32*)0x5c5308=(u32)reference;
 u8 original_call[6],native_call[6];for(u32 i=0;i<6;++i)original_call[i]=((u8*)0x58d34b)[i];
 void* native=LoadLibraryA("MnmJpegPixels.dll");if(!native)ExitProcess(165);
 Count native_calls=(Count)GetProcAddress(native,"NativeJpegCallsV1"),original_calls=(Count)GetProcAddress(native,"OriginalJpegCallsV1");if(!native_calls || !original_calls)ExitProcess(166);
 for(u32 i=0;i<6;++i)native_call[i]=((u8*)0x58d34b)[i];if(native_call[0]!=0xe8)ExitProcess(167);
 Wrapper wrapper=(Wrapper)0x58d320;
 u16* a=VirtualAlloc(0,20*1024*1024,0x3000,4),*b=VirtualAlloc(0,20*1024*1024,0x3000,4);if(!a || !b)ExitProcess(158);
 u32 at=32;
 for(u32 i=0;i<count;++i){const u8* r=cache+at;const u32 length=word(r+4),w=word(r+8),h=word(r+12);if(length>239 || !w || w>2048 || !h || h>2048)ExitProcess(159);
  char name[240];for(u32 j=0;j<length;++j)name[j]=(char)r[20+j];name[length]=0;
  for(u32 k=0;k<4;++k){const u32 width=k&2?(w>2?w-2:1):w+3,height=k&2?(h>1?h-1:1):h+2,stride=width+5,total=stride*height+64;
   if(total>10*1024*1024)ExitProcess(160);for(u32 j=0;j<total;++j)a[j]=b[j]=(u16)(j*7919+i*43+k);
   u32 table[33];for(u32 j=0;j<33;++j)table[j]=0;table[32]=(u32)unlock;u32 interface=(u32)table;
   u32 object[10];for(u32 j=0;j<10;++j)object[j]=0;object[1]=(u32)&interface;object[4]=width;object[5]=height;object[6]=stride*2;
   expected_interface=(u32)&interface;*(u32*)0x6e1f88=k&1;u32* out=results[i][k];locks=unlocks=wrong_unlock=0;
   for(u32 j=0;j<6;++j)((u8*)0x58d34b)[j]=original_call[j];expected_pixels=object[9]=(u32)(a+32);
   out[0]=wrapper(object,name)&255;if(locks!=1 || unlocks!=1 || wrong_unlock)ExitProcess(168);
   locks=unlocks=wrong_unlock=0;
   for(u32 j=0;j<6;++j)((u8*)0x58d34b)[j]=native_call[j];expected_pixels=object[9]=(u32)(b+32);
   out[1]=wrapper(object,name)&255;if(locks!=1 || unlocks!=1 || wrong_unlock)ExitProcess(169);
   out[2]=total;out[4]=width;out[5]=height;out[6]=stride;out[7]=k&1;
   for(u32 j=0;j<total;++j)if(a[j]!=b[j])++out[3];
  }at+=word(r);
 }
 if(native_calls()!=count*4 || original_calls())ExitProcess(170);
 HANDLE output=CreateFileA("jpeg-results.bin",0x40000000,0,0,1,0x80,0);u32 size=count*4*8*4;done=0;
 if(output==(HANDLE)-1 || !WriteFile(output,results,size,&done,0) || done!=size)ExitProcess(161);
 CloseHandle(output);ExitProcess(0);
}
