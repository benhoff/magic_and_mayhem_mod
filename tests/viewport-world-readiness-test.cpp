#include "gl_viewport.hpp"
#include <QApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QSurfaceFormat>
#include <QThread>
#include <cstdio>
#include <stdexcept>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
}
int main(int argc,char** argv){
    const bool raster=argc>1&&QByteArray(argv[1])=="raster";
    const bool capture=argc>2&&QByteArray(argv[2])=="capture";
    if(raster)qputenv("MNM_NATIVE_DEVICE_RECOVERY","1");else qunsetenv("MNM_NATIVE_DEVICE_RECOVERY");
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);QTemporaryDir output;
    try{
        require(output.isValid(),"Cannot create bounded capture directory");
        qunsetenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY");
        if(capture){qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",output.path().toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","world-readiness-control");}
        {
            GlViewport viewport;viewport.resize(320,240);viewport.show();app.processEvents();
            require(viewport.ready()&&viewport.rasterPresentation()==raster,"Wrong viewport transport");
            mnm::render::GlBlitter renderer(viewport.context());
            const auto surface=renderer.create({2,2,{0xf800,0x7e0,31,65535}},{16,{0xf800,0x7e0,31}});
            auto frame=renderer.presentGpu(surface).snapshot();
            const auto paint=[&](std::uint64_t expected){
                const auto before=viewport.swappedPaint();viewport.repaint();QElapsedTimer timer;timer.start();
                while((viewport.swappedWorldGeneration()!=expected||viewport.swappedPaint()<=before)&&timer.elapsed()<2000){app.processEvents();QThread::msleep(5);}
                require(viewport.error().isEmpty()&&viewport.swappedWorldGeneration()==expected&&viewport.swappedPaint()>before,"Paint/swap did not publish World identity");
            };
            const auto pending=[&](std::uint64_t previous,std::uint64_t generation){
                viewport.setGpuFrame(frame,123,generation);
                require(viewport.swappedWorldGeneration()==previous,"Queued frame was accepted before paint/swap");
            };
            pending(0,7);paint(7);
            pending(7,8);paint(8);
            // The legacy GUI renderer supplies identity through its observer;
            // the dedicated renderer passes it with the immutable frame lease.
            viewport.acceptanceState=[]{return QJsonObject{{"world","11"},{"lease","321"}};};
            pending(8,0);paint(11);
            viewport.acceptanceState={};QImage menu(2,2,QImage::Format_RGBA8888);menu.fill(Qt::red);
            viewport.setFrame(menu);require(viewport.swappedWorldGeneration()==11,"Menu assignment bypassed paint/swap");paint(0);
            frame={};renderer.destroy(surface);
        }
        const auto files=QDir(output.path()).entryList({"*.bin"},QDir::Files);
        require(capture?!files.isEmpty():files.isEmpty(),"Capture mode wrote the wrong output set");
        std::printf("{\"success\":true,\"raster\":%s,\"capture\":%s,\"pending_frame_refused\":true,\"worlds\":[7,8,11,0],\"capture_files\":%lld}\n",raster?"true":"false",capture?"true":"false",static_cast<long long>(files.size()));
        return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
