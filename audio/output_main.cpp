#include "qt_output.hpp"
#include <QCoreApplication>
#include <QMediaDevices>
#include <QTimer>
#include <cmath>
#include <iostream>
using namespace mnm::audio;
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);
    const auto args=app.arguments();
    if(args.size()!=2 || (args[1]!="--probe" && args[1]!="--tone")){
        std::cout<<"Usage: mnm-audio-output --probe | --tone\n--tone plays a quiet 440 Hz fixture for two seconds; no game required.\n";return args.contains("--help")?0:2;
    }
    const auto output=QMediaDevices::defaultAudioOutput();const auto format=selectOutputFormat(output);
    std::cout<<"Device: "<<output.description().toStdString()<<"; stereo Int16 rate: "<<format.sampleRate()<<'\n';
    if(!format.isValid()){std::cerr<<"No supported stereo Int16 output device\n";return 1;}
    if(args[1]=="--probe")return 0;
    Device device(16*1024*1024,128,std::uint32_t(format.sampleRate()));
    const auto rate=device.outputRate();PcmFormat pcm{1,1,rate,rate*2,2,16,0};BufferId id=0;
    if(device.createStatic(0xea,pcm,rate*2,id)!=Error::ok)return 1;
    WriteLock lock;if(device.lock(id,0,rate*2,0,lock)!=Error::ok)return 1;
    for(std::uint32_t i=0;i<rate;++i){const auto sample=std::uint16_t(std::int16_t(std::lround(2000*std::sin(6.283185307179586*440*i/rate))));lock.first.data[2*i]=std::uint8_t(sample);lock.first.data[2*i+1]=std::uint8_t(sample>>8);}
    if(device.unlock(lock,rate*2,0)!=Error::ok || device.play(id,1)!=Error::ok)return 1;
    QtOutput sink(device);sink.failed=[&](const QString& error){std::cerr<<error.toStdString()<<'\n';app.exit(1);};
    if(!sink.start(output)){std::cerr<<sink.lastError().toStdString()<<'\n';return 1;}
    QTimer::singleShot(1000,&app,[&]{
        std::cout<<"Before restart, processed microseconds: "<<sink.processedUSecs()<<'\n';
        if(sink.processedUSecs()<=0){sink.stop();app.exit(1);return;}
        sink.stop();
        if(!sink.start(output)){std::cerr<<sink.lastError().toStdString()<<'\n';app.exit(1);}
    });
    QTimer::singleShot(2000,&app,[&]{
        const auto processed=sink.processedUSecs();
        std::cout<<"Backend processed microseconds: "<<processed<<'\n';
        sink.stop();app.exit(processed>0?0:1);
    });return app.exec();
}
