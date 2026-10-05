#include "snapshot.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace mnm::game {
namespace {
std::runtime_error failure(const char* action) {return std::runtime_error(std::string(action)+": "+std::strerror(errno));}
struct Temporary {
    int fd=-1;std::string name;
    ~Temporary(){if(fd>=0) ::close(fd);if(!name.empty()) ::unlink(name.c_str());}
};
struct Directory {int fd;~Directory(){::close(fd);}};
}
CommitResult writeSnapshot(const std::filesystem::path& path,const State& state,bool overwrite,const Limits& limits) {
    if(path.native().find('\0')!=std::string::npos) throw std::invalid_argument("snapshot path contains NUL");
    auto bytes=encodeSnapshot(state,limits);
    if(path.filename().empty()) throw std::invalid_argument("snapshot filename required");
    auto parent=path.parent_path();if(parent.empty()) parent=".";
    Directory directory{::open(parent.c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC)};
    if(directory.fd<0) throw failure("open snapshot directory");
    auto pattern=(parent/("."+path.filename().string()+".XXXXXX")).string();
    std::vector<char> name(pattern.begin(),pattern.end());name.push_back(0);
    Temporary temp;temp.fd=::mkstemp(name.data());if(temp.fd<0) throw failure("create snapshot temporary");temp.name=name.data();
    std::size_t p=0;
    while(p<bytes.size()) {
        auto n=::write(temp.fd,bytes.data()+p,bytes.size()-p);
        if(n<0 && errno==EINTR) continue;
        if(n<=0) throw failure("write snapshot temporary");
        p+=static_cast<std::size_t>(n);
    }
    if(::fsync(temp.fd)!=0) throw failure("sync snapshot temporary");
    int fd=temp.fd;temp.fd=-1;if(::close(fd)!=0) throw failure("close snapshot temporary");
    if(overwrite) {
        if(::rename(temp.name.c_str(),path.c_str())!=0) throw failure("publish snapshot");
        temp.name.clear();
    } else {
        // link admits only absent destinations atomically, including concurrent writers.
        if(::link(temp.name.c_str(),path.c_str())!=0) throw failure("publish new snapshot");
        if(::unlink(temp.name.c_str())!=0) return {false,"snapshot published; temporary cleanup failed"};
        temp.name.clear();
    }
    if(::fsync(directory.fd)!=0) return {false,"snapshot published; directory sync failed"};
    return {};
}
}
