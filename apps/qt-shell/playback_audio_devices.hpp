#pragma once
#include <QAudioDevice>
#include <functional>
#include <memory>

// Topology source for the default movie/file-sound output. Delivery belongs to
// the player's Qt thread; explicit device users own their separate recovery.
class PlaybackAudioDevices {
public:
    virtual ~PlaybackAudioDevices() = default;
    virtual QAudioDevice defaultOutput() const = 0;
    std::function<void()> changed;
};
std::unique_ptr<PlaybackAudioDevices> makePlaybackAudioDevices();
