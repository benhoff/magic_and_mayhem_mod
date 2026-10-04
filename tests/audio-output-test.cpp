#include "qt_output.hpp"
#include <QCoreApplication>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
class Writer:public QIODevice {
public:
    Writer(){open(QIODevice::WriteOnly|QIODevice::Unbuffered);}
    qint64 limit=100000;bool broken=false;QByteArray bytes;
    qint64 readData(char*,qint64) override{return -1;}
    qint64 writeData(const char* p,qint64 n) override{
        if(broken)return -1;
        const auto count=std::min(n,limit);bytes.append(p,qsizetype(count));return count;
    }
};
BufferId fixture(Device& d){
    PcmFormat f{1,1,48000,96000,2,16,0};BufferId id=0;
    check(d.createStatic(0xea,f,8,id)==Error::ok,"create");WriteLock lock;
    check(d.lock(id,0,8,0,lock)==Error::ok,"lock");
    const std::int16_t samples[]{-32768,-1,1000,32767};
    for(int i=0;i<4;++i){const auto v=std::uint16_t(samples[i]);lock.first.data[2*i]=std::uint8_t(v);lock.first.data[2*i+1]=std::uint8_t(v>>8);}
    check(d.unlock(lock,8,0)==Error::ok && d.play(id,1)==Error::ok,"play");return id;
}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);
    int queries=0;
    auto f=selectOutputFormat(48000,96000,[&](const QAudioFormat& candidate){++queries;check(candidate.channelCount()==2 && candidate.sampleFormat()==QAudioFormat::Int16,"format policy");return candidate.sampleRate()==96000;});
    check(f.sampleRate()==96000 && queries==2,"preferred rate fallback");
    queries=0;f=selectOutputFormat(48000,48000,[&](const auto&){++queries;return false;});
    check(!f.isValid() && queries==3,"no unsupported coercion, duplicate candidates skipped");
    check(!selectOutputFormat(QAudioDevice()).isValid(),"null device negotiation");
    Device d,reference;const auto id=fixture(d);fixture(reference);PcmQueue queue(d);Writer writer;
    const auto initial=d.voice(id)->frame;
    check(queue.pump(writer,0) && queue.pump(writer,3) && d.voice(id)->frame==initial,"no advance without frame capacity");
    writer.limit=3;check(queue.pump(writer,16) && queue.pendingBytes()==13,"odd partial write retained");
    const auto cursor=d.voice(id)->frame;
    writer.limit=0;check(queue.pump(writer,100) && queue.pendingBytes()==13 && d.voice(id)->frame==cursor,"zero write preserves pending and cursor");
    writer.limit=2;
    while(queue.pendingBytes())check(queue.pump(writer,100),"partial writes");
    std::vector<std::int16_t> expected;check(reference.mixStereo(4,expected)==Error::ok,"reference");
    check(writer.bytes==QByteArray(reinterpret_cast<const char*>(expected.data()),qsizetype(expected.size()*2)),"byte exact PCM and no lost partial samples");
    writer.limit=100000;check(queue.pump(writer,1000000),"bounded write");
    check(writer.bytes.size()==16+4096,"bounded block size");
    writer.limit=0;check(queue.pump(writer,16) && queue.pendingBytes()==16,"pending before failure");
    writer.broken=true;check(!queue.pump(writer,16) && queue.pendingBytes()==16,"write failure retains pending");
    queue.clear();check(!queue.pendingBytes(),"stop discards queue");
    writer.close();check(!queue.pump(writer,16),"closed output failure");
    QtOutput output(d);output.stop();output.stop();
    check(!output.start(QAudioDevice()) && !output.running() && !output.lastError().isEmpty(),"null device start fails cleanly");
    check(!output.start(QAudioDevice()) && d.voice(id)->frame==cursor,"failed restart does not advance");
    std::cout<<"Qt PCM output adapter passed; simulated partial writes and lifecycle; no audio device opened\n";
}
