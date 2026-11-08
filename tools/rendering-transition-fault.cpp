// Controlled diagnostic only: use the existing terminal-state helper. Never
// bind a writer, move a cursor or alter payload. Caller supplies an owned ring.
#include "../protocols/include/mnm/render_command_ring.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>

static void need(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static unsigned long long now(){return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
static std::string hex(const uint32_t* p){static const char h[]="0123456789abcdef";std::string s;for(unsigned i=0;i<64;++i){auto v=reinterpret_cast<const unsigned char*>(p)[i];s+=h[v>>4];s+=h[v&15];}return s;}
static void inject(uint32_t* map,uint32_t session){
    need(mnm_ring_identity(map,session)&&map[2]==3,"Wrong producer identity/version");
    need(mnm_ring_load(map+6)==1&&!mnm_ring_load(map+7)&&!mnm_ring_load(map+8),"Producer not active/unfailed/uncancelled");
    need(mnm_ring_load(map+5)==mnm_ring_load(map+9),"Require drained Mini boundary, not a pending-read fault");
    mnm_ring_writer fault{};fault.map=map;fault.session=session;fault.version=3;mnm_ring_fail(&fault,MNM_RENDER_COMMANDS_V2_REASON_INVALID);
    need(mnm_ring_load(map+6)==3&&mnm_ring_load(map+7)==MNM_RENDER_COMMANDS_V2_REASON_INVALID,"Failure did not publish");
}
int main(int argc,char** argv){
    int fd=-1;void* mapped=MAP_FAILED;
    try{
        if(argc==2&&std::string(argv[1])=="--self-test"){
            std::vector<uint32_t> w(MNM_RENDER_COMMANDS_V2_SIZE/4);auto reset=[&]{std::fill(w.begin(),w.end(),0);std::memcpy(w.data(),MNM_RENDER_COMMANDS_V3_MAGIC,8);w[2]=3;w[3]=MNM_RENDER_COMMANDS_V2_SIZE;w[4]=37;w[6]=1;w[5]=w[9]=123;};
            reset();auto before=w;inject(w.data(),37);for(unsigned i=0;i<w.size();++i)need((i==6&&w[i]==3)||(i==7&&w[i]==5)||w[i]==before[i],"Changed cursor/payload");
            unsigned refusals=0;for(unsigned n=0;n<6;++n){reset();uint32_t identity=37;if(n==0)identity=38;if(n==1)w[2]=2;if(n==2)w[6]=2;if(n==3)w[7]=4;if(n==4)w[8]=1;if(n==5)w[9]=122;before=w;try{inject(w.data(),identity);}catch(const std::runtime_error&){++refusals;}need(w==before,"Refusal mutated input");}
            need(refusals==6,"Missing negative controls");std::printf("{\"success\":true,\"cases\":7,\"only_state_reason_changed\":true}\n");return 0;
        }
        need(argc==4,"Usage: helper owned-repository channel expected-session");
        const auto root=std::filesystem::canonical(argv[1]);const auto channel=std::filesystem::canonical(argv[2]);const auto base=root/"working/experiments/menu-observer";
        need(std::filesystem::absolute(argv[2]).lexically_normal()==channel,"Channel path aliases are not admitted");
        auto relative=channel.lexically_relative(base);need(!relative.empty()&&*relative.begin()!=".."&&channel.filename()=="render-commands.bin","Unowned channel path");
        char* end=nullptr;const auto parsed=std::strtoull(argv[3],&end,10);need(end&&!*end&&parsed>0&&parsed<=UINT32_MAX,"Session range");
        fd=::open(channel.c_str(),O_RDWR|O_NOFOLLOW|O_CLOEXEC);need(fd>=0,"Open channel");struct stat status{};need(!fstat(fd,&status)&&S_ISREG(status.st_mode)&&status.st_uid==getuid()&&status.st_nlink==1&&status.st_size==MNM_RENDER_COMMANDS_V2_SIZE,"Channel ownership/extent");
        mapped=mmap(nullptr,status.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);need(mapped!=MAP_FAILED,"Map channel");auto* words=static_cast<uint32_t*>(mapped);const auto before=hex(words);const auto begin=now();inject(words,uint32_t(parsed));const auto endTime=now();const auto after=hex(words);
        std::printf("{\"success\":true,\"schema\":1,\"reason\":5,\"producer\":%llu,\"begin_host_ns\":\"%llu\",\"end_host_ns\":\"%llu\",\"before_header_hex\":\"%s\",\"after_header_hex\":\"%s\",\"device\":%llu,\"inode\":%llu,\"writer_claimed\":false,\"cursor_or_payload_written\":false}\n",parsed,begin,endTime,before.c_str(),after.c_str(),static_cast<unsigned long long>(status.st_dev),static_cast<unsigned long long>(status.st_ino));
        munmap(mapped,status.st_size);close(fd);return 0;
    }catch(const std::exception& e){if(mapped!=MAP_FAILED)munmap(mapped,MNM_RENDER_COMMANDS_V2_SIZE);if(fd>=0)close(fd);std::fprintf(stderr,"%s\n",e.what());return 1;}
}
