// Exercise the actual private preparer and FIFO sink without a GL context.
#include "../renderer/command_copy_diagnostics.cpp"
#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <chrono>
#include <atomic>
using namespace mnm::render;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void orderedPreparation(){
 CopyPreparers pool;detail::DiagnosticWorker sink(512,8);std::mutex mutex;std::condition_variable ready;
 bool release=false;unsigned fast=0;std::vector<unsigned> written;
 for(unsigned i=0;i<8;++i){
  check(sink.reserve(64),"Initial bounded reservation failed");
  auto future=pool.submit([&,i]{
   {std::unique_lock<std::mutex> lock(mutex);if(!i)ready.wait(lock,[&]{return release;});else{++fast;ready.notify_all();}}
   auto result=std::make_unique<PreparedCopy>();result->sequence=i;return result;
  });
  sink.post(64,[&,future]{auto result=future->get();std::lock_guard<std::mutex> lock(mutex);written.push_back(result->sequence);});
 }
 bool concurrent=false,empty=false;
 {std::unique_lock<std::mutex> lock(mutex);concurrent=ready.wait_for(lock,std::chrono::seconds(3),[&]{return fast==7;});empty=written.empty();}
 const bool held=sink.outstanding()==512&&!sink.reserve(1);std::atomic<bool> admitted{false};
 std::thread waiter([&]{admitted=sink.reserve_for(64,std::chrono::seconds(3));});
 const auto waitingUntil=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!sink.waiting()&&std::chrono::steady_clock::now()<waitingUntil)std::this_thread::sleep_for(std::chrono::milliseconds(1));
 const bool waiting=sink.waiting()==1;
 const auto began=std::chrono::steady_clock::now();check(sink.outstanding()==512,"Reservation disappeared while FIFO first task blocked");
 const auto observation=std::chrono::steady_clock::now()-began;
 {std::lock_guard<std::mutex> lock(mutex);release=true;}ready.notify_all();waiter.join();
 if(admitted)sink.cancel(64);
 sink.finish();
 check(concurrent&&empty&&held&&admitted&&waiting,"Preparation/FIFO/bounded admission control failed");
 check(observation<std::chrono::seconds(1),"Budget wait retained producer mutex");
 check(written==std::vector<unsigned>({0,1,2,3,4,5,6,7}),"Parallel CPU completion reordered FIFO writes");
 check(!sink.outstanding()&&sink.peak()<=512&&pool.peak()>=2&&pool.peak()<=8,"Owned results escaped reservation bounds");
 check(!sink.reserve_for(1,std::chrono::seconds(1)),"Closed FIFO admitted work");
}
void failedPreparation(){
 CopyPreparers pool;detail::DiagnosticWorker sink(128,2);check(sink.reserve(64)&&sink.reserve(64),"Failure reservations");
 auto failed=pool.submit([]()->std::unique_ptr<PreparedCopy>{throw std::runtime_error("first preparation failure");});
 sink.post(64,[failed]{failed->get();});
 auto next=pool.submit([]{return std::make_unique<PreparedCopy>();});bool completed=false;
 sink.post(64,[&completed,next]{next->get();completed=true;throw std::runtime_error("later failure");});sink.finish();
 check(completed&&!sink.outstanding()&&sink.error()=="first preparation failure","Failed preparation leaked ownership or replaced first failure");
}
void timedRefusal(){
 detail::DiagnosticWorker sink(64,1);check(sink.reserve(64),"Timed reservation");
 check(!sink.reserve_for(1,std::chrono::milliseconds(5)),"Exhausted budget did not refuse after bounded wait");
 sink.cancel(64);check(!sink.outstanding(),"Cancelled unpublished reservation leaked");sink.finish();
}
}
int main()try{orderedPreparation();failedPreparation();timedRefusal();std::puts("{\"success\":true,\"controls\":3,\"parallel_preparation\":true,\"FIFO_write_order\":true,\"reservation_lifetime\":true,\"first_failure\":true,\"Original_runs\":0}");return 0;}
catch(const std::exception& error){std::fprintf(stderr,"%s\n",error.what());return 1;}
