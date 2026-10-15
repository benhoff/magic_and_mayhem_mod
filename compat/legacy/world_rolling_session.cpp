#include "world_rolling_session.hpp"
#include <chrono>
#include <limits>
#include <cstdlib>
#include <cstring>
namespace mnm::legacy {
namespace {
using Clock=std::chrono::steady_clock;
double elapsed(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
}
WorldRollingSession::WorldRollingSession(render::GlBlitter &renderer,
                                         const assets::AssetStore &store,
                                         bool verify, bool resident, bool preparedReplay,
                                         bool retainCompletedImage)
    : renderer_(renderer), mirror_(resident ? std::make_unique<render::GpuCanvasMirror>(renderer) : nullptr),
      producer_([store](const auto &name) {
        auto opened = store.open(name);
        if (auto *e = std::get_if<assets::Error>(&opened))
          throw std::runtime_error(e->detail);
        auto f =
            std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));
        auto read = assets::readWhole(*f, 16 * 1024 * 1024);
        if (auto *e = std::get_if<assets::Error>(&read))
          throw std::runtime_error(e->detail);
        return std::get<std::vector<std::uint8_t>>(std::move(read));
      }, mirror_.get()),
      world_(renderer, store), verify_(verify), resident_(resident) {
  const auto profile=std::getenv("MNM_WORLD_OWNER_PROFILE");ownerProfile_=profile&&std::strcmp(profile,"1")==0;
  if(resident && verify)throw std::invalid_argument("Resident and pixel-reference modes are separate");
  if(retainCompletedImage && !preparedReplay)throw std::invalid_argument("Retained World image requires prepared resident presentation");
  if(preparedReplay){if(!resident)throw std::invalid_argument("Prepared World requires resident native history");world_.enablePreparedReplay(retainCompletedImage);}
}
render::Image WorldRollingSession::consume(
    std::uint64_t sequence, const std::vector<std::uint8_t> &bytes,
    const std::function<void(const CanvasProducer &, const render::Image &)>
        &checkpoint,
    const std::function<void(const CanvasProducer &, const render::Image &)>
        &entry) {
  if (failed_)
    throw std::runtime_error(
        "Rolling renderer session is terminal; create a fresh session");
  try {
    if (completed_ == std::numeric_limits<std::uint64_t>::max() ||
        sequence != completed_ + 1)
      throw std::invalid_argument(
          "Rolling renderer sequence gap or exhaustion");
    if (!cached_ || cachedBytes_ != bytes) {
      auto decoded = decodeRollingQueue(bytes);
      auto owned = bytes;
      cached_ = std::move(decoded);
      cachedBytes_ = std::move(owned);
      ++decodes_;
    } else
      ++hits_;
    const auto &stream = *cached_;
    return consumeStream(sequence, stream, checkpoint, entry);
  } catch (...) {
    failed_ = true;
    throw;
  }
}
render::Image
WorldRollingSession::consumeOwned(std::uint64_t sequence,
                                  const CanvasProducerStream &stream) {
  if (failed_)
    throw std::runtime_error("Terminal rolling session");
  try {
    ownerCosts_={};ownerCosts_.enabled=ownerProfile_;
    const auto start=ownerProfile_?Clock::now():Clock::time_point{};
    validateRollingQueue(stream);
    if(ownerProfile_)ownerCosts_.validationMs=elapsed(start);
    return consumeStream(sequence, stream, {}, {});
  } catch (...) {
    failed_ = true;
    throw;
  }
}
render::Image WorldRollingSession::consumeStream(
    std::uint64_t sequence, const CanvasProducerStream &stream,
    const std::function<void(const CanvasProducer &, const render::Image &)>
        &checkpoint,
    const std::function<void(const CanvasProducer &, const render::Image &)>
        &entry) {
  try {
    if (completed_ == std::numeric_limits<std::uint64_t>::max() ||
        sequence != completed_ + 1)
      throw std::invalid_argument(
          "Rolling renderer sequence gap or exhaustion");
    assertionMs_ = 0;
    double cpuMs = 0;
    unsigned canvas = 0;
    for (const auto &c : stream.operations) {
      const auto &r = c.fields;
      const auto start = std::chrono::steady_clock::now();
      if (resident_ && world_.active() && (r[2] == 9 || r[2] == 14 || r[2] == 26))
        {} // GPU World consumes these requests; CPU knowledge was discarded once.
      else if (verify_ || !world_.active() || r[2] != 9)
        producer_.apply(c);
      cpuMs += std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now() - start)
                   .count();
      if(ownerProfile_ && r[2]<ownerCosts_.producerMs.size()){
        ownerCosts_.producerMs[r[2]]+=elapsed(start);++ownerCosts_.producerCalls[r[2]];
      }
      const auto phaseStart=ownerProfile_?Clock::now():Clock::time_point{};
      if (r[2] == 11) {
        canvas = r[3];
        if(resident_) {
          producer_.publishWrites();
          producer_.discardCpuWorld(canvas);
          world_.beginResident(c,mirror_->surface(canvas),sequence);
          if(ownerProfile_)ownerCosts_.entryMs+=elapsed(phaseStart);
          continue;
        }
        auto history = producer_.read(canvas);
        if (entry) {
          const auto assertStart = std::chrono::steady_clock::now();
          entry(c, history);
          assertionMs_ += std::chrono::duration<double, std::milli>(
                              std::chrono::steady_clock::now() - assertStart)
                              .count();
        }
        world_.beginRolling(c, history, sequence);
      } else if (world_.active()) {
        if (r[2] == 9 || r[2] == 14 || r[2] == 26){
          world_.append(c);
          if(ownerProfile_)ownerCosts_.appendMs+=elapsed(phaseStart);
        }else if (r[2] == 10 && r[3] == canvas && resident_){
          world_.completeResident();
          if(ownerProfile_)ownerCosts_.completeMs+=elapsed(phaseStart);
        }else if (r[2] == 10 && r[3] == canvas)
          producer_.commitNativeWorld(
              canvas, verify_ ? world_.complete(producer_.read(canvas))
                              : world_.completeNormal());
        else if (r[2] == 12){
          world_.end(c);
          if(ownerProfile_)ownerCosts_.returnMs+=elapsed(phaseStart);
        }
      }
      if (r[2] == 10 && checkpoint) {
        const auto assertStart = std::chrono::steady_clock::now();
        checkpoint(c, producer_.read(r[3]));
        assertionMs_ += std::chrono::duration<double, std::milli>(
                            std::chrono::steady_clock::now() - assertStart)
                            .count();
      }
    }
    if (world_.active() || world_.completedSequence() != sequence)
      throw std::runtime_error("Rolling renderer did not complete the packet");
    const auto &p = world_.profile();
    producerMs_ = cpuMs;
    nativeMs_ = cpuMs + p.adoptMs + p.prepareMs + p.submitMs + p.readbackMs +
                p.compareMs;
    auto result = resident_ ? render::Image{} : producer_.read(canvas);
    if(resident_){
      if(!firstWorldCanvas_)firstWorldCanvas_=canvas;
      if(lastWorldCanvas_&&lastWorldCanvas_!=canvas)++worldCanvasChanges_;
      lastWorldCanvas_=canvas;
    }
    completed_ = sequence;
    return result;
  } catch (...) {
    failed_ = true;
    throw;
  }
}
render::GpuFrame WorldRollingSession::consumeResident(std::uint64_t sequence,
                                                       const CanvasProducerStream &stream) {
  if(!resident_)throw std::logic_error("Session does not own resident canvases");
  render::GpuFrame frame;
  const auto start=ownerProfile_?Clock::now():Clock::time_point{};
  renderer_.batch([&] { consumeOwned(sequence,stream);
    const auto present=ownerProfile_?Clock::now():Clock::time_point{};frame=presentGpu();
    if(ownerProfile_)ownerCosts_.presentMs=elapsed(present);
  });
  if(ownerProfile_)ownerCosts_.totalMs=elapsed(start);
  return frame;
}
render::GpuFrame WorldRollingSession::replayPrepared(){
  if(failed_ || !completed_)throw std::logic_error("Prepared replay after failed/incomplete source update");
  try{return world_.replayPrepared();}catch(...){failed_=true;throw;}
}
render::Image WorldRollingSession::diagnosticReadResident(render::GlBlitter &renderer) {
  if(!resident_ || failed_ || !completed_)throw std::logic_error("No resident diagnostic frame");
  return renderer.read(mirror_->surface(world_.canvas()));
}
WorldRollingSession::PacketCacheStats
WorldRollingSession::packetCacheStats() const {
  return {decodes_,
          hits_,
          cachedBytes_.capacity(),
          cached_ ? cached_->operations.size() : 0,
          cached_ ? cached_->ownedPayloads.size() : 0,
          cached_ ? cached_->ownedPayloadBytes : 0};
}
} // namespace mnm::legacy
