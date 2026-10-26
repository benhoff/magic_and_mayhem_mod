#include "sprite_timeline.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::sprites {
namespace {
void validRate(std::uint32_t n,std::uint32_t d) {
    if(!n || !d || n>1000000 || d>1000000)
        throw std::invalid_argument("Native timeline rate outside1..1e6");
}
}
TimelineClip::TimelineClip(std::shared_ptr<const SpriteTable> table,std::vector<TimelineEntry> entries)
    :table_(std::move(table)),entries_(std::move(entries)) {
    if(!table_ || entries_.empty() || entries_.size()>4096)
        throw std::invalid_argument("Native timeline needs1..4096 entries and an owned table");
    frames_.reserve(entries_.size());
    for(const auto& e:entries_) {
        if(!e.ticks || e.ticks>1000000000)
            throw std::invalid_argument("Native timeline duration outside1..1e9");
        frames_.push_back(table_->find(e.name,e.occurrence));
    }
}
void SpriteTimeline::assign(TimelineCheckpoint s) {
    validRate(s.numerator,s.denominator);
    if(!s.clip || s.fraction>=s.denominator ||
       (s.mode!=TimelineMode::loop && s.mode!=TimelineMode::once && s.mode!=TimelineMode::pingPong))
        throw std::invalid_argument("Invalid native timeline checkpoint");
    const auto count=s.clip->entries().size();
    std::vector<std::size_t> order;order.reserve(count*2);
    for(std::size_t i=0;i<count;++i)order.push_back(s.reverse?count-1-i:i);
    // The two endpoints occur once per cycle; singleton clips remain nonempty.
    if(s.mode==TimelineMode::pingPong && count>2)
        for(std::size_t i=count-2;i>0;--i)order.push_back(s.reverse?count-1-i:i);
    std::vector<std::uint64_t> starts;starts.reserve(order.size());
    std::uint64_t duration=0;
    for(auto i:order) {starts.push_back(duration);duration+=s.clip->entries()[i].ticks;}
    // <=8190*1e9; all later modular products by a rate<=1e6 fit uint64.
    if(s.phase>duration || (s.mode!=TimelineMode::once && s.phase==duration) ||
       (s.mode==TimelineMode::once && s.phase==duration && s.fraction))
        throw std::invalid_argument("Native timeline phase outside schedule");
    state_=std::move(s);order_=std::move(order);starts_=std::move(starts);duration_=duration;
}
SpriteTimeline::SpriteTimeline(std::shared_ptr<const TimelineClip> clip,TimelineMode mode,bool reverse) {
    TimelineCheckpoint s;s.clip=std::move(clip);s.mode=mode;s.reverse=reverse;assign(std::move(s));
}
TimelinePosition SpriteTimeline::position() const {
    const auto i=state_.phase==duration_?order_.size()-1:
        std::size_t(std::upper_bound(starts_.begin(),starts_.end(),state_.phase)-starts_.begin()-1);
    return {order_[i],i,state_.phase-starts_[i],state_.mode==TimelineMode::once&&state_.phase==duration_};
}
std::size_t SpriteTimeline::frame() const {return state_.clip->frame(position().entry);}
void SpriteTimeline::restore(TimelineCheckpoint s) {assign(std::move(s));}
void SpriteTimeline::rate(std::uint32_t n,std::uint32_t d) {
    validRate(n,d);state_.numerator=n;state_.denominator=d;state_.fraction=0;
}
void SpriteTimeline::seek(std::uint64_t ticks) {
    state_.phase=state_.mode==TimelineMode::once?std::min(ticks,duration_):ticks%duration_;
    state_.fraction=0;
}
void SpriteTimeline::seekEntry(std::size_t entry,std::uint64_t tick) {
    if(entry>=order_.size() || tick>=state_.clip->entries()[order_[entry]].ticks)
        throw std::out_of_range("Native timeline entry seek outside schedule");
    state_.phase=starts_[entry]+tick;state_.fraction=0;
}
std::vector<TimelineEvent> SpriteTimeline::advance(std::uint64_t ticks,bool collect,std::size_t budget) {
    if(budget>4096)throw std::length_error("Native timeline event budget exceeds4096");
    std::vector<TimelineEvent> events;
    if(state_.paused || (state_.mode==TimelineMode::once && state_.phase==duration_))return events;
    // Decompose before multiplying. Even UINT64_MAX host deltas are accepted
    // without overflow: the large quotient is reduced or capped first.
    const auto q=ticks/state_.denominator;
    const auto tail=(ticks%state_.denominator)*state_.numerator+state_.fraction;
    const auto extra=tail/state_.denominator;
    auto fraction=std::uint32_t(tail%state_.denominator);
    std::uint64_t movement=0,phase=0;
    if(state_.mode==TimelineMode::once) {
        const auto left=duration_-state_.phase;
        if(extra>=left || q>(left-extra)/state_.numerator)movement=left;
        else movement=std::min(left,q*state_.numerator+extra);
        phase=state_.phase+movement;
        if(phase==duration_)fraction=0;
    } else {
        const auto mod=((q%duration_)*state_.numerator+extra)%duration_;
        phase=(state_.phase+mod)%duration_;
        if(collect) {
            const auto cap=duration_*(std::uint64_t(budget)+1);
            if(extra>cap || q>(cap-extra)/state_.numerator)
                throw std::length_error("Native timeline events exceed finite budget");
            movement=q*state_.numerator+extra;
        }
    }
    if(collect && movement) {
        auto i=position().scheduleEntry;
        auto pos=state_.phase;
        for(;;) {
            const auto end=starts_[i]+state_.clip->entries()[order_[i]].ticks;
            const auto step=end-pos;
            if(movement<step)break;
            movement-=step;
            if(i+1==order_.size() && state_.mode==TimelineMode::once)break;
            i=(i+1)%order_.size();pos=starts_[i];
            if(events.size()==budget)throw std::length_error("Native timeline events exceed finite budget");
            events.push_back({order_[i],state_.clip->entries()[order_[i]].event});
        }
    }
    state_.phase=phase;state_.fraction=fraction;
    return events;
}
void SpriteTimeline::reload(std::shared_ptr<const SpriteTable> table) {
    auto s=state_;
    s.clip=std::make_shared<const TimelineClip>(std::move(table),state_.clip->entries());
    assign(std::move(s));
}
render::SpriteDrawPlan SpriteTimeline::plan(std::int32_t x,std::int32_t y,
                                          std::optional<render::SpriteColourTable> colours) const {
    const auto& sprite=state_.clip->table()->sprite();
    if(colours && sprite->storage!=assets::SpriteStorage::indexed8)
        throw std::invalid_argument("Direct timeline frame refuses palette override");
    return {sprite,{{frame(),x,y,std::move(colours)}}};
}
}
