#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
#include <cstddef>
#else
#include "effect_animation_catalog.hpp"
#endif
#include "effect_animation_metadata.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace mnm::reconstruction;
static EffectAnimationMetadataFields fields;
static std::string countText;
#ifdef ORIGINAL_REFERENCE
static std::uint32_t requests=0;
static bool countWindow=false;
#endif
static unsigned word(std::istream& in){unsigned n=0;in.read(reinterpret_cast<char*>(&n),4);if(!in)throw std::runtime_error("Fixture extent");return n;}
static std::string text(std::istream& in){const auto n=word(in);if(n>255)throw std::runtime_error("String capacity");std::string s(n,'\0');in.read(s.data(),n);if(!in || s.find('\0')!=s.npos)throw std::runtime_error("String extent");return s;}
#ifdef ORIGINAL_REFERENCE
static unsigned __attribute__((stdcall,force_align_arg_pointer)) profile(const char* section,const char* key,const char* fallback,char* output,unsigned capacity,const char* path){
    const char* keys[]={"AnimationNo","AnimationFileRef","SpritePrinter","Data1"};
    if(capacity!=256 || *fallback || std::strcmp(path,"C:\\game\\cfg\\effectani.cfg"))_exit(90);
    const std::string* s=nullptr;
    if(countWindow){if(requests || std::strcmp(section,"HEADER") || std::strcmp(key,"NumberOfEffectsFile"))_exit(91);s=&countText;}
    else{const unsigned i=requests/4,k=requests%4;
        if(i>=fields.size() || section!=std::string("ANI_")+std::to_string(i) || std::strcmp(key,keys[k]))_exit(92);
        s=&fields[i][k];}
    std::memcpy(output,s->c_str(),s->size()+1);++requests;return s->size();
}
static long trace(enum __ptrace_request request,pid_t child,void* address=nullptr,void* data=nullptr){
    errno=0;const auto n=ptrace(request,child,address,data);if(n==-1 && errno)throw std::runtime_error("ptrace: "+std::string(std::strerror(errno)));return n;
}
static std::uint32_t peek(pid_t child,const void* address){return std::uint32_t(trace(PTRACE_PEEKDATA,child,const_cast<void*>(address)));}
// Hardware execution breakpoint stops before leaving the selected region.
// No original instruction bytes, child calls or function returns are patched.
static std::vector<std::uint32_t> execute(unsigned entry,unsigned end,std::vector<std::uint32_t>& stack,
    std::vector<std::uint32_t>& allocation,unsigned files){
    requests=0;stack.assign(16384,0xa5a5a5a5u);auto* frame=stack.data()+8192;
    frame[0x10/4]=0;
    frame[0x14/4]=reinterpret_cast<std::uintptr_t>(allocation.data()+4);
    frame[0x18/4]=files;frame[0x1c/4]=fields.size();
    const pid_t child=fork();if(child<0)throw std::runtime_error("fork");
    if(child==0){if(ptrace(PTRACE_TRACEME,0,nullptr,nullptr)<0)_exit(93);raise(SIGSTOP);_exit(94);}
    try{
        int status=0;if(waitpid(child,&status,0)!=child || !WIFSTOPPED(status))throw std::runtime_error("Child initial stop");
        struct user_regs_struct registers{};trace(PTRACE_GETREGS,child,nullptr,&registers);
        registers.eip=entry;registers.esp=reinterpret_cast<std::uintptr_t>(frame);registers.ebp=reinterpret_cast<std::uintptr_t>(profile);
        registers.eax=registers.ebx=registers.ecx=registers.edx=registers.esi=registers.edi=0;registers.eflags=0x202;
        trace(PTRACE_SETREGS,child,nullptr,&registers);
        trace(PTRACE_POKEUSER,child,reinterpret_cast<void*>(offsetof(struct user,u_debugreg[0])),reinterpret_cast<void*>(end));
        trace(PTRACE_POKEUSER,child,reinterpret_cast<void*>(offsetof(struct user,u_debugreg[7])),reinterpret_cast<void*>(1));
        trace(PTRACE_CONT,child);
        if(waitpid(child,&status,0)!=child || !WIFSTOPPED(status) || WSTOPSIG(status)!=SIGTRAP)
            {trace(PTRACE_GETREGS,child,nullptr,&registers);
             std::cerr<<"Failed region "<<std::hex<<entry<<" at "<<registers.eip<<" esp "<<registers.esp<<" ebp "<<registers.ebp<<std::dec<<'\n';
             throw std::runtime_error("Original region failed status "+std::to_string(status));}
        trace(PTRACE_GETREGS,child,nullptr,&registers);
        if(registers.eip!=end || registers.esp!=reinterpret_cast<std::uintptr_t>(frame))throw std::runtime_error("Boundary/stack differs");
        if(peek(child,&requests)!=(countWindow?1:fields.size()*4))throw std::runtime_error("Profile request count got "+std::to_string(peek(child,&requests))+" expected "+std::to_string(countWindow?1:fields.size()*4));
        std::vector<std::uint32_t> result;
        if(countWindow)result={peek(child,frame+0x18/4)};
        else{
            for(unsigned i=0;i<allocation.size();++i){const auto n=peek(child,allocation.data()+i);
                if((i<4 || i>=allocation.size()-4) && n!=allocation[i])throw std::runtime_error("Entry allocation guard");
                if(i>=4 && i<allocation.size()-4)result.push_back(n);}
            if(peek(child,frame+0x14/4)!=reinterpret_cast<std::uintptr_t>(allocation.data()+4)+8+fields.size()*16 || peek(child,frame+0x10/4)!=fields.size())
                throw std::runtime_error("Original final cursor/count");
        }
        // Instruction-byte immutability is checked in the child's private image.
        for(unsigned at=entry;at<end;at+=4)if(peek(child,reinterpret_cast<void*>(at))!=*reinterpret_cast<unsigned*>(at))throw std::runtime_error("Original code changed");
        trace(PTRACE_KILL,child);waitpid(child,&status,0);return result;
    }catch(...){kill(child,SIGKILL);waitpid(child,nullptr,0);throw;}
}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    if(argc!=4)throw std::runtime_error("PE fixtures output");map_image(read(argv[1]));const int start=2;
    const unsigned char entry[]={0x8b,0x4c,0x24,0x14,0x83,0xc1,0x08};
    if(std::memcmp(reinterpret_cast<void*>(0x49c836),entry,sizeof(entry)))throw std::runtime_error("Metadata entry bytes");
    global(0x5c507c,reinterpret_cast<std::uintptr_t>(profile));
    std::strcpy(reinterpret_cast<char*>(0x5fe66c),"C:\\game\\cfg\\effectani.cfg");
    std::vector<std::uint32_t> stack,allocation;
