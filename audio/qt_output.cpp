#include "qt_output.hpp"
#include <QThread>
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace mnm::audio {
QAudioFormat selectOutputFormat(int requested,int preferred,const std::function<bool(const QAudioFormat&)>& supported){
    std::vector<int> rates{requested,preferred,48000,44100,22050};
    for(std::size_t i=0;i<rates.size();++i){
        const int rate=rates[i];
        if(rate<=0 || std::find(rates.begin(),rates.begin()+i,rate)!=rates.begin()+i)continue;
        QAudioFormat format;format.setSampleRate(rate);format.setChannelCount(2);format.setSampleFormat(QAudioFormat::Int16);
        if(supported(format))return format;
    }
    return {};
}
QAudioFormat selectOutputFormat(const QAudioDevice& device,int requested){
    if(device.isNull())return {};
    return selectOutputFormat(requested,device.preferredFormat().sampleRate(),
                              [&device](const auto& f){return device.isFormatSupported(f);});
}
bool PcmQueue::pump(QIODevice& output,qint64 capacity){
    if(capacity<=0)return true;
    if(!output.isWritable())return false;
    // One bounded block per tick. Capacity is bytesFree(), not a requested duration.
    if(!pendingBytes()){
        const auto frames=std::size_t(std::min<qint64>(capacity/4,1024));
        if(!frames)return true;
        pending_.resize(qsizetype(frames*4));offset_=0;
        std::vector<std::int16_t> samples;
        if(device_.mixStereo(frames,samples)!=Error::ok){clear();return false;}
        // QAudioFormat Int16 uses native byte order, matching these host integers.
        std::memcpy(pending_.data(),samples.data(),std::size_t(pending_.size()));offset_=0;
    }
    const auto wanted=std::min<qint64>(capacity,pendingBytes());
    const auto written=output.write(pending_.constData()+offset_,wanted);
    if(written<0 || written>wanted)return false;
    offset_+=written;
    if(!pendingBytes())clear();
    return true;
}
QtOutput::QtOutput(Device& device,QObject* parent):QObject(parent),device_(device),queue_(device){
    timer_.setInterval(5);timer_.setTimerType(Qt::PreciseTimer);
    connect(&timer_,&QTimer::timeout,this,[this]{pump();});
}
QtOutput::~QtOutput(){stop();}
bool QtOutput::start(const QAudioDevice& device){
    if(QThread::currentThread()!=thread())throw std::logic_error("Audio output used from another thread");
    stop();error_.clear();
    if(device_.outputRate()>std::uint32_t(std::numeric_limits<int>::max())){
        error_="Mixer rate exceeds Qt's supported integer range";return false;
    }
    QAudioFormat format;format.setSampleRate(int(device_.outputRate()));format.setChannelCount(2);format.setSampleFormat(QAudioFormat::Int16);
    if(device.isNull() || !device.isFormatSupported(format)){
        error_="No output device supports the mixer's stereo Int16 clock";return false;
    }
    sink_=std::make_unique<QAudioSink>(device,format);
    sink_->setBufferSize(qsizetype(device_.outputRate()/20)*4); // Request ~50 ms; actual size is backend-defined.
    writer_=sink_->start();
    if(!writer_ || sink_->error()!=QtAudio::NoError){stop();error_="QAudioSink could not start";return false;}
    timer_.start();pump();return running();
}
void QtOutput::stop(){
    if(QThread::currentThread()!=thread())throw std::logic_error("Audio output used from another thread");
    timer_.stop();writer_=nullptr;
    // reset() discards queued audio; stop() may drain/block on some backends.
    if(sink_)sink_->reset();
    sink_.reset();queue_.clear();
}
void QtOutput::fail(const QString& message){stop();error_=message;if(failed)failed(message);}
void QtOutput::pump(){
    if(!sink_ || !writer_)return;
    // Old Qt reports starvation as an error in IdleState; newer Qt only uses
    // IdleState. Keep feeding in either case. StoppedState cannot accept data.
    if(sink_->state()==QtAudio::StoppedState){fail("QAudioSink stopped; backend error "+QString::number(int(sink_->error())));return;}
    try{if(!queue_.pump(*writer_,sink_->bytesFree()))fail("Audio stream write failed");}
    catch(const std::exception& e){fail(QString::fromUtf8(e.what()));}
}
}
