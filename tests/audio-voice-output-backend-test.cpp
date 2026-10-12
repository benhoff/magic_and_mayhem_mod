#include "voice_output.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void wait(QCoreApplication& app,int milliseconds){
    QElapsedTimer elapsed;elapsed.start();
    while(elapsed.elapsed()<milliseconds){app.processEvents();QThread::msleep(1);}
}
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    const auto target=QMediaDevices::defaultAudioOutput();const auto format=selectOutputFormat(target);
    check(format.isValid(),"no supported current Qt output");
    Device device(1024*1024,8,std::uint32_t(format.sampleRate()));
    QAudioDevice selected=target;
    VoiceOutput output(std::make_unique<QtOutput>(device),[&]{return selected;},false);
    BufferId id=0;const auto rate=device.outputRate();
    check(device.createStatic(0xea,{1,1,rate,rate*2,2,16,0},rate*2,id)==Error::ok,"silent sample");
    check(device.setVolume(id,-1200)==Error::ok && device.setPan(id,3333)==Error::ok &&
          device.setFrequency(id,rate/2)==Error::ok,"retained preferences");
    check(output.timeline().play(id,1)==Error::ok && output.start(),"native output admission");
    wait(app,100);output.refreshClock();check(output.timeline().consumedFrames()>0,"initial backend progress");
    selected={};output.rescan();
    const auto frozen=output.timeline().cursor(id);
    check(!output.available() && output.losses()==1 && device.generatedCursor(id)==frozen,"endpoint loss rewinds consumed state");
    wait(app,50);check(output.timeline().cursor(id)==frozen,"unavailable clock remains frozen");
    selected=target;QElapsedTimer elapsed;elapsed.start();
    while(!output.available() && elapsed.elapsed()<2000){app.processEvents();QThread::msleep(1);}
    check(output.available() && output.recoveries()==1 && output.timeline().cursor(id)==frozen,"automatic Qt endpoint recovery retains cursor");
    wait(app,100);output.refreshClock();
    check(output.timeline().consumedFrames()>0 && output.timeline().cursor(id)!=frozen,"new backend epoch progresses");
    check(device.voice(id)->volume==-1200 && device.voice(id)->pan==3333 && device.frequency(id)==rate/2 &&
          output.timeline().voice(id)->status()==5,"Qt recovery retains gain/pan/pitch/loop identity");
    output.stop();check(!output.recover(),"retired endpoint cannot restart");
    std::cout<<"Controlled silent Qt endpoint loss and automatic recovery passed; device: "<<target.description().toStdString()
             <<"; rate: "<<rate<<"; system default unchanged; no physical disappearance/default-event or speaker equivalence asserted\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
