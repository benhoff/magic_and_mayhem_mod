#include "music_bridge.hpp"
#include "music_service.hpp"
#include <QFile>
#include <QTimer>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QtEndian>
#include <algorithm>
#include <exception>
#include <cstring>
namespace mnm::audio {
namespace {
quint32 load(const uchar* map,unsigned offset){return qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(map+offset),__ATOMIC_ACQUIRE));}
void put(uchar* map,unsigned offset,quint32 value){__atomic_store_n(reinterpret_cast<quint32*>(map+offset),qToLittleEndian(value),__ATOMIC_RELEASE);}
unsigned phase(MusicPhase value){
    switch(value){case MusicPhase::closed:return MNM_MUSIC_V1_PHASE_CLOSED;case MusicPhase::stopped:return MNM_MUSIC_V1_PHASE_STOPPED;
    case MusicPhase::loading:return MNM_MUSIC_V1_PHASE_LOADING;case MusicPhase::playing:return MNM_MUSIC_V1_PHASE_PLAYING;
    case MusicPhase::paused:return MNM_MUSIC_V1_PHASE_PAUSED;case MusicPhase::completed:return MNM_MUSIC_V1_PHASE_COMPLETED;
    case MusicPhase::unavailable:return MNM_MUSIC_V1_PHASE_UNAVAILABLE;case MusicPhase::failed:return MNM_MUSIC_V1_PHASE_FAILED;}
    return MNM_MUSIC_V1_PHASE_FAILED;
}
}
struct MusicBroker::State {
    QFile file;uchar* map=nullptr;QTimer timer;MusicService service;QString error;
    quint32 last=0,status=0,value=0,plays=0,requests=0,failures=0,playRequest=0,eventId=0,eventKind=0,eventSequence=0;
    quint64 eventGeneration=0;bool audible,dispatching=false;
    explicit State(bool output):service(makeQtMusicOutput(output)),audible(output){
        timer.setInterval(MNM_MUSIC_V1_HEARTBEAT_MS);QObject::connect(&timer,&QTimer::timeout,&service,[this]{poll();});
        service.changed=[this](const auto& state){
            if(playRequest&&(state.phase==MusicPhase::completed||state.phase==MusicPhase::failed)&&eventGeneration!=state.generation){
                eventGeneration=state.generation;eventId=playRequest;eventKind=state.phase==MusicPhase::completed?MNM_MUSIC_V1_EVENT_COMPLETE:MNM_MUSIC_V1_EVENT_FAILURE;eventSequence+=2;
            }
            publish();
        };
    }
    ~State(){stop();if(map)file.unmap(map);}
    bool create(const QString& channel,const QString& root){
        if(audible&&QMediaDevices::defaultAudioOutput().isNull()){error="No native music output device is available";return false;}
        if(!service.initialize(root.toStdString())){error=service.snapshot().error;return false;}
        if(service.catalog()->tracks().size()>MNM_MUSIC_V1_MAX_TRACKS){error="Native music catalog exceeds wire track count";return false;}
        file.setFileName(channel);if(!file.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file.resize(MNM_MUSIC_V1_SIZE)){error=file.errorString();return false;}
        QByteArray initial(MNM_MUSIC_V1_SIZE,0);std::memcpy(initial.data(),MNM_MUSIC_V1_MAGIC,8);
        qToLittleEndian<quint32>(MNM_MUSIC_V1_VERSION,initial.data()+8);qToLittleEndian<quint32>(MNM_MUSIC_V1_SIZE,initial.data()+12);
        if(file.write(initial)!=initial.size()||!file.flush()){error=file.errorString();return false;}
        map=file.map(0,MNM_MUSIC_V1_SIZE);if(!map){error=file.errorString();return false;}
        const auto& tracks=service.catalog()->tracks();
        for(unsigned i=0;i<tracks.size();++i){put(map,MNM_MUSIC_V1_TOC_OFFSET+i*8,tracks[i].startSector);put(map,MNM_MUSIC_V1_TOC_OFFSET+i*8+4,tracks[i].endSector);}
        put(map,MNM_MUSIC_V1_TRACK_COUNT_OFFSET,unsigned(tracks.size()));put(map,MNM_MUSIC_V1_LEADOUT_SECTOR_OFFSET,tracks.back().endSector);
        put(map,MNM_MUSIC_V1_READY_OFFSET,1);poll();timer.start();return true;
    }
    void stop(){timer.stop();if(map)put(map,MNM_MUSIC_V1_READY_OFFSET,0);service.changed={};service.close();}
    void publish(){
        if(!map||dispatching)return;
        const auto snapshot=service.snapshot();const auto report=service.outputReport();
        const auto sequence=load(map,MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET);put(map,MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET,sequence+1);
        put(map,MNM_MUSIC_V1_RESPONSE_ID_OFFSET,last);put(map,MNM_MUSIC_V1_RESPONSE_STATUS_OFFSET,status);put(map,MNM_MUSIC_V1_RESPONSE_VALUE_OFFSET,value);
        put(map,MNM_MUSIC_V1_PHASE_OFFSET,phase(snapshot.phase));put(map,MNM_MUSIC_V1_CURRENT_TRACK_OFFSET,snapshot.track);
        put(map,MNM_MUSIC_V1_POSITION_CD_FRAMES_OFFSET,quint32(std::max<qint64>(0,snapshot.positionMs)*75/1000));put(map,MNM_MUSIC_V1_VOLUME_OFFSET,snapshot.volume);
        put(map,MNM_MUSIC_V1_FAILED_REQUESTS_OFFSET,failures);put(map,MNM_MUSIC_V1_RECOVERIES_OFFSET,quint32(snapshot.recoveries));put(map,MNM_MUSIC_V1_LOSSES_OFFSET,quint32(snapshot.losses));
        put(map,MNM_MUSIC_V1_REQUESTS_OFFSET,requests);put(map,MNM_MUSIC_V1_PLAYS_OFFSET,plays);
        const quint64 frames=quint64(report.value("audio_frames").toInteger());put(map,MNM_MUSIC_V1_DECODED_FRAMES_LOW_OFFSET,quint32(frames));put(map,MNM_MUSIC_V1_DECODED_FRAMES_HIGH_OFFSET,quint32(frames>>32));
        // Event publication is independent of command acknowledgements.
        put(map,MNM_MUSIC_V1_EVENT_SEQUENCE_OFFSET,eventSequence|1u);put(map,MNM_MUSIC_V1_EVENT_ID_OFFSET,eventId);put(map,MNM_MUSIC_V1_EVENT_KIND_OFFSET,eventKind);put(map,MNM_MUSIC_V1_EVENT_SEQUENCE_OFFSET,eventSequence);
        put(map,MNM_MUSIC_V1_RESPONSE_SEQUENCE_OFFSET,sequence+2);
    }
    void poll(){
        if(!map)return;
        if(std::memcmp(map,MNM_MUSIC_V1_MAGIC,8)||load(map,8)!=MNM_MUSIC_V1_VERSION||load(map,12)!=MNM_MUSIC_V1_SIZE||file.size()!=MNM_MUSIC_V1_SIZE){
            error="Native music channel identity changed";put(map,MNM_MUSIC_V1_READY_OFFSET,0);timer.stop();service.stop();return;
        }
        put(map,MNM_MUSIC_V1_HEARTBEAT_OFFSET,load(map,MNM_MUSIC_V1_HEARTBEAT_OFFSET)+1);
        if(playRequest&&load(map,MNM_MUSIC_V1_CANCELLED_REQUEST_ID_OFFSET)==playRequest){
            eventId=playRequest;eventKind=MNM_MUSIC_V1_EVENT_CANCELLED;eventSequence+=2;playRequest=0;service.stop();
        }
        const auto sequence=load(map,MNM_MUSIC_V1_REQUEST_SEQUENCE_OFFSET);if(sequence&1){publish();return;}
        const auto id=load(map,MNM_MUSIC_V1_REQUEST_ID_OFFSET);if(!id||id==last){publish();return;}
        const auto operation=load(map,MNM_MUSIC_V1_OPERATION_OFFSET),track=load(map,MNM_MUSIC_V1_TRACK_OFFSET),from=load(map,MNM_MUSIC_V1_FROM_FRAME_OFFSET),to=load(map,MNM_MUSIC_V1_TO_FRAME_OFFSET),flags=load(map,MNM_MUSIC_V1_FLAGS_OFFSET);
        if(sequence!=load(map,MNM_MUSIC_V1_REQUEST_SEQUENCE_OFFSET))return;
        last=id;++requests;status=MNM_MUSIC_V1_STATUS_INVALID;value=0;
        if(id==load(map,MNM_MUSIC_V1_CANCELLED_REQUEST_ID_OFFSET)){publish();return;}
        if(flags&~(MNM_MUSIC_V1_FLAG_LOOP|MNM_MUSIC_V1_FLAG_PAUSED)){++failures;publish();return;}
        bool accepted=false;dispatching=true;
        try{
        switch(operation){
        case MNM_MUSIC_V1_OPERATION_PING:case MNM_MUSIC_V1_OPERATION_OPEN:case MNM_MUSIC_V1_OPERATION_STATUS:
            accepted=!track&&!from&&!to&&!flags;value=1;break;
        case MNM_MUSIC_V1_OPERATION_CLOSE:case MNM_MUSIC_V1_OPERATION_STOP:
            accepted=!track&&!from&&!to&&!flags;if(accepted){playRequest=0;service.stop();}break;
        case MNM_MUSIC_V1_OPERATION_PLAY:{
            const auto* entry=service.catalog()->track(track);
            if(!entry||entry->file.empty()||to){accepted=false;break;}
            accepted=service.play(track,flags&MNM_MUSIC_V1_FLAG_LOOP,qint64(from)*1000/75);
            if(accepted){playRequest=id;++plays;if(flags&MNM_MUSIC_V1_FLAG_PAUSED)accepted=service.pause();}
            break;}
        case MNM_MUSIC_V1_OPERATION_PAUSE:accepted=!track&&!from&&!to&&!flags&&service.pause();break;
        case MNM_MUSIC_V1_OPERATION_RESUME:accepted=!track&&!from&&!to&&!flags&&service.resume();break;
        case MNM_MUSIC_V1_OPERATION_VOLUME:accepted=!track&&!to&&!flags&&from<=65535&&service.volume(quint16(from));break;
        case MNM_MUSIC_V1_OPERATION_SEEK:accepted=!track&&!to&&!flags&&service.seek(qint64(from)*1000/75);break;
        default:break;
        }
        }catch(const std::exception& problem){error=QString::fromUtf8(problem.what());service.stop();status=MNM_MUSIC_V1_STATUS_FAILED;}
        if(status!=MNM_MUSIC_V1_STATUS_FAILED)status=accepted?MNM_MUSIC_V1_STATUS_OK:MNM_MUSIC_V1_STATUS_INVALID;
        dispatching=false;if(!accepted)++failures;publish();
    }
    QJsonObject activity()const{
        const auto snapshot=service.snapshot();return {{"requests",int(requests)},{"plays",int(plays)},{"failed_requests",int(failures)},
            {"ready",map&&load(map,MNM_MUSIC_V1_READY_OFFSET)!=0},{"producer_features",map?int(load(map,MNM_MUSIC_V1_PRODUCER_FEATURES_OFFSET)):0},
            {"unsupported_cd_calls",map?int(load(map,MNM_MUSIC_V1_UNSUPPORTED_CD_CALLS_OFFSET)):0},{"legacy_other_mci_calls",map?int(load(map,MNM_MUSIC_V1_LEGACY_OTHER_MCI_CALLS_OFFSET)):0},
            {"recoveries",qint64(snapshot.recoveries)},{"losses",qint64(snapshot.losses)},{"error",error},{"output",service.outputReport()},{"audible_output_requested",audible}};
    }
};
MusicBroker::MusicBroker(QObject* parent):QObject(parent){}
MusicBroker::~MusicBroker(){stop();}
bool MusicBroker::create(const QString& channel,const QString& root,bool audible){
    if(worker_)return false;
    worker_=new QObject;worker_->moveToThread(&thread_);QObject::connect(&thread_,&QThread::finished,worker_,&QObject::deleteLater);thread_.start();
    bool accepted=false;QMetaObject::invokeMethod(worker_,[&]{
        try{state_=std::make_unique<State>(audible);accepted=state_->create(channel,root);if(!accepted)error_=state_->error;}
        catch(const std::exception& error){error_=QString::fromUtf8(error.what());}
    },Qt::BlockingQueuedConnection);
    if(!accepted)stop();
    return accepted;
}
void MusicBroker::stop(){
    if(!worker_)return;
    QMetaObject::invokeMethod(worker_,[&]{if(state_){lastActivity_=state_->activity();state_->stop();lastActivity_["ready"]=false;state_.reset();}},Qt::BlockingQueuedConnection);thread_.quit();thread_.wait();worker_=nullptr;
}
QJsonObject MusicBroker::activity()const{if(!worker_)return lastActivity_;QJsonObject result;QMetaObject::invokeMethod(worker_,[&]{result=state_->activity();},Qt::BlockingQueuedConnection);return result;}
}
