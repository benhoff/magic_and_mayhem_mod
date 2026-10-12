#include "world_snapshot_queue.hpp"
#include <chrono>
#include <algorithm>
#include <stdexcept>
namespace mnm::legacy {
namespace {
using Clock=std::chrono::steady_clock;
double ms(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
CanvasProducer closing(const std::array<std::uint8_t,192>& bytes,unsigned offset){
 CanvasProducer c{};
 for(unsigned i=0;i<24;++i)c.fields[i]=mnm_ack_word(bytes.data()+offset,i);
 if(c.fields[0]!=96 || c.fields[18] || c.fields[23])throw std::invalid_argument("Snapshot closing record");
 return c;
}
}
WorldSnapshotQueue::WorldSnapshotQueue(WorldContinuousChannel& channel,bool retainWire):channel_(channel),retainWire_(retainWire){
 if(!channel.snapshot())throw std::invalid_argument("Snapshot queue requires explicit V5 owned delivery");
 owner_=std::thread([this]{capture();});
}
WorldSnapshotQueue::~WorldSnapshotQueue(){stop();}
void WorldSnapshotQueue::stop(){
 stopping_.store(true,std::memory_order_release);channel_.cancel();progress_.notify_all();
 if(owner_.joinable())owner_.join();
}
WorldSnapshotQueue::Stats WorldSnapshotQueue::stats() const{
 std::lock_guard<std::mutex> lock(mutex_);auto result=state_;result.ready=ready_.size();return result;
}
bool WorldSnapshotQueue::take(OwnedWorldSnapshot& result){
 std::lock_guard<std::mutex> lock(mutex_);
 if(!state_.error.empty())throw std::runtime_error(state_.error);
 if(ready_.empty())return false;
 if(ready_.front().generation!=taken_+1)throw std::logic_error("Snapshot take order");
 result=std::move(ready_.front());ready_.pop_front();++taken_;return true;
}
void WorldSnapshotQueue::imported(std::uint64_t generation){
 std::lock_guard<std::mutex> lock(mutex_);
 if(!state_.error.empty())throw std::runtime_error(state_.error);
 if(!generation || generation!=state_.imports+1 || generation>taken_ || !state_.outstanding)
  throw std::invalid_argument("Snapshot import identity/order");
 ++state_.imports;--state_.outstanding;progress_.notify_all();
}
void WorldSnapshotQueue::capture(){
 bool active=false;
 try{
  while(!stopping_.load(std::memory_order_acquire)){
   if(channel_.cancelled())break;
   {
    std::unique_lock<std::mutex> lock(mutex_);
    if(state_.outstanding==2){progress_.wait_for(lock,std::chrono::milliseconds(1));continue;}
   }
   if(channel_.cancelled())break;
   WorldContinuousChannel::Block block;
   if(!channel_.poll(block)){
    std::unique_lock<std::mutex> lock(mutex_);progress_.wait_for(lock,std::chrono::milliseconds(1));continue;
   }
   active=true;
   {
    std::lock_guard<std::mutex> lock(mutex_);++state_.outstanding;
    state_.highWater=std::max(state_.highWater,state_.outstanding);
   }
   OwnedWorldSnapshot snapshot;snapshot.generation=block.generation;snapshot.sourceBytes=block.source.size();
   auto start=Clock::now();auto stream=decodeCanvasProducers(block.source,false);snapshot.decodeMs=ms(start);
   start=Clock::now();snapshot.handoff=prepareRollingHandoff(stream,0,block.request,1,0,WorldBatchSourcePolicy::ownedV5,false);snapshot.prepareMs=ms(start);
   std::vector<std::uint8_t> ack(MNM_CONT_ACK_BYTES);
   mnm_snapshot_ack_encode(ack.data(),block.request.data(),unsigned(block.generation),unsigned(block.generation>>32),unsigned(channel_.nonce()),unsigned(channel_.nonce()>>32));
   channel_.reply(block.generation,ack);
   {std::lock_guard<std::mutex> lock(mutex_);++state_.accepted;}
   std::array<std::uint8_t,192> actual{};start=Clock::now();
   while(!channel_.returnRecords(block.generation,block.source,actual)){
    if(stopping_.load(std::memory_order_acquire) || ms(start)>120000.)throw std::runtime_error("Snapshot original return interrupted/deadline");
    std::unique_lock<std::mutex> lock(mutex_);progress_.wait_for(lock,std::chrono::milliseconds(1));
   }
   verifyRollingHandoffReturn(snapshot.handoff,closing(actual,0),closing(actual,96),true);
   snapshot.ticks=channel_.ticks(block.generation);
   channel_.release(block.generation);
   if(retainWire_){snapshot.wireSource=std::move(block.source);snapshot.wireRequest=std::move(block.request);}
   {
    std::lock_guard<std::mutex> lock(mutex_);++state_.returned;
    if(snapshot.generation==64)state_.firstTick=snapshot.ticks[0];
    if(snapshot.generation>=64)state_.lastTick=snapshot.ticks[3];
    state_.bytes+=snapshot.sourceBytes;ready_.push_back(std::move(snapshot));
   }
   // Block bytes can now be destroyed: queued payloads and descriptors own all
   // source storage; mapped slots may immediately be reused by the producer.
   active=false;
  }
 }catch(const std::exception& e){
  std::lock_guard<std::mutex> lock(mutex_);
  if(!stopping_.load(std::memory_order_acquire) && (active || !channel_.cancelled() || state_.outstanding))state_.error=e.what();
  channel_.cancel();
 }
 {std::lock_guard<std::mutex> lock(mutex_);state_.terminal=true;}
 progress_.notify_all();
}
}
