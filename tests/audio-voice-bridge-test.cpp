#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QThread>
#include <QtEndian>
#include <atomic>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace mnm::audio;
void check(bool condition,const char* text){if(!condition)throw std::runtime_error(text);}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);Device d;VoiceCommands commands(d);std::uint32_t w[16]{};
    auto run=[&](unsigned op,unsigned id=0,unsigned value=0,QByteArray bytes={}){w[5]=op;w[6]=id;w[7]=value;return commands.execute(w,bytes);};
    w[8]=8;w[9]=48000;w[10]=1;w[11]=16;w[12]=2;w[13]=96000;w[14]=1;w[15]=0xea;
    auto reply=run(MNM_AUDIO_CREATE);check(!reply.status && reply.value,"create");const auto id=reply.value;
    QByteArray pcm=QByteArray::fromHex("0080ffffe803ff7f");
    check(!run(MNM_AUDIO_UPLOAD,id,0,pcm).status && d.samples(id)==std::vector<std::uint8_t>({0,128,255,255,232,3,255,127}),"wire payload commits exact PCM");
    w[8]=9;check(run(MNM_AUDIO_UPLOAD,id,0,pcm).status && d.samples(id)[0]==0,"truncated payload rejected atomically");w[8]=8;
    check(!run(MNM_AUDIO_VOLUME,id,std::uint32_t(-2000)).status && !run(MNM_AUDIO_PAN,id,2000).status,"signed controls");
    check(run(MNM_AUDIO_VOLUME,id,1).status && d.voice(id)->volume==-2000,"invalid controls retain state");
    check(!run(MNM_AUDIO_PLAY,id,1).status && run(MNM_AUDIO_STATUS,id).value==5,"loop status");
    check(!run(MNM_AUDIO_FREQUENCY,id,24000).status&&run(MNM_AUDIO_GET_FREQUENCY,id).value==24000,"wire pitch roundtrip");
    check(run(MNM_AUDIO_FREQUENCY,id,99).status&&run(MNM_AUDIO_GET_FREQUENCY,id).value==24000,"wire pitch refusal preserves clock");
    check(!run(MNM_AUDIO_SEEK,id,2).status&&run(MNM_AUDIO_CURSOR,id).value==2,"wire aligned seek/cursor");
    check(run(MNM_AUDIO_SEEK,id,1).status&&run(MNM_AUDIO_CURSOR,id).value==2,"wire unaligned seek preserves cursor");
    check(!run(MNM_AUDIO_RESET,id).status&&!run(MNM_AUDIO_FREQUENCY,id,0).status,"wire reset/source clock");
    reply=run(MNM_AUDIO_DUPLICATE,id);check(!reply.status,"duplicate");const auto child=reply.value;
    check(!run(MNM_AUDIO_RELEASE,id).status && !run(MNM_AUDIO_PLAY,child).status,"duplicate survives source");
    std::vector<std::int16_t> output;check(d.mixStereo(5,output)==Error::ok,"mix wire voice");
    check(output[0]==-328 && output[1]==-3277 && output[8]==0 && run(MNM_AUDIO_STATUS,child).value==0,"wire PCM reaches mixer controls/completion");
    check(!run(MNM_AUDIO_RESET,child).status && run(MNM_AUDIO_RESET,child,1).status,"zero reset scope");
    check(!run(MNM_AUDIO_RELEASE,child).status && run(MNM_AUDIO_STATUS,child).status,"released ID rejected");
    check(run(999).status,"unknown operation rejected");
    w[10]=65537;check(run(MNM_AUDIO_CREATE).status && !d.count(),"format narrowing rejected");
    QTemporaryDir directory;const auto path=directory.filePath("voices.bin");VoiceBroker broker;
    QFile wave(directory.filePath("fixture.wav"));check(wave.open(QIODevice::WriteOnly),"create native WAV fixture");
    const auto header=QByteArray::fromHex("524946462c00000057415645666d7420100000000100010080bb000000770100020010006461746108000000");
    check(wave.write(header+pcm)==header.size()+pcm.size(),"write exact source bytes");wave.close();
    QFile malformed(directory.filePath("bad.wav"));check(malformed.open(QIODevice::WriteOnly),"create bad input");malformed.write("RIFF");malformed.close();
    check(broker.create(path,false,directory.path()),"silent broker creation");QFile file(path);check(file.open(QIODevice::ReadWrite),"open channel");
    auto* map=file.map(0,MNM_AUDIO_SIZE);check(map,"map channel");auto* words=reinterpret_cast<quint32*>(map);
    check(qFromLittleEndian(words[20])==1,"broker ready");
    words[5]=qToLittleEndian(999u);__atomic_store_n(words+4,qToLittleEndian(1u),__ATOMIC_RELEASE);
    auto acknowledgement=[&]{return qFromLittleEndian(__atomic_load_n(words+16,__ATOMIC_ACQUIRE));};
    for(int i=0;i<10 && !acknowledgement();++i){QCoreApplication::processEvents();QThread::msleep(2);}
    check(acknowledgement()==1 && qFromLittleEndian(words[17])==0x80004001,"mapped command acknowledgement");
    unsigned serial=1;
    auto mapped=[&](unsigned op,unsigned voice=0){
        w[5]=op;w[6]=voice;w[7]=0;w[8]=8;w[10]=1;
        for(unsigned i=5;i<16;++i)words[i]=qToLittleEndian(w[i]);
        if(op==MNM_AUDIO_UPLOAD)for(int i=0;i<pcm.size();++i)map[128+i]=uchar(pcm[i]);
        __atomic_store_n(words+4,qToLittleEndian(++serial),__ATOMIC_RELEASE);
        for(int i=0;i<20&&acknowledgement()!=serial;++i){QCoreApplication::processEvents();QThread::msleep(2);}
        check(acknowledgement()==serial&&!words[17],"mapped activity request");
        return qFromLittleEndian(words[18]);
    };
    const auto mappedId=mapped(MNM_AUDIO_CREATE);mapped(MNM_AUDIO_UPLOAD,mappedId);mapped(MNM_AUDIO_PLAY,mappedId);mapped(MNM_AUDIO_RELEASE,mappedId);
    // The peer retains the PE32 adapter's one-second deadline while the owner
    // thread does no event processing for longer than that deadline.
    std::atomic<bool> responded{false};const auto stalledSerial=++serial;
    std::unique_ptr<QThread> peer(QThread::create([&]{
        words[5]=qToLittleEndian(unsigned(MNM_AUDIO_PING));
        __atomic_store_n(words+4,qToLittleEndian(stalledSerial),__ATOMIC_RELEASE);
        QElapsedTimer deadline;deadline.start();
        while(deadline.elapsed()<1000){
            if(acknowledgement()==stalledSerial){responded.store(!words[17]);return;}
            QThread::msleep(1);
        }
    }));
    peer->start();QThread::msleep(1200);peer->wait();
    check(responded.load(),"peer acknowledged during owner/GUI stall within original deadline");
    const auto activity=broker.activity();
    check(activity.requests==6&&activity.failures==1&&activity.creates==1&&activity.uploads==1&&activity.uploadedBytes==8&&activity.plays==1&&activity.releases==1&&activity.ready&&!activity.audible,"activity reflects successful operations separately from refusal");
    auto waveRequest=[&](const QByteArray& name,bool success){
        for(unsigned i=5;i<16;++i)words[i]=0;
        words[5]=qToLittleEndian(unsigned(MNM_AUDIO_LOAD_WAVE));words[8]=qToLittleEndian(unsigned(name.size()));
        std::memcpy(map+128,name.constData(),std::size_t(name.size()));
        __atomic_store_n(words+4,qToLittleEndian(++serial),__ATOMIC_RELEASE);
        QElapsedTimer deadline;deadline.start();
        while(acknowledgement()!=serial&&deadline.elapsed()<1000)QThread::msleep(1);
        check(acknowledgement()==serial,"native WAV response within original producer deadline");
        check((qFromLittleEndian(words[17])==0)==success,"native WAV outcome");
        if(success){
            check(qFromLittleEndian(words[18])==8&&qFromLittleEndian(words[24])==8&&qFromLittleEndian(words[25])==48000&&
                  qFromLittleEndian(words[26])==96000&&qFromLittleEndian(words[27])==0x10001&&
                  qFromLittleEndian(words[28])==0x100002&&qFromLittleEndian(words[29])==0&&words[30]==0,
                  "native WAV exact format/size/recovered duration metadata");
            check(QByteArray(reinterpret_cast<char*>(map+128),8)==pcm,"native WAV transfers exact PCM");
        }else check(words[18]==0&&words[24]==0,"failed wave does not publish old result metadata");
    };
    check(qFromLittleEndian(words[31])==(MNM_AUDIO_CAP_NATIVE_WAVE|MNM_AUDIO_CAP_EXTENDED_CONTROLS|MNM_AUDIO_CAP_OUTPUT_RECOVERY),"native capabilities published before ready");
    waveRequest("FIXTURE.wav",true);waveRequest("'fixture'.wav",true);
    waveRequest("missing.wav",false);waveRequest("../fixture.wav",false);waveRequest("bad.wav",false);
    waveRequest(QByteArray("fixture.wav\0",12),false);
    check(broker.activity().waveLoads==2&&broker.activity().waveBytes==16&&broker.activity().failures==5,
          "native wave successes and refusals counted independently");
    broker.stop();check(!words[20]&&!broker.activity().ready,"stop withdraws readiness");broker.stop();
    check(broker.create(directory.filePath("next.bin"),false),"worker restart with new channel");
    const auto retiredSerial=serial;__atomic_store_n(words+4,qToLittleEndian(retiredSerial+1),__ATOMIC_RELEASE);
    QThread::msleep(20);check(acknowledgement()==retiredSerial,"retired channel never serviced by restarted worker");
    check(broker.activity().requests==0&&broker.activity().ready,"new channel has fresh request namespace");broker.stop();
    file.unmap(map);file.close();
    check(!broker.create(path,false),"existing channel never overwritten");
    std::cout<<"Voice bridge commands and mapped lifecycle passed; no game or output device\n";
}
