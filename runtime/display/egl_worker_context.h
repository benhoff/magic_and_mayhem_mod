#include <sys/mman.h>
#include "../../protocols/include/mnm/drawing_worker_context_v1.h"
static int egl_worker_fd=-1;
static const uint32_t* egl_worker_map;
static int egl_worker_journal=-1;
static uint32_t egl_worker_records;
static pthread_mutex_t egl_worker_mutex=PTHREAD_MUTEX_INITIALIZER;
static void egl_worker_context(uint32_t call,uint32_t phase){
 const char* mode=getenv("MNM_DRAW_WINE_WORKER_CONTEXT");if(!mode)return;
 int saved=errno;if(strcmp(mode,"1"))refuse("EGL worker context mode");
 if(pthread_mutex_lock(&egl_worker_mutex)){fail();}
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory||directory[0]!='/')refuse("EGL worker context directory");char path[4096];
 if(egl_worker_journal<0){if(snprintf(path,sizeof(path),"%s/egl-worker-context.bin",directory)>=(int)sizeof(path))fail();egl_worker_journal=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);if(egl_worker_journal<0)fail();unsigned char h[64]={0};memcpy(h,"MNMEWC01",8);put(h+8,1);put(h+12,128);put(h+16,131072);if(write(egl_worker_journal,h,64)!=64)fail();}
 if(!egl_worker_map){if(snprintf(path,sizeof(path),"%s/wine-worker-context.bin",directory)>=(int)sizeof(path))fail();egl_worker_fd=open(path,O_RDONLY|O_CLOEXEC);if(egl_worker_fd>=0){struct stat stat;if(fstat(egl_worker_fd,&stat)||stat.st_size!=64)refuse("EGL worker context size");void* map=mmap(0,64,PROT_READ,MAP_SHARED,egl_worker_fd,0);if(map==MAP_FAILED)fail();egl_worker_map=map;}}
 unsigned char r[128]={0};put(r,++egl_worker_records);put(r+4,phase);put(r+8,call);put(r+12,(uint32_t)getpid());put(r+16,(uint32_t)syscall(SYS_gettid));struct timespec time;if(clock_gettime(CLOCK_MONOTONIC,&time))fail();uint64_t stamp=(uint64_t)time.tv_sec*1000000000u+(uint64_t)time.tv_nsec;put(r+20,(uint32_t)stamp);put(r+24,(uint32_t)(stamp>>32));uint32_t snapshot[16]={0};uint32_t status=1;
 if(egl_worker_map){status=2;for(unsigned retry=0;retry<8;++retry){uint32_t before=__atomic_load_n(egl_worker_map+4,__ATOMIC_SEQ_CST);if(before&1)continue;for(unsigned i=0;i<16;++i)snapshot[i]=__atomic_load_n(egl_worker_map+i,__ATOMIC_SEQ_CST);uint32_t after=__atomic_load_n(egl_worker_map+4,__ATOMIC_SEQ_CST);if(before==after&&snapshot[4]==before){status=0;break;}}}
 put(r+28,status);if(status==0){if(memcmp(snapshot,MNM_DRAW_WORKER_CONTEXT_MAGIC,8)||snapshot[2]!=1||snapshot[3]!=64||snapshot[5]>1||snapshot[11]>1||snapshot[12]||snapshot[13]||snapshot[14]||snapshot[15])refuse("EGL worker context wire");for(unsigned i=0;i<16;++i)put(r+32+i*4,snapshot[i]);}
 if(egl_worker_records>131072||write(egl_worker_journal,r,128)!=128)refuse("EGL worker context journal");
 if(pthread_mutex_unlock(&egl_worker_mutex)){fail();}
 errno=saved;
}
