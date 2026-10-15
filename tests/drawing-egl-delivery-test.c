#define _GNU_SOURCE
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <X11/Xlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
typedef EGLBoolean(*Swap)(EGLDisplay,EGLSurface);
int main(void){
 Display* xdisplay=XOpenDisplay(0);if(!xdisplay)return 1;
 EGLDisplay display=eglGetDisplay((EGLNativeDisplayType)xdisplay);EGLint major,minor;if(!eglInitialize(display,&major,&minor))return 2;
 EGLint attributes[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_NONE};EGLConfig config;EGLint count;if(!eglChooseConfig(display,attributes,&config,1,&count)||count!=1)return 3;
 Window window=XCreateSimpleWindow(xdisplay,DefaultRootWindow(xdisplay),0,0,8,8,0,0,0);XMapWindow(xdisplay,window);XSync(xdisplay,False);
 EGLSurface surface=eglCreateWindowSurface(display,config,(EGLNativeWindowType)window,0);EGLint ctx[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};EGLContext context=eglCreateContext(display,config,EGL_NO_CONTEXT,ctx);if(surface==EGL_NO_SURFACE||context==EGL_NO_CONTEXT||!eglMakeCurrent(display,surface,surface,context))return 4;
 glViewport(0,0,8,8);glDisable(GL_DITHER);void* library=dlopen("libEGL.so.1",RTLD_NOW);if(!library)return 5;
 Swap methods[4]={eglSwapBuffers,0,0,0};void* pointer=dlsym(library,"eglSwapBuffers");memcpy(methods+1,&pointer,sizeof(Swap));__eglMustCastToProperFunctionPointerType proc=eglGetProcAddress("eglSwapBuffers");memcpy(methods+2,&proc,sizeof(Swap));pointer=dlsym(RTLD_DEFAULT,"eglSwapBuffers");memcpy(methods+3,&pointer,sizeof(Swap));
 for(unsigned i=0;i<4;++i){
  if(!methods[i])return 6;
  glClearColor(i==0||i==3,i==1||i==3,i==2||i==3,1);glClear(GL_COLOR_BUFFER_BIT);glFinish();unsigned char pixels[256];glReadPixels(0,0,8,8,GL_RGBA,GL_UNSIGNED_BYTE,pixels);if(glGetError()!=GL_NO_ERROR)return 7;
  for(unsigned x=0;x<64;++x)if(pixels[x*4]!=(i==0||i==3?255:0)||pixels[x*4+1]!=(i==1||i==3?255:0)||pixels[x*4+2]!=(i==2||i==3?255:0))return 8;
  for(unsigned x=0;x<256;++x){printf("%02x",pixels[x]);}
  errno=77+(int)i;EGLBoolean result=methods[i](display,surface);int error=errno;EGLint egl_error=eglGetError();printf(" %u:%d:%04x\n",result,error,egl_error);if(!result||egl_error!=EGL_SUCCESS)return 9;
 }
 dlerror();pointer=dlsym(library,"mnm_missing_egl_symbol");printf("missing:%d\n",pointer==0&&dlerror()!=0);
 eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);eglDestroySurface(display,surface);eglDestroyContext(display,context);eglTerminate(display);XDestroyWindow(xdisplay,window);XCloseDisplay(xdisplay);dlclose(library);return 0;
}