#else
    if(argc!=3)throw std::runtime_error("fixtures output");
    const int start=1;
    unsigned accepted=0,refused=0;std::vector<unsigned> differences,refusals;
#endif
    std::ifstream input(argv[start],std::ios::binary);std::ofstream output(argv[start+1],std::ios::binary);
    const auto fixtures=word(input);if(fixtures>2048)throw std::runtime_error("Fixture count");unsigned rows=0,counts=0;
    for(unsigned fixture=0;fixture<fixtures;++fixture){
        countText=text(input);const auto files=word(input),n=word(input);if(files<1 || files>64 || n<1 || n>4096)throw std::runtime_error("Fixture capacity");
        fields.resize(n);for(auto& row:fields)for(auto& s:row)s=text(input);
        const auto count=effectAnimationFileCount(countText);
#ifdef ORIGINAL_REFERENCE
        allocation.assign(8,0x12345678u);countWindow=true;const auto originalCount=execute(0x49c6e2,0x49c735,stack,allocation,files);
        if(originalCount[0]!=count)throw std::runtime_error("Count clamp differs");
#endif
        output.write(reinterpret_cast<const char*>(&count),4);++counts;
        for(unsigned seed: {0u,0xa5u,0xffu}){
            EffectAnimationMetadataWords initial(n);for(auto& row:initial)for(auto& w:row)w=seed*0x01010101u;
            const auto expected=decodeEffectAnimationMetadata(fields,files,initial);
#ifdef ORIGINAL_REFERENCE
            allocation.assign(8+4*n,0x12345678u);std::memcpy(allocation.data()+4,initial.data(),n*16);countWindow=false;
            const auto actual=execute(0x49c836,0x49caae,stack,allocation,files);
            if(std::memcmp(actual.data(),expected.data(),n*16))throw std::runtime_error("Metadata differs fixture "+std::to_string(fixture));
#endif
            output.write(reinterpret_cast<const char*>(expected.data()),n*16);rows+=n;
        }
#ifndef ORIGINAL_REFERENCE
        mnm::assets::Config config;config.sections["header"]={{"numberofanimations",std::to_string(n)},{"numberofeffectsfile",countText}};
        const char* keys[]={"animationno","animationfileref","spriteprinter","data1"};
        for(unsigned i=0;i<n;++i)for(unsigned k=0;k<4;++k)config.sections["ani_"+std::to_string(i)][keys[k]]=fields[i][k];
        try{
            auto native=mnm::assets::readEffectAnimationRecipes(config);EffectAnimationMetadataWords nativeSeed(n);for(auto& row:nativeSeed)for(auto& w:row)w=0xa5a5a5a5u;
            const auto expected=decodeEffectAnimationMetadata(fields,files,nativeSeed);
            if(native.fileCount!=files || native.entries.size()!=n)throw std::runtime_error("Native complete count differs");
            bool differs=false;for(unsigned i=0;i<n;++i){const auto& r=native.entries[i];if(expected[i]!=std::array<std::uint32_t,4>{r.sequence,r.assetIndex,r.printer,r.data1})differs=true;}
            if(differs)differences.push_back(fixture);
            ++accepted;
        }catch(const std::invalid_argument&){++refused;refusals.push_back(fixture);}
#endif
    }
    if(!output)throw std::runtime_error("Output failed");
    std::cout<<"{\"fixtures\":"<<fixtures<<",\"count_windows\":"<<counts<<",\"metadata_rows\":"<<rows;
#ifndef ORIGINAL_REFERENCE
    std::cout<<",\"native_accepted\":"<<accepted<<",\"native_refused\":"<<refused<<",\"native_difference_fixtures\":[";
    for(unsigned i=0;i<differences.size();++i){if(i)std::cout<<',';std::cout<<differences[i];}std::cout<<"],\"native_refusal_fixtures\":[";
    for(unsigned i=0;i<refusals.size();++i){if(i)std::cout<<',';std::cout<<refusals[i];}std::cout<<']';
#endif
    std::cout<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
