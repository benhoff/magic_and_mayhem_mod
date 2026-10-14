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
    static constexpr std::size_t TurnLimit=16384,GuiLimit=8192,HitchLimit=8,HitchTurnLimit=2048,HitchGuiLimit=512,GuiActivityCount=13;
    struct Turn {
        std::uint64_t turn=0,sourceBefore=0,sourceAfter=0,framesBefore=0,framesAfter=0,commands=0,presents=0,imports=0,exportSerial=0;
        std::int64_t begin=0,worldEnd=0,commandEnd=0,extraEnd=0,exportEnd=0,statusEnd=0,waitEnd=0,deadline=0,published=0;
        bool draw=false,paused=false,pending=false;
        int reason=-1;
        NativeFrameSourceTiming source;
        std::array<std::uint64_t,8> delivery{};
        std::uint64_t uiGeneration=0,preparedGeneration=0,sealedSource=0;
        bool uiSealed=false,worldOutput=false,exportBlocked=false;
    };
    struct Gui {std::uint64_t serial=0,generation=0;std::int64_t time=0;int kind=0;bool draw=false,paused=false;int activity=0;std::int64_t duration=0;};
    explicit NativeFrameTrace(bool enabled=false,bool hitches=false):hitches_(enabled&&hitches),enabled_(enabled),origin_(now()) {
        if(enabled_){turns_.resize(TurnLimit);gui_.resize(GuiLimit);}
        if(hitches_){recentTurns_.resize(HitchTurnLimit);recentGui_.resize(HitchGuiLimit);for(auto& w:windows_){w.turns.resize(HitchTurnLimit);w.gui.resize(HitchGuiLimit);}}
    }
    bool enabled() const{return enabled_;}
    bool capturesHitches() const{return hitches_;}
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
        if(hitches_)recordHitchTurn(t);
    }
    void recordGui(Gui g){if(enabled_){
        if(g.kind==4&&g.activity>=0&&g.activity<int(GuiActivityCount)){if(!activityDepth_[g.activity]++)activityBegin_[g.activity]=g.time;}
        bool completedActivity=false;
        if(g.kind==5&&g.activity>=0&&g.activity<int(GuiActivityCount)){
            if(!activityDepth_[g.activity])++unmatchedActivities_;
            else if(!--activityDepth_[g.activity]){g.duration=g.time-activityBegin_[g.activity];activityTimings_[g.activity].add(double(g.duration)/1000.);completedActivity=true;}
        }
        ++allGuiEvents_;
        // Aggregate every1ms facade poll, but keep slow completed spans and one
        // fast sample/second. Preserve full duration without flooding hitch rings.
        if(hitches_&&(!g.draw||g.paused))previousPaint_=0;
        if((g.kind==4||g.kind==5)&&(g.activity==0||g.activity==12)){
            if(!completedActivity|| (g.duration<1000&&g.time-lastActivitySample_[g.activity]<1000000)){++sampledGuiEvents_;return;}
            g.kind=9;lastActivitySample_[g.activity]=g.time;
        }
        if(g.kind==2)paintBegin_=g.time;
        if(g.kind==3&&paintBegin_){paintTiming_.add(double(g.time-paintBegin_)/1000.);paintBegin_=0;}
        if(g.kind==8)composeBegin_=g.time;
        if(g.kind==1&&composeBegin_){composeTiming_.add(double(g.time-composeBegin_)/1000.);composeBegin_=0;}
        if(g.kind==6)heartbeatTiming_.add(double(g.duration)/1000.);
        gui_[guiCount_%GuiLimit]=g;++guiCount_;if(hitches_)recordHitchGui(g);
    }}
    QJsonObject report() const{
        if(!enabled_)return QJsonObject{{"enabled",false}};
        const auto summary=[](const auto& timings){QJsonObject r;const char* names[]={"world","commands","extra_world_ui","export","status_publication","wait","loop_gap"};for(unsigned i=0;i<7;++i){const auto& t=timings[i];r[names[i]]=QJsonObject{{"samples",double(t.samples)},{"total_ms",t.totalMs},{"mean_ms",t.samples?t.totalMs/t.samples:0.},{"max_ms",t.maxMs}};}return r;};
        QJsonArray rows;const auto count=std::min(savedTurns_,std::uint64_t(TurnLimit));
        for(std::uint64_t i=savedTurns_-count;i<savedTurns_;++i){const auto& t=turns_[i%TurnLimit];rows.append(turnRow(t));}

        QJsonArray gui;const auto n=std::min(guiCount_,std::uint64_t(GuiLimit));for(std::uint64_t i=guiCount_-n;i<guiCount_;++i){const auto& g=gui_[i%GuiLimit];gui.append(guiRow(g));}
        return QJsonObject{{"enabled",true},{"schema",4},{"gui_delivery",guiDeliveryReport()},{"host_origin_us",QString::number(origin_)},{"hitch_capture",hitchReport()},{"turn_limit",int(TurnLimit)},{"gui_limit",int(GuiLimit)},{"all_turns",QString::number(allTurns_)},{"saved_turns",QString::number(savedTurns_)},{"overwritten_turns",QString::number(savedTurns_-count)},{"all_gui_events",QString::number(allGuiEvents_)},{"sampled_out_gui_events",QString::number(sampledGuiEvents_)},{"gui_events",QString::number(guiCount_)},{"overwritten_gui_events",QString::number(guiCount_-n)},{"timings",summary(timings_)},{"visible_timings",summary(activeTimings_)},{"visible_owner_seconds",double(activeUs_)/1000000.},{"visible_native_world_frames",QString::number(activeFrames_)},{"visible_native_world_fps",activeUs_?double(activeFrames_)*1000000./activeUs_:0.},
            {"columns",QJsonArray{"turn","begin_us","world_end_us","commands_end_us","extra_end_us","export_end_us","status_end_us","wait_end_us","deadline_us","source_before","source_after","frames_before","frames_after","commands","presents","imports","export_serial","reason","draw_allowed","paused","pending_after","source_queue_ms","source_ready_ms","source_decode_ms","source_handoff_ms","native_producer_ms","native_prepare_ms","native_submit_ms","original_tick","original_capture_ms","original_ack_wait_ms","original_return_ms","published_us","source_accepted","source_returned","source_outstanding","source_ready","gpu_submitted","gpu_produced","gpu_taken","gpu_outstanding","ui_generation","prepared_generation","sealed_source_generation","ui_sealed","world_output","export_blocked"}},
            {"rows",rows},{"gui_columns",QJsonArray{"kind","serial","generation","time_us","draw_allowed","paused","activity","duration_us"}},{"gui_rows",gui},
            {"scope","Opt-in bounded native host timeline. GUI kind0 is frame pickup,1 is Qt frameSwapped tagged by completed paint,2/3 are GPU widget paint begin/end; swaps are not physical scanout or distinct simulation updates. Steady-clock microseconds are relative to the exported native host origin; campaign host_us fields use this same clock. Optional bounded hitch windows preserve all owner polls around warm active paint gaps. Original clocks remain separate. Original uint32 millisecond durations are separate, coarse and wrap-subtracted; no cross-clock timestamp subtraction or original CPU attribution. CPU wall spans include driver waits. Source ready includes decode/ACK/return verification; queue delay begins only after verification. Tail samples may overwrite; aggregate timings include all turns. Full functional and60FPS validation remain separate."}};
    }
