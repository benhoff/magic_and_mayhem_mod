#include "command_channel.hpp"
#include <QtEndian>
#include <cstring>
#include <stdexcept>
namespace {
quint32 load(const uchar* p,unsigned offset){return qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(p+offset),__ATOMIC_ACQUIRE));}
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
}
CommandChannel::~CommandChannel(){cancel();if(mapping_)file_.unmap(mapping_);}
void CommandChannel::validate() const {
    require(mapping_ && file_.size()==MNM_RENDER_COMMANDS_V1_SIZE,"Invalid command channel size");
    require(!std::memcmp(mapping_,MNM_RENDER_COMMANDS_V1_MAGIC,8) && qFromLittleEndian<quint32>(mapping_+8)==1 &&
        qFromLittleEndian<quint32>(mapping_+12)==MNM_RENDER_COMMANDS_V1_SIZE &&
        qFromLittleEndian<quint32>(mapping_+16)==session_ && session_,"Invalid command channel identity");
    for(unsigned i=36;i<64;++i)require(!mapping_[i],"Nonzero command channel reserved byte");
}
bool CommandChannel::map(){
    if(file_.size()!=MNM_RENDER_COMMANDS_V1_SIZE){error_="Invalid command channel size";return false;}
    mapping_=file_.map(0,MNM_RENDER_COMMANDS_V1_SIZE);
    if(!mapping_){error_=file_.errorString();return false;}
    session_=qFromLittleEndian<quint32>(mapping_+16);
    try{validate();return true;}catch(const std::exception& e){error_=e.what();file_.unmap(mapping_);mapping_=nullptr;return false;}
}
bool CommandChannel::create(const QString& path,quint32 session){
    if(file_.isOpen() || !session)return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(MNM_RENDER_COMMANDS_V1_SIZE)){error_=file_.errorString();return false;}
    QByteArray header(64,0);std::memcpy(header.data(),MNM_RENDER_COMMANDS_V1_MAGIC,8);
    qToLittleEndian<quint32>(1,header.data()+8);qToLittleEndian<quint32>(MNM_RENDER_COMMANDS_V1_SIZE,header.data()+12);qToLittleEndian(session,header.data()+16);
    if(file_.write(header)!=64 || !file_.flush()){error_=file_.errorString();return false;}return map();
}
bool CommandChannel::open(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite)){error_=file_.errorString();return false;}return map();
}
QByteArray CommandChannel::poll(quint32 budget){
    if(!error_.isEmpty())throw std::runtime_error(error_.toStdString());
    try{
        validate();require(budget && budget<=MNM_RENDER_COMMANDS_V1_POLL_BYTES,"Invalid command poll budget");
        const auto state=load(mapping_,24);const auto published=load(mapping_,20);const auto cancelled=load(mapping_,32);
        require(state<=MNM_RENDER_COMMANDS_V1_STATE_FAILED && cancelled<=1,"Invalid command channel state");
        require(published<=MNM_RENDER_COMMANDS_V1_CAPACITY && published>=published_,"Command channel publication regressed or overflowed");
        require(state>=state_ && (state_<2 || state==state_),"Command channel terminal state changed");
        require(state!=0 || published==0,"Unclaimed command channel has data");
        require(state_<2 || published==published_,"Command channel changed after terminal state");
        published_=published;state_=state;reason_=state>=2?load(mapping_,28):0;
        require(state!=3 && !cancelled,"Command channel failed or cancelled");
        require(state!=2 || reason_==0,"Ended command channel has failure reason");
        const auto count=qMin(budget,published-consumed_);
        QByteArray result(reinterpret_cast<const char*>(mapping_+64+consumed_),count);consumed_+=count;return result;
    }catch(const std::exception& e){error_=e.what();cancel();throw;}
}
void CommandChannel::cancel(){if(mapping_)__atomic_store_n(reinterpret_cast<quint32*>(mapping_+32),qToLittleEndian<quint32>(1),__ATOMIC_RELEASE);}
