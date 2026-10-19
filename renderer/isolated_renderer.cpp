#include "isolated_renderer.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <sys/socket.h>
#include <QElapsedTimer>
#include <QtEndian>
#include <spawn.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <thread>
extern char** environ;
namespace mnm::render {
namespace {
struct TransportFailure:std::runtime_error {using std::runtime_error::runtime_error;};
bool exited(pid_t pid){int result;do{result=::waitpid(pid,nullptr,WNOHANG);}while(result<0&&errno==EINTR);return result!=0;}
// A driver stuck in uninterruptible kernel work must never be joined on a game
// or GUI thread. Reap only our children after exit, with a global retirement cap.
class ChildReaper {
    std::mutex mutex_;std::condition_variable wake_;std::vector<pid_t> children_;
    bool stopping_=false;unsigned workers_=0;std::thread owner_;
public:
    ChildReaper():owner_([this]{
        std::unique_lock<std::mutex> lock(mutex_);
        while(!stopping_){
            children_.erase(std::remove_if(children_.begin(),children_.end(),[this](pid_t pid){if(!exited(pid))return false;--workers_;return true;}),children_.end());
            wake_.wait_for(lock,std::chrono::milliseconds(25));
        }
        for(auto pid:children_)::waitpid(pid,nullptr,WNOHANG);
    }){}
    ~ChildReaper(){{std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}wake_.notify_one();owner_.join();}
    void add(pid_t pid){std::lock_guard<std::mutex> lock(mutex_);children_.push_back(pid);wake_.notify_one();}
    void reserve(){std::lock_guard<std::mutex> lock(mutex_);if(workers_>=8)throw TransportFailure("Render worker retirement budget exhausted");++workers_;}
    void release(){std::lock_guard<std::mutex> lock(mutex_);--workers_;}
};
ChildReaper& reaper(){static ChildReaper value;return value;}

void transfer(int fd,char* bytes,std::size_t size,bool writing,int timeout,QElapsedTimer& clock){
    while(size){
        const auto remaining=timeout-clock.elapsed();if(remaining<=0)throw TransportFailure("Render process deadline exceeded");
        pollfd p{fd,short(writing?POLLOUT:POLLIN),0};
        const int n=::poll(&p,1,int(remaining));
        if(n<0 && errno==EINTR)continue;
        if(n<=0)throw TransportFailure("Render process deadline or pipe failure");
        // Drain buffered data even if the writer exited after its last write.
        if(!(p.revents&(writing?POLLOUT:POLLIN)))throw TransportFailure("Render process exited");
        auto count=writing ? ::send(fd,bytes,size,MSG_NOSIGNAL) : ::read(fd,bytes,size);
        if(count<0 && (errno==EINTR || errno==EAGAIN))continue;
        if(count<=0)throw TransportFailure("Render process pipe closed");
        size-=std::size_t(count);bytes+=count;
    }
}
}
bool IsolatedRenderer::enabled(){return qEnvironmentVariableIntValue("MNM_NATIVE_DEVICE_RECOVERY")==1 && qEnvironmentVariableIntValue("MNM_RENDER_SERVICE")!=1;}
IsolatedRenderer::IsolatedRenderer(){
    const int requested=qEnvironmentVariableIntValue("MNM_RENDER_PROCESS_TIMEOUT_MS");
    if(requested>=100 && requested<=10000)timeout_=requested;
    start();
}
IsolatedRenderer::~IsolatedRenderer(){retire();}
void IsolatedRenderer::retire(){
    if(input_>=0)::close(input_);
    if(output_>=0)::close(output_);
    input_=output_=-1;
    if(pid_>0){::kill(pid_,SIGKILL);if(!exited(pid_))reaper().add(pid_);else reaper().release();pid_=-1;}
}
void IsolatedRenderer::start(){
    reaper().reserve();
    struct Reservation {bool transferred=false;~Reservation(){if(!transferred)reaper().release();}} reservation;
    auto binary=qEnvironmentVariable("MNM_RENDER_SERVICE_PATH");
    if(binary.isEmpty()){
        const auto base=QCoreApplication::applicationDirPath();
        binary=QDir(base).filePath("mnm-render-service");
        if(!QFileInfo::exists(binary))binary=QDir(base).filePath("renderer/mnm-render-service");
    }
    int toChild[2],fromChild[2];
    if(::socketpair(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0,toChild))throw TransportFailure("Cannot create render process input");
    if(::pipe2(fromChild,O_CLOEXEC)){::close(toChild[0]);::close(toChild[1]);throw TransportFailure("Cannot create render process output");}
    posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions,toChild[0],STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions,fromChild[1],STDOUT_FILENO);
    posix_spawn_file_actions_addclosefrom_np(&actions,3);
    std::vector<QByteArray> environment;
    for(auto e=environ;*e;++e){
        QByteArray v(*e);
        const bool vendorOverride=v.startsWith("LIBGL_ALWAYS_SOFTWARE=") || v.startsWith("__GLX_VENDOR_LIBRARY_NAME=") || v.startsWith("__EGL_VENDOR_LIBRARY_FILENAMES=") || v.startsWith("MESA_LOADER_DRIVER_OVERRIDE=") || v.startsWith("GALLIUM_DRIVER=");
        if(v.startsWith("MNM_RENDER_SERVICE=") || (software_ && vendorOverride))continue;
        environment.push_back(v);
    }
    environment.push_back("MNM_RENDER_SERVICE=1");
    if(software_){
        environment.push_back("LIBGL_ALWAYS_SOFTWARE=1");
        environment.push_back("__GLX_VENDOR_LIBRARY_NAME=mesa");
        const QString vendor="/usr/share/glvnd/egl_vendor.d/50_mesa.json";
        if(QFileInfo::exists(vendor))environment.push_back("__EGL_VENDOR_LIBRARY_FILENAMES="+QFile::encodeName(vendor));
    }
    std::vector<char*> env;for(auto& v:environment)env.push_back(v.data());env.push_back(nullptr);
    auto path=QFile::encodeName(binary);char* argv[]={path.data(),nullptr};
    const int error=posix_spawn(&pid_,path.constData(),&actions,nullptr,argv,env.data());
    posix_spawn_file_actions_destroy(&actions);::close(toChild[0]);::close(fromChild[1]);
    if(error){::close(toChild[1]);::close(fromChild[0]);pid_=-1;throw TransportFailure("Cannot launch render service: "+std::string(std::strerror(error)));}
    input_=toChild[1];output_=fromChild[0];reservation.transferred=true;
    // Nonblocking writes ensure a stopped driver cannot strand a pipe writer.
    ::fcntl(input_,F_SETFL,::fcntl(input_,F_GETFL)|O_NONBLOCK);
    ::fcntl(output_,F_SETFL,::fcntl(output_,F_GETFL)|O_NONBLOCK);
}
QByteArray IsolatedRenderer::exchange(const QByteArray& request){
    const auto packet=wire::message(wire::version,request);
    if(packet.size()>wire::maxPacket)throw std::runtime_error("Render message budget exceeded");
    QElapsedTimer clock;clock.start();auto size=qToBigEndian(quint32(packet.size()));
    transfer(input_,reinterpret_cast<char*>(&size),4,true,timeout_,clock);
    transfer(input_,const_cast<char*>(packet.constData()),packet.size(),true,timeout_,clock);
    transfer(output_,reinterpret_cast<char*>(&size),4,false,timeout_,clock);
    const auto length=qFromBigEndian(size);if(length>wire::maxPacket)throw TransportFailure("Render response exceeds protocol budget");
    QByteArray response(length,Qt::Uninitialized);transfer(output_,response.data(),length,false,timeout_,clock);
    QDataStream stream(response);stream.setVersion(QDataStream::Qt_6_0);quint32 version,status;QByteArray body;
    wire::values(stream,version,status,body);wire::check(stream);
    if(version!=wire::version)throw TransportFailure("Render service protocol mismatch");
    if(status==2)throw TransportFailure(body.toStdString());
    if(status)throw std::runtime_error(body.toStdString());
    return body;
}
void IsolatedRenderer::restore(){
    if(!checkpoint_.isEmpty())exchange(wire::message(wire::Op::load,checkpoint_));
    for(const auto& command:journal_)exchange(command);
}
void IsolatedRenderer::restart(){
    if(QThread::currentThread()!=owner_)throw std::runtime_error("Render process used on another thread");
    retire();software_=true;++recoveries_;start();restore();
}
QByteArray IsolatedRenderer::call(const QByteArray& request,bool mutation){
    if(QThread::currentThread()!=owner_)throw std::runtime_error("Render process used on another thread");
    // Retain the operation before submission; commit only a complete success reply.
    // Retry replays only renderer work, never application callbacks/acknowledgments.
    QByteArray result;
    for(unsigned attempt=0;;++attempt){
        try{
            if(attempt)restart();
            if(fault_){auto mode=fault_;fault_=0;exchange(wire::message(wire::Op::fault,mode));}
            result=exchange(request);break;
        }catch(const TransportFailure&){if(attempt==2)throw;}
    }
    if(mutation){journal_.push_back(request);journalBytes_+=request.size();}
    if(journal_.size()>=256 || journalBytes_>=8*1024*1024)checkpoint();
    return result;
}
void IsolatedRenderer::checkpoint(){
    // Keep the old checkpoint and full journal until a new snapshot is received.
    // A crash while taking the snapshot retries from the old committed state.
    for(unsigned attempt=0;;++attempt){try{if(attempt)restart();
        if(fault_){auto mode=fault_;fault_=0;exchange(wire::message(wire::Op::fault,mode));}
        auto next=exchange(wire::message(wire::Op::checkpoint));checkpoint_=std::move(next);journal_.clear();journalBytes_=0;return;}
        catch(const TransportFailure&){if(attempt==2)throw;}}
}
}
