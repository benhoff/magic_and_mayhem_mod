#include "live_command_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <cstdio>
#include <stdexcept>
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void save(const QString& path,const QJsonObject& data){QFile f(path);check(f.open(QIODevice::WriteOnly),"write report");f.write(QJsonDocument(data).toJson());}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QApplication app(argc,argv);
    try{
        check(argc==2,"args: case directory");QDir dir(QString::fromLocal8Bit(argv[1]));
        GlViewport viewport;viewport.setMinimumSize(1,1);viewport.resize(512,512);viewport.show();app.processEvents();check(viewport.ready(),"viewport ready");
        auto* context=viewport.context();LiveCommandSession session(viewport);QJsonArray frames,states;unsigned clears=0;QElapsedTimer elapsed;elapsed.start();
        check(session.create(dir.filePath("commands.bin"),123,2),"session create");
        session.stateChanged=[&](auto state){
            states.append(int(state));check(context==viewport.context(),"context changed during handoff");
            if(state==LiveCommandSession::State::Recovering || state==LiveCommandSession::State::Fallback){check(viewport.frameSize().isEmpty(),"old presentation retained");++clears;}
        };
        session.framePresented=[&]{
            const auto image=viewport.grabFramebuffer();check(image.size()==QSize(512,512),"framebuffer size");
            const auto pixel=image.pixel(0,0);unsigned index=40;
            for(unsigned i=0;i<40;++i){const auto color=i*0x254713u&0xffffff;if(pixel==qRgb((color>>16)&255,(color>>8)&255,color&255)){index=i;break;}}
            check(index<40,"unexpected color");for(int y=0;y<512;++y)for(int x=0;x<512;++x)check(image.pixel(x,y)==pixel,"incomplete independent frame");
            if(!frames.isEmpty())check(index>unsigned(frames.last().toObject()["index"].toInt()),"stale or reordered frame");
            frames.append(QJsonObject{{"index",int(index)},{"session",int(session.recoveries())}});
            check(viewport.imageUploads()==0,"CPU viewport upload");
            check(session.result() && session.result()->stats.nativeReadbacks==0 && session.result()->stats.rgbaReadbacks==0,"ordinary consumer readback");save(dir.filePath("progress.json"),{{"frames",frames},{"states",states}});
        };
        QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{
            try{
                const bool healthy=session.poll();
                if(!healthy || session.ended() || elapsed.elapsed()>22000){
                    timer.stop();check(elapsed.elapsed()<=22000,"test deadline");
                    check(!session.result() || session.result()->liveSurfaces==0,"consumer resources retained at terminal");
                    save(dir.filePath("qt.json"),{{"ended",session.ended()},{"error",session.error()},{"frames",frames},{"states",states},{"clears",int(clears)},{"recoveries",int(session.recoveries())},{"same_context",context==viewport.context()},{"viewport_uploads",int(viewport.imageUploads())}});app.exit(0);
                }
            }catch(const std::exception& e){std::fprintf(stderr,"recovery test failed: %s\n",e.what());app.exit(8);}
        });save(dir.filePath("ready.json"),{{"ready",true}});timer.start(1);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"recovery test failed: %s\n",e.what());return 8;}
}
