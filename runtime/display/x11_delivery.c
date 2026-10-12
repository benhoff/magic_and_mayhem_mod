#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
/* Diagnostic metadata only. No XImage bytes, GC pointers or Display pointers. */
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static int fd=-1,initialized;
static uint32_t calls;
static void put(unsigned char* p,uint32_t v){p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static void fail(void){_exit(97);}
static void record(uint32_t phase,uint32_t id,uint32_t method,const uint32_t args[8],int result,int original_errno){
 int saved=errno;if(pthread_mutex_lock(&mutex))fail();
 if(!initialized){
  const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");initialized=1;
  if(directory){char path[4096];if(directory[0]!='/'||snprintf(path,sizeof(path),"%s/x11-delivery.bin",directory)>=(int)sizeof(path))fail();fd=open(path,O_RDWR|O_CREAT|O_APPEND|O_CLOEXEC,0600);if(fd<0)fail();}
 }
 if(fd>=0){
  if(flock(fd,LOCK_EX))fail();
  struct stat status;if(fstat(fd,&status))fail();
  if(!status.st_size){unsigned char h[64]={0};memcpy(h,"MNMX1101",8);put(h+8,1);put(h+12,96);put(h+16,65536);if(write(fd,h,64)!=64)fail();status.st_size=64;}
  if(status.st_size<64||(status.st_size-64)%96||status.st_size>=64+65536*96)fail();
  unsigned char r[96]={0};struct timespec now;if(clock_gettime(CLOCK_MONOTONIC,&now))fail();uint64_t ns=(uint64_t)now.tv_sec*1000000000u+(uint64_t)now.tv_nsec;
  put(r,phase);put(r+4,id);put(r+8,method);put(r+12,(uint32_t)getpid());put(r+16,(uint32_t)syscall(SYS_gettid));put(r+20,(uint32_t)ns);put(r+24,(uint32_t)(ns>>32));put(r+28,(uint32_t)result);put(r+32,(uint32_t)original_errno);
  for(unsigned i=0;i<8;++i)put(r+36+i*4,args[i]);
  if(write(fd,r,96)!=96||flock(fd,LOCK_UN))fail();
 }
 if(pthread_mutex_unlock(&mutex))fail();
 errno=saved;
}
static uint32_t begin(uint32_t method,const uint32_t args[8]){int saved=errno;uint32_t id=__atomic_add_fetch(&calls,1,__ATOMIC_RELAXED);record(1,id,method,args,0,saved);errno=saved;return id;}
#define RESOLVE(name,type) static type cached; type real=__atomic_load_n(&cached,__ATOMIC_ACQUIRE); if(!real){int e=errno;void* sym=dlsym(RTLD_NEXT,#name);if(!sym)fail();memcpy(&real,&sym,sizeof(real));__atomic_store_n(&cached,real,__ATOMIC_RELEASE);errno=e;}
#define START(method) uint32_t id=begin(method,args)
#define END(method) int saved=errno;record(2,id,method,args,result,saved);errno=saved;return result
int XPutImage(Display* d,Drawable dst,GC gc,XImage* image,int sx,int sy,int dx,int dy,unsigned w,unsigned h){
 typedef int(*Fn)(Display*,Drawable,GC,XImage*,int,int,int,int,unsigned,unsigned);RESOLVE(XPutImage,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)sx,(uint32_t)sy,(uint32_t)dx,(uint32_t)dy,w,h,0};START(1);int result=real(d,dst,gc,image,sx,sy,dx,dy,w,h);END(1);
}
Bool XShmPutImage(Display* d,Drawable dst,GC gc,XImage* image,int sx,int sy,int dx,int dy,unsigned w,unsigned h,Bool event){
 typedef Bool(*Fn)(Display*,Drawable,GC,XImage*,int,int,int,int,unsigned,unsigned,Bool);RESOLVE(XShmPutImage,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)sx,(uint32_t)sy,(uint32_t)dx,(uint32_t)dy,w,h,(uint32_t)event};START(2);int result=real(d,dst,gc,image,sx,sy,dx,dy,w,h,event);END(2);
}
int XCopyArea(Display* d,Drawable src,Drawable dst,GC gc,int sx,int sy,unsigned w,unsigned h,int dx,int dy){
 typedef int(*Fn)(Display*,Drawable,Drawable,GC,int,int,unsigned,unsigned,int,int);RESOLVE(XCopyArea,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)src,(uint32_t)sx,(uint32_t)sy,(uint32_t)dx,(uint32_t)dy,w,h};START(3);int result=real(d,src,dst,gc,sx,sy,w,h,dx,dy);END(3);
}
int XCopyPlane(Display* d,Drawable src,Drawable dst,GC gc,int sx,int sy,unsigned w,unsigned h,int dx,int dy,unsigned long plane){
 typedef int(*Fn)(Display*,Drawable,Drawable,GC,int,int,unsigned,unsigned,int,int,unsigned long);RESOLVE(XCopyPlane,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)src,(uint32_t)sx,(uint32_t)sy,(uint32_t)dx,(uint32_t)dy,w,h};START(4);int result=real(d,src,dst,gc,sx,sy,w,h,dx,dy,plane);END(4);
}
int XFillRectangle(Display* d,Drawable dst,GC gc,int x,int y,unsigned w,unsigned h){
 typedef int(*Fn)(Display*,Drawable,GC,int,int,unsigned,unsigned);RESOLVE(XFillRectangle,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)x,(uint32_t)y,w,h,0,0,0};START(5);int result=real(d,dst,gc,x,y,w,h);END(5);
}
int XFillRectangles(Display* d,Drawable dst,GC gc,XRectangle* rect,int count){
 typedef int(*Fn)(Display*,Drawable,GC,XRectangle*,int);RESOLVE(XFillRectangles,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)count,0,0,0,0,0,0};START(6);int result=real(d,dst,gc,rect,count);END(6);
}
int XClearArea(Display* d,Window dst,int x,int y,unsigned w,unsigned h,Bool exposures){
 typedef int(*Fn)(Display*,Window,int,int,unsigned,unsigned,Bool);RESOLVE(XClearArea,Fn);
 uint32_t args[8]={(uint32_t)dst,(uint32_t)x,(uint32_t)y,w,h,(uint32_t)exposures,0,0};START(7);int result=real(d,dst,x,y,w,h,exposures);END(7);
}
int XFlush(Display* d){typedef int(*Fn)(Display*);RESOLVE(XFlush,Fn);uint32_t args[8]={0};START(8);int result=real(d);END(8);}
int XSync(Display* d,Bool discard){typedef int(*Fn)(Display*,Bool);RESOLVE(XSync,Fn);uint32_t args[8]={(uint32_t)discard,0,0,0,0,0,0,0};START(9);int result=real(d,discard);END(9);}
