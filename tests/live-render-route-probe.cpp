#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSurfaceFormat>
#include <QTimer>
#include <cstdio>
#include <stdexcept>
static void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
static void save(const QString& path,const QJsonObject& value){QSaveFile file(path);check(file.open(QIODevice::WriteOnly),"report open");check(file.write(QJsonDocument(value).toJson())>0 && file.commit(),"report commit");}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QApplication app(argc,argv);
    try{
        check(argc==2,"args: observation directory");QDir dir(QString::fromLocal8Bit(argv[1]));
        GlViewport viewport;viewport.resize(800,600);viewport.move(900,0);viewport.show();app.processEvents();check(viewport.ready(),"GPU ready");
        LiveCommandSession session(viewport);check(session.create(dir.filePath("commands.bin"),123,2),"session create");
        QElapsedTimer elapsed;elapsed.start();unsigned frames=0,lastRequest=0,snapshots=0;bool attached=false;
        const auto status=[&]{
            auto result=session.result();return QJsonObject{{"state",int(session.state())},{"error",session.error()},{"frames",int(frames)},{"recoveries",int(session.recoveries())},{"ms",int(elapsed.elapsed())},{"width",viewport.frameSize().width()},{"height",viewport.frameSize().height()},{"commands",result?int(result->commands):0},{"live_surfaces",result?int(result->liveSurfaces):0},{"live_palettes",result?int(result->livePalettes):0},{"ordinary_native_readbacks",result?int(result->stats.nativeReadbacks):0},{"ordinary_rgba_readbacks",result?int(result->stats.rgbaReadbacks):0},{"viewport_uploads",int(viewport.imageUploads())},{"explicit_framebuffer_captures",int(snapshots)}};
        };
        session.framePresented=[&]{++frames;check(!viewport.imageUploads() && session.result() && !session.result()->stats.nativeReadbacks && !session.result()->stats.rgbaReadbacks,"ordinary readback/upload");save(dir.filePath("progress.json"),status());};
        QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{
            try{
                if(!attached && QFile::exists(dir.filePath("attach"))){attached=true;elapsed.restart();}
                if(attached)session.poll();
                QFile request(dir.filePath("request.json"));
                if(request.open(QIODevice::ReadOnly)){
                    check(request.size()<=1024,"request budget");auto value=QJsonDocument::fromJson(request.readAll()).object();unsigned id=value["id"].toInt();
                    if(id>lastRequest){
                        check(id==lastRequest+1 && id<=160,"request order/budget");lastRequest=id;const auto operation=value["op"].toString();auto result=status();
                        if(operation=="snapshot"){
                            check(session.state()==LiveCommandSession::State::Active && !viewport.frameSize().isEmpty(),"snapshot requires native frame");
                            const auto name=QString("native-%1.png").arg(id,8,10,QChar('0'));check(viewport.grabFramebuffer().save(dir.filePath(name)),"native capture");++snapshots;result=status();result["image"]=name;
                        }else if(operation=="finish"){
                            session.abort();check(!session.result() || (!session.result()->liveSurfaces && !session.result()->livePalettes),"resource cleanup");result["terminal_resources"]=0;result["success"]=true;
                            save(dir.filePath("summary.json"),result);timer.stop();app.quit();
                        }else check(operation=="status","unknown request");
                        result["id"]=int(id);result["op"]=operation;save(dir.filePath(QString("reply-%1.json").arg(id,8,10,QChar('0'))),result);
                    }
                }
                check(elapsed.elapsed()<180000,"observation deadline");
            }catch(const std::exception& e){std::fprintf(stderr,"route probe: %s\n",e.what());session.abort();timer.stop();app.exit(8);}
        });save(dir.filePath("ready.json"),{{"ready",true},{"poll_ms",16}});timer.start(16);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"route probe: %s\n",e.what());return 8;}
}
