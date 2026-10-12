#pragma once
#include "qt_output.hpp"
#include "../runtime/audio/protocol.h"
#include <QFile>
#include <QObject>
#include <QThread>
#include <memory>
namespace mnm::audio {
struct BridgeReply {std::uint32_t status=0,value=0;};
struct VoiceActivity {
    quint64 requests=0,creates=0,uploads=0,uploadedBytes=0,plays=0,releases=0,failures=0;
    quint32 nativeSelections=0,fallbacks=0,adapterFailures=0;
    bool ready=false,audible=false;
};
class VoiceCommands {
public:
    explicit VoiceCommands(Device& device):device_(device){}
    BridgeReply execute(const std::uint32_t* words,const QByteArray& payload);
private:
    Device& device_;
};
class VoiceBroker:public QObject {
public:
    // Facade methods belong to the owner Qt thread. Device/PCM/dispatch run
    // together on an owned worker; failure callbacks return to the owner.
    explicit VoiceBroker(QObject* parent=nullptr);
    ~VoiceBroker() override;
    bool create(const QString& path,bool audible=true);
    void stop();
    QString lastError() const;
    VoiceActivity activity() const;
    std::function<void(const QString&)> failed;
private:
    struct State;
    QThread workerThread_;QObject* worker_=nullptr;std::unique_ptr<State> state_;
    QString error_;VoiceActivity lastActivity_;quint64 generation_=0;
};
}
