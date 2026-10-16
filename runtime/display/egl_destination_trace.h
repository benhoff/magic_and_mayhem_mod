/* Explicit comparator-only read of the nominal drawable rectangle before swap.
 * Binding/readback can perturb driver validation; no observation neutrality. */
static uint32_t egl_destination_total;
static void egl_destination_before_swap(uint32_t call,EGLDisplay display,EGLSurface surface){
 const char* enabled=getenv("MNM_DRAW_EGL_GPU_DESTINATION_WITNESS");if(!enabled)return;
 if(strcmp(enabled,"1")||!getenv("MNM_DRAW_EGL_SWAP_COMPLETION"))refuse("EGL destination witness mode");
 int saved=errno;
 typedef EGLDisplay(*CurrentDisplay)(void);typedef EGLSurface(*CurrentSurface)(EGLint);typedef EGLContext(*CurrentContext)(void);typedef EGLBoolean(*Query)(EGLDisplay,EGLSurface,EGLint,EGLint*);
 CurrentDisplay current_display;CurrentSurface current_surface;CurrentContext current_context;Query query;EglProc proc;void* pointer;
 #define EGL_DEST_SYMBOL(target,name) do{pointer=symbol_resolver()(RTLD_NEXT,name);memcpy(&(target),&pointer,sizeof(target));if(!(target))refuse("EGL destination procedure");}while(0)
 EGL_DEST_SYMBOL(current_display,"eglGetCurrentDisplay");EGL_DEST_SYMBOL(current_surface,"eglGetCurrentSurface");EGL_DEST_SYMBOL(current_context,"eglGetCurrentContext");EGL_DEST_SYMBOL(query,"eglQuerySurface");EGL_DEST_SYMBOL(proc,"eglGetProcAddress");
 EGLContext context=current_context();if(!context||current_display()!=display||current_surface(EGL_DRAW)!=surface)refuse("EGL destination context");
 EGLint width=0,height=0;if(!query(display,surface,EGL_WIDTH,&width)||!query(display,surface,EGL_HEIGHT,&height)||width<1||height<1||width>2048||height>2048)refuse("EGL destination extent");
 typedef void(*Get)(unsigned,int*);typedef void(*Bind)(unsigned,unsigned);typedef void(*Buffer)(unsigned);typedef void(*Pixels)(int,int,int,int,unsigned,unsigned,void*);
 Get get;Bind bind;Buffer buffer;Pixels pixels;
 #define EGL_DEST_GL(target,name) do{__eglMustCastToProperFunctionPointerType fn=proc(name);memcpy(&(target),&fn,sizeof(target));if(!(target))refuse("EGL destination GL procedure");}while(0)
 EGL_DEST_GL(get,"glGetIntegerv");EGL_DEST_GL(bind,"glBindFramebuffer");EGL_DEST_GL(buffer,"glReadBuffer");EGL_DEST_GL(pixels,"glReadPixels");
 int draw,read,read_buffer,draw_buffer;get(0x8ca6,&draw);get(0x8caa,&read);get(0x0c02,&read_buffer);get(0x0c01,&draw_buffer);
 if(draw||(draw_buffer!=0x405&&draw_buffer!=0x402&&draw_buffer!=0x404&&draw_buffer!=0x400))refuse("EGL destination buffer route");
 const unsigned names[6]={0x0d05,0x0d02,0x0d03,0x0d04,0x0d00,0x88ed};int pack[6];for(unsigned i=0;i<6;++i)get(names[i],pack+i);
 if((pack[0]!=1&&pack[0]!=2&&pack[0]!=4&&pack[0]!=8)||(width*2)%pack[0]||pack[1]||pack[2]||pack[3]||pack[4]||pack[5])refuse("EGL destination pack state");
 uint32_t size=(uint32_t)width*(uint32_t)height*2,total=__atomic_add_fetch(&egl_destination_total,size,__ATOMIC_RELAXED);if(total>512u*1024u*1024u)refuse("EGL destination budget");unsigned char* data=malloc(size);if(!data)refuse("EGL destination allocation");memset(data,0xcd,size);
 struct timespec stamp;if(clock_gettime(CLOCK_MONOTONIC,&stamp))fail();uint64_t begin_ns=(uint64_t)stamp.tv_sec*1000000000u+stamp.tv_nsec;
 bind(0x8ca8,0);int default_read;get(0x0c02,&default_read);buffer((unsigned)draw_buffer);pixels(0,0,width,height,0x1907,0x8363,data);buffer((unsigned)default_read);bind(0x8ca8,(unsigned)read);
 int restored_read,restored_buffer,restored_draw,restored_draw_buffer;get(0x8caa,&restored_read);get(0x0c02,&restored_buffer);get(0x8ca6,&restored_draw);get(0x0c01,&restored_draw_buffer);
 if(restored_read!=read||restored_buffer!=read_buffer||restored_draw!=draw||restored_draw_buffer!=draw_buffer||current_context()!=context||current_display()!=display||current_surface(EGL_DRAW)!=surface)refuse("EGL destination restoration");
 if(clock_gettime(CLOCK_MONOTONIC,&stamp))fail();
 uint64_t end_ns=(uint64_t)stamp.tv_sec*1000000000u+stamp.tv_nsec;
 unsigned char raw[128]={0};memcpy(raw,"MNMEDB01",8);put(raw+8,1);put(raw+12,128);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,width);put(raw+32,height);put(raw+36,draw);put(raw+40,read);put(raw+44,read_buffer);put(raw+48,default_read);put(raw+52,draw_buffer);put(raw+56,size);put(raw+60,1);for(unsigned i=0;i<6;++i)put(raw+64+i*4,(uint32_t)pack[i]);put(raw+88,(uint32_t)begin_ns);put(raw+92,begin_ns>>32);put(raw+96,(uint32_t)end_ns);put(raw+100,end_ns>>32);put(raw+104,1);put(raw+108,1);
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");char path[4096];if(!directory||snprintf(path,sizeof(path),"%s/egl-destination-before-swap-%06u.565",directory,call)>=(int)sizeof(path))refuse("EGL destination pixel path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,data,size)!=(ssize_t)size||close(fd))refuse("EGL destination pixels");free(data);
 if(snprintf(path,sizeof(path),"%s/egl-destination-before-swap-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL destination metadata path");
 fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,128)!=128||close(fd))refuse("EGL destination metadata");errno=saved;
 #undef EGL_DEST_SYMBOL
 #undef EGL_DEST_GL
}
