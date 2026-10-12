#pragma once
#include "music_catalog.hpp"
#include <QObject>
#include <QJsonObject>
#include <QString>
#include <functional>
#include <memory>
namespace mnm::audio {
enum class MusicPhase {closed,stopped,loading,playing,paused,completed,unavailable,failed};
struct MusicOutput {
    virtual ~MusicOutput()=default;
    virtual bool start(const QString& file,bool loop,qint64 offsetMs,bool paused,QString& error)=0;
    virtual void stop()=0;
    virtual bool pause(bool paused)=0;
    virtual bool seek(qint64 milliseconds)=0;
    virtual qint64 positionMs()const=0;
    virtual bool available()const=0;
    virtual void setVolume(std::uint16_t level)=0;
    virtual QJsonObject report()const=0;
    std::function<void()> ready,complete,outputAvailable;
    std::function<void(const QString&)> failed,lost;
};
std::unique_ptr<MusicOutput> makeQtMusicOutput(bool audible=true);
struct MusicSnapshot {
    MusicPhase phase=MusicPhase::closed;
    std::uint32_t track=0;
    std::uint16_t volume=65535;
    qint64 positionMs=0;
    std::uint64_t generation=0,completions=0,recoveries=0,losses=0;
    bool loop=false;
    QString error;
};
// Native audio-only session/controller; no widget, legacy address or MCI type.
// Backend callbacks and methods run on its owning Qt thread. Completion/fault
// delivery is deferred so decoder ownership never changes on its callback stack.
class MusicService final:public QObject {
public:
    explicit MusicService(std::unique_ptr<MusicOutput>,QObject* parent=nullptr);
    ~MusicService()override;
    bool initialize(const std::filesystem::path& root);
    void close();
    bool play(std::uint32_t track,bool loop=false,qint64 offsetMs=0);
    bool pause();
    bool resume();
    bool seek(qint64 milliseconds);
    bool volume(std::uint16_t level);
    void stop();
    bool recover();
    MusicSnapshot snapshot()const;
    const MusicCatalog* catalog()const{checkThread();return catalog_.get();}
    QJsonObject outputReport()const{checkThread();return output_->report();}
    std::function<void(const MusicSnapshot&)> changed;
private:
    bool launch(std::uint32_t track,bool loop,qint64 offsetMs,bool paused,bool recovery);
    void outputLost(const QString&);
    void watchAvailability();
    void retire();
    void publish();
    void checkThread()const;
    std::unique_ptr<MusicOutput> output_;
    std::unique_ptr<MusicCatalog> catalog_;
    MusicSnapshot state_;
    bool recoveryPending_=false,resumePaused_=false;
};
}
