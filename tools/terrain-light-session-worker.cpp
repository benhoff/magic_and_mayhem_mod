/* Persistent host for the unchanged V1 replay decoder. The two reusable memfds
 * keep its existing file interface entirely in RAM; no subprocess or per-tick
 * disk file is created. Model state is restored from each owned input. */
#define main terrainLightingReplay
#include "terrain-light-replay.cpp"
#undef main
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <streambuf>
struct LightingIo {
 int input=memfd_create("mnm-light-source",MFD_CLOEXEC),output=memfd_create("mnm-light-reply",MFD_CLOEXEC);
 ~LightingIo(){if(input!=-1)close(input);if(output!=-1)close(output);}
};
struct QuietLighting:std::streambuf {int overflow(int c)override{return traits_type::not_eof(c);}std::streamsize xsputn(const char*,std::streamsize n)override{return n;}};
static Bytes evaluateLighting(const Bytes& input){
 static LightingIo io;static QuietLighting quiet;
 if(io.input<0||io.output<0||input.size()>MNM_LIGHT_V1_MAX_BYTES)throw std::runtime_error("Lighting RAM transport");
 if(ftruncate(io.input,0)||lseek(io.input,0,SEEK_SET)<0||ftruncate(io.output,0))throw std::runtime_error("Lighting RAM reset");
 std::size_t at=0;while(at<input.size()){auto n=write(io.input,input.data()+at,input.size()-at);if(n<=0)throw std::runtime_error("Lighting RAM input");at+=std::size_t(n);}
 auto source=std::string("/proc/self/fd/")+std::to_string(io.input),target=std::string("/proc/self/fd/")+std::to_string(io.output);char label[]="lighting-session";char* args[3]={label,source.data(),target.data()};
 auto* previous=std::cout.rdbuf(&quiet);int result=terrainLightingReplay(3,args);std::cout.rdbuf(previous);
 if(result)throw std::runtime_error("Lighting replay refused input");
 struct stat st{};if(fstat(io.output,&st)||st.st_size<80||st.st_size>MNM_LIGHT_V1_MAX_BYTES||lseek(io.output,0,SEEK_SET)<0)throw std::runtime_error("Lighting RAM reply extent");
 Bytes reply(std::size_t(st.st_size));at=0;while(at<reply.size()){auto n=read(io.output,reply.data()+at,reply.size()-at);if(n<=0)throw std::runtime_error("Lighting RAM reply");at+=std::size_t(n);}return reply;
}
#include "terrain-light-session-server.hpp"
int main(int argc,char** argv)try{
 if(argc!=4||std::string(argv[1])!="--session")throw std::runtime_error("Expected --session channel corpus");return serveSession(argv[2],argv[3]);
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
