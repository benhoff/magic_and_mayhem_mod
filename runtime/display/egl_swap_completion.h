static void egl_surface_before_swap(uint32_t call,EGLDisplay display,EGLSurface surface){
 if(!getenv("MNM_DRAW_EGL_SWAP_COMPLETION"))return;
 typedef EGLBoolean(*Query)(EGLDisplay,EGLSurface,EGLint,EGLint*);void* pointer=symbol_resolver()(RTLD_NEXT,"eglQuerySurface");Query query;memcpy(&query,&pointer,sizeof(query));EGLint width=0,height=0;
 if(!query||!query(display,surface,EGL_WIDTH,&width)||!query(display,surface,EGL_HEIGHT,&height)||width<1||height<1||width>2048||height>2048)refuse("EGL pre-swap extent");
 unsigned char raw[64]={0};memcpy(raw,"MNMESBF1",8);put(raw+8,1);put(raw+12,64);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,width);put(raw+32,height);
 char path[4096];const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory||snprintf(path,sizeof(path),"%s/egl-surface-before-swap-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL pre-swap path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,64)!=64||close(fd))refuse("EGL pre-swap publication");
}
/* Optional reference-only post-swap GPU completion and GL geometry witness.
 * No pixel reads. Query/error-state neutrality and hardware scanout unclaimed. */
static void egl_swap_completion(uint32_t call,uint32_t width,uint32_t height){
 const char* flag=getenv("MNM_DRAW_EGL_SWAP_COMPLETION");if(!flag)return;if(strcmp(flag,"1"))refuse("EGL completion mode");
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory)refuse("EGL completion directory");
 EglProc getproc;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");memcpy(&getproc,&pointer,sizeof(getproc));if(!getproc)refuse("EGL completion resolver");
 typedef void*(*Fence)(unsigned,unsigned);typedef unsigned(*Wait)(void*,unsigned,uint64_t);typedef void(*Delete)(void*);typedef void(*Get)(unsigned,int*);typedef void*(*Context)(void);
 Fence fence;Wait wait;Delete destroy;Get get;Context context;
 #define EGL_COMPLETION_BIND(target,name) do{__eglMustCastToProperFunctionPointerType proc=getproc(name);memcpy(&(target),&proc,sizeof(target));if(!(target))refuse("EGL completion procedure");}while(0)
 EGL_COMPLETION_BIND(fence,"glFenceSync");EGL_COMPLETION_BIND(wait,"glClientWaitSync");EGL_COMPLETION_BIND(destroy,"glDeleteSync");EGL_COMPLETION_BIND(get,"glGetIntegerv");
 pointer=symbol_resolver()(RTLD_NEXT,"eglGetCurrentContext");memcpy(&context,&pointer,sizeof(context));if(!context||!context())refuse("EGL completion context");void* before=context();
 struct timespec stamp;if(clock_gettime(CLOCK_MONOTONIC,&stamp))fail();uint64_t start=(uint64_t)stamp.tv_sec*1000000000u+stamp.tv_nsec;
 void* sync=fence(0x9117,0);if(!sync)refuse("EGL completion fence");unsigned result=wait(sync,1,1000000000u);destroy(sync);
 if(result!=0x911a&&result!=0x911c)refuse("EGL completion wait");
 if(context()!=before)refuse("EGL completion context changed");
 int viewport[4],scissor[4],state[5];get(0x0ba2,viewport);get(0x0c10,scissor);const unsigned queries[5]={0x0c11,0x8ca6,0x8caa,0x0c01,0x0c02};for(unsigned i=0;i<5;++i)get(queries[i],state+i);
 if(clock_gettime(CLOCK_MONOTONIC,&stamp))fail();
 uint64_t end=(uint64_t)stamp.tv_sec*1000000000u+stamp.tv_nsec;
 unsigned char raw[128]={0};memcpy(raw,"MNMESGC1",8);put(raw+8,1);put(raw+12,128);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,width);put(raw+32,height);put(raw+36,result);put(raw+40,1);put(raw+44,1);put(raw+48,1);put(raw+56,(uint32_t)start);put(raw+60,start>>32);put(raw+64,(uint32_t)end);put(raw+68,end>>32);
 for(unsigned i=0;i<4;++i){put(raw+72+i*4,(uint32_t)viewport[i]);put(raw+88+i*4,(uint32_t)scissor[i]);}for(unsigned i=0;i<5;++i)put(raw+104+i*4,(uint32_t)state[i]);
 char path[4096];if(snprintf(path,sizeof(path),"%s/egl-swap-completion-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL completion path");int output=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(output<0||write(output,raw,128)!=128||close(output))refuse("EGL completion publication");
 egl_blit_publish(call);
 #undef EGL_COMPLETION_BIND
}
