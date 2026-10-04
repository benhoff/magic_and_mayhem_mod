#pragma once
#include "catalog_preflight.hpp"
#include "qt_output.hpp"
#include <QElapsedTimer>
#include <QObject>
#include <QRandomGenerator>

// Application output seam: real Qt sink in production, bounded writer in tests.
// stop() must discard pending PCM and cease all callbacks before returning.
class AudioSessionOutput {
public:
    virtual ~AudioSessionOutput()=default;
    virtual std::uint32_t rate(QString& error)=0;
    virtual bool start(mnm::audio::Device&,QString& error)=0;
    virtual void stop()=0;
    virtual bool running() const=0;
    virtual QString error() const=0;
    // Deliver on the owning Qt thread. Receivers defer destructive recovery.
    std::function<void(const QString&)> failed;
    std::function<void()> available;
};
std::unique_ptr<AudioSessionOutput> makeQtSessionOutput(const QAudioDevice&);
// Follow the current default output, refreshing device information on retry.
std::unique_ptr<AudioSessionOutput> makeQtSessionOutput();
enum class AudioSessionState {stopped,running,failed,recovering};

// Application orchestration, independent of widgets, Wine and hook channels.
// All operations and sink pumping belong to this QObject's Qt thread.
class AudioSession final:public QObject {
public:
    explicit AudioSession(std::unique_ptr<AudioSessionOutput>,QObject* parent=nullptr);
    ~AudioSession() override;
    bool start(const QString& soundsRoot,std::uint32_t map,
               mnm::reconstruction::audio::NativeSourcePathPolicy policy);
    bool play(std::int32_t sound,bool looping=false,std::int32_t volume=0,std::int32_t pan=0);
    bool clearVoices();
    bool setMasterVolume(std::int32_t level);
    bool recover();
    bool canRecover() const{return bool(configuration_);}
    AudioSessionState state() const{return state_;}
    std::function<void(AudioSessionState,const QString&)> changed;
    void stop();
    bool running() const;
    QString lastError() const;
    const mnm::reconstruction::audio::CatalogPreflight& preflight() const{return report_;}
    std::uint32_t outputRate() const;
private:
    struct Configuration {QString root;std::uint32_t map;mnm::reconstruction::audio::NativeSourcePathPolicy policy;};
    bool launch(const Configuration&);
    void clearRuntime();
    void setState(AudioSessionState);
    void outputFailed(const QString&);
    void outputAvailable();
    void checkThread() const;
    std::unique_ptr<AudioSessionOutput> output_;
    // Order matters: backend holds references to manager, clock and caller slots.
    QElapsedTimer clock_;QRandomGenerator random_{0x4d4e4d};
    mnm::reconstruction::audio::AudioManager manager_;
    mnm::reconstruction::audio::ManagerGlobals globals_;
    std::vector<std::uint32_t> slots_;
    std::unique_ptr<mnm::reconstruction::audio::NativeManagerBackend> backend_;
    mnm::reconstruction::audio::CatalogPreflight report_;
    QString error_;
    std::optional<Configuration> configuration_;
    AudioSessionState state_=AudioSessionState::stopped;
    std::int32_t masterVolume_=0;
    std::uint64_t generation_=0;
    bool failurePending_=false,availabilityPending_=false;
};
