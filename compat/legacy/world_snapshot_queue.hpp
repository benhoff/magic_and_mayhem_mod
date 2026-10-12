#pragma once
#include "world_continuous.hpp"
#include "world_rolling_handoff.hpp"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
namespace mnm::legacy {
// A source receipt, not a GPU frame. No original pointers or GL/widget objects
// enter this queue. The handoff owns its source payloads through immutable refs.
struct OwnedWorldSnapshot {
  std::uint64_t generation=0;
  RollingHandoff handoff;
  std::array<std::uint32_t,10> ticks{};
  std::size_t sourceBytes=0;
  double decodeMs=0,prepareMs=0;
  std::vector<std::uint8_t> wireSource,wireRequest;
};
class WorldSnapshotQueue final {
public:
  struct Stats {
    std::uint64_t accepted=0,returned=0,imports=0,bytes=0;
    std::size_t outstanding=0,ready=0,highWater=0;
    std::uint32_t firstTick=0,lastTick=0;
    bool terminal=false;
    std::string error;
  };
  explicit WorldSnapshotQueue(WorldContinuousChannel&,bool retainWire=false);
  ~WorldSnapshotQueue();
  WorldSnapshotQueue(const WorldSnapshotQueue&)=delete;
  bool take(OwnedWorldSnapshot&);
  void imported(std::uint64_t);
  Stats stats() const;
  void stop();
private:
  void capture();
  WorldContinuousChannel& channel_;
  bool retainWire_=false;
  mutable std::mutex mutex_;
  std::condition_variable progress_;
  std::deque<OwnedWorldSnapshot> ready_;
  Stats state_;
  std::uint64_t taken_=0;
  std::atomic<bool> stopping_{false};
  std::thread owner_;
};
}
