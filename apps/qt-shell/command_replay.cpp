#include "command_replay.hpp"
#include "commands.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QFile>
#include <QTimer>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

int runCommandReplay(const QString& path,bool smokeTest,bool verifyChecks){
    auto* application=qobject_cast<QApplication*>(QCoreApplication::instance());
    if(!application){std::fputs("Command replay requires QApplication\n",stderr);return 8;}
    auto& app=*application;
    try {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly) || file.size()>mnm::render::maxCommandBytes)
            throw std::runtime_error("Cannot read bounded command stream");
        const auto commands=mnm::render::decodeCommands(file.read(mnm::render::maxCommandBytes+1));
        GlViewport viewport;viewport.setWindowTitle("Magic & Mayhem — incremental GPU command replay");
        viewport.resize(640,480);viewport.show();
        QImage expected;if(smokeTest)expected=mnm::render::replayCommands(commands).presentation;
        std::unique_ptr<mnm::render::GlBlitter> renderer;
        std::unique_ptr<mnm::render::CommandConsumer> consumer;
        std::size_t cursor=0;unsigned batches=0;QTimer timer;
        QObject::connect(&timer,&QTimer::timeout,&viewport,[&]{
            try {
                if(!consumer){
                    if(!viewport.ready())throw std::runtime_error("Viewport OpenGL context is unavailable");
                    renderer=std::make_unique<mnm::render::GlBlitter>(viewport.context());
                    consumer=std::make_unique<mnm::render::CommandConsumer>(*renderer,[&](auto frame){
                        viewport.setGpuFrame(std::move(frame));viewport.repaint();
                        if(!viewport.error().isEmpty())throw std::runtime_error(viewport.error().toStdString());
                    },mnm::render::CommandConsumerOptions{verifyChecks?mnm::render::CommandDiagnostics::Verify:mnm::render::CommandDiagnostics::Skip,false});
                }
                const auto count=std::min<std::size_t>(32,commands.size()-cursor);
                consumer->submit(commands.data()+cursor,count);cursor+=count;++batches;
                if(cursor!=commands.size())return;
                consumer->finish();timer.stop();const auto& result=consumer->result();
                std::fprintf(stderr,"GPU replay: %u presentations, %llu native CHECK readbacks, %llu RGBA CHECK readbacks, %llu viewport uploads; %u batches, %u skipped native checks, %u skipped RGBA checks\n",
                    result.presents,static_cast<unsigned long long>(result.stats.nativeReadbacks),
                    static_cast<unsigned long long>(result.stats.rgbaReadbacks),static_cast<unsigned long long>(viewport.imageUploads()),
                    batches,result.skippedChecks,result.skippedColorChecks);
                if(smokeTest)QTimer::singleShot(0,&viewport,[&]{
                    if(!viewport.ready() || !viewport.error().isEmpty()){app.exit(6);return;}
                    const auto actual=viewport.grabFramebuffer();const auto& image=expected;
                    const auto scale=qMin(double(actual.width())/image.width(),double(actual.height())/image.height());
                    const int w=qRound(image.width()*scale),h=qRound(image.height()*scale);
                    const int left=(actual.width()-w)/2,top=actual.height()-h-(actual.height()-h)/2;
                    bool ok=viewport.imageUploads()==0;
                    for(int y=0;y<3;++y)for(int x=0;x<3;++x){
                        const int px=(2*x+1)*w/6,py=(2*y+1)*h/6;
                        const int ix=qMin(image.width()-1,int((px+0.5)*image.width()/w));
                        const int iy=qMin(image.height()-1,int((py+0.5)*image.height()/h));
                        if(actual.pixelColor(left+px,top+py)!=image.pixelColor(ix,iy))ok=false;
                    }
                    app.exit(ok?0:7);
                });
            }catch(const std::exception& e){timer.stop();std::fprintf(stderr,"GPU replay failed: %s\n",e.what());app.exit(8);}
        });
        timer.start(0);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"Command replay failed: %s\n",e.what());return 8;}
}
