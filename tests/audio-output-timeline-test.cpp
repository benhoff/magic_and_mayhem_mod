#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMediaDevices>
#include <QThread>
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void ok(Error error){check(error==Error::ok,"unexpected native audio error");}
BufferId sample(Device& d,std::size_t length=5,std::uint32_t rate=1000){
    BufferId id=0;ok(d.createStatic(0xea,{1,1,rate,rate*2,2,16,0},length*2,id));return id;
}
class Writer:public QIODevice {
public:
    Writer(){open(QIODevice::WriteOnly|QIODevice::Unbuffered);}
    qint64 limit=3;
    qint64 readData(char*,qint64) override{return -1;}
    qint64 writeData(const char*,qint64 bytes) override{return std::min(bytes,limit);}
};
void completion(){
    Device d(1024,8,2000);OutputTimeline clock(d);auto id=sample(d);ok(clock.play(id,0));
    std::vector<std::int16_t> pcm;ok(clock.render(12,pcm));
    check(d.voice(id)->playback==Playback::completed && clock.voice(id)->status()==1,"generated completion must wait for backend");
    check(clock.cursor(id)==0,"initial presented cursor");
    clock.consume(8);check(clock.cursor(id)==8 && clock.voice(id)->status()==1,"last source frame still playing");
    ok(clock.play(id,0));check(d.voice(id)->frame==5 && clock.cursor(id)==8,"repeated Play cannot replay queued tail");
    clock.consume(9);check(clock.voice(id)->status()==1,"fractional tail still playing");
    clock.consume(10);check(clock.voice(id)->status()==0 && clock.cursor(id)==0,"completion at consumed endpoint");
    clock.consume(1);check(clock.voice(id)->status()==0,"regressing backend clock ignored");
    clock.consume(100);check(clock.consumedFrames()==12 && clock.pendingBlocks()==0,"consumption clamped to generated frames");
    ok(clock.play(id,0));check(clock.voice(id)->status()==1 && clock.cursor(id)==0,"completed replay rewinds");
    ok(clock.render(12,pcm));clock.consume(14);
    check(d.voice(id)->playback==Playback::completed && clock.voice(id)->status()==1,"queued seek fixture");
    ok(clock.seek(id,2));check(d.voice(id)->status()==1 && clock.voice(id)->status()==1 && clock.cursor(id)==2,"seek queued one-shot retains logical playback");
    clock.consume(24);check(clock.cursor(id)==2 && clock.voice(id)->status()==1,"old tail cannot complete newly sought voice");
    ok(clock.render(8,pcm));clock.consume(32);check(clock.voice(id)->status()==0,"sought tail eventually completes");
}
void controls(){
    Device d(1024,8,2000);OutputTimeline clock(d);auto id=sample(d,20);ok(clock.play(id,1));
    std::vector<std::int16_t> pcm;ok(clock.render(6,pcm));clock.consume(3);
    check(clock.cursor(id)==2,"fractional consumption before pitch");
    ok(d.setFrequency(id,2000));ok(clock.render(4,pcm));
    clock.consume(5);check(clock.cursor(id)==4,"queued old pitch retained");
    clock.consume(8);check(clock.cursor(id)==10,"new pitch starts at rendered boundary");
    ok(clock.stop(id));check(d.voice(id)->frame==5 && clock.voice(id)->status()==0,"stop retains consumed cursor");
    clock.consume(10);check(clock.cursor(id)==10,"old queued generation cannot move stopped cursor");
    ok(clock.play(id,1));ok(clock.render(4,pcm));clock.consume(12);check(clock.cursor(id)==14,"resume consumed cursor");
    ok(d.setPosition(id,2));clock.changed(id);clock.consume(14);
    check(clock.cursor(id)==2 && clock.voice(id)->status()==5,"seek supersedes old generation");
    ok(clock.render(6,pcm));clock.consume(16);check(clock.cursor(id)==6,"seek advances only its own new PCM");
    const auto cursor=clock.cursor(id);clock.discard();
    check(d.generatedCursor(id)==cursor && clock.generatedFrames()==0 && clock.pendingBlocks()==0,"output reset rewinds unconsumed frames");
    ok(clock.render(2,pcm));clock.consume(1);check(clock.cursor(id)==*cursor+2,"fresh backend clock after reset");
    BufferId child=0;ok(d.duplicate(id,child));clock.changed(child);ok(clock.play(child,0));
    ok(clock.render(2,pcm));ok(d.release(child));clock.changed(child);clock.consume(4);
    check(!clock.voice(child) && clock.voice(id)->status()==5,"released queued identity cannot resurrect");
    ok(d.setVolume(id,-1200));check(clock.voice(id)->volume==-1200,"logical gain visible before consumption");
    const auto beforeLoopChange=clock.cursor(id);ok(clock.play(id,0));
    check(clock.cursor(id)==beforeLoopChange && clock.voice(id)->status()==1,"loop flag change preserves consumed position");
}
void oracle(){
    unsigned cases=0;
    for(auto rate:{700u,1000u,1700u,2000u})for(auto length:{1u,3u,7u})for(bool loop:{false,true}){
        Device d(1024,8,2000);OutputTimeline clock(d);auto id=sample(d,length,rate);ok(clock.play(id,loop?1:0));
        std::vector<std::int16_t> pcm;ok(clock.render(6,pcm));ok(clock.render(9,pcm));
        std::uint64_t source=0,phase=0;bool playing=true;
        for(unsigned frame=1;frame<=15;++frame){
            if(playing){phase+=rate;while(phase>=2000 && playing){phase-=2000;++source;
                if(source==length){if(loop)source=0;else{playing=false;phase=0;}}}}
            clock.consume(frame);const auto v=clock.voice(id);
            check(v->frame==source && (v->playback==Playback::playing)==playing,"independent per-frame phase oracle");++cases;
        }
    }
    std::cout<<cases<<" independent consumed-frame oracle cases\n";
}
void queueAndWire(){
    Device d(1024,8,2000);OutputTimeline clock(d);VoiceCommands commands(d,&clock);auto id=sample(d,2);ok(clock.play(id,0));
    PcmQueue queue(d,&clock);Writer writer;check(queue.pump(writer,16),"partial pump");
    check(queue.acceptedFrames()==0 && clock.voice(id)->status()==1,"partial byte is not consumed frame");
    clock.consume(queue.acceptedFrames());writer.limit=1;check(queue.pump(writer,16),"complete first output frame");
    clock.consume(queue.acceptedFrames());check(clock.voice(id)->status()==1,"one complete output frame");
    writer.limit=100;check(queue.pump(writer,16),"finish block");
    std::uint32_t request[16]{};request[6]=std::uint32_t(id);request[5]=MNM_AUDIO_STATUS;
    check(commands.execute(request,{}).value==1,"wire status must not report generated completion");
    clock.consume(3);request[5]=MNM_AUDIO_CURSOR;check(commands.execute(request,{}).value==2,"wire cursor uses consumed clock");
    clock.consume(4);request[5]=MNM_AUDIO_STATUS;check(commands.execute(request,{}).value==0,"wire completion at backend endpoint");
    queue.reset();check(queue.acceptedFrames()==0,"new sink accepted-byte epoch");
}
void bounds(){
    Device d(1024,8,1000);OutputTimeline clock(d);std::vector<std::int16_t> pcm;
    ok(clock.render(2000,pcm));const auto before=pcm;
    check(clock.render(1,pcm)==Error::busy && pcm==before,"two-second queue bound preserves output");clock.discard();
    for(std::size_t i=0;i<OutputTimeline::maxBlocks;++i)ok(clock.render(1,pcm));
    check(clock.render(1,pcm)==Error::busy,"journal entry bound");clock.consume(256);ok(clock.render(1,pcm));
}
void backend(QCoreApplication& app){
    const auto output=QMediaDevices::defaultAudioOutput();const auto format=selectOutputFormat(output);
    check(format.isValid(),"no supported Qt backend for consumed-clock test");
    Device d(1024*1024,8,std::uint32_t(format.sampleRate()));QtOutput sink(d);auto id=sample(d,d.outputRate()/5,d.outputRate());
    auto& clock=sink.timeline();ok(clock.play(id,0));check(sink.start(output),"Qt backend start");
    bool sawQueuedTail=false,completed=false;QElapsedTimer elapsed;elapsed.start();
    while(elapsed.elapsed()<2000){
        app.processEvents();sink.refreshClock();
        check(sink.running(),"backend stopped unexpectedly");
        const auto generated=d.voice(id),presented=clock.voice(id);
        if(generated->playback==Playback::completed && presented->playback==Playback::playing)sawQueuedTail=true;
        if(presented->playback==Playback::completed){completed=true;break;}
        QThread::msleep(1);
    }
    check(completed && sawQueuedTail,"backend must consume queued tail before completion");
    check(clock.consumedFrames()<=clock.generatedFrames() && clock.consumedFrames()>0,"backend consumption bound");
    sink.stop();ok(clock.play(id,1));check(sink.start(output),"backend restart");
    elapsed.restart();while(elapsed.elapsed()<100){app.processEvents();QThread::msleep(1);}
    sink.refreshClock();const auto position=clock.cursor(id);check(position && *position>0,"restarted backend clock progress");
    sink.stop();check(d.generatedCursor(id)==position,"sink reset retains consumed cursor");
    check(sink.start(output),"second backend restart");
    check(clock.voice(id)->status()==5 && clock.cursor(id)==position,"new sink epoch retains voice state");
    sink.stop();
    std::cout<<"Qt silent backend consumed-clock/restart passed; device: "<<output.description().toStdString()<<"; rate: "<<format.sampleRate()<<"; no physical speaker timing asserted\n";
}
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    if(app.arguments().contains("--backend")){backend(app);return 0;}
    completion();controls();oracle();queueAndWire();bounds();
    std::cout<<"Output-consumed timing passed: completion, pitch boundaries, controls, reset, release, partial writes, wire dispatch and bounded queue; fake backend clock only\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
