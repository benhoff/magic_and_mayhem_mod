#include "render_control.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
#include <cstring>
static void check(bool v,const char* reason){if(!v)throw std::runtime_error(reason);}
struct Peer{
    QFile f;uchar* p;
    explicit Peer(const QString& path):f(path){check(f.open(QIODevice::ReadWrite),"peer open");p=f.map(0,576);check(p,"peer map");}
    ~Peer(){f.unmap(p);}
    void set(unsigned at,quint32 v){__atomic_store_n(reinterpret_cast<quint32*>(p+at),qToLittleEndian(v),__ATOMIC_RELEASE);}
    quint32 get(unsigned at){return qFromLittleEndian(__atomic_load_n(reinterpret_cast<quint32*>(p+at),__ATOMIC_ACQUIRE));}
};
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    for(unsigned mode=0;mode<9;++mode){
        QTemporaryDir dir;const auto path=dir.filePath("control");RenderControl host;check(host.create(path,17),"host create");Peer peer(path);
        check(std::memcmp(peer.p,"MNMRCV01",8)==0 && peer.get(12)==576 && peer.get(16)==17,"literal wire identity");
        check(host.recover(dir.filePath("commands"),18),"request");
        check(peer.get(20)==1 && peer.get(24)==1 && peer.get(28)==18 && peer.get(44)>0 && peer.p[64]=='Z' && peer.p[65]==':',"published request");
        if(mode==0){check(host.poll()==0,"no early acceptance");peer.set(48,1);peer.set(36,1);peer.set(32,1);check(host.poll()==1,"matching accepted response");
            for(unsigned i=2;i<=3;++i){check(host.recover(dir.filePath("next"),18+i),"next request");peer.set(32,i);check(host.poll()==1,"next accepted");}
            check(!host.recover(dir.filePath("exhausted"),22),"finite request lifetime");
        }else{
            if(mode==1)peer.set(16,99);
            if(mode==2)peer.set(52,1);
            if(mode==3)peer.set(36,99);
            if(mode==4)peer.set(48,3);
            if(mode==5)peer.set(32,2);
            if(mode==6)peer.set(40,1);
            if(mode==7)peer.set(48,2);
            if(mode==8){peer.set(36,2);peer.set(32,1);}
            check(host.poll()==-1 && !host.error().isEmpty(),"refusal");check(!host.recover(dir.filePath("late"),20),"no revival after failure");
        }
        check(peer.get(40)==1,"permanent cancellation");
    }
    {QTemporaryDir dir;auto path=dir.filePath("existing");QFile f(path);check(f.open(QIODevice::WriteOnly),"existing");f.write("keep");f.close();RenderControl host;check(!host.create(path,17),"existing rejected");check(f.open(QIODevice::ReadOnly) && f.readAll()=="keep","existing unchanged");}
    {QTemporaryDir dir;const auto path=dir.filePath("checkpoint");RenderControl host;check(host.create(path,123),"checkpoint create");Peer peer(path);
     check(host.checkpoint(dir.filePath("late"),124),"checkpoint request");check(peer.get(24)==2 && peer.get(20)==1 && peer.get(28)==124,"literal checkpoint operation");
     check(host.poll()==0,"checkpoint before readiness");peer.set(36,1);peer.set(32,1);check(host.poll()==1,"checkpoint matching ready");host.cancel();check(peer.get(40)==1,"checkpoint cancellation");}
    for(unsigned mode=0;mode<5;++mode){QTemporaryDir dir;const auto path=dir.filePath("stop");RenderControl host;check(host.create(path,123),"stop create");Peer peer(path);
        if(mode==1)for(unsigned i=1;i<=3;++i){check(host.recover(dir.filePath("next"),123+i),"before stop recovery");peer.set(36,1);peer.set(32,i);check(host.poll()==1,"before stop response");}
        if(mode==2){check(host.recover(dir.filePath("pending"),124),"pending request");check(!host.stop() && peer.get(40)==1,"pending stop refuses and cancels");continue;}
        check(host.stop() && host.stop(),"idempotent stop request");check(peer.get(20)==(mode==1?4u:1u) && peer.get(24)==3 && !peer.get(28) && !peer.get(44),"literal terminal stop header");
        for(unsigned i=64;i<576;++i)check(!peer.p[i],"empty stop payload");
        check(host.poll()==0,"stop waits for response");peer.set(48,2);peer.set(36,mode==3?2:1);peer.set(32,peer.get(20));if(mode==4)peer.set(16,125);
        check(host.poll()==(mode>=3?-1:1),"stop response and stopped worker ordering");check(!host.recover(dir.filePath("late"),130),"terminal stop never recovers");
    }
    std::puts("{\"success\":true,\"cases\":16}");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
