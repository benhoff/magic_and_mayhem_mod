#define _GNU_SOURCE
#include <EGL/egl.h>
#include <GL/glx.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char** argv){
 if(argc!=3)return 1;
 const char* route=argv[1];const char* name=argv[2];void* pointer=0;
 if(!strcmp(route,"ordinary")){
  char path[4096];if(snprintf(path,sizeof(path),"%s/provider-a.so",name)>=(int)sizeof(path))return 2;
  void* first=dlopen(path,RTLD_NOW|RTLD_GLOBAL);if(!first)return 3;
  if(snprintf(path,sizeof(path),"%s/provider-b.so",name)>=(int)sizeof(path))return 4;
  void* second=dlopen(path,RTLD_NOW|RTLD_GLOBAL);if(!second)return 5;
  pointer=dlsym(first,"mnm_route_next");int(*next)(void);memcpy(&next,&pointer,sizeof(next));
  if(!next||next()!=2)return 6;
  dlerror();if(dlsym(RTLD_NEXT,"mnm_missing_route_symbol")||!dlerror())return 7;
  puts("caller-relative next and missing error preserved");return 0;
 }
 errno=77;
 if(!strcmp(route,"next"))pointer=dlsym(RTLD_NEXT,name);
 else if(!strcmp(route,"default"))pointer=dlsym(RTLD_DEFAULT,name);
 else if(!strcmp(route,"handle")){
  void* lib=dlopen(!strncmp(name,"egl",3)?"libEGL.so.1":"libGL.so.1",RTLD_NOW);if(!lib)return 8;
  pointer=dlsym(lib,name);
 }else if(!strcmp(route,"egl")){
  __eglMustCastToProperFunctionPointerType fn=eglGetProcAddress(name);memcpy(&pointer,&fn,sizeof(pointer));
 }else if(!strcmp(route,"glx")){
  __GLXextFuncPtr fn=glXGetProcAddress((const GLubyte*)name);memcpy(&pointer,&fn,sizeof(pointer));
 }else if(!strcmp(route,"arb")){
  __GLXextFuncPtr fn=glXGetProcAddressARB((const GLubyte*)name);memcpy(&pointer,&fn,sizeof(pointer));
 }else return 9;
 printf("lookup returned %d\n",pointer!=0);return 0;
}
