#pragma once
#include "../protocols/include/mnm/terrain_light_session_v1.h"
#include <chrono>
#include <csignal>
#include <thread>
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
namespace {
volatile std::sig_atomic_t lightRunning=1;
void lightStop(int){lightRunning=0;}
struct LightMapping {
 int fd=-1,trace=-1;void* address=MAP_FAILED;
 ~LightMapping(){if(trace!=-1){fsync(trace);close(trace);}if(address!=MAP_FAILED){msync(address,MNM_LIGHT_SESSION_BYTES,MS_SYNC);munmap(address,MNM_LIGHT_SESSION_BYTES);}if(fd!=-1)close(fd);}
};
void lightWrite(int fd,const void* p,std::size_t n){auto b=static_cast<const unsigned char*>(p);while(n){auto k=write(fd,b,n);if(k<0&&errno==EINTR)continue;if(k<=0)throw std::runtime_error("Session trace write");b+=k;n-=std::size_t(k);}}
}
static int serveSession(const char* filename,const char* corpus){
 LightMapping resource;resource.fd=open(filename,O_RDWR|O_NOFOLLOW|O_CLOEXEC);
 struct stat st{};if(resource.fd<0 || fstat(resource.fd,&st) || !S_ISREG(st.st_mode) || st.st_size!=MNM_LIGHT_SESSION_BYTES)throw std::runtime_error("Session extent/type");
 resource.address=mmap(nullptr,MNM_LIGHT_SESSION_BYTES,PROT_READ|PROT_WRITE,MAP_SHARED,resource.fd,0);
 if(resource.address==MAP_FAILED)throw std::runtime_error("Session mapping");
 auto* p=static_cast<U*>(resource.address);auto* bytes=static_cast<unsigned char*>(resource.address);
 if(std::memcmp(bytes,MNM_LIGHT_SESSION_MAGIC,8)||p[MNM_LS_VERSION]!=1||p[MNM_LS_CAPACITY]!=MNM_LIGHT_V1_MAX_BYTES||p[MNM_LS_MODE]>1||p[MNM_LS_TRACE]>2||p[MNM_LS_ARM]||p[31]||(!p[9]&&!p[10]))throw std::runtime_error("Session header");
 for(U i=14;i<31;++i)if(p[i])throw std::runtime_error("Stale session statistics");
 const U low=p[9],high=p[10],mode=p[11],trace=p[12];U last=0;Bytes input,reply;std::size_t traceBytes=8;
 if(p[MNM_LS_TRACE]){resource.trace=open(corpus,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);if(resource.trace<0)throw std::runtime_error("Session trace ownership");lightWrite(resource.trace,"MNMLSC01",8);}
 auto load=[&](){return __atomic_load_n(p+MNM_LS_PHASE,__ATOMIC_ACQUIRE);};
 auto publish=[&](U expected,U value){return __atomic_compare_exchange_n(p+MNM_LS_PHASE,&expected,value,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);};
 if(!publish(MNM_LS_BOOT,MNM_LS_IDLE))throw std::runtime_error("Session already started");
 std::signal(SIGTERM,lightStop);std::signal(SIGINT,lightStop);
 while(lightRunning && load()!=MNM_LS_CLOSED){
  U phase=load();
  if(phase==MNM_LS_REQUEST){
   auto start=std::chrono::steady_clock::now();
   try{
    const U n=p[MNM_LS_INPUT_LENGTH],seq=p[MNM_LS_SEQUENCE];
    if(n<300||n>MNM_LIGHT_V1_MAX_BYTES||!seq||seq!=last+1||p[9]!=low||p[10]!=high||p[11]!=mode||p[12]!=trace||p[13]>1||p[31])throw std::runtime_error("Session request identity");
    input.assign(bytes+128,bytes+128+n);
    U words[8];std::memcpy(words,input.data(),32);
    if(words[5]!=seq||words[6]!=low||words[7]!=high||input.size()<116||std::memcmp(input.data()+112,&mode,4))throw std::runtime_error("Session request nonce/sequence");
    reply=evaluateLighting(input);if(reply.size()>MNM_LIGHT_V1_MAX_BYTES)throw std::runtime_error("Session reply extent");
    std::memcpy(bytes+128+MNM_LIGHT_V1_MAX_BYTES,reply.data(),reply.size());p[MNM_LS_REPLY_LENGTH]=U(reply.size());last=seq;++p[MNM_LS_PROCESSED];
    auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();p[MNM_LS_MAX_COMPUTE]=std::max(p[MNM_LS_MAX_COMPUTE],U(std::min<std::int64_t>(elapsed,0xffffffffu)));
    // A timed-out guest closes permanently. Never publish over that latch.
    publish(MNM_LS_REQUEST,MNM_LS_REPLY);
   }catch(const std::exception&){++p[MNM_LS_REJECTED];p[MNM_LS_ERROR]=1;publish(MNM_LS_REQUEST,MNM_LS_DECLINED);}
  }else if(phase==MNM_LS_RESULT){
   U n=p[MNM_LS_RESULT_LENGTH];
   if(p[MNM_LS_SEQUENCE]!=last||n<48||n>MNM_LIGHT_V1_MAX_BYTES||reply.size()!=n+32){p[MNM_LS_ERROR]=2;publish(MNM_LS_RESULT,MNM_LS_DECLINED);continue;}
   auto* result=bytes+128+2*MNM_LIGHT_V1_MAX_BYTES;++p[MNM_LS_COMPARED];
   if(std::memcmp(reply.data()+32,result,n)){++p[MNM_LS_MISMATCHES];p[MNM_LS_ERROR]=3;publish(MNM_LS_RESULT,MNM_LS_DECLINED);continue;}
   auto size=12+input.size()+reply.size()+n;
   if(resource.trace!=-1&&p[MNM_LS_RECORDS]<MNM_LS_TRACE_SAMPLES&&size<=MNM_LS_TRACE_BYTES-traceBytes){
    U lens[3]={U(input.size()),U(reply.size()),n};lightWrite(resource.trace,lens,12);lightWrite(resource.trace,input.data(),input.size());lightWrite(resource.trace,reply.data(),reply.size());lightWrite(resource.trace,result,n);traceBytes+=size;++p[MNM_LS_RECORDS];
   }
   publish(MNM_LS_RESULT,MNM_LS_ACK);
  }else if(phase==MNM_LS_IDLE||phase==MNM_LS_REPLY||phase==MNM_LS_ACK||phase==MNM_LS_DECLINED){std::this_thread::sleep_for(std::chrono::microseconds(200));}
  else{p[MNM_LS_ERROR]=4;__atomic_store_n(p+MNM_LS_PHASE,MNM_LS_CLOSED,__ATOMIC_RELEASE);break;}
 }
 __atomic_store_n(p+MNM_LS_PHASE,MNM_LS_CLOSED,__ATOMIC_RELEASE);
 return p[MNM_LS_ERROR]?1:0;
}
