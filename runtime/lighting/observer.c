#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/terrain_light_v1.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API void WIN Sleep(u32);
API u32 WIN GetTickCount(void);
static char directory[1024];static u32 nonce[2],sequence,limit=16,replace_mode,busy;
static void* trampoline;
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;for(u32 i=0;i<4;++i)b[i]=(u8)(v>>(i*8));}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;for(u32 i=0;i<n;++i)a[i]=b[i];}
static void zero(void* p,u32 n){u8* b=p;for(u32 i=0;i<n;++i)b[i]=0;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;for(u32 i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static int readable(u32 p,u32 n){if(n && (!p || n>0xffffffffu-p))return 0;u32 end=p+n;while(p<end){u32 info[7];if(VirtualQuery((void*)p,info,sizeof(info))!=sizeof(info) || info[4]!=0x1000 || (info[5]&0x101) || !info[5] || info[0]>p || info[3]>0xffffffffu-info[0] || info[0]+info[3]<=p)return 0;p=info[0]+info[3];}return 1;}
static void path(char* out,u32 id,const char* extension){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';const char* word="light-";for(u32 k=0;word[k];++k)out[i++]=word[k];u32 at=i;i+=8;for(u32 k=0;k<8;++k){out[at+7-k]=(char)('0'+id%10);id/=10;}for(u32 k=0;extension[k];++k)out[i++]=extension[k];out[i]=0;}
static int save(u32 id,const char* ext,const void* bytes,u32 n){char name[1100];path(name,id,ext);HANDLE h=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(h==(HANDLE)-1)return 0;u32 written=0;int ok=WriteFile(h,bytes,n,&written,0)&&written==n;CloseHandle(h);return ok;}
static void states(u8* dst,u32 map){const u32 a[11]={0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,0,0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,0,0};for(u32 i=0;i<11;++i)put(dst+4*i,get((void*)(a[i]?a[i]:map+(i==4?0x642e:i==9?0x642a:0x6426))));}
static u8* capture(u32 map,u32 caller,u32* length){
 if(map!=0x6c5490 || !readable(map,0x66c0))return 0;
 u32 w=get((void*)(map+4)),h=get((void*)(map+8)),layers=get((void*)(map+12)),count=get((void*)(map+0x7e0));
 u32 cc=get((void*)0x6def5c),cs=get((void*)0x6df180),oc=get((void*)0x68949c),os=get((void*)0x6898dc),cp=get((void*)0x6def58),op=get((void*)0x689498),owners=get((void*)0x5fbf28);
 if(!w||w>128||!h||h>128||!layers||layers>32||count!=w*h*((layers+1)/2)||cc>65536||cs>65536||oc>65536||os>oc||!readable(cp,cc*0xe4b)||!readable(op,oc*0x22e)||!readable(owners,8*0x154))return 0;
 for(u32 i=0;i<5;++i)if(!readable(get((void*)(map+0x7cc+i*4)),count))return 0;
 u32 n=300+cc*28+oc*32+count*5;if(n>MNM_LIGHT_V1_MAX_BYTES)return 0;u8* b=HeapAlloc(GetProcessHeap(),0,n);if(!b)return 0;zero(b,n);copy(b,MNM_LIGHT_V1_MAGIC,8);
 put(b+8,1);put(b+12,128);put(b+16,n);put(b+20,sequence);put(b+24,nonce[0]);put(b+28,nonce[1]);put(b+32,w);put(b+36,h);put(b+40,layers);put(b+44,count);
 put(b+48,get((void*)0x5e18e8));put(b+52,get((void*)0x5e18ec));put(b+56,(u32)(i32)(signed char)*(u8*)(map+0x6380));put(b+60,get((void*)0x644520));put(b+64,get((void*)0x5e41a8));put(b+68,get((void*)(map+0x808))!=0);put(b+72,get((void*)(map+0x80c))!=0);
 put(b+76,cc);put(b+80,cs);put(b+84,oc);put(b+88,os);put(b+92,get((void*)(0x689930+0x39)));put(b+96,get((void*)(0x689930+0x3d)));put(b+100,get((void*)(0x689930+0x59)));put(b+104,get((void*)0x6c4830));put(b+108,caller);put(b+112,replace_mode);
 for(u32 i=0;i<8;++i){copy(b+128+i*16,(void*)(owners+i*0x154+0x18),8);copy(b+136+i*16,(void*)(owners+i*0x154+0x20),8);}states(b+256,map);
 u32 at=300;
 for(u32 i=0;i<cc;++i,at+=28){u32 p=cp+i*0xe4b;put(b+at,get((void*)(p+4))!=0);put(b+at+4,get((void*)(p+0xe4))!=0);put(b+at+8,get((void*)(p+0x174)));for(u32 j=0;j<3;++j)put(b+at+12+j*4,get((void*)(p+8+j*4)));}
 // 0049cf60 is a collection lookup taking the record's type as a stack
 // argument, not a record method. Read its confirmed table selection directly.
 for(u32 i=0;i<oc;++i,at+=32){u32 p=op+i*0x22e,active=get((void*)(p+4))!=0,index=0;if(active){u32 type=get((void*)(p+0x28));if(type>(0xffffffffu-0x6894a4)/12||!readable(0x6894a4+type*12,4)){HeapFree(GetProcessHeap(),0,b);return 0;}index=get((void*)(0x6894a4+type*12));}put(b+at,active);put(b+at+4,index);for(u32 j=0;j<3;++j)put(b+at+8+j*4,get((void*)(p+8+j*4)));put(b+at+20,get((void*)(p+0x1c2)));put(b+at+24,get((void*)(p+0x1c6)));}
 for(u32 i=0;i<5;++i){copy(b+at,(void*)get((void*)(map+0x7cc+i*4)),count);at+=count;}put(b+120,mnm_light_checksum(b,n));*length=n;return b;
}
static void readResult(u8* b,u32 map,u32 count){put(b,get((void*)0x5e41a8));states(b+4,map);for(u32 i=0;i<5;++i)copy(b+48+i*count,(void*)get((void*)(map+0x7cc+i*4)),count);}
static int applyReply(const u8* input,u32 map,u32 count){
 char name[1100];path(name,sequence,".reply");u32 size=80+count*5;u8* b=HeapAlloc(GetProcessHeap(),0,size+1);if(!b)return 0;u32 start=GetTickCount();int ok=0;
 while(GetTickCount()-start<3000){HANDLE h=CreateFileA(name,0x80000000,1,0,3,0x80,0);if(h!=(HANDLE)-1){u32 read=0;int valid=ReadFile(h,b,size+1,&read,0)&&read==size;CloseHandle(h);
  if(valid&&equal(b,MNM_LIGHT_V1_REPLY,8)&&get(b+8)==1&&get(b+12)==size&&get(b+16)==sequence&&get(b+20)==nonce[0]&&get(b+24)==nonce[1]&&get(b+28)==get(input+120)&&!get(b+32)&&get(b+36)<8&&get(b+52)<2&&get(b+48)<=65536&&get(b+56)<2&&get(b+68)<=65536&&get(b+72)<2&&get(b+76)<2){
   for(u32 i=0;i<5;++i)copy((void*)get((void*)(map+0x7cc+i*4)),b+80+i*count,count);
   const u32 a[11]={0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,0,0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,0,0};for(u32 i=0;i<11;++i)put((void*)(a[i]?a[i]:map+(i==4?0x642e:i==9?0x642a:0x6426)),get(b+36+i*4));put((void*)0x5e41a8,0);ok=1;
  }break;
 }Sleep(2);}
 HeapFree(GetProcessHeap(),0,b);return ok;
}
static void __attribute__((fastcall)) hook(void* receiver,void* unused){
 (void)unused;typedef void(__attribute__((thiscall))*Call)(void*);u32 map=(u32)receiver,error=GetLastError();
 if(busy||sequence>=limit){((Call)trampoline)(receiver);SetLastError(error);return;}busy=1;++sequence;
 u32 length=0;u8* input=capture(map,(u32)__builtin_return_address(0),&length);u32 replaced=0;
 if(input&&save(sequence,".in",input,length)&&replace_mode)replaced=applyReply(input,map,get(input+44));
 if(!replaced)((Call)trampoline)(receiver);
 if(input){u32 count=get(input+44),size=48+count*5;u8* result=HeapAlloc(GetProcessHeap(),0,size);if(result){readResult(result,map,count);save(sequence,".oracle",result,size);HeapFree(GetProcessHeap(),0,result);}u32 done[4]={sequence,replaced,replace_mode,get((void*)0x6c4830)};save(sequence,".done",done,sizeof(done));HeapFree(GetProcessHeap(),0,input);}else{u32 refused[2]={sequence,map};save(sequence,".refused",refused,sizeof(refused));}
 busy=0;SetLastError(error);
}
__declspec(dllexport) void LightAnchor(void){}
int WIN DllMain(void* instance,u32 reason,void* reserved){
 (void)instance;(void)reserved;if(reason!=1)return 1;u32 n=GetEnvironmentVariableA("MNM_LIGHT_DIR",directory,sizeof(directory));if(!n)return 1;if(n>=sizeof(directory))return 0;
 char value[32];n=GetEnvironmentVariableA("MNM_LIGHT_MODE",value,sizeof(value));replace_mode=n==7&&equal(value,"replace",7);
 n=GetEnvironmentVariableA("MNM_LIGHT_LIMIT",value,sizeof(value));if(n){limit=0;for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9')return 0;limit=limit*10+value[i]-'0';}if(!limit||limit>64)return 0;}
 nonce[0]=GetTickCount();nonce[1]=(u32)instance;u8 expected[8]={0x56,0x8b,0xf1,0xe8,0x28,0,0,0};u32 site=0x4f27a0,old,unused;
 if(!equal((void*)site,expected,8))return 0;u8* code=VirtualAlloc(0,13,0x3000,4);if(!code)return 0;copy(code,expected,8);put(code+4,0x4f27d0-(u32)code-8);code[8]=0xe9;put(code+9,site+8-(u32)code-13);
 if(!VirtualProtect(code,13,0x20,&old))return 0;FlushInstructionCache(GetCurrentProcess(),code,13);trampoline=code;
 if(!VirtualProtect((void*)site,8,0x40,&old))return 0;u8 patch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};put(patch+1,(u32)&hook-site-5);copy((void*)site,patch,8);FlushInstructionCache(GetCurrentProcess(),(void*)site,8);VirtualProtect((void*)site,8,old,&unused);return 1;
}
