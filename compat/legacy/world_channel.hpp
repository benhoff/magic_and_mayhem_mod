#pragma once
#include <QFile>
#include <QByteArray>
#include <optional>
#include "../../protocols/include/mnm/world_channel_v2.h"
namespace mnm::legacy {
struct WorldPacket {quint32 sequence=0;QByteArray inputs,oracle;quint32 canvas=0,sourceSequence=0;bool reset=false;};
class WorldChannel final {
public:
    static void create(const QString& path,quint32 session,bool verify);
    static void createHistory(const QString& path,quint32 session,bool verify,quint32 frames=16);
    explicit WorldChannel(const QString& path);
    ~WorldChannel();
    WorldChannel(const WorldChannel&)=delete;
    WorldChannel& operator=(const WorldChannel&)=delete;
    std::optional<WorldPacket> poll();
    void presented(quint32 sequence);
    void cancel();
    quint32 state() const;
    quint32 dropped() const;
    quint32 reason() const;
    bool history() const {return history_;}
    quint32 targetFrames() const {return history_?slots_:0;}
    bool drained() const;
    bool verify() const{return verify_;}
    quint32 superseded() const{return superseded_;}
private:
    void identity() const;
    quint32* word(quint32 offset) const;
    QFile file_;uchar* mapping_=nullptr;quint32 session_=0,last_=0,superseded_=0;bool verify_=false,history_=false;
    quint32 slots_=2,slotBytes_=MNM_WCH_SLOT,payloadOffset_=16,extent_=MNM_WCH_SIZE,lastCanvas_=0,lastSource_=0;
};
}
