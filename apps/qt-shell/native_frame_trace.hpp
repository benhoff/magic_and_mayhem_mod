#pragma once
#include "render_deadline_probe.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <chrono>
#include <vector>
#include <stdexcept>

struct NativeFrameSourceTiming {
    double queueMs=0,readyMs=0,decodeMs=0,handoffMs=0,producerMs=0,prepareMs=0,submitMs=0;
    std::array<std::uint32_t,10> ticks{};
};
// Application-local diagnostics. Worker owns turn records; GUI event records
// are protected by the worker mutex. No wire clocks, host pointers or pixels.
class NativeFrameTrace {
public:
    static constexpr std::size_t TurnLimit=16384,GuiLimit=8192;
    struct Turn {
        std::uint64_t turn=0,sourceBefore=0,sourceAfter=0,framesBefore=0,framesAfter=0,commands=0,presents=0,imports=0,exportSerial=0;
        std::int64_t begin=0,worldEnd=0,commandEnd=0,extraEnd=0,exportEnd=0,statusEnd=0,waitEnd=0,deadline=0,published=0;
        bool draw=false,paused=false,pending=false;
        int reason=-1;
        NativeFrameSourceTiming source;
    };
    struct Gui {std::uint64_t serial=0,generation=0;std::int64_t time=0;int kind=0;bool draw=false,paused=false;};
    explicit NativeFrameTrace(bool enabled=false):enabled_(enabled),origin_(now()) {if(enabled_){turns_.resize(TurnLimit);gui_.resize(GuiLimit);}}
    bool enabled() const{return enabled_;}
    static std::int64_t now(){return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
    std::int64_t stamp() const{return enabled_?now():0;}
    void record(Turn t){
        if(!enabled_)return;
        if(!(t.begin<=t.worldEnd && t.worldEnd<=t.commandEnd && t.commandEnd<=t.extraEnd && t.extraEnd<=t.exportEnd && t.exportEnd<=t.statusEnd && t.statusEnd<=t.waitEnd) || t.framesAfter<t.framesBefore || t.framesAfter-t.framesBefore>1)
            throw std::logic_error("Native frame trace ordering");
        ++allTurns_;const bool active=t.draw&&!t.paused&&t.sourceAfter>=64;
        const auto add=[&](std::array<RenderStageTiming,7>& a){
            const std::int64_t bounds[]={t.begin,t.worldEnd,t.commandEnd,t.extraEnd,t.exportEnd,t.statusEnd,t.waitEnd};
            for(unsigned i=0;i<6;++i)a[i].add(double(bounds[i+1]-bounds[i])/1000.);
            a[6].add(previousEnd_?double(t.begin-previousEnd_)/1000.:0.);
        };
        add(timings_);if(active){add(activeTimings_);activeUs_+=t.waitEnd-t.begin+(previousEnd_?t.begin-previousEnd_:0);activeFrames_+=t.framesAfter-t.framesBefore;}
        previousEnd_=t.waitEnd;
        // Keep meaningful work, slow waits and one idle sample/second. The tail
        // ring remains bounded even when the engine is in a static menu.
        if(t.commands || t.framesAfter!=t.framesBefore || t.exportSerial || t.draw!=lastDraw_ || t.paused!=lastPaused_ || t.waitEnd-t.statusEnd>2000 || t.waitEnd-lastSaved_>=1000000){turns_[savedTurns_%TurnLimit]=t;++savedTurns_;lastSaved_=t.waitEnd;}
        lastDraw_=t.draw;lastPaused_=t.paused;
    }
    void recordGui(Gui g){if(enabled_){gui_[guiCount_%GuiLimit]=g;++guiCount_;}}
    QJsonObject report() const{
        if(!enabled_)return QJsonObject{{"enabled",false}};
        const auto summary=[](const auto& timings){QJsonObject r;const char* names[]={"world","commands","extra_world_ui","export","status_publication","wait","loop_gap"};for(unsigned i=0;i<7;++i){const auto& t=timings[i];r[names[i]]=QJsonObject{{"samples",double(t.samples)},{"total_ms",t.totalMs},{"mean_ms",t.samples?t.totalMs/t.samples:0.},{"max_ms",t.maxMs}};}return r;};
        QJsonArray rows;const auto count=std::min(savedTurns_,std::uint64_t(TurnLimit));
        for(std::uint64_t i=savedTurns_-count;i<savedTurns_;++i){const auto& t=turns_[i%TurnLimit];const auto& s=t.source;
            QJsonArray r{QString::number(t.turn),double(t.begin-origin_),double(t.worldEnd-origin_),double(t.commandEnd-origin_),double(t.extraEnd-origin_),double(t.exportEnd-origin_),double(t.statusEnd-origin_),double(t.waitEnd-origin_),t.deadline?double(t.deadline-origin_):0.,QString::number(t.sourceBefore),QString::number(t.sourceAfter),QString::number(t.framesBefore),QString::number(t.framesAfter),double(t.commands),double(t.presents),double(t.imports),QString::number(t.exportSerial),t.reason,t.draw,t.paused,t.pending,s.queueMs,s.readyMs,s.decodeMs,s.handoffMs,s.producerMs,s.prepareMs,s.submitMs,double(s.ticks[0]),double(std::uint32_t(s.ticks[1]-s.ticks[0])),double(std::uint32_t(s.ticks[2]-s.ticks[1])),double(std::uint32_t(s.ticks[3]-s.ticks[2])),t.published?double(t.published-origin_):0.};rows.append(r);
        }
        QJsonArray gui;const auto n=std::min(guiCount_,std::uint64_t(GuiLimit));for(std::uint64_t i=guiCount_-n;i<guiCount_;++i){const auto& g=gui_[i%GuiLimit];gui.append(QJsonArray{g.kind,QString::number(g.serial),QString::number(g.generation),double(g.time-origin_),g.draw,g.paused});}
        return QJsonObject{{"enabled",true},{"schema",1},{"turn_limit",int(TurnLimit)},{"gui_limit",int(GuiLimit)},{"all_turns",QString::number(allTurns_)},{"saved_turns",QString::number(savedTurns_)},{"overwritten_turns",QString::number(savedTurns_-count)},{"gui_events",QString::number(guiCount_)},{"overwritten_gui_events",QString::number(guiCount_-n)},{"timings",summary(timings_)},{"visible_timings",summary(activeTimings_)},{"visible_owner_seconds",double(activeUs_)/1000000.},{"visible_native_world_frames",QString::number(activeFrames_)},{"visible_native_world_fps",activeUs_?double(activeFrames_)*1000000./activeUs_:0.},
            {"columns",QJsonArray{"turn","begin_us","world_end_us","commands_end_us","extra_end_us","export_end_us","status_end_us","wait_end_us","deadline_us","source_before","source_after","frames_before","frames_after","commands","presents","imports","export_serial","reason","draw_allowed","paused","pending_after","source_queue_ms","source_ready_ms","source_decode_ms","source_handoff_ms","native_producer_ms","native_prepare_ms","native_submit_ms","original_tick","original_capture_ms","original_ack_wait_ms","original_return_ms","published_us"}},
            {"rows",rows},{"gui_columns",QJsonArray{"kind","serial","generation","time_us","draw_allowed","paused"}},{"gui_rows",gui},
            {"scope","Opt-in bounded native host timeline. GUI kind0 is frame pickup,1 is Qt frameSwapped tagged by completed paint,2/3 are GPU widget paint begin/end; swaps are not physical scanout or distinct simulation updates. Steady-clock microseconds are relative to one host origin. Original uint32 millisecond durations are separate, coarse and wrap-subtracted; no cross-clock timestamp subtraction or original CPU attribution. CPU wall spans include driver waits. Source ready includes decode/ACK/return verification; queue delay begins only after verification. Tail samples may overwrite; aggregate timings include all turns. Full functional and60FPS validation remain separate."}};
    }
private:
    bool lastDraw_=false,lastPaused_=false;
    bool enabled_;std::int64_t origin_,previousEnd_=0,lastSaved_=0,activeUs_=0;
    std::uint64_t allTurns_=0,savedTurns_=0,guiCount_=0,activeFrames_=0;
    std::vector<Turn> turns_;std::vector<Gui> gui_;
    std::array<RenderStageTiming,7> timings_{},activeTimings_{};
};
