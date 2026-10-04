#pragma once
#include "qt_output.hpp"
#include "../runtime/audio/protocol.h"
#include <QFile>
#include <QObject>
#include <QTimer>
#include <memory>
namespace mnm::audio {
struct BridgeReply {std::uint32_t status=0,value=0;};
class VoiceCommands {
public:
    explicit VoiceCommands(Device& device):device_(device){}
    BridgeReply execute(const std::uint32_t* words,const QByteArray& payload);
private:
    Device& device_;
};
class VoiceBroker:public QObject {
public:
    explicit VoiceBroker(QObject* parent=nullptr);
    ~VoiceBroker() override;
    bool create(const QString& path,bool audible=true);
    void stop();
    QString lastError() const{return error_;}
    std::function<void(const QString&)> failed;
private:
    void poll();
    QFile file_;uchar* map_=nullptr;std::unique_ptr<Device> device_;
    std::unique_ptr<VoiceCommands> commands_;std::unique_ptr<QtOutput> output_;
    QTimer timer_;std::uint32_t last_=0;QString error_;
};
}
