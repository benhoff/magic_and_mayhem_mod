/* Explicit resize policy: rebind the same current EGL objects before final blit.
 * No image translation, source substitution or original-equivalence claim. */
static __thread EGLContext egl_rebind_context;static __thread EGLSurface egl_rebind_surface;static __thread EGLDisplay egl_rebind_display;
static __thread EGLint egl_rebind_width,egl_rebind_height;
static __thread uint32_t egl_rebind_count,egl_rebind_last[7];
static void egl_drawable_rebind(const uint32_t row[16]){
 const char* flag=getenv("MNM_DRAW_EGL_REBIND_DRAWABLE");if(!flag||row[11])return;if(strcmp(flag,"1")&&strcmp(flag,"2"))refuse("EGL drawable rebind mode");
 typedef EGLDisplay(*DisplayFn)(void);typedef EGLSurface(*SurfaceFn)(EGLint);typedef EGLContext(*ContextFn)(void);typedef EGLBoolean(*Query)(EGLDisplay,EGLSurface,EGLint,EGLint*);typedef EGLBoolean(*Make)(EGLDisplay,EGLSurface,EGLSurface,EGLContext);typedef void(*Get)(unsigned,int*);
 DisplayFn display;SurfaceFn surface;ContextFn context;Query query;Make make;EglProc proc;void* pointer;
 #define EGL_REBIND_SYMBOL(target,name) do{pointer=symbol_resolver()(RTLD_NEXT,name);memcpy(&(target),&pointer,sizeof(target));if(!(target))refuse("EGL drawable rebind procedure");}while(0)
 EGL_REBIND_SYMBOL(display,"eglGetCurrentDisplay");EGL_REBIND_SYMBOL(surface,"eglGetCurrentSurface");EGL_REBIND_SYMBOL(context,"eglGetCurrentContext");EGL_REBIND_SYMBOL(query,"eglQuerySurface");EGL_REBIND_SYMBOL(make,"eglMakeCurrent");EGL_REBIND_SYMBOL(proc,"eglGetProcAddress");
 EGLDisplay d=display();EGLSurface draw=surface(EGL_DRAW),read=surface(EGL_READ);EGLContext c=context();EGLint width=0,height=0;if(!d||!draw||!c||!query(d,draw,EGL_WIDTH,&width)||!query(d,draw,EGL_HEIGHT,&height)||width<1||height<1||width>2048||height>2048)refuse("EGL drawable rebind identity/extent");
 unsigned reason=c!=egl_rebind_context||d!=egl_rebind_display||draw!=egl_rebind_surface?1:width!=egl_rebind_width||height!=egl_rebind_height?2:0;if(!reason)return;
 Get get;__eglMustCastToProperFunctionPointerType fn=proc("glGetIntegerv");memcpy(&get,&fn,sizeof(get));if(!get)refuse("EGL drawable rebind GL query");
 int before[8],after[8];const unsigned names[4]={0x8ca6,0x8caa,0x0c01,0x0c02};for(unsigned i=0;i<4;++i)get(names[i],before+i);get(0x0ba2,before+4);
 if(!strcmp(flag,"2")&&(!make(d,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT)||context()!=EGL_NO_CONTEXT||display()!=EGL_NO_DISPLAY||surface(EGL_DRAW)!=EGL_NO_SURFACE||surface(EGL_READ)!=EGL_NO_SURFACE))refuse("EGL drawable release result");
 if(!make(d,draw,read,c)||display()!=d||surface(EGL_DRAW)!=draw||surface(EGL_READ)!=read||context()!=c)refuse("EGL drawable rebind result");
 for(unsigned i=0;i<4;++i)get(names[i],after+i);
 get(0x0ba2,after+4);if(memcmp(before,after,sizeof(before)))refuse("EGL drawable rebind GL state changed");
 egl_rebind_last[0]=reason;egl_rebind_last[1]=(uint32_t)egl_rebind_width;egl_rebind_last[2]=(uint32_t)egl_rebind_height;egl_rebind_last[3]=width;egl_rebind_last[4]=height;egl_rebind_last[5]=1;egl_rebind_last[6]=1;if(++egl_rebind_count>16384)refuse("EGL drawable rebind budget");
 egl_rebind_context=c;egl_rebind_display=d;egl_rebind_surface=draw;egl_rebind_width=width;egl_rebind_height=height;
 #undef EGL_REBIND_SYMBOL
}
static void egl_drawable_rebind_publish(uint32_t call){
 if(!getenv("MNM_DRAW_EGL_REBIND_DRAWABLE"))return;
 unsigned char raw[96]={0};memcpy(raw,"MNMERB01",8);put(raw+8,2);put(raw+12,96);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,egl_rebind_count);for(unsigned i=0;i<7;++i)put(raw+32+i*4,egl_rebind_last[i]);
 put(raw+60,!strcmp(getenv("MNM_DRAW_EGL_REBIND_DRAWABLE"),"2")?2:1);if(egl_rebind_count&&!strcmp(getenv("MNM_DRAW_EGL_REBIND_DRAWABLE"),"2"))put(raw+64,1);
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");char path[4096];if(!directory||snprintf(path,sizeof(path),"%s/egl-drawable-rebind-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL drawable rebind path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,96)!=96||close(fd))refuse("EGL drawable rebind publication");memset(egl_rebind_last,0,sizeof(egl_rebind_last));egl_rebind_count=0;
}