private:
    QJsonArray turnRow(const Turn& t) const{
        const auto& s=t.source;
        QJsonArray r{QString::number(t.turn),double(t.begin-origin_),double(t.worldEnd-origin_),double(t.commandEnd-origin_),double(t.extraEnd-origin_),double(t.exportEnd-origin_),double(t.statusEnd-origin_),double(t.waitEnd-origin_),t.deadline?double(t.deadline-origin_):0.,QString::number(t.sourceBefore),QString::number(t.sourceAfter),QString::number(t.framesBefore),QString::number(t.framesAfter),double(t.commands),double(t.presents),double(t.imports),QString::number(t.exportSerial),t.reason,t.draw,t.paused,t.pending,s.queueMs,s.readyMs,s.decodeMs,s.handoffMs,s.producerMs,s.prepareMs,s.submitMs,double(s.ticks[0]),double(std::uint32_t(s.ticks[1]-s.ticks[0])),double(std::uint32_t(s.ticks[2]-s.ticks[1])),double(std::uint32_t(s.ticks[3]-s.ticks[2])),t.published?double(t.published-origin_):0.};
        for(auto v:t.delivery)r.append(QString::number(v));
        for(auto v:{t.uiGeneration,t.preparedGeneration,t.sealedSource})r.append(QString::number(v));
        r.append(t.uiSealed);r.append(t.worldOutput);r.append(t.exportBlocked);return r;
    }
    QJsonArray guiRow(const Gui& g) const{return QJsonArray{g.kind,QString::number(g.serial),QString::number(g.generation),double(g.time-origin_),g.draw,g.paused,g.activity,double(g.duration)};}
    QJsonObject guiDeliveryReport() const{
        const auto summary=[](const RenderStageTiming& t){return QJsonObject{{"samples",QString::number(t.samples)},{"total_ms",t.totalMs},{"mean_ms",t.samples?t.totalMs/t.samples:0.},{"max_ms",t.maxMs}};};
        const char* names[]={"presentation_poll","route_capture","reference_read","screen_capture","native_readback","image_encoding","diagnostic_capture","campaign_poll","progress_publication","capture_request","window_discovery","input_heartbeat","render_status_pickup"};QJsonObject activities;
        for(unsigned i=0;i<GuiActivityCount;++i){auto row=summary(activityTimings_[i]);row["open_depth"]=int(activityDepth_[i]);activities[names[i]]=row;}
        return QJsonObject{{"activities",activities},{"paint_cpu",summary(paintTiming_)},{"compose_to_swap",summary(composeTiming_)},{"heartbeat_lateness",summary(heartbeatTiming_)},{"unmatched_activity_ends",QString::number(unmatchedActivities_)},{"scope","GUI kind4/5 activity begin/end (nested same activity counts outer span),6 heartbeat lateness beyond16ms interval,8 Qt aboutToCompose. Activity IDs0..12 follow listed names. Kind9 is completed sampled activity0/12 (end minus duration gives begin): every >=1ms span and one fast sample/second; every span still contributes to aggregates. Bounded host wall timing includes driver waits; compose/swap is not scanout. Capture phases overlap their outer capture; totals are not additive. No added readbacks or live JSON."}};
    }
    struct HitchWindow {
        std::int64_t before=0,trigger=0,until=0,gap=0;
        std::uint64_t serial=0,generation=0;
        std::size_t turnCount=0,guiCount=0;
        bool clipped=false;
        std::vector<Turn> turns;std::vector<Gui> gui;
    };
    void append(HitchWindow& w,const Turn& t){if(w.turnCount==HitchTurnLimit)w.clipped=true;else w.turns[w.turnCount++]=t;}
    void append(HitchWindow& w,const Gui& g){if(w.guiCount==HitchGuiLimit)w.clipped=true;else w.gui[w.guiCount++]=g;}
    void recordHitchTurn(const Turn& t){
        recentTurns_[recentTurnCount_++%HitchTurnLimit]=t;
        for(auto& w:windows_)if(w.trigger && t.waitEnd>=w.before && t.begin<=w.until)append(w,t);
    }
    void recordHitchGui(const Gui& g){
        recentGui_[recentGuiCount_++%HitchGuiLimit]=g;
        for(auto& w:windows_)if(w.trigger && g.time>=w.before && g.time<=w.until)append(w,g);
        if(!g.draw||g.paused){previousPaint_=0;return;}
        if(g.kind!=3)return;
        if(g.generation<64){previousPaint_=0;return;}
        const auto prior=previousPaint_;previousPaint_=g.time;
        if(!prior || g.time-prior<50000)return;
        ++hitchCount_;const auto gap=g.time-prior;
        HitchWindow* chosen=nullptr;
        for(auto& w:windows_)if(!w.trigger){chosen=&w;break;}
        if(!chosen){for(auto& w:windows_)if(w.until<g.time && (!chosen||w.gap<chosen->gap))chosen=&w;
            if(!chosen||gap<=chosen->gap){++unretainedHitches_;return;}++replacedHitches_;}
        auto& w=*chosen;w.before=prior-100000;w.trigger=g.time;w.until=g.time+250000;w.gap=gap;w.serial=g.serial;w.generation=g.generation;w.turnCount=w.guiCount=0;w.clipped=false;
        const auto turns=std::min(recentTurnCount_,std::uint64_t(HitchTurnLimit));
        for(auto i=recentTurnCount_-turns;i<recentTurnCount_;++i){const auto& t=recentTurns_[i%HitchTurnLimit];if(t.waitEnd>=w.before && t.begin<=g.time)append(w,t);}
        if(turns && recentTurns_[(recentTurnCount_-turns)%HitchTurnLimit].begin>w.before)w.clipped=true;
        const auto events=std::min(recentGuiCount_,std::uint64_t(HitchGuiLimit));
        for(auto i=recentGuiCount_-events;i<recentGuiCount_;++i){const auto& e=recentGui_[i%HitchGuiLimit];if(e.time>=w.before && e.time<=g.time)append(w,e);}
        if(events && recentGui_[(recentGuiCount_-events)%HitchGuiLimit].time>w.before)w.clipped=true;
    }
    QJsonObject hitchReport() const{
        if(!hitches_)return QJsonObject{{"enabled",false}};
        QJsonArray results;
        for(const auto& w:windows_)if(w.trigger){QJsonArray turns,events;
            for(std::size_t i=0;i<w.turnCount;++i)turns.append(turnRow(w.turns[i]));
            for(std::size_t i=0;i<w.guiCount;++i)events.append(guiRow(w.gui[i]));
            results.append(QJsonObject{{"trigger_us",double(w.trigger-origin_)},{"previous_paint_us",double(w.trigger-w.gap-origin_)},{"until_us",double(w.until-origin_)},{"paint_gap_ms",double(w.gap)/1000.},{"serial",QString::number(w.serial)},{"generation",QString::number(w.generation)},{"clipped",w.clipped},{"post_complete",previousEnd_>=w.until},{"rows",turns},{"gui_rows",events}});
        }
        return QJsonObject{{"enabled",true},{"threshold_ms",50},{"before_previous_paint_ms",100},{"after_trigger_ms",250},{"window_limit",int(HitchLimit)},{"turn_limit",int(HitchTurnLimit)},{"gui_limit",int(HitchGuiLimit)},{"observed_hitches",QString::number(hitchCount_)},{"replaced_windows",QString::number(replacedHitches_)},{"unretained_hitches",QString::number(unretainedHitches_)},{"windows",results},{"scope","Largest eight eligible warm active completed-paint gaps; overlapping windows allowed, bounded histories can clip. Preallocated owner/GUI records, no live JSON or pixels. Cold/non-world/hidden/movie transitions reset paint continuity. Paint gaps are observations, not GPU busy/scanout or cause attribution."}};
    }
    bool hitches_=false;std::int64_t previousPaint_=0;
    std::uint64_t recentTurnCount_=0,recentGuiCount_=0,hitchCount_=0,replacedHitches_=0,unretainedHitches_=0;
    std::vector<Turn> recentTurns_;std::vector<Gui> recentGui_;std::array<HitchWindow,HitchLimit> windows_;
    bool lastDraw_=false,lastPaused_=false;
    bool enabled_;std::int64_t origin_,previousEnd_=0,lastSaved_=0,activeUs_=0;
    std::uint64_t allTurns_=0,savedTurns_=0,guiCount_=0,activeFrames_=0;
    std::vector<Turn> turns_;std::vector<Gui> gui_;
    std::array<RenderStageTiming,7> timings_{},activeTimings_{};
    std::array<RenderStageTiming,GuiActivityCount> activityTimings_{};
    std::array<std::int64_t,GuiActivityCount> activityBegin_{};
    std::array<unsigned,GuiActivityCount> activityDepth_{};
    RenderStageTiming paintTiming_,composeTiming_,heartbeatTiming_;
    std::int64_t paintBegin_=0,composeBegin_=0;
    std::uint64_t unmatchedActivities_=0,allGuiEvents_=0,sampledGuiEvents_=0;
    std::array<std::int64_t,GuiActivityCount> lastActivitySample_{};
};
