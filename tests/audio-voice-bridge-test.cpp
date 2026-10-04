#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QThread>
#include <QtEndian>
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
    reply=run(MNM_AUDIO_DUPLICATE,id);check(!reply.status,"duplicate");const auto child=reply.value;
    check(!run(MNM_AUDIO_RELEASE,id).status && !run(MNM_AUDIO_PLAY,child).status,"duplicate survives source");
    std::vector<std::int16_t> output;check(d.mixStereo(5,output)==Error::ok,"mix wire voice");
    check(output[0]==-328 && output[1]==-3277 && output[8]==0 && run(MNM_AUDIO_STATUS,child).value==0,"wire PCM reaches mixer controls/completion");
    check(!run(MNM_AUDIO_RESET,child).status && run(MNM_AUDIO_RESET,child,1).status,"zero reset scope");
    check(!run(MNM_AUDIO_RELEASE,child).status && run(MNM_AUDIO_STATUS,child).status,"released ID rejected");
    check(run(999).status,"unknown operation rejected");
    w[10]=65537;check(run(MNM_AUDIO_CREATE).status && !d.count(),"format narrowing rejected");
    QTemporaryDir directory;const auto path=directory.filePath("voices.bin");VoiceBroker broker;
    check(broker.create(path,false),"silent broker creation");QFile file(path);check(file.open(QIODevice::ReadWrite),"open channel");
    auto* map=file.map(0,MNM_AUDIO_SIZE);check(map,"map channel");auto* words=reinterpret_cast<quint32*>(map);
    check(qFromLittleEndian(words[20])==1,"broker ready");
    words[5]=qToLittleEndian(999u);__atomic_store_n(words+4,qToLittleEndian(1u),__ATOMIC_RELEASE);
    for(int i=0;i<10 && !words[16];++i){QCoreApplication::processEvents();QThread::msleep(2);}
    check(qFromLittleEndian(words[16])==1 && qFromLittleEndian(words[17])==0x80004001,"mapped command acknowledgement");
    broker.stop();check(!words[20],"stop withdraws readiness");broker.stop();file.unmap(map);file.close();
    check(!broker.create(path,false),"existing channel never overwritten");
    std::cout<<"Voice bridge commands and mapped lifecycle passed; no game or output device\n";
}
