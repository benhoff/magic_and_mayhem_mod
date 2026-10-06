#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QtEndian>
#include <QDir>
#include <cstdio>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void save(const QString& path,const QJsonObject& data){QFile f(path);check(f.open(QIODevice::WriteOnly),"report write");f.write(QJsonDocument(data).toJson());}
int main(int argc,char** argv){QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QApplication app(argc,argv);
    try{check(argc==2,"case directory");QDir dir(QString::fromLocal8Bit(argv[1]));GlViewport viewport;viewport.resize(600,400);viewport.show();app.processEvents();check(viewport.ready(),"GPU ready");auto* context=viewport.context();LiveCommandSession session(viewport);check(session.create(dir.filePath("commands.bin"),123,2),"session create");QJsonArray frames,states;unsigned requested=0;QElapsedTimer elapsed;elapsed.start();
        session.stateChanged=[&](auto state){states.append(int(state));check(context==viewport.context(),"context changed");if(state==LiveCommandSession::State::Recovering || state==LiveCommandSession::State::Fallback)check(viewport.frameSize().isEmpty(),"stale frame");};
        session.framePresented=[&]{const auto image=viewport.grabFramebuffer();const auto size=viewport.frameSize();const auto rect=viewport.imageRect();QByteArray pixels(size.width()*size.height()*4,0);
            for(int y=0;y<size.height();++y)for(int x=0;x<size.width();++x)qToLittleEndian<quint32>(image.pixel(int(rect.left()+(x+0.5)*rect.width()/size.width()),int(rect.top()+(y+0.5)*rect.height()/size.height())),pixels.data()+(y*size.width()+x)*4);
            check(session.result() && !session.result()->stats.nativeReadbacks && !session.result()->stats.rgbaReadbacks && !viewport.imageUploads(),"ordinary readbacks/uploads");
            frames.append(QJsonObject{{"hash",QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex())},{"session",int(session.recoveries())}});save(dir.filePath("progress.json"),{{"frames",frames},{"states",states}});
        };
        QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{try{
            check(elapsed.elapsed()<20000,"test deadline");
            if(QFile::exists(dir.filePath(QString("attach-%1").arg(requested)))){++requested;check(session.attachCheckpoint(),"attach request");}
            // The initial consumer deliberately never reads incremental history.
            const bool healthy=!requested || session.poll();
            if(!healthy || session.ended()){timer.stop();check(!session.result() || (!session.result()->liveSurfaces && !session.result()->livePalettes),"terminal resources");save(dir.filePath("qt.json"),{{"ended",session.ended()},{"error",session.error()},{"frames",frames},{"states",states},{"requests",int(requested)},{"same_context",context==viewport.context()}});app.exit(0);}
        }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());app.exit(8);}});save(dir.filePath("ready.json"),{{"ready",true}});timer.start(1);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 8;}}
