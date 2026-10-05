#pragma once
#include "simulation/tick_clock.hpp"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>
#include <string>
namespace mnm::scene {
// Orchestration adapter: widgets emit actions; only the host supplies simulation ticks.
class Playback final:public QObject {
    QTimer timer_;
    QElapsedTimer elapsed_;
    game::TickClock clock_;
    std::function<void()> tick_;
    std::function<std::uint64_t()> now_;
    bool advancing_=false;
    void advance(bool single);
    void failed(const std::string&);
public:
    Playback(std::function<void()> tick,std::function<std::uint64_t()> now={},QObject* parent=nullptr);
    bool playing() const {return clock_.playing();}
    void play();
    void pause();
    void step();
    void poll();
    std::function<void()> onRefresh;
    std::function<void(bool)> onPlaying;
    std::function<void(const std::string&)> onError;
};
}
