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
    qint64 writeData(const char* p,qint64 n) override{const auto size=std::min(n,limit);bytes.append(p,qsizetype(size));return size;}
};
class TestOutput final:public AudioSessionOutput {
public:
    std::uint32_t selectedRate=48000;bool refuse=false;Writer writer;
    mnm::audio::Device* device=nullptr;std::unique_ptr<mnm::audio::PcmQueue> queue;unsigned stops=0;
    std::uint32_t rate(QString&) override{return selectedRate;}
    bool start(mnm::audio::Device& d,QString& error) override{
        if(refuse){error="Synthetic output failure";return false;}
        device=&d;queue=std::make_unique<mnm::audio::PcmQueue>(d);return true;
    }
    void stop() override{if(device)check(device->count()>0,"output stops before buffers destroyed");queue.reset();device=nullptr;++stops;}
    bool running() const override{return bool(queue);}
    QString error() const override{return {};}
    void pump(unsigned bytes){check(queue && queue->pump(writer,bytes),"fake sink PCM write");}
};
std::int16_t first(const QByteArray& bytes){check(bytes.size()>=2,"PCM captured");std::int16_t n;std::memcpy(&n,bytes.constData(),2);return n;}
int main(int argc,char** argv){QCoreApplication app(argc,argv);try{
    QTemporaryDir dir;check(dir.isValid(),"temp fixture");const auto root=std::filesystem::path(dir.path().toStdString());
    write(root/"Tone.wav",wave(4000));
    const std::string base="[Sounds]\n10='Tone' ; comment\n20=Stream\n90=Logical\n[Randomised]\n90=10\n[Optimisation]\nMaxSimultaneousSounds=2\n";
    profile(root,base+"[7 Load Permanent]\n10\n");
    auto output=std::make_unique<TestOutput>();auto* sink=output.get();AudioSession session(std::move(output));
    check(session.start(dir.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf) && session.running(),"manager connected to queue sink");
    check(session.play(90,true) && session.play(10,true),"group and overlapping direct voice with stable slots");
    check(!session.play(10,true) && !session.lastError().isEmpty(),"full scheduler returns no publication rather than false success");
    sink->pump(16);check(first(sink->writer.bytes)==8000,"reconstructed admission through output queue exact overlapping PCM");
    check(!session.play(20) && !session.play(999) && !session.play(10,false,1),"preflight and native control gates");
    sink->writer.limit=1;sink->pump(16);check(sink->queue->pendingBytes()==15,"partial pending PCM before stop");
    session.stop();session.stop();check(!session.running() && !sink->device && !session.outputRate(),"idempotent stop discards queue before device teardown");
    check(!session.play(10),"stopped session rejects controls");
    check(!session.start(dir.path(),7,r::NativeSourcePathPolicy::literal) && !session.running() && !sink->device,"failed permanent preload unwinds complete scheduler and source ring");
    check(session.start(dir.path(),7,r::NativeSourcePathPolicy::dequoteMissingLeaf),"map permanent preload with shared filename policy");
    check(session.start(dir.path(),1,r::NativeSourcePathPolicy::literal) && !session.preflight().playable(10) && !session.play(90),"restart changes policy; no stale playable catalog");
    sink->writer.bytes.clear();sink->writer.limit=100000;sink->pump(16);check(first(sink->writer.bytes)==0,"restart leaves no old queued audio or voices");
    sink->selectedRate=44100;check(session.start(dir.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf) && session.outputRate()==44100,"negotiated sink clock precedes device construction");
    check(session.play(10),"one-shot after rate/map restart");sink->pump(1024);sink->writer.bytes.clear();sink->pump(16);check(first(sink->writer.bytes)==0,"one-shot completion through output queue");
    sink->refuse=true;check(!session.start(dir.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf) && !session.running() && !sink->device && session.lastError()=="Synthetic output failure","sink failure safely releases initialized manager");
    sink->refuse=false;check(session.start(dir.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf),"recovery after sink failure");
    bool threadRejected=false;std::thread other([&]{try{session.play(10);}catch(const std::logic_error&){threadRejected=true;}});other.join();check(threadRejected,"cross-thread controls rejected before manager access");
    profile(root,"[Sounds]\n10=Tone\n[Optimisation]\nMaxSimultaneousSounds=1\n");check(!session.start(dir.path(),1,r::NativeSourcePathPolicy::literal) && !session.running(),"invalid startup unwinds prior output and manager");
    profile(root,base);AudioSession unavailable(makeQtSessionOutput(QAudioDevice()));check(!unavailable.start(dir.path(),1,r::NativeSourcePathPolicy::dequoteMissingLeaf) && !unavailable.running() && !unavailable.lastError().isEmpty(),"real Qt output null-device path fails without physical device");
    std::cout<<"Qt audio session passed; synthetic sink, exact PCM, pending discard, restart, errors and thread confinement; no physical device opened\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
