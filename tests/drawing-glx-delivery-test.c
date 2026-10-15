#define _GNU_SOURCE
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xutil.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
typedef void(*Swap)(Display*,GLXDrawable);
int main(void){
 Display* display=XOpenDisplay(0);if(!display)return 1;
 int screen=DefaultScreen(display),attributes[]={GLX_RGBA,GLX_DOUBLEBUFFER,GLX_RED_SIZE,8,GLX_GREEN_SIZE,8,GLX_BLUE_SIZE,8,None};
 XVisualInfo* visual=glXChooseVisual(display,screen,attributes);if(!visual)return 2;
 XSetWindowAttributes settings={0};settings.colormap=XCreateColormap(display,RootWindow(display,screen),visual->visual,AllocNone);settings.event_mask=StructureNotifyMask;
 Window window=XCreateWindow(display,RootWindow(display,screen),0,0,8,8,0,visual->depth,InputOutput,visual->visual,CWColormap|CWEventMask,&settings);
 GLXContext context=glXCreateContext(display,visual,0,True);if(!context||!glXMakeCurrent(display,window,context))return 3;
 XMapWindow(display,window);XSync(display,False);glViewport(0,0,8,8);glDisable(GL_DITHER);
 void* library=dlopen("libGL.so.1",RTLD_NOW);if(!library)return 4;
 Swap methods[5]={glXSwapBuffers,0,0,0,0};void* pointer=dlsym(library,"glXSwapBuffers");memcpy(methods+1,&pointer,sizeof(Swap));
 __GLXextFuncPtr proc=glXGetProcAddressARB((const GLubyte*)"glXSwapBuffers");memcpy(methods+2,&proc,sizeof(Swap));proc=glXGetProcAddress((const GLubyte*)"glXSwapBuffers");memcpy(methods+3,&proc,sizeof(Swap));
 pointer=dlsym(RTLD_DEFAULT,"glXSwapBuffers");memcpy(methods+4,&pointer,sizeof(Swap));
 for(unsigned i=0;i<5;++i){
  if(!methods[i])return 5;
  glDrawBuffer(GL_BACK);glClearColor(i==0||i>=3,i==1||i==3,i==2||i==3,1);glClear(GL_COLOR_BUFFER_BIT);
  glFinish();glReadBuffer(GL_BACK);unsigned char pixels[8*8*4];glReadPixels(0,0,8,8,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
  if(glGetError()!=GL_NO_ERROR)return 6;
  for(unsigned x=0;x<64;++x)if(pixels[x*4]!=(i==0||i>=3?255:0)||pixels[x*4+1]!=(i==1||i==3?255:0)||pixels[x*4+2]!=(i==2||i==3?255:0))return 7;
  for(unsigned x=0;x<sizeof(pixels);++x){printf("%02x",pixels[x]);}
  errno=77+(int)i;methods[i](display,window);printf(" errno:%d ",errno);
  glFinish();glReadBuffer(GL_FRONT);glReadPixels(0,0,8,8,GL_RGBA,GL_UNSIGNED_BYTE,pixels);if(glGetError()!=GL_NO_ERROR)return 8;
  for(unsigned x=0;x<sizeof(pixels);++x){printf("%02x",pixels[x]);}puts("");
 }
 dlerror();pointer=dlsym(library,"mnm_missing_glx_symbol");printf("missing:%d\n",pointer==0&&dlerror()!=0);
 glXMakeCurrent(display,None,0);glXDestroyContext(display,context);XDestroyWindow(display,window);XFreeColormap(display,settings.colormap);XFree(visual);XCloseDisplay(display);dlclose(library);return 0;
}
