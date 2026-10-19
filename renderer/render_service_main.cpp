#include "blit.hpp"
#include "isolated_renderer.hpp"
#include <QGuiApplication>
#include <QtEndian>
#include <unistd.h>
#include <cerrno>
#include <sys/prctl.h>
#include <signal.h>
namespace {
bool readAll(char* bytes,std::size_t size){while(size){auto n=::read(STDIN_FILENO,bytes,size);if(n<0&&errno==EINTR)continue;if(n<=0)return false;bytes+=n;size-=std::size_t(n);}return true;}
bool writeAll(const char* bytes,std::size_t size){while(size){auto n=::write(STDOUT_FILENO,bytes,size);if(n<0&&errno==EINTR)continue;if(n<=0)return false;bytes+=n;size-=std::size_t(n);}return true;}
}
int main(int argc,char** argv){
    const auto parent=::getppid();
    if(parent==1 || ::prctl(PR_SET_PDEATHSIG,SIGKILL) || ::getppid()!=parent)return 4;
    qputenv("MNM_RENDER_SERVICE","1");QGuiApplication app(argc,argv);
    using namespace mnm::render;
    try{
        GlBlitter renderer;unsigned fault=0;
        for(;;){
            quint32 size;if(!readAll(reinterpret_cast<char*>(&size),4))return 0;
            const auto length=qFromBigEndian(size);if(length>wire::maxPacket)return 2;
            QByteArray envelope(length,Qt::Uninitialized);if(!readAll(envelope.data(),length))return 2;
            QDataStream envelopeStream(envelope);envelopeStream.setVersion(QDataStream::Qt_6_0);quint32 version;QByteArray request;wire::values(envelopeStream,version,request);wire::check(envelopeStream);if(version!=wire::version)return 2;
            quint32 status=0;QByteArray body;
            try{
                QDataStream in(request);in.setVersion(QDataStream::Qt_6_0);wire::Op op;wire::get(in,op);
                if(op==wire::Op::fault){wire::get(in,fault);wire::check(in);}
                else{
                    body=renderer.dispatchIsolated(request);
                    const auto mode=fault;fault=0;
                    if(mode==1)::_exit(71);
                    if(mode==2)for(;;)::pause();
                    if(mode==3)throw GraphicsFailure("Injected driver failure after operation, before acknowledgment");
                }
            }catch(const GraphicsFailure& e){status=2;body=e.what();}
            catch(const std::exception& e){status=1;body=e.what();}
            auto response=wire::message(wire::version,status,body);size=qToBigEndian(quint32(response.size()));
            if(!writeAll(reinterpret_cast<const char*>(&size),4)||!writeAll(response.constData(),response.size()))return 0;
        }
    }catch(const std::exception& e){qCritical("Render service: %s",e.what());return 3;}
}
