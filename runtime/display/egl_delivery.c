#define _GNU_SOURCE
#define MNM_X11_PROTOCOL_VERSION 3
#define MNM_EGL_DELIVERY 1
#include <EGL/egl.h>
typedef EGLBoolean(*EglSwap)(EGLDisplay,EGLSurface);
typedef __eglMustCastToProperFunctionPointerType(*EglProc)(const char*);
static EglSwap bind_egl_swap(EglSwap);
static EglProc bind_egl_proc(EglProc);
#include "glx_delivery.c"
static EglSwap egl_originals[16];
static EglProc egl_proc_originals[16];
static struct {void* handle;unsigned kind,id;} egl_handles[256];
#include "egl_physical.h"
/* Equality is local only; reuse/lifetime is not inferred from these handles. */
static unsigned egl_handle(void* handle,unsigned kind){
 int saved=errno;if(pthread_mutex_lock(&mutex))fail();unsigned id=0;
 for(unsigned i=0;i<256;++i)if(egl_handles[i].id&&egl_handles[i].handle==handle&&egl_handles[i].kind==kind){id=egl_handles[i].id;break;}
 if(!id)for(unsigned i=0;i<256;++i)if(!egl_handles[i].id){egl_handles[i].handle=handle;egl_handles[i].kind=kind;id=egl_handles[i].id=i+1;break;}
 if(pthread_mutex_unlock(&mutex))fail();
 if(!id)gl_refuse("EGL local handle IDs exhausted");
 errno=saved;return id;
}
static EGLBoolean egl_swap_call(unsigned slot,EGLDisplay display,EGLSurface surface){
 EglSwap real=__atomic_load_n(&egl_originals[slot],__ATOMIC_ACQUIRE);if(!real)fail();
 uint32_t args[8]={egl_handle(display,1),egl_handle(surface,2),slot+1,0,0,0,0,0};uint32_t id=begin(11,args);
 EGLBoolean result=real(display,surface);int saved=errno;record(2,id,11,args,(int)result,saved);egl_physical(id,args,result,saved);errno=saved;return result;
}
#define EGL_SLOT(n) static EGLBoolean egl_swap_##n(EGLDisplay d,EGLSurface s){return egl_swap_call(n,d,s);}
EGL_SLOT(0) EGL_SLOT(1) EGL_SLOT(2) EGL_SLOT(3) EGL_SLOT(4) EGL_SLOT(5) EGL_SLOT(6) EGL_SLOT(7)
EGL_SLOT(8) EGL_SLOT(9) EGL_SLOT(10) EGL_SLOT(11) EGL_SLOT(12) EGL_SLOT(13) EGL_SLOT(14) EGL_SLOT(15)
static EglSwap egl_wrappers[16]={egl_swap_0,egl_swap_1,egl_swap_2,egl_swap_3,egl_swap_4,egl_swap_5,egl_swap_6,egl_swap_7,egl_swap_8,egl_swap_9,egl_swap_10,egl_swap_11,egl_swap_12,egl_swap_13,egl_swap_14,egl_swap_15};
static EglSwap bind_egl_swap(EglSwap real){
 if(!real||real==eglSwapBuffers)return real;
 for(unsigned i=0;i<16;++i)if(real==egl_wrappers[i])return real;
 for(unsigned i=0;i<16;++i){EglSwap seen=__atomic_load_n(&egl_originals[i],__ATOMIC_ACQUIRE);if(seen==real)return egl_wrappers[i];if(!seen&&__atomic_compare_exchange_n(&egl_originals[i],&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))return egl_wrappers[i];if(seen==real)return egl_wrappers[i];}
 gl_refuse("EGL swap aliases exhausted");return 0;
}
static __eglMustCastToProperFunctionPointerType egl_proc_call(unsigned slot,const char* name){
 EglProc real=__atomic_load_n(&egl_proc_originals[slot],__ATOMIC_ACQUIRE);if(!real)fail();__eglMustCastToProperFunctionPointerType result=real(name);
 if(result&&!strcmp(name,"eglSwapBuffers")){int saved=errno;EglSwap fn;memcpy(&fn,&result,sizeof(fn));fn=bind_egl_swap(fn);memcpy(&result,&fn,sizeof(result));errno=saved;}
 return result;
}
#define EGL_PROC(n) static __eglMustCastToProperFunctionPointerType egl_proc_##n(const char* s){return egl_proc_call(n,s);}
EGL_PROC(0) EGL_PROC(1) EGL_PROC(2) EGL_PROC(3) EGL_PROC(4) EGL_PROC(5) EGL_PROC(6) EGL_PROC(7)
EGL_PROC(8) EGL_PROC(9) EGL_PROC(10) EGL_PROC(11) EGL_PROC(12) EGL_PROC(13) EGL_PROC(14) EGL_PROC(15)
static EglProc egl_proc_wrappers[16]={egl_proc_0,egl_proc_1,egl_proc_2,egl_proc_3,egl_proc_4,egl_proc_5,egl_proc_6,egl_proc_7,egl_proc_8,egl_proc_9,egl_proc_10,egl_proc_11,egl_proc_12,egl_proc_13,egl_proc_14,egl_proc_15};
static EglProc bind_egl_proc(EglProc real){
 if(!real||real==eglGetProcAddress)return real;
 for(unsigned i=0;i<16;++i)if(real==egl_proc_wrappers[i])return real;
 for(unsigned i=0;i<16;++i){EglProc seen=__atomic_load_n(&egl_proc_originals[i],__ATOMIC_ACQUIRE);if(seen==real)return egl_proc_wrappers[i];if(!seen&&__atomic_compare_exchange_n(&egl_proc_originals[i],&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))return egl_proc_wrappers[i];if(seen==real)return egl_proc_wrappers[i];}
 gl_refuse("EGL proc aliases exhausted");return 0;
}
EGLBoolean eglSwapBuffers(EGLDisplay display,EGLSurface surface){
 int saved=errno;void* pointer=symbol_resolver()(RTLD_NEXT,"eglSwapBuffers");EglSwap real;memcpy(&real,&pointer,sizeof(real));EglSwap wrapped=bind_egl_swap(real);if(!wrapped)fail();errno=saved;return wrapped(display,surface);
}
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* name){
 int saved=errno;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");EglProc real;memcpy(&real,&pointer,sizeof(real));EglProc wrapped=bind_egl_proc(real);if(!wrapped)fail();errno=saved;return wrapped(name);
}
