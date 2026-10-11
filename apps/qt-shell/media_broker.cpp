#include "media_broker.hpp"
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QtEndian>
#include <cstring>

MediaBroker::MediaBroker(GlViewport& viewport,QString root,bool audible):QObject(&viewport),viewport_(viewport),root_(QFileInfo(root).canonicalFilePath()),audible_(audible){
    viewport_.installEventFilter(this);timer_.setInterval(MNM_MEDIA_V1_HEARTBEAT_MS);connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
MediaBroker::~MediaBroker(){stop();viewport_.removeEventFilter(this);if(map_)file_.unmap(map_);}
bool MediaBroker::create(const QString& path){
    if(root_.isEmpty() || file_.isOpen())return false;
    file_.setFileName(path);if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(Size))return false;
    QByteArray bytes(Size,0);std::memcpy(bytes.data(),MNM_MEDIA_V1_MAGIC,MNM_MEDIA_V1_MAGIC_SIZE);qToLittleEndian<quint32>(MNM_MEDIA_V1_VERSION,bytes.data()+MNM_MEDIA_V1_VERSION_OFFSET);qToLittleEndian<quint32>(MNM_MEDIA_V1_DECLARED_SIZE,bytes.data()+MNM_MEDIA_V1_DECLARED_SIZE_OFFSET);
    if(file_.write(bytes)!=Size || !file_.flush())return false;
    map_=file_.map(0,Size);if(!map_)return false;timer_.start();poll();return true;
}
void MediaBroker::respond(unsigned id,unsigned status){
    if(!map_)return;
    auto* words=reinterpret_cast<quint32*>(map_);
    const auto seq=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED));
    __atomic_store_n(words+MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET/4,qToLittleEndian(seq+1),__ATOMIC_SEQ_CST);
    if(status==MNM_MEDIA_V1_STATUS_LOADING)__atomic_store_n(words+MNM_MEDIA_V1_ACCEPTED_ID_OFFSET/4,0u,__ATOMIC_RELAXED);
    if(status==MNM_MEDIA_V1_STATUS_ACCEPTED)__atomic_store_n(words+MNM_MEDIA_V1_ACCEPTED_ID_OFFSET/4,qToLittleEndian(id),__ATOMIC_RELAXED);
    __atomic_store_n(words+MNM_MEDIA_V1_RESPONSE_ID_OFFSET/4,qToLittleEndian(id),__ATOMIC_RELAXED);__atomic_store_n(words+MNM_MEDIA_V1_RESPONSE_STATUS_OFFSET/4,qToLittleEndian(status),__ATOMIC_RELAXED);
    __atomic_store_n(words+MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET/4,qToLittleEndian(seq+2),__ATOMIC_RELEASE);
}
QString MediaBroker::asset(const QByteArray& path,unsigned operation) const{
    for(auto c:path)if(uchar(c)<32 || uchar(c)>126)return {};
    auto parts=QString::fromLatin1(path).replace('\\','/').split('/',Qt::SkipEmptyParts);
    // Original startup passes .\fmv/intro*.avi. Admit that single harmless
    // current-directory prefix; later dot/traversal components still refuse.
    if(!parts.isEmpty() && parts.first()==".")parts.removeFirst();
    if(parts.size()<2 || parts.contains("..") || parts.contains("."))return {};
    const auto directory=operation==MNM_MEDIA_V1_OPERATION_MOVIE?QStringLiteral("FMV"):QStringLiteral("Sounds");
    const auto extension=operation==MNM_MEDIA_V1_OPERATION_MOVIE?QStringLiteral("avi"):QStringLiteral("wav");
    if(parts[parts.size()-2].compare(directory,Qt::CaseInsensitive) || QFileInfo(parts.last()).suffix().compare(extension,Qt::CaseInsensitive))return {};
    const QDir assets(QDir(root_).filePath(directory));
    const auto matches=assets.entryList(QDir::Files);
    QString name;
    for(const auto& candidate:matches)if(candidate.compare(parts.last(),Qt::CaseInsensitive)==0){if(!name.isEmpty())return {};name=candidate;}
    if(name.isEmpty())return {};
    const QFileInfo requested(assets.filePath(name));
    const auto canonical=requested.canonicalFilePath();
    if(!requested.isFile() || canonical.isEmpty() || !canonical.startsWith(root_+"/"+directory+"/"))return {};
    return canonical;
}
void MediaBroker::record(unsigned id,unsigned operation,unsigned status,const QString& path,QJsonObject report){
    report.insert("request",int(id));report.insert("operation",int(operation));report.insert("status",int(status));report.insert("asset",path);
    if(history_.size()<128)history_.append(report);
    if(recorded)recorded();
}
void MediaBroker::poll(){
    if(!map_)return;
    auto* words=reinterpret_cast<quint32*>(map_);
    const auto heartbeat=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_HEARTBEAT_OFFSET/4,__ATOMIC_RELAXED));
    __atomic_store_n(words+MNM_MEDIA_V1_HEARTBEAT_OFFSET/4,qToLittleEndian(heartbeat+1),__ATOMIC_RELEASE);
    const auto soundCancelled=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_SOUND_CANCEL_GENERATION_OFFSET/4,__ATOMIC_ACQUIRE));
    if(soundCancelled!=soundCancelled_){soundCancelled_=soundCancelled;if(sound_ && sound_->active())sound_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);}
    const auto cancelled=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_CANCELLED_REQUEST_ID_OFFSET/4,__ATOMIC_ACQUIRE));
    if(cancelled && movieId_==cancelled && movieActive())movie_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);
    if(cancelled && soundId_==cancelled && sound_ && sound_->active())sound_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);
    const auto seq=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_REQUEST_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE));if(seq&1)return;
    const auto id=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_REQUEST_ID_OFFSET/4,__ATOMIC_RELAXED));
    if(!id || id==last_)return;
    const auto operation=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_OPERATION_OFFSET/4,__ATOMIC_RELAXED));
    const auto flags=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_FLAGS_OFFSET/4,__ATOMIC_RELAXED));
    const auto length=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_PATH_LENGTH_OFFSET/4,__ATOMIC_RELAXED));
    QByteArray path;
    if(length<=MNM_MEDIA_V1_MAX_PATH_LENGTH)path=QByteArray(reinterpret_cast<const char*>(map_+MNM_MEDIA_V1_PATH_OFFSET),int(length));
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(seq!=qFromLittleEndian(__atomic_load_n(words+MNM_MEDIA_V1_REQUEST_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE)))return;
    last_=id;
    if(length>MNM_MEDIA_V1_MAX_PATH_LENGTH || (operation==MNM_MEDIA_V1_OPERATION_STOP_FILE_SOUND && (length || flags))){respond(id,MNM_MEDIA_V1_STATUS_UNSUPPORTED);record(id,operation,MNM_MEDIA_V1_STATUS_UNSUPPORTED,{});return;}
    if(id==cancelled){respond(id,MNM_MEDIA_V1_STATUS_CANCELLED);record(id,operation,MNM_MEDIA_V1_STATUS_CANCELLED,{});return;}
    if(operation==MNM_MEDIA_V1_OPERATION_STOP_FILE_SOUND){if(sound_ && sound_->active())sound_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);respond(id,MNM_MEDIA_V1_STATUS_COMPLETE);record(id,MNM_MEDIA_V1_OPERATION_STOP_FILE_SOUND,MNM_MEDIA_V1_STATUS_COMPLETE,{});return;}
    if(operation!=MNM_MEDIA_V1_OPERATION_MOVIE && operation!=MNM_MEDIA_V1_OPERATION_FILE_SOUND){respond(id,MNM_MEDIA_V1_STATUS_UNSUPPORTED);record(id,operation,MNM_MEDIA_V1_STATUS_UNSUPPORTED,{});return;}
    if((operation==MNM_MEDIA_V1_OPERATION_MOVIE && flags) || (operation==MNM_MEDIA_V1_OPERATION_FILE_SOUND && (!(flags&MNM_MEDIA_V1_SOUND_FLAG_FILENAME) || (flags&~MNM_MEDIA_V1_SOUND_ALLOWED_FLAGS) || ((flags&MNM_MEDIA_V1_SOUND_FLAG_LOOP) && !(flags&MNM_MEDIA_V1_SOUND_FLAG_ASYNC))))){respond(id,MNM_MEDIA_V1_STATUS_UNSUPPORTED);record(id,operation,MNM_MEDIA_V1_STATUS_UNSUPPORTED,{});return;}
    if(operation==MNM_MEDIA_V1_OPERATION_FILE_SOUND && (flags&MNM_MEDIA_V1_SOUND_FLAG_NO_STOP) && sound_ && sound_->active()){respond(id,MNM_MEDIA_V1_STATUS_SOUND_BUSY);record(id,MNM_MEDIA_V1_OPERATION_FILE_SOUND,MNM_MEDIA_V1_STATUS_SOUND_BUSY,QString::fromLatin1(path));return;}
    const auto resolved=asset(path,operation);
    if(resolved.isEmpty()){respond(id,MNM_MEDIA_V1_STATUS_UNSUPPORTED);record(id,operation,MNM_MEDIA_V1_STATUS_UNSUPPORTED,QString::fromLatin1(path));return;}
    auto& player=operation==MNM_MEDIA_V1_OPERATION_MOVIE?movie_:sound_;
    if(player && player->active())player->stop(MNM_MEDIA_V1_STATUS_CANCELLED);
    player=std::make_unique<NativePlayback>(operation==MNM_MEDIA_V1_OPERATION_MOVIE,audible_);
    if(operation==MNM_MEDIA_V1_OPERATION_MOVIE){movieId_=id;if(movieChanged)movieChanged(true);}else soundId_=id;
    player->accepted=[this,id]{respond(id,MNM_MEDIA_V1_STATUS_ACCEPTED);};
    player->frame=[this,id,operation,resolved](QImage image){
        if(operation!=MNM_MEDIA_V1_OPERATION_MOVIE)return;
        if(qFromLittleEndian(__atomic_load_n(reinterpret_cast<quint32*>(map_)+MNM_MEDIA_V1_CANCELLED_REQUEST_ID_OFFSET/4,__ATOMIC_ACQUIRE))==id){movie_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);return;}
        if(frame)frame(std::move(image));
        if(skipFixture && QFileInfo(resolved).fileName()=="Skip.avi")skipMovie();
    };
    player->finished=[this,id,operation,resolved](unsigned result){
        auto* playback=operation==MNM_MEDIA_V1_OPERATION_MOVIE?movie_.get():sound_.get();
        if(last_==id)respond(id,result);
        if(operation==MNM_MEDIA_V1_OPERATION_MOVIE && movieChanged)movieChanged(false);
        record(id,operation,result,resolved,playback->report());
    };
    respond(id,MNM_MEDIA_V1_STATUS_LOADING);player->start(resolved,operation==MNM_MEDIA_V1_OPERATION_FILE_SOUND && (flags&MNM_MEDIA_V1_SOUND_FLAG_LOOP));
}
void MediaBroker::skipMovie(){if(movieActive())movie_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);}
void MediaBroker::stop(){timer_.stop();if(movie_ && movie_->active())movie_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);if(sound_ && sound_->active())sound_->stop(MNM_MEDIA_V1_STATUS_CANCELLED);}
bool MediaBroker::eventFilter(QObject* object,QEvent* event){
    if(object!=&viewport_ || !movieActive())return false;
    if(event->type()==QEvent::ShortcutOverride){event->accept();return true;}
    if(event->type()==QEvent::KeyPress){if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape)skipMovie();event->accept();return true;}
    if(event->type()==QEvent::KeyRelease || event->type()==QEvent::MouseButtonDblClick || event->type()==QEvent::MouseMove || event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseButtonRelease || event->type()==QEvent::Wheel){event->accept();return true;}
    return false;
}
