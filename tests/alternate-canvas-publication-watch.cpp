// Catch transient as well as persistent writes to both original pointer slots
// during the complete bounded initializer experiment. Authored fixture stores
// must trigger the same hardware watchpoints, making absence observable.
#define MNM_ALTERNATE_INITIALIZER_ENTRY initializer_cases
#include "alternate-canvas-initializer.cpp"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstddef>

int main(int argc,char** argv)try{
  const auto child=fork();require(child>=0,"fork");
  if(!child){
    if(ptrace(PTRACE_TRACEME,0,nullptr,nullptr))_exit(100);
    raise(SIGSTOP);
    const int result=initializer_cases(argc,argv);std::cout.flush();std::cerr.flush();_exit(result);
  }
  int status=0;require(waitpid(child,&status,0)==child&&WIFSTOPPED(status),"Initial trace stop");
  require(!ptrace(PTRACE_SETOPTIONS,child,nullptr,reinterpret_cast<void*>(PTRACE_O_EXITKILL)),"Tracer exit cleanup");
  const auto debug=[&](unsigned reg,U value){
    require(!ptrace(PTRACE_POKEUSER,child,reinterpret_cast<void*>(offsetof(user,u_debugreg)+reg*sizeof(U)),reinterpret_cast<void*>(value)),"Hardware watchpoint");
  };
  debug(0,0x6e1f68);debug(1,0x6e1f6c);debug(6,0);debug(7,0x00dd0005);
  unsigned originalWrites=0,fixtureWrites=0;std::array<unsigned,2> slotWrites{};
  std::vector<std::array<U,4>> events;
  while(true){
    require(!ptrace(PTRACE_CONT,child,nullptr,nullptr),"Continue trace");
    require(waitpid(child,&status,0)==child,"Wait trace");
    if(WIFEXITED(status)){require(WEXITSTATUS(status)==0,"Initializer child failed");break;}
    require(WIFSTOPPED(status)&&WSTOPSIG(status)==SIGTRAP,"Unexpected child stop");
    const auto hit=U(ptrace(PTRACE_PEEKUSER,child,reinterpret_cast<void*>(offsetof(user,u_debugreg)+6*sizeof(U)),nullptr));
    require(hit&3,"Unclassified trap");user_regs_struct regs{};
    require(!ptrace(PTRACE_GETREGS,child,nullptr,&regs),"Get trace registers");
    const bool original=regs.eip>=0x400000&&regs.eip<0x700000;
    if(original)++originalWrites;else ++fixtureWrites;
    for(unsigned slot=0;slot<2;++slot)if(hit&(1u<<slot)){
      ++slotWrites[slot];const U address=0x6e1f68+slot*4;
      const auto value=U(ptrace(PTRACE_PEEKDATA,child,reinterpret_cast<void*>(address),nullptr));
      events.push_back({address,U(regs.eip),value,U(original)});
    }
    debug(6,0);
  }
  require(originalWrites==0&&fixtureWrites>=576&&slotWrites[0]>=288&&slotWrites[1]>=288,"Publication watch denominator");
  std::cout<<"{\"success\":true,\"original_pointer_writes\":"<<originalWrites
    <<",\"authored_pointer_writes\":"<<fixtureWrites<<",\"slot_writes\":["<<slotWrites[0]<<','<<slotWrites[1]
    <<"],\"hardware_watchpoints\":true,\"transient_writes_observed\":true,\"live_session\":false,\"events\":[";
  for(unsigned i=0;i<events.size();++i){if(i)std::cout<<',';auto e=events[i];std::cout<<'['<<e[0]<<','<<e[1]<<','<<e[2]<<','<<e[3]<<']';}
  std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
