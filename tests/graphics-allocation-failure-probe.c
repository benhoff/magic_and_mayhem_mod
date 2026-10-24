/* Test-only checked IAT forwarding. The production DLL is built unchanged.
 * Fail one allocation, account real resources, then retry through public COM. */
#include "graphics-lifetime-common.h"
API int WIN VirtualFree(void*,u32,u32);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(void*);
API void* WIN CreateCompatibleDC(void*);
API void* WIN CreateDIBSection(void*,void*,u32,void**,HANDLE,u32);
API void* WIN SelectObject(void*,void*);
API int WIN DeleteObject(void*);
API int WIN DeleteDC(void*);
static void* owned[6][64];
static u32 family,ordinal,seen,fired,rows[32][8],count;
static void track(u32 k,void* p){if(!p)return;for(u32 i=0;i<64;++i)if(!owned[k][i]){owned[k][i]=p;return;}ExitProcess(220);}
static void untrack(u32 k,void* p){for(u32 i=0;i<64;++i)if(owned[k][i]==p){owned[k][i]=0;return;}}
static u32 total(void){u32 n=0;for(u32 k=0;k<6;++k)for(u32 i=0;i<64;++i)n+=owned[k][i]!=0;return n;}
static int fail(u32 k){if(family!=k)return 0;if(++seen==ordinal){++fired;return 1;}return 0;}
static void* WIN halloc(HANDLE h,u32 f,u32 n){if(fail(1))return 0;void* p=HeapAlloc(h,f,n);track(0,p);return p;}
static int WIN hfree(HANDLE h,u32 f,void* p){int r=HeapFree(h,f,p);if(r)untrack(0,p);return r;}
static void* WIN valloc(void* p,u32 n,u32 t,u32 f){if(fail(2))return 0;void* r=VirtualAlloc(p,n,t,f);track(1,r);return r;}
static int WIN vfree(void* p,u32 n,u32 t){int r=VirtualFree(p,n,t);if(r&&t==0x8000)untrack(1,p);return r;}
static HANDLE WIN mapping(HANDLE h,void* a,u32 p,u32 hi,u32 lo,const char* n){if(fail(3))return 0;HANDLE r=CreateFileMappingA(h,a,p,hi,lo,n);track(2,r);return r;}
static int WIN close_handle(HANDLE h){int r=CloseHandle(h);if(r)untrack(2,h);return r;}
static void* WIN view(HANDLE h,u32 a,u32 hi,u32 lo,u32 n){if(fail(4))return 0;void* r=MapViewOfFile(h,a,hi,lo,n);track(3,r);return r;}
static int WIN unview(void* p){int r=UnmapViewOfFile(p);if(r)untrack(3,p);return r;}
static void* WIN dc_create(void* p){if(fail(5))return 0;void* r=CreateCompatibleDC(p);track(4,r);return r;}
static int WIN dc_delete(void* p){int r=DeleteDC(p);if(r)untrack(4,p);return r;}
static void* WIN dib(void* d,void* i,u32 u,void** b,HANDLE h,u32 o){if(fail(6))return 0;void* r=CreateDIBSection(d,i,u,b,h,o);track(5,r);return r;}
static int WIN object_delete(void* p){int r=DeleteObject(p);if(r)untrack(5,p);return r;}
static void* WIN select_object(void* d,void* b){if(fail(7))return (void*)-1;return SelectObject(d,b);}
static int equal(const char* a,const char* b){while(*a&&*a==*b){++a;++b;}return *a==*b;}
static void patch(void* module,const char* name,void* replacement){
 u8* base=module;gl_require(*(u16*)base==0x5a4d,221);u32 pe=*(u32*)(base+60);
 gl_require(*(u32*)(base+pe)==0x4550&&*(u16*)(base+pe+24)==0x10b,222);
 u32* desc=(u32*)(base+*(u32*)(base+pe+128));u32 matches=0;
 for(;desc[3];desc+=5){void* provider=GetModuleHandleA((char*)base+desc[3]);gl_require(provider!=0,223);
  u32* names=(u32*)(base+desc[0]);void** slots=(void**)(base+desc[4]);
  for(u32 i=0;names[i];++i)if(!(names[i]&0x80000000u)&&equal((char*)base+names[i]+2,name)){
   gl_require(slots[i]==GetProcAddress(provider,name),224);u32 old,unused;
   gl_require(VirtualProtect(slots+i,4,4,&old),225);slots[i]=replacement;
   gl_require(VirtualProtect(slots+i,4,old,&unused),226);++matches;
  }
 }
 gl_require(matches==1,227);
}
static void arm(u32 f,u32 n){family=f;ordinal=n;seen=fired=0;}
static void record(u32 stage,u32 f,u32 n,i32 hr,u32 baseline){
 gl_require(fired==1&&hr&&total()==baseline,228);u32* r=rows[count++];
 r[0]=stage;r[1]=f;r[2]=n;r[3]=(u32)hr;r[4]=fired;r[5]=baseline;r[6]=total();r[7]=1;family=0;
}
void start(void){
 u32 native;void* module=gl_module(&native);gl_require(native,229);
 #define PATCH(n,f) patch(module,n,(void*)f)
 PATCH("HeapAlloc",halloc);PATCH("HeapFree",hfree);PATCH("VirtualAlloc",valloc);PATCH("VirtualFree",vfree);
 PATCH("CreateFileMappingA",mapping);PATCH("CloseHandle",close_handle);PATCH("MapViewOfFile",view);PATCH("UnmapViewOfFile",unview);
 PATCH("CreateCompatibleDC",dc_create);PATCH("DeleteDC",dc_delete);PATCH("CreateDIBSection",dib);PATCH("DeleteObject",object_delete);PATCH("SelectObject",select_object);
 GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");gl_require(create!=0,230);
 void* draw=0;arm(1,1);i32 hr=create(0,&draw,0);gl_require(!draw,231);record(0,1,1,hr,0);gl_counts(module,0,0,0,0,0);
 gl_require(!create(0,&draw,0),232);gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),233);
 char mode[2];u32 guarded=GetEnvironmentVariableA("MNM_NATIVE_CPU_GUARD",mode,2)==1&&mode[0]=='3';
 /* Offscreen plus a primary/back pair: fail each resource stage independently. */
 for(u32 primary=0;primary<2;++primary)for(u32 f=1;f<=4;++f){
  if((guarded&&f==2)||(!guarded&&f>=3))continue;
  u32 limit=f==1?(guarded?2:1):f==4?2:1;limit*=primary?2:1;
  for(u32 n=1;n<=limit;++n){u32 d[31];gl_desc(d,0);if(primary){d[1]=0x21;d[5]=1;d[26]=0x218;}
   void* s=0;u32 baseline=total();arm(f,n);hr=((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,d,&s,0);
   gl_require(!s,234);record(primary+1,f,n,hr,baseline);gl_counts(module,1,0,0,0,0);
   gl_require(!((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,d,&s,0)&&s,235);gl_drop(s);
   gl_require(total()==baseline,236);gl_counts(module,1,0,0,0,0);
  }
 }
 void* surface=gl_surface(draw);gl_pixels(surface,0,0,0x1234,1);
 for(u32 f=5;f<=7;++f){u32 baseline=total();void* dc=0;arm(f,1);hr=gl_dc(surface,&dc);gl_require(!dc,237);record(3,f,1,hr,baseline);
  gl_counts(module,1,1,0,0,0);gl_require(!gl_dc(surface,&dc)&&dc,238);gl_require(!gl_undc(surface,dc),239);gl_require(total()==baseline,240);
  u32 pixels[117];gl_pixels(surface,0,pixels,0,0);for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x)gl_require(pixels[y*13+x]==(u16)(0x1234+x*337+y*971),241);
 }
 gl_drop(surface);gl_drop(draw);gl_require(total()==0,242);gl_counts(module,0,0,0,0,0);
 gl_require(count==(guarded?19:10),243);gl_save("allocation-failures.bin",rows,count*32);ExitProcess(0);
}
