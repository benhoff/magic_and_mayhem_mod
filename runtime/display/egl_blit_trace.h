#include "egl_viewport_policy.h"
#include "egl_drawable_rebind.h"
/* Opt-in metadata only. Unknown/RTLD_NEXT GL routes remain unobserved. */
typedef void(*EglBlit)(int,int,int,int,int,int,int,int,unsigned,unsigned);
static EglBlit egl_blit_originals[16];
static __thread uint32_t egl_blit_last[16],egl_blit_count;
static __thread unsigned char* egl_blit_gpu_bytes;static __thread uint32_t egl_blit_gpu_size;
static uint32_t egl_blit_gpu_total;
static void egl_blit_gpu_read(const uint32_t row[16]){
 if(egl_blit_gpu_bytes){free(egl_blit_gpu_bytes);egl_blit_gpu_bytes=0;egl_blit_gpu_size=0;}
 if(!getenv("MNM_DRAW_EGL_GPU_SOURCE_WITNESS")||row[11]||!row[12]||!row[13]||!row[14])return;
 if(row[13]>2048||row[14]>2048)refuse("EGL GPU source extent");
 EglProc proc;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");memcpy(&proc,&pointer,sizeof(proc));if(!proc)refuse("EGL GPU source resolver");
 typedef void(*Get)(unsigned,int*);typedef void(*Image)(unsigned,int,unsigned,unsigned,int,void*);Get get;Image image;__eglMustCastToProperFunctionPointerType fn=proc("glGetIntegerv");memcpy(&get,&fn,sizeof(get));fn=proc("glGetTextureImage");memcpy(&image,&fn,sizeof(image));if(!get||!image)refuse("EGL GPU source procedures");
 const unsigned names[6]={0x0d05,0x0d02,0x0d03,0x0d04,0x0d00,0x88ed};int state[6];for(unsigned i=0;i<6;++i)get(names[i],state+i);if(state[0]<1||state[0]>8||(row[13]*2)%(unsigned)state[0]||state[1]||state[2]||state[3]||state[4]||state[5])refuse("EGL GPU source pack state");
 uint32_t size=row[13]*row[14]*2,total=__atomic_add_fetch(&egl_blit_gpu_total,size,__ATOMIC_RELAXED);if(total>512u*1024u*1024u)refuse("EGL GPU source budget");egl_blit_gpu_bytes=malloc(size);if(!egl_blit_gpu_bytes)refuse("EGL GPU source allocation");memset(egl_blit_gpu_bytes,0xcd,size);image(row[12],0,0x1907,0x8363,(int)size,egl_blit_gpu_bytes);egl_blit_gpu_size=size;
}
static void egl_blit_call(unsigned slot,int a,int b,int c,int d,int e,int f,int g,int h,unsigned mask,unsigned filter){
 EglBlit real=__atomic_load_n(egl_blit_originals+slot,__ATOMIC_ACQUIRE);if(!real)fail();int saved=errno;
 uint32_t row[16]={(uint32_t)a,(uint32_t)b,(uint32_t)c,(uint32_t)d,(uint32_t)e,(uint32_t)f,(uint32_t)g,(uint32_t)h,mask,filter,0,0,0,0,0,0};
 const char* enabled=getenv("MNM_DRAW_EGL_SWAP_COMPLETION");
 if(enabled){
  EglProc proc;void* pointer=symbol_resolver()(RTLD_NEXT,"eglGetProcAddress");memcpy(&proc,&pointer,sizeof(proc));if(!proc)refuse("EGL blit resolver");
  typedef void(*Get)(unsigned,int*);typedef void(*Attachment)(unsigned,unsigned,unsigned,int*);typedef void(*Level)(unsigned,int,unsigned,int*);Get get;Attachment attachment;Level level;
  __eglMustCastToProperFunctionPointerType fn=proc("glGetIntegerv");memcpy(&get,&fn,sizeof(get));fn=proc("glGetFramebufferAttachmentParameteriv");memcpy(&attachment,&fn,sizeof(attachment));fn=proc("glGetTextureLevelParameteriv");memcpy(&level,&fn,sizeof(level));if(!get||!attachment||!level)refuse("EGL blit metadata procedures");
  int read,draw,buffer;get(0x8caa,&read);get(0x8ca6,&draw);get(0x0c02,&buffer);row[10]=read;row[11]=draw;row[15]=buffer;
  if(read&&buffer>=0x8ce0&&buffer<=0x8cef){int type=0,object=0,width=0,height=0;attachment(0x8ca8,(unsigned)buffer,0x8cd0,&type);if(type==0x1702){attachment(0x8ca8,(unsigned)buffer,0x8cd1,&object);if(object){level((unsigned)object,0,0x1000,&width);level((unsigned)object,0,0x1001,&height);row[12]=object;row[13]=width;row[14]=height;}}}
 }
 if(enabled)egl_drawable_rebind(row);
 int previous[4];int refresh=enabled?egl_viewport_policy_begin(row,previous):0;
 errno=saved;real(a,b,c,d,e,f,g,h,mask,filter);saved=errno;
 if(refresh)egl_viewport_policy_end(previous);
 if(enabled){egl_blit_gpu_read(row);memcpy(egl_blit_last,row,sizeof(row));if(++egl_blit_count>16384)refuse("EGL blit count budget");}
 errno=saved;
}
#define EGL_BLIT_SLOT(n) static void egl_blit_##n(int a,int b,int c,int d,int e,int f,int g,int h,unsigned m,unsigned q){egl_blit_call(n,a,b,c,d,e,f,g,h,m,q);}
EGL_BLIT_SLOT(0) EGL_BLIT_SLOT(1) EGL_BLIT_SLOT(2) EGL_BLIT_SLOT(3) EGL_BLIT_SLOT(4) EGL_BLIT_SLOT(5) EGL_BLIT_SLOT(6) EGL_BLIT_SLOT(7)
EGL_BLIT_SLOT(8) EGL_BLIT_SLOT(9) EGL_BLIT_SLOT(10) EGL_BLIT_SLOT(11) EGL_BLIT_SLOT(12) EGL_BLIT_SLOT(13) EGL_BLIT_SLOT(14) EGL_BLIT_SLOT(15)
static EglBlit egl_blit_wrappers[16]={egl_blit_0,egl_blit_1,egl_blit_2,egl_blit_3,egl_blit_4,egl_blit_5,egl_blit_6,egl_blit_7,egl_blit_8,egl_blit_9,egl_blit_10,egl_blit_11,egl_blit_12,egl_blit_13,egl_blit_14,egl_blit_15};
static void* bind_egl_blit(void* pointer){
 if(!pointer||!getenv("MNM_DRAW_EGL_SWAP_COMPLETION"))return pointer;
 EglBlit real;memcpy(&real,&pointer,sizeof(real));
 for(unsigned i=0;i<16;++i)if(real==egl_blit_wrappers[i])return pointer;
 for(unsigned i=0;i<16;++i){EglBlit seen=__atomic_load_n(egl_blit_originals+i,__ATOMIC_ACQUIRE);if(seen==real||(!seen&&__atomic_compare_exchange_n(egl_blit_originals+i,&seen,real,0,__ATOMIC_RELEASE,__ATOMIC_ACQUIRE))){EglBlit wrapper=egl_blit_wrappers[i];memcpy(&pointer,&wrapper,sizeof(pointer));return pointer;}}
 refuse("EGL blit aliases exhausted");return 0;
}
static void egl_blit_publish(uint32_t call){
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");unsigned char raw[96]={0};memcpy(raw,"MNMEGBT1",8);put(raw+8,1);put(raw+12,96);put(raw+16,call);put(raw+20,(uint32_t)getpid());put(raw+24,(uint32_t)syscall(SYS_gettid));put(raw+28,egl_blit_count);for(unsigned i=0;i<16;++i)put(raw+32+i*4,egl_blit_last[i]);
 egl_viewport_policy_publish(call);egl_drawable_rebind_publish(call);
 char path[4096];if(!directory||snprintf(path,sizeof(path),"%s/egl-blit-before-swap-%06u.bin",directory,call)>=(int)sizeof(path))refuse("EGL blit path");int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(fd<0||write(fd,raw,96)!=96||close(fd))refuse("EGL blit publication");if(egl_blit_gpu_bytes){if(snprintf(path,sizeof(path),"%s/egl-gl-source-%06u.565",directory,call)>=(int)sizeof(path))refuse("EGL GPU source path");int output=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(output<0||write(output,egl_blit_gpu_bytes,egl_blit_gpu_size)!=(ssize_t)egl_blit_gpu_size||close(output))refuse("EGL GPU source publication");free(egl_blit_gpu_bytes);egl_blit_gpu_bytes=0;egl_blit_gpu_size=0;}memset(egl_blit_last,0,sizeof(egl_blit_last));egl_blit_count=0;
}
#undef EGL_BLIT_SLOT
