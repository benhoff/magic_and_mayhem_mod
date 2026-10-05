#include "playback.hpp"
#include <stdexcept>
namespace mnm::scene {
Playback::Playback(std::function<void()> tick,std::function<std::uint64_t()> now,QObject* parent):QObject(parent),tick_(std::move(tick)),now_(std::move(now)) {
    if(!tick_) throw std::invalid_argument("Playback requires a tick action");
    elapsed_.start();if(!now_) now_=[this] {return std::uint64_t(elapsed_.elapsed());};
    timer_.setInterval(16);timer_.setTimerType(Qt::PreciseTimer);
    connect(&timer_,&QTimer::timeout,this,[this] {poll();});
}
void Playback::failed(const std::string& message) {
    timer_.stop();const bool was=playing();clock_.pause();
    try {if(was && onPlaying) onPlaying(false);} catch(...) {} // Keep faults inside Qt dispatch.
    try {if(onError) onError(message);} catch(...) {}
}
void Playback::play() {
    if(advancing_ || playing()) return;
    try {clock_.play(now_());timer_.start();if(onPlaying) onPlaying(true);}
    catch(const std::exception& e) {failed(e.what());}
}
void Playback::pause() {
    timer_.stop();const bool was=playing();clock_.pause();
    try {if(was && onPlaying) onPlaying(false);} catch(const std::exception& e) {failed(e.what());}
}
void Playback::advance(bool single) {
    if(advancing_ || (single && playing()) || (!single && !playing())) return;
    advancing_=true;
    struct Guard {bool& flag;~Guard(){flag=false;}} guard{advancing_};
    try {
        const auto count=single?1:clock_.admit(now_()).ticks;
        unsigned applied=0;
        for(unsigned i=0;i<count && (single || playing());++i) {tick_();++applied;}
        if(applied && onRefresh) onRefresh();
    } catch(const std::exception& e) {failed(e.what());}
}
void Playback::step() {advance(true);}
void Playback::poll() {advance(false);}
}
