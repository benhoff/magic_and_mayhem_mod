#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Host-only fault injection: delay durable sync of the CPU reply alone. */
static void delay_reply(int fd){
    char link[64],path[4096];
    snprintf(link,sizeof(link),"/proc/self/fd/%d",fd);
    ssize_t size=readlink(link,path,sizeof(path)-1);
    if(size<0)return;
    path[size]=0;
    /* QSaveFile may use O_TMPFILE: /proc names it #inode (deleted), so
     * constrain the fault to this fixture's directory, not its target name. */
    const char* directory=getenv("MNM_TEST_CPU_SYNC_DIRECTORY");
    if(!directory || strncmp(path,directory,strlen(directory)) || path[strlen(directory)]!='/')return;
    const char* marker=getenv("MNM_TEST_CPU_SYNC_MARKER");
    if(!marker)return;
    int out=open(marker,O_WRONLY|O_CREAT|O_EXCL,0600);
    if(out>=0){(void)write(out,path,(size_t)size);close(out);}
    struct timespec delay={11,0};
    while(nanosleep(&delay,&delay)!=0){}
}
int fsync(int fd){
    static int (*real_sync)(int);
    if(!real_sync)*(void**)(&real_sync)=dlsym(RTLD_NEXT,"fsync");
    delay_reply(fd);return real_sync(fd);
}
int fdatasync(int fd){
    static int (*real_sync)(int);
    if(!real_sync)*(void**)(&real_sync)=dlsym(RTLD_NEXT,"fdatasync");
    delay_reply(fd);return real_sync(fd);
}
