#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QTimer>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
static void check(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static void save(const QString& p,const QJsonObject& data){QFile f(p);check(f.open(QIODevice::WriteOnly),"report open");f.write(QJsonDocument(data).toJson());}
int main(int argc,char** argv){QSurfaceFormat fmt;fmt.setVersion(3,3);fmt.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(fmt);QApplication app(argc,argv);
try{check(argc==3,"arguments");QDir dir(QString::fromLocal8Bit(argv[1]));const bool host=QString::fromLatin1(argv[2])=="host";
    GlViewport viewport;viewport.resize(600,600);viewport.show();app.processEvents();check(viewport.ready(),"GPU ready");
    LiveCommandSession session(viewport);LiveCommandRenderer renderer(viewport);RenderControl control;
    check(host?session.create(dir.filePath("commands.bin"),123,2):renderer.create(dir.filePath("commands.bin"),123,2),"command create");
    if(!host)check(control.create(dir.filePath("commands.bin.control"),123),"control create");
    QJsonArray frames;bool finished=false,requested=false;QElapsedTimer elapsed;elapsed.start();
    auto result=[&](){return host?session.result():renderer.result();};
    auto frame=[&](){auto image=viewport.grabFramebuffer();const auto size=viewport.frameSize();const auto rect=viewport.imageRect();QByteArray bytes(size.width()*size.height()*4,0);
        for(int y=0;y<size.height();++y)for(int x=0;x<size.width();++x)qToLittleEndian<quint32>(image.pixel(int(rect.left()+(x+.5)*rect.width()/size.width()),int(rect.top()+(y+.5)*rect.height()/size.height())),bytes.data()+(y*size.width()+x)*4);
        frames.append(QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()));save(dir.filePath("progress.json"),{{"frames",frames}});
    };
    if(host)session.framePresented=frame;else renderer.framePresented=frame;
    QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&](){try{
        check(elapsed.elapsed()<20000,"deadline");
        if(QFile::exists(dir.filePath("qt-exit"))){app.exit(0);return;}
        if(finished || QFile::exists(dir.filePath("pause")))return;
        if(host && !requested && QFile::exists(dir.filePath("host-request"))){requested=true;check(session.requestStop(),"host stop request");check(session.requestStop(),"idempotent host request");}
        const bool healthy=host?session.poll():renderer.poll();const bool ended=host?session.ended():renderer.ended();
        if(!healthy || ended){const auto* r=result();check(r && !r->liveSurfaces && !r->livePalettes,"terminal resources");check(!r->stats.nativeReadbacks && !r->stats.rgbaReadbacks && !viewport.imageUploads(),"ordinary readbacks/uploads");
            save(dir.filePath("qt.json"),{{"error",host?session.error():renderer.error()},{"success",healthy},{"ended",ended},{"frames",frames},{"native_readbacks",double(r->stats.nativeReadbacks)},{"rgba_readbacks",double(r->stats.rgbaReadbacks)},{"live_surfaces",double(r->liveSurfaces)},{"host_request",requested}});
            save(dir.filePath("host-ended-00000000.bin"),{{"ended",ended}});finished=true;
        }
    }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());app.exit(8);}});
    save(dir.filePath("ready.json"),{{"ready",true}});timer.start(1);return app.exec();
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 8;}}
