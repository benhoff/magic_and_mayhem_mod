/* Persistent worker uses the one-shot replay's checked byte decoder directly.
 * Derived kernels are cached; all mutable cycle state comes from each request. */
#define MNM_LIGHT_SESSION 1
#include "terrain-light-replay.cpp"
#include <sys/prctl.h>
#include <unistd.h>
static Bytes evaluateLighting(const Bytes& input){return evaluateTerrainLighting(input);}
#include "terrain-light-session-server.hpp"
int main(int argc,char** argv)try{
 if(argc!=6||std::string(argv[1])!="--session"||std::string(argv[4])!="--parent")throw std::runtime_error("Expected --session channel corpus --parent pid");
 const auto parent=std::stol(argv[5]);
 if(parent<=1||prctl(PR_SET_PDEATHSIG,SIGKILL)||getppid()!=parent)throw std::runtime_error("Lighting supervisor lifetime");
 return serveSession(argv[2],argv[3]);
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
