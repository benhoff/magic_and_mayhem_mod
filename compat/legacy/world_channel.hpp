#pragma once
#include <QFile>
#include <QByteArray>
#include <optional>
#include "../../protocols/include/mnm/world_channel_v1.h"
namespace mnm::legacy {
struct WorldPacket {quint32 sequence=0;QByteArray inputs,oracle;};
class WorldChannel final {
public:
    static void create(const QString& path,quint32 session,bool verify);
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
    bool verify() const{return verify_;}
    quint32 superseded() const{return superseded_;}
private:
    void identity() const;
    quint32* word(quint32 offset) const;
    QFile file_;uchar* mapping_=nullptr;quint32 session_=0,last_=0,superseded_=0;bool verify_=false;
};
}
