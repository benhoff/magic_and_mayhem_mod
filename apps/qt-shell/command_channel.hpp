#pragma once
#include "../../protocols/include/mnm/render_commands_v1.h"
#include "../../protocols/include/mnm/render_command_ring.h"
#include <QFile>
#include <QByteArray>
// One native reader per launch; mapping survives until producer has exited.
class CommandChannel final {
public:
    quint32 version() const{return version_;}
    ~CommandChannel();
    bool create(const QString& path,quint32 session,quint32 version=1);
    bool open(const QString& path);
    QByteArray poll(quint32 budget=MNM_RENDER_COMMANDS_V1_POLL_BYTES);
    void cancel();
    quint32 state() const{return state_;}
    quint32 reason() const{return reason_;}
    quint32 consumed() const{return consumed_;}
    bool drained() const{return consumed_==published_;}
    QString error() const{return error_;}
private:
    bool map();
    void validate() const;
    QFile file_;uchar* mapping_=nullptr;quint32 session_=0,consumed_=0,published_=0,state_=0,reason_=0;
    QString error_;
    quint32 version_=1,size_=MNM_RENDER_COMMANDS_V1_SIZE;
    mnm_ring_reader ring_{};
};
