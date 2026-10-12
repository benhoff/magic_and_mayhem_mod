/* Post-original-return diagnostic handoff; no recorder lock or driver query. */
static uint32_t egl_physical_count,egl_physical_owner,egl_physical_last_call,egl_physical_closed;
static pthread_mutex_t egl_physical_mutex=PTHREAD_MUTEX_INITIALIZER;
static void egl_artifact(char* path,size_t size,const char* directory,const char* type,uint32_t ordinal){
 if(snprintf(path,size,"%s/egl-physical-%s-%06u.bin",directory,type,ordinal)>=(int)size)refuse("EGL physical path");
}
static void egl_physical(uint32_t call,const uint32_t args[8],EGLBoolean result,int error){
 const char* enabled=getenv("MNM_DRAW_EGL_PHYSICAL");if(!enabled)return;
 if(strcmp(enabled,"1"))refuse("EGL physical mode");
 if(egl_physical_closed)return;
 const char* directory=getenv("MNM_DRAW_X11_DIRECTORY");if(!directory||directory[0]!='/')refuse("EGL physical directory");
 char stop_path[4096];if(snprintf(stop_path,sizeof(stop_path),"%s/egl-physical-close.flag",directory)>=(int)sizeof(stop_path))fail();
 if(access(stop_path,F_OK)==0){
  unsigned char marker[32]={0};memcpy(marker,"MNMEGD01",8);put(marker+8,1);put(marker+12,(uint32_t)getpid());put(marker+16,egl_physical_count);put(marker+20,egl_physical_last_call);put(marker+24,1);
  if(snprintf(stop_path,sizeof(stop_path),"%s/egl-physical.done",directory)>=(int)sizeof(stop_path))fail();
  int output=open(stop_path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
  if(output<0||write(output,marker,32)!=32||close(output))refuse("EGL physical close marker");
  egl_physical_closed=1;return;
 }

 if(!result)return;
 if(pthread_mutex_lock(&egl_physical_mutex))fail();
 uint32_t owner=(uint32_t)syscall(SYS_gettid);if(egl_physical_owner&&egl_physical_owner!=owner)refuse("EGL physical competing swap threads");egl_physical_owner=owner;uint32_t ordinal=++egl_physical_count;
 if(pthread_mutex_unlock(&egl_physical_mutex))fail();
 if(ordinal>1024)refuse("EGL physical count budget");
 unsigned char request[128]={0};memcpy(request,"MNMEGL01",8);put(request+8,1);put(request+12,128);put(request+16,ordinal);put(request+20,(uint32_t)getpid());put(request+24,(uint32_t)syscall(SYS_gettid));put(request+28,call);put(request+32,11);
 put(request+36,args[0]);put(request+40,args[1]);put(request+44,args[2]);put(request+48,(uint32_t)result);put(request+52,(uint32_t)error);put(request+56,800);put(request+60,600);
 struct timespec time;if(clock_gettime(CLOCK_MONOTONIC,&time))fail();uint64_t stamp=(uint64_t)time.tv_sec*1000000000u+(uint64_t)time.tv_nsec;put(request+64,(uint32_t)stamp);put(request+68,(uint32_t)(stamp>>32));
 char path[4096];egl_artifact(path,sizeof(path),directory,"request",ordinal);int output=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
 if(output<0||write(output,request,128)!=128||close(output))refuse("EGL physical request");
 egl_artifact(path,sizeof(path),directory,"ack",ordinal);uint64_t start=stamp;
 for(;;){
  int input=open(path,O_RDONLY|O_CLOEXEC);
  if(input>=0){unsigned char ack[17];ssize_t count=read(input,ack,sizeof(ack));close(input);
   if(count!=16||memcmp(ack,"MNMEACK1",8)||((uint32_t)ack[8]|(uint32_t)ack[9]<<8|(uint32_t)ack[10]<<16|(uint32_t)ack[11]<<24)!=ordinal||((uint32_t)ack[12]|(uint32_t)ack[13]<<8|(uint32_t)ack[14]<<16|(uint32_t)ack[15]<<24)!=call)refuse("EGL physical ACK");
   egl_physical_last_call=call;return;
  }
  if(errno!=ENOENT)refuse("EGL physical ACK open");
  if(clock_gettime(CLOCK_MONOTONIC,&time))fail();
  stamp=(uint64_t)time.tv_sec*1000000000u+(uint64_t)time.tv_nsec;if(stamp-start>5000000000u)refuse("EGL physical ACK timeout");
  struct timespec wait={0,1000000};nanosleep(&wait,0);
 }
}
