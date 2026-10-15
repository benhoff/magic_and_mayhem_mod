#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
typedef void(*Swap)(void*,unsigned long);
typedef void(*Function)(void);
typedef Function(*Proc)(const unsigned char*);
typedef unsigned(*Count)(void);
int main(int argc,char** argv){
 if(argc!=3)return 1;
 int limit=strstr(argv[2],"exhaust")!=0?17:16;
 for(int i=0;i<limit;++i){
  char path[4096];if(snprintf(path,sizeof(path),"%s/provider-%02d.so",argv[1],i)>=(int)sizeof(path))return 2;
  void* library=dlopen(path,RTLD_NOW|RTLD_LOCAL);if(!library)return 3;
  void* pointer=dlsym(library,strstr(argv[2],"proc")!=0?"glXGetProcAddressARB":"glXSwapBuffers");if(!pointer)return 4;
  Swap swap;
  if(strstr(argv[2],"proc")!=0){Proc proc;memcpy(&proc,&pointer,sizeof(proc));Function fn=proc((const unsigned char*)"glXSwapBuffers");memcpy(&swap,&fn,sizeof(swap));}
  else memcpy(&swap,&pointer,sizeof(swap));
  errno=77;swap(0,(unsigned long)i+1);if(errno!=77)return 5;
  Count count;pointer=dlsym(library,"mnm_provider_calls");if(!pointer)return 6;memcpy(&count,&pointer,sizeof(count));if(count()!=1)return 7;
  /* A repeated lookup must keep the same observer binding and exact provider. */
  pointer=dlsym(library,"glXSwapBuffers");Swap repeated;memcpy(&repeated,&pointer,sizeof(repeated));repeated(0,(unsigned long)i+1);if(count()!=2)return 8;
 }
 puts("MNM-GLX-ALIASES-PASS-16-EXACT");return 0;
}
