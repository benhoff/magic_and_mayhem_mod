#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QDir>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSurfaceFormat>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
static void check(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static void save(const QString& path,const QJsonObject& data){QFile f(path);check(f.open(QIODevice::WriteOnly),"report write");f.write(QJsonDocument(data).toJson());}
int main(int argc,char** argv){QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QApplication app(argc,argv);
    try{check(argc==3,"args: directory checkpoint|recover");QDir dir(QString::fromLocal8Bit(argv[1]));const bool checkpoint=QString::fromLocal8Bit(argv[2])=="checkpoint";GlViewport viewport;viewport.resize(800,600);viewport.show();app.processEvents();check(viewport.ready(),"GPU ready");LiveCommandSession session(viewport);check(session.create(dir.filePath("commands.bin"),123,2),"session create");QJsonArray states,frames,negotiations;QElapsedTimer time;bool requested=false;time.start();
        session.stateChanged=[&](auto state){states.append(QJsonObject{{"state",int(state)},{"ms",int(time.elapsed())}});
            QFile control(dir.filePath("commands.bin.control"));if(control.open(QIODevice::ReadOnly)){auto bytes=control.read(64);if(bytes.size()==64)negotiations.append(QJsonObject{{"state",int(state)},{"request",int(qFromLittleEndian<quint32>(bytes.data()+20))},{"operation",int(qFromLittleEndian<quint32>(bytes.data()+24))},{"target",int(qFromLittleEndian<quint32>(bytes.data()+28))},{"response",int(qFromLittleEndian<quint32>(bytes.data()+32))},{"status",int(qFromLittleEndian<quint32>(bytes.data()+36))}});}};
        session.framePresented=[&]{frames.append(QJsonObject{{"session",int(session.recoveries())},{"ms",int(time.elapsed())},{"width",viewport.frameSize().width()},{"height",viewport.frameSize().height()}});check(!viewport.imageUploads() && session.result() && !session.result()->stats.nativeReadbacks && !session.result()->stats.rgbaReadbacks,"ordinary readbacks/uploads");};
        QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{try{
            if(!requested && QFile::exists(dir.filePath("attach"))){requested=true;time.restart();if(checkpoint)check(session.attachCheckpoint(),"checkpoint request");}
            bool healthy=!requested || session.poll();
            if(requested && (!healthy || session.ended() || time.elapsed()>12000)){
                timer.stop();const auto state=session.state();const auto error=session.error();session.abort();check(!session.result() || (!session.result()->liveSurfaces && !session.result()->livePalettes),"resource cleanup");
                save(dir.filePath("qt.json"),{{"success",true},{"state",int(state)},{"error",error},{"states",states},{"negotiations",negotiations},{"frames",frames},{"recoveries",int(session.recoveries())},{"checkpoint",checkpoint},{"ordinary_readbacks",0},{"viewport_uploads",0},{"terminal_resources",0}});app.exit(0);
            }
        }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());app.exit(8);}});save(dir.filePath("ready.json"),{{"ready",true}});timer.start(1);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 8;}}
