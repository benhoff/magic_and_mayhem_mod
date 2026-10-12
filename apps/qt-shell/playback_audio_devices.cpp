#include "playback_audio_devices.hpp"
#include <QMediaDevices>

namespace {
class QtPlaybackAudioDevices final : public QObject, public PlaybackAudioDevices {
public:
    QtPlaybackAudioDevices() {
        connect(&devices_, &QMediaDevices::audioOutputsChanged, this, [this] {
            if (changed) changed();
        });
    }
    QAudioDevice defaultOutput() const override {
        return QMediaDevices::defaultAudioOutput();
    }
private:
    QMediaDevices devices_;
};
}
std::unique_ptr<PlaybackAudioDevices> makePlaybackAudioDevices() {
    return std::make_unique<QtPlaybackAudioDevices>();
}
