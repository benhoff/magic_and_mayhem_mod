/* Explicit diagnostic native policy, never an original pixel replacement. */
static __thread uint32_t egl_viewport_policy_last[6],egl_viewport_policy_count;
static int egl_viewport_policy_begin(const uint32_t row[16],int previous[4]){
 const char* flag=getenv("MNM_DRAW_EGL_REFRESH_VIEWPORT");if(!flag||row[11])return 0;if(strcmp(flag,"1"))refuse("EGL viewport policy mode");
 typedef EGLDisplay(*DisplayFn)(void);typedef EGLSurface(*SurfaceFn)(EGLint);typedef EGLBoolean(*Query)(EGLDisplay,EGLSurface,EGLint,EGLint*);typedef void(*Get)(unsigned,int*);typedef void(*Viewport)(int,int,int,int);
 DisplayFn display;SurfaceFn surface;Query query;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetCurrentDisplay");memcpy(&display,&pointer,sizeof(display));pointer=symbol_resolver()(RTLD_NEXT,"eglGetCurrentSurface");memcpy(&surface,&pointer,sizeof(surface));pointer=symbol_resolver()(RTLD_NEXT,"eglQuerySurface");memcpy(&query,&pointer,sizeof(query));EglProc proc;pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");memcpy(&proc,&pointer,sizeof(proc));if(!display||!surface||!query||!proc)refuse("EGL viewport policy procedures");
 Get get;Viewport viewport;__eglMustCastToProperFunctionPointerType fn=proc("glGetIntegerv");memcpy(&get,&fn,sizeof(get));fn=proc("glViewport");memcpy(&viewport,&fn,sizeof(viewport));if(!get||!viewport)refuse("EGL viewport GL procedures");
 EGLint width=0,height=0;if(!query(display(),surface(EGL_DRAW),EGL_WIDTH,&width)||!query(display(),surface(EGL_DRAW),EGL_HEIGHT,&height)||width<1||height<1||width>2048||height>2048)refuse("EGL viewport policy extent");
 get(0x0ba2,previous);for(unsigned i=0;i<4;++i)egl_viewport_policy_last[i]=(uint32_t)previous[i];egl_viewport_policy_last[4]=(uint32_t)width;egl_viewport_policy_last[5]=(uint32_t)height;viewport(0,0,width,height);if(++egl_viewport_policy_count>16384)refuse("EGL viewport policy budget");return 1;
}
static void egl_viewport_policy_end(const int previous[4]){
 EglProc proc;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");memcpy(&proc,&pointer,sizeof(proc));typedef void(*Viewport)(int,int,int,int);Viewport viewport;__eglMustCastToProperFunctionPointerType fn=proc("glViewport");memcpy(&viewport,&fn,sizeof(viewport));if(!viewport)refuse("EGL viewport restore procedure");viewport(previous[0],previous[1],previous[2],previous[3]);
}
static void egl_viewport_policy_publish(uint32_t call){
 if(!getenv("MNM_DRAW_EGL_REFRESH_VIEWPORT"))return;
 unsigned char raw[64]={0};memcpy(raw,"MNMEVP01",8);put(raw+8,1);put(raw+12,64);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,egl_viewport_policy_count);for(unsigned i=0;i<6;++i)put(raw+32+i*4,egl_viewport_policy_last[i]);put(raw+56,1);char path[4096];const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory||snprintf(path,sizeof(path),"%s/egl-viewport-refresh-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL viewport policy path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,64)!=64||close(fd))refuse("EGL viewport policy publication");memset(egl_viewport_policy_last,0,sizeof(egl_viewport_policy_last));egl_viewport_policy_count=0;
}
