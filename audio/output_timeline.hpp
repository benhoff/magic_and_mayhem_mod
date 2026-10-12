#pragma once
#include "buffers.hpp"
#include <deque>

namespace mnm::audio {
// Thread-confined, bounded association of rendered PCM with backend consumption.
// Controls are immediate logical actions; already queued PCM has bounded latency.
// This clock is backend progress, not a measurement at a physical loudspeaker.
class OutputTimeline {
public:
    explicit OutputTimeline(Device& device):device_(device){}
    Error render(std::size_t frames,std::vector<std::int16_t>& output);
    void consume(std::uint64_t frames); // Absolute whole output frames, monotonic.
    std::optional<VoiceInfo> voice(BufferId id) const;
    std::optional<std::size_t> cursor(BufferId id) const;
    Error play(BufferId id,std::uint32_t flags);
    Error stop(BufferId id);
    Error seek(BufferId id,std::size_t byteOffset);
    void changed(BufferId id); // Successful create/duplicate/seek/reset/release.
    void discard(); // Rewind unconsumed playback before retiring/restarting output.
    std::uint64_t generatedFrames() const{return generated_;}
    std::uint64_t consumedFrames() const{return consumed_;}
    std::size_t pendingBlocks() const{return blocks_.size();}
    static constexpr std::size_t maxBlocks=256;
private:
    struct Point {BufferId id;std::uint64_t epoch;VoiceInfo voice;std::uint32_t phase;};
    struct Block {std::uint64_t begin,frames;std::vector<Point> points;};
    static Point advance(Point,std::uint64_t frames,std::uint32_t outputRate);
    Point point(BufferId id) const;
    void restore(const Point&);
    Device& device_;std::deque<Block> blocks_;
    std::unordered_map<BufferId,Point> presented_;
    std::uint64_t nextEpoch_=1,generated_=0,consumed_=0;
};
}
