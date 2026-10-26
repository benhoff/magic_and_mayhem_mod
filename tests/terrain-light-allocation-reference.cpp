#include "allocation-supplier-support.hpp"
#ifdef ORIGINAL_REFERENCE
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
static U failureAt=0,allocationIndex=0,faultAddress=0;
static U allocateFault(U n){++allocationIndex;if(allocationIndex==failureAt){memory.events.insert(memory.events.end(),{77,n,0});return 0;}return memory.allocate(n);}
struct FaultMemory {
 U get32(U p){return memory.get32(p);}std::uint8_t get8(U p){return memory.get8(p);}
 void set32(U p,U v){memory.set32(p,v);}void set8(U p,std::uint8_t v){if(!p){faultAddress=p;throw std::runtime_error("Owned guest null write");}memory.set8(p,v);}
 U allocate(U n){return allocateFault(n);}void release(U p){memory.release(p);}void rebuildKernels(U p){memory.rebuildKernels(p);}
};
#ifdef ORIGINAL_REFERENCE
static int childFd=-1;
static void sendRaw(const void* p,std::size_t n){const auto* b=static_cast<const unsigned char*>(p);while(n){ssize_t done=write(childFd,b,n);if(done<=0)_exit(91);b+=done;n-=std::size_t(done);}}
static void dump(U success){U head[4]={success,faultAddress,memory.used,U(memory.events.size())};sendRaw(head,sizeof(head));sendRaw(memory.ptr(Map),0x66c0);sendRaw(memory.ptr(Arena),memory.used);sendRaw(memory.events.data(),memory.events.size()*4);}
static void failed(int, siginfo_t* info,void*){faultAddress=U(reinterpret_cast<std::uintptr_t>(info->si_addr));dump(0);_exit(0);}
extern "C" U allocate_failure(U n){return allocateFault(n);}
#include "refresh-adapter-bytes.hpp"
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");auto pe=read(argv[1]);map_image(pe);if(mmap(reinterpret_cast<void*>(Arena),0x100000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Arena");
#define REDIRECT(address,name) redirect(address,adapterBytes.at(address),reinterpret_cast<std::uintptr_t>(&name))
 REDIRECT(0x597850,allocate_failure);REDIRECT(0x4f1890,rebuild_light);
 U ph=u32(pe,0x3c),opt=ph+24;unsigned sections=pe[ph+6]|(pe[ph+7]<<8),os=pe[ph+20]|(pe[ph+21]<<8);
 for(unsigned i=0;i<sections;++i){U s=opt+os+i*40;if(u32(pe,s+36)&0x20000000){U rva=u32(pe,s+12),n=(std::max(u32(pe,s+8),u32(pe,s+16))+4095)&~4095u;if(mprotect(memory.ptr(0x400000+rva),n,PROT_READ|PROT_EXEC))throw std::runtime_error("RX");}}
 const char* output=argv[2];
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream out(output,std::ios::binary);std::uint64_t bytes=0;U cases=0;
 for(U trial=0;trial<18;++trial){failureAt=trial%6;allocationIndex=faultAddress=memory.used=0;memory.events.clear();std::memset(memory.ptr(Map),0x69,0x66c0);memory.set32(Map+0x7cc,0);memory.set32(Map+12,1+trial%5);memory.set32(Map+16,19*21);memory.set8(0x5e18f0,std::uint8_t(trial*17));memory.set32(0x5e41a8,0x80);
#ifdef ORIGINAL_REFERENCE
  int fds[2];if(pipe(fds))throw std::runtime_error("Pipe");pid_t child=fork();if(child<0)throw std::runtime_error("Fork");if(!child){close(fds[0]);childFd=fds[1];struct sigaction sa{};sa.sa_sigaction=failed;sa.sa_flags=SA_SIGINFO;sigemptyset(&sa.sa_mask);sigaction(SIGSEGV,&sa,nullptr);reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x4ecdd0)(memory.ptr(Map));dump(1);_exit(0);}close(fds[1]);std::array<char,8192> buffer;for(;;){ssize_t n=::read(fds[0],buffer.data(),buffer.size());if(n<0)throw std::runtime_error("Pipe read");if(!n)break;out.write(buffer.data(),n);bytes+=U(n);}close(fds[0]);int status;waitpid(child,&status,0);if(!WIFEXITED(status)||WEXITSTATUS(status))throw std::runtime_error("Child failure");
#else
  U success=1;FaultMemory admitted;try{TerrainLightResourceContracts<FaultMemory>(admitted).initialize(Map);}catch(const std::runtime_error&){success=0;}
  U header[4]={success,faultAddress,memory.used,U(memory.events.size())};auto emit=[&](const void* p,std::size_t n){out.write(static_cast<const char*>(p),n);bytes+=n;};emit(header,sizeof(header));emit(memory.ptr(Map),0x66c0);emit(memory.ptr(Arena),memory.used);emit(memory.events.data(),memory.events.size()*4);
#endif
  ++cases;
 }
#ifdef ORIGINAL_REFERENCE
 for(const auto& v:protectedBytes)if(std::memcmp(memory.ptr(v.first),v.second.data(),v.second.size()))throw std::runtime_error("Original code changed");
#endif
 if(!out)throw std::runtime_error("Output");std::cout<<"{\"success\":true,\"allocation_cases\":"<<cases<<",\"compared_bytes\":"<<bytes<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
