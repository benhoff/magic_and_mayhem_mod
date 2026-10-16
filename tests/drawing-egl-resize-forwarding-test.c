#define _GNU_SOURCE
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static void pattern(uint16_t* p,int w,int h){for(int y=0;y<h;y++)for(int x=0;x<w;x++)p[y*w+x]=(uint16_t)(((y%31)<<11)|((x%63)<<5)|((y+x)%31));}
int main(void){
 Display* x=XOpenDisplay(0);if(!x)return 1;EGLDisplay d=eglGetDisplay((EGLNativeDisplayType)x);if(!eglInitialize(d,0,0)||!eglBindAPI(EGL_OPENGL_API))return 2;
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,0,EGL_NONE};EGLConfig cfg;EGLint n;if(!eglChooseConfig(d,attrs,&cfg,1,&n)||!n)return 3;
 Window win=XCreateSimpleWindow(x,DefaultRootWindow(x),0,0,800,600,0,0,0);XMapWindow(x,win);XSync(x,0);EGLSurface s=eglCreateWindowSurface(d,cfg,(EGLNativeWindowType)win,0);EGLContext c=eglCreateContext(d,cfg,EGL_NO_CONTEXT,0);if(!eglMakeCurrent(d,s,s,c))return 4;
 typedef void(*Blit)(int,int,int,int,int,int,int,int,unsigned,unsigned);Blit blit;__eglMustCastToProperFunctionPointerType pointer=eglGetProcAddress("glBlitFramebuffer");memcpy(&blit,&pointer,sizeof(blit));if(!blit)return 7;
 glViewport(0,0,800,600);glDisable(GL_DITHER);GLuint fbo,tex;glGenFramebuffers(1,&fbo);glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
 const int sizes[][2]={{800,600},{640,480},{640,480},{800,600},{800,600},{640,480},{640,480}};int lastw=0,lasth=0;
 for(unsigned step=0;step<sizeof(sizes)/sizeof(*sizes);step++){
  int w=sizes[step][0],h=sizes[step][1];if(w!=lastw||h!=lasth){XResizeWindow(x,win,w,h);XSync(x,0);}
  uint16_t* data=malloc((size_t)w*h*2);pattern(data,w,h);glBindTexture(GL_TEXTURE_2D,tex);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB565,w,h,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,data);glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_READ_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);glReadBuffer(GL_COLOR_ATTACHMENT0);glDrawBuffer(GL_BACK);
  EGLint prew,preh;eglQuerySurface(d,s,EGL_WIDTH,&prew);eglQuerySurface(d,s,EGL_HEIGHT,&preh);blit(0,0,w,h,0,h,w,0,GL_COLOR_BUFFER_BIT,GL_NEAREST);if(!eglSwapBuffers(d,s))return 5;glFinish();int viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);if(viewport[0]||viewport[1]||viewport[2]!=800||viewport[3]!=600)return 8;XSync(x,0);XImage* image=XGetImage(x,DefaultRootWindow(x),0,0,(unsigned)w,(unsigned)h,AllPlanes,ZPixmap);if(!image)return 6;
  unsigned bad[3]={0};const int shifts[]={0,120,-120};for(int y=0;y<h;y++)for(int xx=0;xx<w;xx++){unsigned long pixel=XGetPixel(image,xx,y);unsigned word=((pixel>>19&31)<<11)|((pixel>>10&63)<<5)|(pixel>>3&31);for(int j=0;j<3;j++){int sy=y-shifts[j];unsigned expected=sy>=0&&sy<h?data[sy*w+xx]:0;bad[j]+=word!=expected;}}
  printf("%u %dx%d pre %dx%d mismatch %u plus120 %u minus120 %u error%x\n",step,w,h,prew,preh,bad[0],bad[1],bad[2],glGetError());if(bad[0])return 9;free(data);XDestroyImage(image);lastw=w;lasth=h;
 }
 return 0;
}
