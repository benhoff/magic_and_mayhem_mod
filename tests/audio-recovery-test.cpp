#include "audio_session.hpp"
#include "audio-catalog-fixture.hpp"
#include <QCoreApplication>
#include <cstring>
#include <iostream>
#include <thread>
namespace r=mnm::reconstruction::audio;using namespace audioFixture;
class Writer final:public QIODevice {
public:
    Writer(){open(QIODevice::WriteOnly|QIODevice::Unbuffered);}
    QByteArray bytes;qint64 limit=100000;
    qint64 readData(char*,qint64) override{return -1;}
    qint64 writeData(const char* data,qint64 count) override{const auto size=std::min(count,limit);bytes.append(data,qsizetype(size));return size;}
};
class Output final:public AudioSessionOutput {
public:
    bool present=true,refuse=false,failDuringStart=false;unsigned selectedRate=48000,queries=0,starts=0;
    QString diagnostic;Writer writer;mnm::audio::Device* device=nullptr;std::unique_ptr<mnm::audio::PcmQueue> queue;
    std::uint32_t rate(QString& error) override{++queries;if(!present){error="Device disconnected";return 0;}return selectedRate;}
    bool start(mnm::audio::Device& d,QString& error) override{
        ++starts;if(refuse){error="Backend could not restart";return false;}
        diagnostic.clear();device=&d;queue=std::make_unique<mnm::audio::PcmQueue>(d);
        if(failDuringStart){fail("Failure during first pump");error=diagnostic;return false;}
        return true;
    }
    void stop() override{if(device)check(device->count()>0,"discard before native buffers destroyed");queue.reset();device=nullptr;}
    bool running() const override{return bool(queue);}
    QString error() const override{return diagnostic;}
    void fail(const QString& message){stop();diagnostic=message;if(failed)failed(message);}
    void changedDevices(){if(available)available();}
    void pump(unsigned bytes){check(queue && queue->pump(writer,bytes),"fake output pump");}
    int sample(){writer.bytes.clear();writer.limit=100000;pump(4);std::int16_t n=0;std::memcpy(&n,writer.bytes.constData(),2);return n;}
};
void flush(){for(int i=0;i<5;++i)QCoreApplication::processEvents();}
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    QTemporaryDir temporary;check(temporary.isValid(),"fixture root");const auto root=std::filesystem::path(temporary.path().toStdString());
    write(root/"Tone.wav",wave(4000));profile(root,"[Sounds]\n10='Tone' ; comment\n[Optimisation]\nMaxSimultaneousSounds=4\n");
    auto output=std::make_unique<Output>();auto* sink=output.get();AudioSession session(std::move(output));
    int failures=0,recoveries=0;session.changed=[&](AudioSessionState state,const QString& message){if(state==AudioSessionState::failed){++failures;check(!message.isEmpty(),"reported failure");}if(state==AudioSessionState::recovering)++recoveries;};
    check(session.setMasterVolume(-2000) && session.start(temporary.path(),7,r::NativeSourcePathPolicy::dequoteMissingLeaf),"configured gain and filename policy startup");
    check(session.play(10,true) && sink->sample()==400,"gain before failure");
    sink->writer.limit=1;sink->pump(16);check(sink->queue->pendingBytes()==15,"partial PCM before failure");
    sink->present=false;sink->fail("Device disconnected");sink->fail("Duplicate failure notification");
    check(!session.running() && session.outputRate()==48000 && !session.play(10),"callback immediately gates controls before deferred destruction");
    check(session.lastError()=="Device disconnected","first diagnostic retained");flush();
    check(failures==1 && session.state()==AudioSessionState::failed && session.canRecover() && !session.outputRate() && !sink->device,"failure coalesced and runtime reclaimed outside callback");
    check(session.setMasterVolume(-1000),"accepted gain changes while offline");
    check(!session.recover() && failures==2 && recoveries==1,"absent device retry reported");const auto attempts=sink->queries;flush();check(sink->queries==attempts,"no timer retry loop while absent");
    sink->present=true;sink->selectedRate=44100;sink->changedDevices();sink->changedDevices();sink->changedDevices();flush();
    check(session.running() && session.outputRate()==44100 && sink->queries==attempts+1 && recoveries==2,"availability coalesced and replacement clock negotiated");
    check(sink->device->primaryState().volume== -1000 && sink->sample()==0,"accepted gain restored and stale loop/queue absent");
    check(session.preflight().policy==r::NativeSourcePathPolicy::dequoteMissingLeaf && session.play(10) && sink->sample()==1265,"new cue after recovery uses retained policy/gain");
    // A device-change event delivered in the same turn as failure must not be lost.
    sink->fail("Default output switched");sink->changedDevices();flush();check(session.running() && sink->sample()==0,"failure/availability in one event turn recovers silently");
    sink->fail("Backend fault");flush();sink->refuse=true;check(!session.recover() && session.lastError()=="Backend could not restart" && session.canRecover(),"failed reopen leaves explicit retry available");
    sink->refuse=false;check(session.recover() && sink->sample()==0,"manual reopen succeeds without reusing old cues");
    sink->failDuringStart=true;sink->fail("Backend stopped");flush();check(!session.recover() && session.lastError()=="Failure during first pump","synchronous first-pump failure unwinds safely");const auto beforeFlush=failures;flush();check(failures==beforeFlush,"stale queued startup fault ignored");
    sink->failDuringStart=false;check(session.recover(),"recovery after first-pump fault");
    bool wrongThread=false;std::thread worker([&]{try{session.recover();}catch(const std::logic_error&){wrongThread=true;}});worker.join();check(wrongThread,"recovery thread confinement");
    sink->fail("Fault just before close");sink->changedDevices();session.stop();const auto beforeClose=sink->starts;flush();sink->changedDevices();flush();
    check(session.state()==AudioSessionState::stopped && !session.canRecover() && sink->starts==beforeClose,"close invalidates pending fault/recovery and future availability");
    sink->present=false;check(!session.start(temporary.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf) && session.canRecover(),"initial missing device retains validated request");
    sink->present=true;sink->changedDevices();flush();check(session.running() && sink->sample()==0,"device arriving after startup failure restores silent output");
    // Client cancellation from the recovering notification must win.
    sink->fail("Fault before cancelled retry");flush();session.changed=[&](AudioSessionState state,const QString&){if(state==AudioSessionState::recovering)session.stop();};
    check(!session.recover() && !session.canRecover() && !session.running(),"reentrant close cancels recovery");
    profile(root,"[Sounds]\n10=Tone\n[Optimisation]\nMaxSimultaneousSounds=1\n");check(!session.start(temporary.path(),1,r::NativeSourcePathPolicy::literal) && !session.canRecover(),"invalid catalog is not a device-recovery request");
    std::cout<<"Audio recovery passed; deferred/coalesced failures, changing clocks, gain/policy retention, stale PCM discard, retries and close cancellation; no physical device\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
