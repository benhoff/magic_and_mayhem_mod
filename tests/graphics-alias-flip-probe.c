#include "graphics-lifetime-common.h"
API void WIN Sleep(u32);
static u32 results[64][16];
static i32 flip(void* front){return ((i32(WIN*)(void*,void*,u32))gl_vt(front)[11])(front,0,1);}
static u32 frame(void* object,u32 version,u32 value,u32 write){
 u32 d[31];gl_require(!gl_lock(object,version,d),200);
 gl_require(d[2]==480&&d[3]==640&&d[4]>=1280&&d[21]==16,201);
 u32 hash=2166136261u;
 for(u32 y=0;y<480;++y)for(u32 x=0;x<640;++x){u16* p=(u16*)(d[9]+y*d[4])+x;if(write)*p=(u16)value;gl_require(*p==(u16)value,202);hash=(hash^*p)*16777619u;}
 gl_require(!gl_unlock(object,version,d[9]),203);return hash;
}
void start(void){
 u32 native;void* module=gl_module(&native);GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");gl_require(create!=0,173);
 void* user=LoadLibraryA("user32.dll");
 typedef void*(WIN*Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
 typedef int(WIN*Foreground)(void*);typedef void*(WIN*Current)(void);typedef int(WIN*Position)(void*,void*,i32,i32,i32,i32,u32);
 typedef int(WIN*Cursor)(i32,i32);typedef void(WIN*Mouse)(u32,u32,u32,u32,u32);
 Window window=(Window)GetProcAddress(user,"CreateWindowExA");Foreground foreground=(Foreground)GetProcAddress(user,"SetForegroundWindow");
 Current current=(Current)GetProcAddress(user,"GetForegroundWindow");Position position=(Position)GetProcAddress(user,"SetWindowPos");
 Cursor cursor=(Cursor)GetProcAddress(user,"SetCursorPos");Mouse mouse=(Mouse)GetProcAddress(user,"mouse_event");
 gl_require(window&&foreground&&current&&position&&cursor&&mouse,204);
 void* hwnd=window(0,"STATIC","alias lease flip",0x10cf0000,0,0,640,480,0,0,0,0);gl_require(hwnd!=0,205);
 for(u32 i=0;i<40&&current()!=hwnd;++i){position(hwnd,(void*)-1,0,0,640,480,0x40);cursor(320,240);mouse(2,0,0,0,0);mouse(4,0,0,0,0);foreground(hwnd);Sleep(50);}
 gl_require(current()==hwnd,206);u32 id=0;
 for(u32 a=0;a<4;++a)for(u32 b=0;b<4;++b)for(u32 held=0;held<4;++held){
  u32* r=results[id];r[0]=id++;r[1]=a;r[2]=b;r[3]=held;
  void *draw=0,*front=0,*back=0,*dc=0;gl_require(!create(0,&draw,0),207);
  gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,hwnd,0x11),208);
  gl_require(!((i32(WIN*)(void*,u32,u32,u32))gl_vt(draw)[21])(draw,640,480,16),209);
  u32 d[31];gl_zero(d,sizeof(d));d[0]=108;d[1]=0x21;d[5]=1;d[26]=0x218;
  gl_require(!((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,d,&front,0),210);
  void* fa=gl_query(front,gl_iids[a]);void* fb=gl_query(front,gl_iids[b]);u32 caps[4]={4,0,0,0};
  gl_require(!((i32(WIN*)(void*,u32*,void**))gl_vt(fa)[12])(fa,caps,&back)&&back,211);
  void* ba=gl_query(back,gl_iids[a]);void* bb=gl_query(back,gl_iids[b]);
  frame(fa,a,0x1234,1);frame(ba,a,0x5678,1);
  void* operand_a=(held&1)?ba:fa;void* operand_b=(held&1)?bb:fb;void* other_b=(held&1)?fb:bb;
  r[4]=held<2?gl_lock(operand_a,a,d):gl_dc(operand_a,&dc);gl_require(!r[4],212);
  r[5]=flip(fa);
  r[6]=held<2?gl_unlock(operand_b,b,d[9]):gl_undc(operand_b,dc);
  r[7]=held<2?gl_unlock(other_b,b,d[9]):gl_undc(other_b,dc);
  if(held>=2){r[8]=flip(fb);r[9]=gl_undc(operand_b,dc);gl_require(!r[9],213);}
  r[10]=frame(fa,a,held<2?0x5678:0x1234,0);r[11]=frame(ba,a,held<2?0x1234:0x5678,0);r[12]=640*480*2;r[15]=1;
  gl_drop(bb);gl_drop(ba);gl_drop(back);gl_drop(fb);gl_drop(fa);gl_drop(front);gl_drop(draw);
  if(native)gl_counts(module,0,0,0,0,0);
 }
 typedef int(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user,"DestroyWindow");gl_require(destroy&&destroy(hwnd),214);
 gl_require(id==64,215);gl_save("alias-flips.bin",results,sizeof(results));ExitProcess(0);
}
