#pragma once
#include <QWidget>
#include <functional>
class QPushButton;
namespace mnm::scene {
class PlaybackControls final:public QWidget {
    QPushButton *play_,*pause_,*step_;
public:
    explicit PlaybackControls(QWidget* parent=nullptr);
    void updatePlaying(bool);
    std::function<void()> onPlay,onPause,onStep;
};
}
