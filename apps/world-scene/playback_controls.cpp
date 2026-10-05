#include "playback_controls.hpp"
#include <QHBoxLayout>
#include <QPushButton>
namespace mnm::scene {
PlaybackControls::PlaybackControls(QWidget* parent):QWidget(parent) {
    auto* layout=new QHBoxLayout(this);layout->setContentsMargins(0,0,0,0);
    play_=new QPushButton("Play",this);pause_=new QPushButton("Pause",this);step_=new QPushButton("Step",this);
    play_->setObjectName("playSimulation");pause_->setObjectName("pauseSimulation");step_->setObjectName("stepSimulation");
    play_->setToolTip("Run the native preview at 10 ticks per second.");
    step_->setToolTip("Advance one simulation tick while paused.");
    for(auto* b:{play_,pause_,step_}) layout->addWidget(b);
    connect(play_,&QPushButton::clicked,this,[this] {if(onPlay) onPlay();});
    connect(pause_,&QPushButton::clicked,this,[this] {if(onPause) onPause();});
    connect(step_,&QPushButton::clicked,this,[this] {if(onStep) onStep();});
    updatePlaying(false);
}
void PlaybackControls::updatePlaying(bool playing) {play_->setEnabled(!playing);pause_->setEnabled(playing);step_->setEnabled(!playing);}
}
