#include "media_broker.hpp"
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QtEndian>
#include <cstring>

MediaBroker::MediaBroker(GlViewport& viewport,QString root,bool audible):QObject(&viewport),viewport_(viewport),root_(QFileInfo(root).canonicalFilePath()),audible_(audible){
    viewport_.installEventFilter(this);timer_.setInterval(25);connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
MediaBroker::~MediaBroker(){stop();viewport_.removeEventFilter(this);if(map_)file_.unmap(map_);}
bool MediaBroker::create(const QString& path){
    if(root_.isEmpty() || file_.isOpen())return false;
    file_.setFileName(path);if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(Size))return false;
    QByteArray bytes(Size,0);std::memcpy(bytes.data(),"MNMMED01",8);qToLittleEndian<quint32>(1,bytes.data()+8);qToLittleEndian<quint32>(Size,bytes.data()+12);
    if(file_.write(bytes)!=Size || !file_.flush())return false;
    map_=file_.map(0,Size);if(!map_)return false;timer_.start();poll();return true;
}
void MediaBroker::respond(unsigned id,unsigned status){
    if(!map_)return;
    auto* words=reinterpret_cast<quint32*>(map_);
    const auto seq=qFromLittleEndian(__atomic_load_n(words+132,__ATOMIC_RELAXED));
    __atomic_store_n(words+132,qToLittleEndian(seq+1),__ATOMIC_SEQ_CST);
    if(status==1)__atomic_store_n(words+131,0u,__ATOMIC_RELAXED);
    if(status==2)__atomic_store_n(words+131,qToLittleEndian(id),__ATOMIC_RELAXED);
    __atomic_store_n(words+128,qToLittleEndian(id),__ATOMIC_RELAXED);__atomic_store_n(words+129,qToLittleEndian(status),__ATOMIC_RELAXED);
    __atomic_store_n(words+132,qToLittleEndian(seq+2),__ATOMIC_RELEASE);
}
QString MediaBroker::asset(const QByteArray& path,unsigned operation) const{
    for(auto c:path)if(uchar(c)<32 || uchar(c)>126)return {};
    auto parts=QString::fromLatin1(path).replace('\\','/').split('/',Qt::SkipEmptyParts);
    if(parts.size()<2 || parts.contains("..") || parts.contains("."))return {};
    const auto directory=operation==1?QStringLiteral("FMV"):QStringLiteral("Sounds");
    const auto extension=operation==1?QStringLiteral("avi"):QStringLiteral("wav");
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
    const auto heartbeat=qFromLittleEndian(__atomic_load_n(words+130,__ATOMIC_RELAXED));
    __atomic_store_n(words+130,qToLittleEndian(heartbeat+1),__ATOMIC_RELEASE);
    const auto soundCancelled=qFromLittleEndian(__atomic_load_n(words+11,__ATOMIC_ACQUIRE));
    if(soundCancelled!=soundCancelled_){soundCancelled_=soundCancelled;if(sound_ && sound_->active())sound_->stop(4);}
    const auto cancelled=qFromLittleEndian(__atomic_load_n(words+12,__ATOMIC_ACQUIRE));
    if(cancelled && movieId_==cancelled && movieActive())movie_->stop(4);
    if(cancelled && soundId_==cancelled && sound_ && sound_->active())sound_->stop(4);
    const auto seq=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE));if(seq&1)return;
    const auto id=qFromLittleEndian(__atomic_load_n(words+5,__ATOMIC_RELAXED));
    if(!id || id==last_)return;
    const auto operation=qFromLittleEndian(__atomic_load_n(words+6,__ATOMIC_RELAXED));
    const auto flags=qFromLittleEndian(__atomic_load_n(words+7,__ATOMIC_RELAXED));
    const auto length=qFromLittleEndian(__atomic_load_n(words+8,__ATOMIC_RELAXED));
    QByteArray path;
    if(length<=259)path=QByteArray(reinterpret_cast<const char*>(map_+64),int(length));
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(seq!=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE)))return;
    last_=id;
    if(length>259 || (operation==3 && (length || flags))){respond(id,6);record(id,operation,6,{});return;}
    if(id==cancelled){respond(id,4);record(id,operation,4,{});return;}
    if(operation==3){if(sound_ && sound_->active())sound_->stop(4);respond(id,3);record(id,3,3,{});return;}
    if(operation!=1 && operation!=2){respond(id,6);record(id,operation,6,{});return;}
    if((operation==1 && flags) || (operation==2 && (!(flags&0x20000) || (flags&~0x2001b) || ((flags&8) && !(flags&1))))){respond(id,6);record(id,operation,6,{});return;}
    if(operation==2 && (flags&16) && sound_ && sound_->active()){respond(id,7);record(id,2,7,QString::fromLatin1(path));return;}
    const auto resolved=asset(path,operation);
    if(resolved.isEmpty()){respond(id,6);record(id,operation,6,QString::fromLatin1(path));return;}
    auto& player=operation==1?movie_:sound_;
    if(player && player->active())player->stop(4);
    player=std::make_unique<NativePlayback>(operation==1,audible_);
    if(operation==1){movieId_=id;if(movieChanged)movieChanged(true);}else soundId_=id;
    player->accepted=[this,id]{respond(id,2);};
    player->frame=[this,id,operation,resolved](QImage image){
        if(operation!=1)return;
        if(qFromLittleEndian(__atomic_load_n(reinterpret_cast<quint32*>(map_)+12,__ATOMIC_ACQUIRE))==id){movie_->stop(4);return;}
        if(frame)frame(std::move(image));
        if(skipFixture && QFileInfo(resolved).fileName()=="Skip.avi")skipMovie();
    };
    player->finished=[this,id,operation,resolved](unsigned result){
        auto* playback=operation==1?movie_.get():sound_.get();
        if(last_==id)respond(id,result);
        if(operation==1 && movieChanged)movieChanged(false);
        record(id,operation,result,resolved,playback->report());
    };
    respond(id,1);player->start(resolved,operation==2 && (flags&8));
}
void MediaBroker::skipMovie(){if(movieActive())movie_->stop(4);}
void MediaBroker::stop(){timer_.stop();if(movie_ && movie_->active())movie_->stop(4);if(sound_ && sound_->active())sound_->stop(4);}
bool MediaBroker::eventFilter(QObject* object,QEvent* event){
    if(object!=&viewport_ || !movieActive())return false;
    if(event->type()==QEvent::ShortcutOverride){event->accept();return true;}
    if(event->type()==QEvent::KeyPress){if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape)skipMovie();event->accept();return true;}
    if(event->type()==QEvent::KeyRelease || event->type()==QEvent::MouseButtonDblClick || event->type()==QEvent::MouseMove || event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseButtonRelease || event->type()==QEvent::Wheel){event->accept();return true;}
    return false;
}
