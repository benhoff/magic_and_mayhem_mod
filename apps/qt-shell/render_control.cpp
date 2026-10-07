#include "render_control.hpp"
#include <QtEndian>
#include <cstring>
quint32 RenderControl::load(unsigned offset) const{return qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(map_+offset),__ATOMIC_ACQUIRE));}
void RenderControl::store(unsigned offset,quint32 value){__atomic_store_n(reinterpret_cast<quint32*>(map_+offset),qToLittleEndian(value),__ATOMIC_RELEASE);}
bool RenderControl::identity() const{
    return map_ && file_.size()==MNM_RENDER_CONTROL_V1_SIZE && std::memcmp(map_,MNM_RENDER_CONTROL_V1_MAGIC,8)==0 &&
        load(8)==1 && load(12)==MNM_RENDER_CONTROL_V1_SIZE && load(16)==launch_ && !load(52) && !load(56) && !load(60);
}
RenderControl::~RenderControl(){cancel();if(map_)file_.unmap(map_);}
bool RenderControl::create(const QString& path,quint32 launch){
    file_.setFileName(path);launch_=launch;
    if(!launch || !file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(MNM_RENDER_CONTROL_V1_SIZE)){
        error_="Cannot create rendering control channel";return false;
    }
    map_=file_.map(0,MNM_RENDER_CONTROL_V1_SIZE);if(!map_){error_="Cannot map rendering control channel";return false;}
    std::memset(map_,0,MNM_RENDER_CONTROL_V1_SIZE);std::memcpy(map_,MNM_RENDER_CONTROL_V1_MAGIC,8);store(8,1);store(12,MNM_RENDER_CONTROL_V1_SIZE);store(16,launch);return true;
}
bool RenderControl::stop(){
    if(stopping_)return true;
    if(!identity() || cancelled_ || pending_){error_="Rendering stop request unavailable";cancel();return false;}
    std::memset(map_+64,0,512);store(24,MNM_RENDER_CONTROL_V1_OPERATION_STOP);store(28,0);store(44,0);
    stopping_=true;pending_=true;deadline_.start();store(20,++sequence_);return true;
}
bool RenderControl::recover(const QString& path,quint32 session){return request(path,session,MNM_RENDER_CONTROL_V1_OPERATION_RECOVER);}
bool RenderControl::checkpoint(const QString& path,quint32 session){return request(path,session,MNM_RENDER_CONTROL_V1_OPERATION_CHECKPOINT);}
bool RenderControl::request(const QString& path,quint32 session,quint32 operation){
    const auto native=QString("Z:")+QString(path).replace('/','\\');const auto bytes=native.toLatin1();
    if(!identity() || cancelled_ || stopping_ || pending_ || !session || sequence_>=MNM_RENDER_CONTROL_V1_MAX_RECOVERIES ||
       bytes.isEmpty() || bytes.size()>MNM_RENDER_CONTROL_V1_MAX_PATH_LENGTH || QString::fromLatin1(bytes)!=native || bytes.contains('\0')){
        error_="Invalid rendering recovery request";cancel();return false;
    }
    std::memset(map_+64,0,512);std::memcpy(map_+64,bytes.constData(),bytes.size());
    store(24,operation);store(28,session);store(44,bytes.size());
    pending_=true;deadline_.start();store(20,++sequence_);return true;
}
int RenderControl::poll(){
    if(!pending_)return cancelled_?-1:0;
    if(!identity() || load(40) || load(48)>MNM_RENDER_CONTROL_V1_ONLINE_STOPPED || load(32)>sequence_ || load(36)>MNM_RENDER_CONTROL_V1_STATUS_REFUSED){
        error_="Invalid rendering recovery response";cancel();return -1;
    }
    if(deadline_.elapsed()>=MNM_RENDER_CONTROL_V1_RESPONSE_TIMEOUT_MS){
        error_="Rendering recovery response timed out";cancel();return -1;
    }
    if(load(32)==sequence_){
        pending_=false;
        if(load(36)==MNM_RENDER_CONTROL_V1_STATUS_READY)return 1;
        error_="Producer refused rendering recovery";cancel();return -1;
    }
    if(load(48)==MNM_RENDER_CONTROL_V1_ONLINE_STOPPED){
        error_="Rendering recovery timed out or producer stopped";cancel();return -1;
    }
    return 0;
}
void RenderControl::cancel(){cancelled_=true;pending_=false;if(map_)store(40,1);}
