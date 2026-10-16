/* Diagnostic X11 native-window association; never export EGL host pointers. */
typedef EGLSurface(*EglWindowCreate)(EGLDisplay,EGLConfig,uintptr_t,const void*);
typedef EGLBoolean(*EglWindowDestroy)(EGLDisplay,EGLSurface);
static EglWindowCreate egl_window_creates[3][16];static EglWindowDestroy egl_window_destroys[16];
static struct {EGLSurface surface;EGLDisplay display;uint32_t generation;Window window;} egl_window_objects[256];
static uint32_t egl_window_generation;static pthread_mutex_t egl_window_lock=PTHREAD_MUTEX_INITIALIZER;
static EGLSurface egl_window_create_call(unsigned kind,unsigned slot,EGLDisplay display,EGLConfig config,uintptr_t native,const void* attributes){
 EglWindowCreate real=__atomic_load_n(&egl_window_creates[kind][slot],__ATOMIC_ACQUIRE);if(!real)fail();EGLSurface surface=real(display,config,native,attributes);int saved=errno;
 if(surface!=EGL_NO_SURFACE&&getenv("MNM_DRAW_EGL_SWAP_COMPLETION")){
  Window window=(Window)native;if(kind&&native)memcpy(&window,(void*)native,sizeof(window));if(!window||window>UINT32_MAX)refuse("EGL X11 window identity");
  if(pthread_mutex_lock(&egl_window_lock))fail();
  unsigned found=256;for(unsigned i=0;i<256;++i)if(!egl_window_objects[i].surface){found=i;break;}
  if(found==256||egl_window_generation==UINT32_MAX)refuse("EGL window registry budget");
  egl_window_objects[found].surface=surface;egl_window_objects[found].display=display;egl_window_objects[found].window=window;egl_window_objects[found].generation=++egl_window_generation;if(pthread_mutex_unlock(&egl_window_lock))fail();
 }
 errno=saved;return surface;
}
static EGLBoolean egl_window_destroy_call(unsigned slot,EGLDisplay display,EGLSurface surface){
 EglWindowDestroy real=__atomic_load_n(egl_window_destroys+slot,__ATOMIC_ACQUIRE);if(!real)fail();EGLBoolean result=real(display,surface);int saved=errno;
 if(result&&getenv("MNM_DRAW_EGL_SWAP_COMPLETION")){if(pthread_mutex_lock(&egl_window_lock))fail();for(unsigned i=0;i<256;++i)if(egl_window_objects[i].surface==surface&&egl_window_objects[i].display==display)memset(egl_window_objects+i,0,sizeof(egl_window_objects[i]));if(pthread_mutex_unlock(&egl_window_lock))fail();}errno=saved;return result;
}
#define EGL_WINDOW_SLOT(k,n) static EGLSurface egl_window_create_##k##_##n(EGLDisplay d,EGLConfig c,uintptr_t w,const void* a){return egl_window_create_call(k,n,d,c,w,a);}
#define EGL_WINDOW_ROW(k) EGL_WINDOW_SLOT(k,0) EGL_WINDOW_SLOT(k,1) EGL_WINDOW_SLOT(k,2) EGL_WINDOW_SLOT(k,3) EGL_WINDOW_SLOT(k,4) EGL_WINDOW_SLOT(k,5) EGL_WINDOW_SLOT(k,6) EGL_WINDOW_SLOT(k,7) EGL_WINDOW_SLOT(k,8) EGL_WINDOW_SLOT(k,9) EGL_WINDOW_SLOT(k,10) EGL_WINDOW_SLOT(k,11) EGL_WINDOW_SLOT(k,12) EGL_WINDOW_SLOT(k,13) EGL_WINDOW_SLOT(k,14) EGL_WINDOW_SLOT(k,15)
EGL_WINDOW_ROW(0) EGL_WINDOW_ROW(1) EGL_WINDOW_ROW(2)
#define EGL_WINDOW_ENTRIES(k) {egl_window_create_##k##_0,egl_window_create_##k##_1,egl_window_create_##k##_2,egl_window_create_##k##_3,egl_window_create_##k##_4,egl_window_create_##k##_5,egl_window_create_##k##_6,egl_window_create_##k##_7,egl_window_create_##k##_8,egl_window_create_##k##_9,egl_window_create_##k##_10,egl_window_create_##k##_11,egl_window_create_##k##_12,egl_window_create_##k##_13,egl_window_create_##k##_14,egl_window_create_##k##_15}
static EglWindowCreate egl_window_create_wrappers[3][16]={EGL_WINDOW_ENTRIES(0),EGL_WINDOW_ENTRIES(1),EGL_WINDOW_ENTRIES(2)};
#define EGL_DESTROY_SLOT(n) static EGLBoolean egl_window_destroy_##n(EGLDisplay d,EGLSurface s){return egl_window_destroy_call(n,d,s);}
EGL_DESTROY_SLOT(0) EGL_DESTROY_SLOT(1) EGL_DESTROY_SLOT(2) EGL_DESTROY_SLOT(3) EGL_DESTROY_SLOT(4) EGL_DESTROY_SLOT(5) EGL_DESTROY_SLOT(6) EGL_DESTROY_SLOT(7) EGL_DESTROY_SLOT(8) EGL_DESTROY_SLOT(9) EGL_DESTROY_SLOT(10) EGL_DESTROY_SLOT(11) EGL_DESTROY_SLOT(12) EGL_DESTROY_SLOT(13) EGL_DESTROY_SLOT(14) EGL_DESTROY_SLOT(15)
static EglWindowDestroy egl_window_destroy_wrappers[16]={egl_window_destroy_0,egl_window_destroy_1,egl_window_destroy_2,egl_window_destroy_3,egl_window_destroy_4,egl_window_destroy_5,egl_window_destroy_6,egl_window_destroy_7,egl_window_destroy_8,egl_window_destroy_9,egl_window_destroy_10,egl_window_destroy_11,egl_window_destroy_12,egl_window_destroy_13,egl_window_destroy_14,egl_window_destroy_15};
static int egl_window_symbol(const char* name){return !strcmp(name,"eglCreateWindowSurface")||!strcmp(name,"eglCreatePlatformWindowSurface")||!strcmp(name,"eglCreatePlatformWindowSurfaceEXT")||!strcmp(name,"eglDestroySurface");}
static void* bind_egl_window(const char* name,void* pointer){
 if(!pointer||!getenv("MNM_DRAW_EGL_SWAP_COMPLETION"))return pointer;
 if(!strcmp(name,"eglDestroySurface")){EglWindowDestroy real;memcpy(&real,&pointer,sizeof(real));for(unsigned i=0;i<16;++i)if(real==egl_window_destroy_wrappers[i])return pointer;for(unsigned i=0;i<16;++i){EglWindowDestroy seen=__atomic_load_n(egl_window_destroys+i,__ATOMIC_ACQUIRE);if(seen==real||(!seen&&__atomic_compare_exchange_n(egl_window_destroys+i,&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))){EglWindowDestroy wrapper=egl_window_destroy_wrappers[i];memcpy(&pointer,&wrapper,sizeof(pointer));return pointer;}}}
 else{unsigned kind=!strcmp(name,"eglCreateWindowSurface")?0:!strcmp(name,"eglCreatePlatformWindowSurface")?1:2;EglWindowCreate real;memcpy(&real,&pointer,sizeof(real));for(unsigned i=0;i<16;++i)if(real==egl_window_create_wrappers[kind][i])return pointer;for(unsigned i=0;i<16;++i){EglWindowCreate seen=__atomic_load_n(&egl_window_creates[kind][i],__ATOMIC_ACQUIRE);if(seen==real||(!seen&&__atomic_compare_exchange_n(&egl_window_creates[kind][i],&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))){EglWindowCreate wrapper=egl_window_create_wrappers[kind][i];memcpy(&pointer,&wrapper,sizeof(pointer));return pointer;}}}
 refuse("EGL window aliases exhausted");return 0;
}
static void egl_window_point(uint32_t call,EGLDisplay display,EGLSurface surface,unsigned phase){
 if(!getenv("MNM_DRAW_EGL_SWAP_COMPLETION"))return;
 unsigned char raw[96]={0};memcpy(raw,"MNMEW001",8);put(raw+8,1);put(raw+12,96);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,phase);
 Window window=0;uint32_t generation=0;if(pthread_mutex_lock(&egl_window_lock))fail();for(unsigned i=0;i<256;++i)if(egl_window_objects[i].surface==surface&&egl_window_objects[i].display==display){window=egl_window_objects[i].window;generation=egl_window_objects[i].generation;break;}if(pthread_mutex_unlock(&egl_window_lock))fail();
 if(window){Display* x=XOpenDisplay(0);XWindowAttributes attrs;Window child;int rootx,rooty;if(!x||!XGetWindowAttributes(x,window,&attrs)||!XTranslateCoordinates(x,window,attrs.root,0,0,&rootx,&rooty,&child))refuse("EGL window geometry");put(raw+32,1);put(raw+36,(uint32_t)window);put(raw+40,generation);put(raw+44,(uint32_t)rootx);put(raw+48,(uint32_t)rooty);put(raw+52,attrs.width);put(raw+56,attrs.height);put(raw+60,attrs.map_state);put(raw+64,attrs.x);put(raw+68,attrs.y);put(raw+72,(uint32_t)attrs.root);put(raw+76,attrs.border_width);put(raw+80,attrs.depth);XCloseDisplay(x);}
 char path[4096];const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory||snprintf(path,sizeof(path),"%s/egl-window-%s-%06u.bin",directory,phase==1?"before":"after",call)>=(int)sizeof(path))refuse("EGL window path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,96)!=96||close(fd))refuse("EGL window publication");
}
EGLSurface eglCreateWindowSurface(EGLDisplay d,EGLConfig c,EGLNativeWindowType w,const EGLint* a){int saved=errno;void* pointer=symbol_resolver()(RTLD_NEXT,"eglCreateWindowSurface");pointer=bind_egl_window("eglCreateWindowSurface",pointer);EglWindowCreate real;memcpy(&real,&pointer,sizeof(real));if(!real)fail();errno=saved;return real(d,c,(uintptr_t)w,a);}
EGLBoolean eglDestroySurface(EGLDisplay d,EGLSurface s){int saved=errno;void* pointer=symbol_resolver()(RTLD_NEXT,"eglDestroySurface");pointer=bind_egl_window("eglDestroySurface",pointer);EglWindowDestroy real;memcpy(&real,&pointer,sizeof(real));if(!real)fail();errno=saved;return real(d,s);}
#undef EGL_WINDOW_SLOT
#undef EGL_WINDOW_ROW
#undef EGL_WINDOW_ENTRIES
#undef EGL_DESTROY_SLOT
