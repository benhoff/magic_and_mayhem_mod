#define MNM_X11_PROTOCOL_VERSION 2
#include "x11_delivery.c"
#include <GL/glx.h>
/* Each alias keeps the exact original function returned by the real resolver. */
typedef void(*Swap)(Display*,GLXDrawable);
typedef void*(*Symbol)(void*,const char*);
typedef __GLXextFuncPtr(*Proc)(const GLubyte*);
static Symbol original_symbol;
static Swap originals[16];
static Proc proc_originals[2][16];
static void gl_refuse(const char* reason){write(2,reason,strlen(reason));write(2,"\n",1);fail();}
static Symbol symbol_resolver(void){
 Symbol real=__atomic_load_n(&original_symbol,__ATOMIC_ACQUIRE);
 if(!real){int saved=errno;void* ptr=dlvsym(RTLD_NEXT,"dlsym","GLIBC_2.2.5");if(!ptr)gl_refuse("GLX dlsym bootstrap refused");memcpy(&real,&ptr,sizeof(real));__atomic_store_n(&original_symbol,real,__ATOMIC_RELEASE);errno=saved;}
 return real;
}
static void swap_call(unsigned slot,Display* display,GLXDrawable drawable){
 Swap real=__atomic_load_n(&originals[slot],__ATOMIC_ACQUIRE);if(!real)fail();
 uint32_t args[8]={(uint32_t)drawable,slot+1,0,0,0,0,0,0};uint32_t id=begin(10,args);
 real(display,drawable);int saved=errno;record(2,id,10,args,0,saved);errno=saved;
}
#define SLOT(n) static void swap_##n(Display* d,GLXDrawable v){swap_call(n,d,v);}
SLOT(0) SLOT(1) SLOT(2) SLOT(3) SLOT(4) SLOT(5) SLOT(6) SLOT(7)
SLOT(8) SLOT(9) SLOT(10) SLOT(11) SLOT(12) SLOT(13) SLOT(14) SLOT(15)
static Swap wrappers[16]={swap_0,swap_1,swap_2,swap_3,swap_4,swap_5,swap_6,swap_7,swap_8,swap_9,swap_10,swap_11,swap_12,swap_13,swap_14,swap_15};
static Swap bind_swap(Swap real){
 if(!real)return 0;
 if(real==glXSwapBuffers)return real;
 for(unsigned i=0;i<16;++i)if(real==wrappers[i])return real;
 for(unsigned i=0;i<16;++i){
  Swap seen=__atomic_load_n(&originals[i],__ATOMIC_ACQUIRE);
  if(seen==real)return wrappers[i];
  if(!seen&&__atomic_compare_exchange_n(&originals[i],&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))return wrappers[i];
  if(seen==real)return wrappers[i];
 }
 gl_refuse("GLX swap aliases exhausted");return 0;
}
static __GLXextFuncPtr proc_call(unsigned which,unsigned slot,const GLubyte* name){
 Proc real=__atomic_load_n(&proc_originals[which][slot],__ATOMIC_ACQUIRE);if(!real)fail();
 __GLXextFuncPtr result=real(name);
 if(result&&!strcmp((const char*)name,"glXSwapBuffers")){int saved=errno;Swap fn;memcpy(&fn,&result,sizeof(fn));fn=bind_swap(fn);memcpy(&result,&fn,sizeof(result));errno=saved;}
 return result;
}
#define PROCS(n) static __GLXextFuncPtr proc_0_##n(const GLubyte* s){return proc_call(0,n,s);} static __GLXextFuncPtr proc_1_##n(const GLubyte* s){return proc_call(1,n,s);}
PROCS(0) PROCS(1) PROCS(2) PROCS(3) PROCS(4) PROCS(5) PROCS(6) PROCS(7)
PROCS(8) PROCS(9) PROCS(10) PROCS(11) PROCS(12) PROCS(13) PROCS(14) PROCS(15)
static Proc proc_wrappers[2][16]={{proc_0_0,proc_0_1,proc_0_2,proc_0_3,proc_0_4,proc_0_5,proc_0_6,proc_0_7,proc_0_8,proc_0_9,proc_0_10,proc_0_11,proc_0_12,proc_0_13,proc_0_14,proc_0_15},{proc_1_0,proc_1_1,proc_1_2,proc_1_3,proc_1_4,proc_1_5,proc_1_6,proc_1_7,proc_1_8,proc_1_9,proc_1_10,proc_1_11,proc_1_12,proc_1_13,proc_1_14,proc_1_15}};
static Proc bind_proc(unsigned which,Proc real){
 if(!real)return 0;
 if(real==(which?glXGetProcAddress:glXGetProcAddressARB))return real;
 for(unsigned i=0;i<16;++i)if(real==proc_wrappers[which][i])return real;
 for(unsigned i=0;i<16;++i){
  Proc seen=__atomic_load_n(&proc_originals[which][i],__ATOMIC_ACQUIRE);
  if(seen==real)return proc_wrappers[which][i];
  if(!seen&&__atomic_compare_exchange_n(&proc_originals[which][i],&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))return proc_wrappers[which][i];
  if(seen==real)return proc_wrappers[which][i];
 }
 gl_refuse("GLX proc aliases exhausted");return 0;
}
static void* wrapped_symbol(void* handle,const char* name){
 Symbol real=symbol_resolver();void* ptr=real(handle,name);
 if(!ptr)return ptr;
 int saved=errno;
 if(!strcmp(name,"glXSwapBuffers")){Swap fn;memcpy(&fn,&ptr,sizeof(fn));if(fn!=glXSwapBuffers)fn=bind_swap(fn);memcpy(&ptr,&fn,sizeof(ptr));}
 else if(!strcmp(name,"glXGetProcAddressARB")||!strcmp(name,"glXGetProcAddress")){
  unsigned which=!strcmp(name,"glXGetProcAddress");Proc fn;memcpy(&fn,&ptr,sizeof(fn));fn=bind_proc(which,fn);memcpy(&ptr,&fn,sizeof(ptr));
 }
 errno=saved;return ptr;
}
/* Assembly tail forwarding preserves caller-relative RTLD_NEXT semantics.
 * RTLD_NEXT-selected GL functions remain an explicit unobserved route. */
__attribute__((visibility("hidden"))) Symbol mnm_delivery_symbol_target(void* handle,const char* name){
 int saved=errno;Symbol target=symbol_resolver();
 if(handle!=RTLD_NEXT&&(!strcmp(name,"glXSwapBuffers")||!strcmp(name,"glXGetProcAddressARB")||!strcmp(name,"glXGetProcAddress")))target=wrapped_symbol;
 errno=saved;return target;
}
void glXSwapBuffers(Display* display,GLXDrawable drawable){
 int saved=errno;void* ptr=symbol_resolver()(RTLD_NEXT,"glXSwapBuffers");Swap real;memcpy(&real,&ptr,sizeof(real));Swap wrapped=bind_swap(real);if(!wrapped)fail();errno=saved;wrapped(display,drawable);
}
static __GLXextFuncPtr proc_lookup(unsigned which,const GLubyte* name){
 int saved=errno;void* ptr=symbol_resolver()(RTLD_NEXT,which?"glXGetProcAddress":"glXGetProcAddressARB");if(!ptr)fail();Proc real;memcpy(&real,&ptr,sizeof(real));Proc wrapped=bind_proc(which,real);errno=saved;return wrapped(name);
}
__GLXextFuncPtr glXGetProcAddressARB(const GLubyte* name){return proc_lookup(0,name);}
__GLXextFuncPtr glXGetProcAddress(const GLubyte* name){return proc_lookup(1,name);}
