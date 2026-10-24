/* Independent PE32 COM callers. Numeric addresses are never lifetime IDs. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN SetPixel(void*,i32,i32,u32);
API int WIN GdiFlush(void);
typedef i32(WIN*GlCreate)(void*,void**,void*);
typedef i32(WIN*GlQuery)(void*,const u8*,void**);
typedef u32(WIN*GlRef)(void*);
static inline void** gl_vt(void* p){return *(void***)p;}
static inline void gl_require(int ok,u32 code){if(!ok)ExitProcess(code);}
static inline void gl_zero(void* p,u32 size){u8* b=p;while(size--)*b++=0;}
static const u8 gl_iids[4][16]={
 {0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},
 {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27},
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b}};
static const u8 gl_unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static inline u32 gl_drop(void* p){return ((GlRef)gl_vt(p)[2])(p);}
static inline void* gl_query(void* p,const u8* iid){void* out=0;gl_require(!((GlQuery)gl_vt(p)[0])(p,iid,&out)&&out,160);return out;}
static inline void gl_desc(u32* d,u32 version){
 gl_zero(d,124);d[0]=version==3?124:108;d[1]=0x1007;d[2]=9;d[3]=13;
 d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
}
static inline i32 gl_lock(void* p,u32 version,u32* d){gl_desc(d,version);return ((i32(WIN*)(void*,void*,u32*,u32,HANDLE))gl_vt(p)[25])(p,0,d,1,0);}
static inline i32 gl_unlock(void* p,u32 version,u32 pointer){return ((i32(WIN*)(void*,void*))gl_vt(p)[32])(p,version==3?0:(void*)pointer);}
static inline i32 gl_dc(void* p,void** dc){return ((i32(WIN*)(void*,void**))gl_vt(p)[17])(p,dc);}
static inline i32 gl_undc(void* p,void* dc){return ((i32(WIN*)(void*,void*))gl_vt(p)[26])(p,dc);}
static inline void* gl_surface(void* draw){u32 d[31];void* out=0;gl_desc(d,0);gl_require(!((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,d,&out,0)&&out,161);return out;}
static inline void gl_pixels(void* surface,u32 version,u32* output,u32 seed,u32 write){
 u32 d[31];gl_require(!gl_lock(surface,version,d),162);
 gl_require(d[2]==9&&d[3]==13&&d[4]>=26&&d[21]==16,163);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* p=(u16*)(d[9]+y*d[4])+x;if(write)*p=(u16)(seed+x*337+y*971);if(output)output[y*13+x]=*p;}
 gl_require(!gl_unlock(surface,version,d[9]),164);
}
static inline void gl_save(const char* name,const void* data,u32 size){u32 done=0;HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);gl_require(file!=(HANDLE)-1&&WriteFile(file,data,size,&done,0)&&done==size,165);CloseHandle(file);}
static inline void* gl_module(u32* native){char value[2];*native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)==1&&value[0]=='1';void* module=LoadLibraryA(*native?"MnmDraw.dll":"ddraw.dll");gl_require(module!=0,166);if(*native)gl_require(!GetModuleHandleA("ddraw.dll"),167);return module;}
static inline void gl_counts(void* module,u32 devices,u32 surfaces,u32 dcs,u32 clippers,u32 palettes){
 const char* names[5]={"NativeDrawObjects","NativeDrawSurfaces","NativeDrawDCs","NativeDrawClippers","NativeDrawPalettes"};u32 expected[5]={devices,surfaces,dcs,clippers,palettes};
 for(u32 i=0;i<5;++i){void* fn=GetProcAddress(module,names[i]);gl_require(fn&&((u32(WIN*)(void))fn)()==expected[i],168+i);}
}
