#define _GNU_SOURCE
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
/* Actual X11 drawing, with results compared to the same uninstrumented calls. */
int main(void){
 Display* display=XOpenDisplay(0);if(!display)return 1;
 int screen=DefaultScreen(display);Window window=XCreateSimpleWindow(display,RootWindow(display,screen),0,0,8,8,0,0,0);Pixmap pixmap=XCreatePixmap(display,window,8,8,DefaultDepth(display,screen));GC gc=XCreateGC(display,window,0,0);
 XMapWindow(display,window);XSync(display,False);
 int result[9],error[9];
 XSetForeground(display,gc,0x112233);errno=77;result[0]=XFillRectangle(display,pixmap,gc,0,0,8,8);error[0]=errno;
 XSetForeground(display,gc,0x445566);XRectangle rect[2]={{1,1,2,2},{5,5,1,1}};errno=78;result[1]=XFillRectangles(display,pixmap,gc,rect,2);error[1]=errno;
 errno=79;result[2]=XCopyArea(display,pixmap,window,gc,0,0,8,8,0,0);error[2]=errno;
 XImage* image=XCreateImage(display,DefaultVisual(display,screen),DefaultDepth(display,screen),ZPixmap,0,calloc(8*8,4),8,8,32,0);if(!image)return 2;
 for(unsigned y=0;y<8;++y)for(unsigned x=0;x<8;++x)XPutPixel(image,x,y,0x778899);
 errno=80;result[3]=XPutImage(display,window,gc,image,0,0,2,2,3,3);error[3]=errno;
 errno=81;result[4]=XClearArea(display,window,0,0,1,1,False);error[4]=errno;
 const char mono[2]={1,2};Pixmap bitmap=XCreateBitmapFromData(display,window,mono,2,2);
 XSetForeground(display,gc,0x993355);XSetBackground(display,gc,0x225544);
 errno=84;result[7]=XCopyPlane(display,bitmap,window,gc,0,0,2,2,6,0,1);error[7]=errno;
 XShmSegmentInfo shared={0};XImage* shm_image=XShmCreateImage(display,DefaultVisual(display,screen),DefaultDepth(display,screen),ZPixmap,0,&shared,2,2);if(!shm_image)return 4;
 shared.shmid=shmget(IPC_PRIVATE,shm_image->bytes_per_line*2,IPC_CREAT|0600);if(shared.shmid<0)return 5;
 shared.shmaddr=shmat(shared.shmid,0,0);if(shared.shmaddr==(char*)-1)return 6;
 shm_image->data=shared.shmaddr;if(!XShmAttach(display,&shared))return 7;XSync(display,False);shmctl(shared.shmid,IPC_RMID,0);
 for(unsigned y=0;y<2;++y)for(unsigned x=0;x<2;++x)XPutPixel(shm_image,x,y,0x224466);
 errno=85;result[8]=XShmPutImage(display,window,gc,shm_image,0,0,6,6,2,2,False);error[8]=errno;
 errno=82;result[5]=XFlush(display);error[5]=errno;errno=83;result[6]=XSync(display,False);error[6]=errno;
 XImage* final=XGetImage(display,window,0,0,8,8,AllPlanes,ZPixmap);if(!final)return 3;
 for(unsigned i=0;i<9;++i){printf("%d:%d ",result[i],error[i]);}puts("");
 for(unsigned y=0;y<8;++y){for(unsigned x=0;x<8;++x)printf("%08lx ",XGetPixel(final,x,y));puts("");}
 XShmDetach(display,&shared);XSync(display,False);shmdt(shared.shmaddr);shm_image->data=0;XDestroyImage(shm_image);
 XDestroyImage(final);XDestroyImage(image);XFreeGC(display,gc);XFreePixmap(display,bitmap);XFreePixmap(display,pixmap);XDestroyWindow(display,window);XCloseDisplay(display);return 0;
}
