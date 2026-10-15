/* Source-only native JPEG cache. Pixels are decoded on the native host before
 * launch, then mapped read-only here. No original destination or capture seed. */
#include "../shadow/win32_min.h"
#include "../../reconstruction/rendering/jpeg_pixels.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetFileSize(HANDLE,u32*);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(const void*);
static HANDLE mapping;
static const u8* catalog;
static u32 bytes,count,offsets[4096],root_length,native_calls,forwarded_calls;
static char root[1024];
static volatile u32 owner;
typedef u32 (__attribute__((fastcall)) *Jpeg)(const char*,u16*,i32,i32,i32,i32);
static u32 word(const u8* b){return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static u8 normal(u8 c){return c=='\\'?'/':c>='A'&&c<='Z'?(u8)(c+32):c;}
static int span(u32 at,u32 n,int write){
 if(!at || !n || at>0xffffffffu-n)return 0;u32 end=at+n;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28 || m[4]!=0x1000 || (m[5]&0x101) ||
  !(m[5]&0xee) || (write && !(m[5]&0xcc)) || m[0]>at || !m[3] || m[0]>0xffffffffu-m[3] || m[0]+m[3]<=at){return 0;}
  at=m[0]+m[3];}return 1;
}
static int relative(const char* name,char* out){
 u32 n=0;while(n<1024){if(!span((u32)(name+n),1,0))return 0;if(!name[n])break;++n;}if(n==1024)return 0;
 u32 start=0;if(n>=2 && name[1]==':'){
  if(n<=root_length || normal((u8)name[root_length])!='/')return 0;
  for(u32 i=0;i<root_length;++i)if(normal((u8)name[i])!=normal((u8)root[i]))return 0;start=root_length+1;
 }
 while(n-start>=2 && name[start]=='.' && normal((u8)name[start+1])=='/')start+=2;
 if(n-start>=240)return 0;for(u32 i=start;i<=n;++i)out[i-start]=(char)normal((u8)name[i]);return 1;
}
__declspec(dllexport) u32 __attribute__((fastcall)) NativeJpegPixelsV1(const char* filename,u16* pixels,i32 width,i32 pitch,i32 height,i32 format){
 const u32 error=GetLastError();u32 result=0,thread=GetCurrentThreadId();char name[240];
 if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);
 if(!catalog || thread!=owner || width<1 || width>2048 || height<1 || height>2048 || pitch<width*2 || pitch>8192 ||
  (pitch&1) || format<0 || format>1 || !relative(filename,name) || !span((u32)pixels,(u32)pitch*(u32)height,1))goto done;
 u32 n=0;while(name[n])++n;
 for(u32 i=0;i<count;++i){const u8* r=catalog+offsets[i];const u32 length=word(r+4);int equal=length==n;
  for(u32 j=0;equal && j<n;++j)if(normal(r[20+j])!=(u8)name[j])equal=0;
  if(equal){struct MnmPixelPlane p={pixels,(u32)width,(u32)height,(u32)pitch/2,(u32)format};result=mnm_jpeg_record_words(r,word(r),&p);break;}}
done:SetLastError(error);return result;
}
static u32 __attribute__((fastcall)) decode(const char* name,u16* p,i32 w,i32 pitch,i32 h,i32 format){
 if(NativeJpegPixelsV1(name,p,w,pitch,h,format)){++native_calls;return 1;}
 ++forwarded_calls;return (*(Jpeg*)0x5c5308)(name,p,w,pitch,h,format);
}
static int load(void){
 char path[1024];const u32 n=GetEnvironmentVariableA("MNM_JPEG_PIXELS",path,sizeof(path));
 if(!n)return 1;if(n>=sizeof(path))return 0;
 root_length=GetEnvironmentVariableA("MNM_JPEG_ROOT",root,sizeof(root));if(!root_length || root_length>=sizeof(root))return 0;
 HANDLE f=CreateFileA(path,0x80000000,1,0,3,0x80,0);if(f==(HANDLE)-1)return 0;
 u32 high=0;bytes=GetFileSize(f,&high);
 if(high || bytes<32 || bytes>1024u*1024u*1024u){CloseHandle(f);return 0;}
 mapping=CreateFileMappingA(f,0,2,0,0,0);CloseHandle(f);if(!mapping)return 0;
 catalog=MapViewOfFile(mapping,4,0,0,bytes);if(!catalog || !mnm_jpeg_catalog_admit(catalog,bytes))return 0;
 count=word(catalog+16);u32 at=32;for(u32 i=0;i<count;++i){offsets[i]=at;at+=word(catalog+at);}
 return 1;
}
static int install(void){
 char flag[4];const u32 n=GetEnvironmentVariableA("MNM_JPEG_TAKEOVER",flag,sizeof(flag));if(!n)return 1;
 if(n!=1 || flag[0]!='1' || !catalog || (u32)GetModuleHandleA(0)!=0x400000)return 0;
 const u8 original[6]={0xff,0x15,8,0x53,0x5c,0};if(!span(0x58d34b,6,0))return 0;
 for(u32 i=0;i<6;++i)if(((const u8*)0x58d34b)[i]!=original[i])return 0;
 u32 old,ignored;if(!VirtualProtect((void*)0x58d34b,6,0x40,&old))return 0;
 u8* at=(u8*)0x58d34b;at[0]=0xe8;*(u32*)(at+1)=(u32)decode-0x58d350;at[5]=0x90;
 if(!VirtualProtect(at,6,old,&ignored) || !FlushInstructionCache(GetCurrentProcess(),at,6))ExitProcess(123);
 return 1;
}
__declspec(dllexport) void JpegPixelsAnchor(void){}
__declspec(dllexport) u32 WIN NativeJpegCallsV1(void){return native_calls;}
__declspec(dllexport) u32 WIN OriginalJpegCallsV1(void){return forwarded_calls;}
int WIN DllMain(HANDLE module,u32 reason,void* reserved){
 (void)module;(void)reserved;const u32 error=GetLastError();int result=1;
 if(reason==1)result=load() && install();else if(reason==0){if(catalog)UnmapViewOfFile(catalog);if(mapping)CloseHandle(mapping);catalog=0;mapping=0;}
 SetLastError(error);return result;
}
