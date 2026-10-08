#include "world_channel.hpp"
#include <cstring>
#include <stdexcept>
namespace mnm::legacy {
namespace { void fail(const char* s){throw std::runtime_error(s);} }
void WorldChannel::create(const QString& path,quint32 session,bool verify){
    if(!session)fail("World channel needs a nonzero session");
    QFile f(path);if(!f.open(QIODevice::NewOnly|QIODevice::ReadWrite)||!f.resize(MNM_WCH_SIZE))fail("Cannot create fresh bounded World channel");
    QByteArray header(MNM_WCH_HEADER,0);std::memcpy(header.data(),MNM_WCH_MAGIC,8);
    auto* w=reinterpret_cast<quint32*>(header.data());w[2]=1;w[3]=MNM_WCH_SIZE;w[4]=session;
    w[11]=verify?MNM_WCH_VERIFY:0;w[13]=2;w[14]=MNM_WORLD_MAX_BYTES;w[15]=MNM_WCH_ORACLE;w[16]=MNM_WORLD_BUILD;
    if(f.write(header)!=header.size()||!f.flush())fail("World channel initialization failed");
}
quint32* WorldChannel::word(quint32 offset) const{return reinterpret_cast<quint32*>(mapping_+offset);}
WorldChannel::WorldChannel(const QString& path):file_(path){
    if(!file_.open(QIODevice::ReadWrite)||file_.size()!=MNM_WCH_SIZE)fail("Invalid World channel extent");
    mapping_=file_.map(0,MNM_WCH_SIZE);if(!mapping_)fail("Cannot map World channel");
    session_=*word(16);verify_=*word(44)==MNM_WCH_VERIFY;
    try{identity();}catch(...){file_.unmap(mapping_);mapping_=nullptr;throw;}
}
WorldChannel::~WorldChannel(){if(mapping_){cancel();file_.unmap(mapping_);}}
void WorldChannel::identity() const{
    if(std::memcmp(mapping_,MNM_WCH_MAGIC,8)||*word(8)!=1||*word(12)!=MNM_WCH_SIZE||
       !session_||*word(16)!=session_||*word(44)!=(verify_?MNM_WCH_VERIFY:0)||*word(52)!=2||
       *word(56)!=MNM_WORLD_MAX_BYTES||*word(60)!=MNM_WCH_ORACLE||*word(64)!=MNM_WORLD_BUILD)fail("World channel identity changed");
    for(quint32 i=68;i<MNM_WCH_HEADER;i+=4)if(*word(i))fail("World channel reserved words changed");
    if(mnm_wch_load(word(20))>MNM_WCH_FAILED||mnm_wch_load(word(24))>1)fail("Invalid World channel state");
}
std::optional<WorldPacket> WorldChannel::poll(){
    identity();if(state()==MNM_WCH_FAILED)fail("World producer refused a complete frame");
    quint32 chosen=2,sequence=0;
    for(quint32 i=0;i<2;++i){auto* s=word(MNM_WCH_HEADER+i*MNM_WCH_SLOT);const auto state=mnm_wch_load(s);
        if(state>3)fail("Invalid World slot ownership");
        if(state==2&&s[1]>sequence){chosen=i;sequence=s[1];}}
    if(chosen==2)return {};
    auto* s=word(MNM_WCH_HEADER+chosen*MNM_WCH_SLOT);if(!mnm_wch_cas(s,2,3))return {};
    try{
        if(s[1]!=sequence||sequence<=last_||s[2]<MNM_WORLD_HEADER||s[2]>MNM_WORLD_MAX_BYTES||
           s[3]>MNM_WCH_ORACLE||(!verify_&&s[3]))fail("Invalid complete World packet");
        WorldPacket packet{sequence,QByteArray(reinterpret_cast<char*>(s)+16,int(s[2])),QByteArray(reinterpret_cast<char*>(s)+16+MNM_WORLD_MAX_BYTES,int(s[3]))};
        // Supersede only READY slots. WRITING and READING remain exclusively owned.
        auto* old=word(MNM_WCH_HEADER+(1-chosen)*MNM_WCH_SLOT);
        if(mnm_wch_load(old)==2&&old[1]<sequence&&mnm_wch_cas(old,2,3)){++superseded_;mnm_wch_store(old,0);}
        last_=sequence;mnm_wch_store(word(32),sequence);mnm_wch_store(s,0);return packet;
    }catch(...){mnm_wch_store(s,0);cancel();throw;}
}
void WorldChannel::presented(quint32 sequence){identity();if(sequence!=last_)fail("Unconsumed World presentation");mnm_wch_store(word(48),sequence);}
void WorldChannel::cancel(){if(mapping_)mnm_wch_store(word(24),1);}
quint32 WorldChannel::state() const{return mnm_wch_load(word(20));}
quint32 WorldChannel::dropped() const{return mnm_wch_load(word(36));}
quint32 WorldChannel::reason() const{return mnm_wch_load(word(40));}
}
