#include "../renderer/diagnostic_worker.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <vector>
using mnm::render::detail::DiagnosticWorker;
void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){try{
    std::promise<void> entered,release;auto gate=release.get_future().share();
    DiagnosticWorker worker(32,2);std::vector<int> order;
    struct Unblock{std::promise<void>& p;~Unblock(){try{p.set_value();}catch(const std::future_error&){}}} unblock{release};
    require(worker.reserve(16),"Initial reservation refused");
    worker.post(16,[&]{entered.set_value();gate.wait();order.push_back(1);});
    entered.get_future().wait();const auto begin=std::chrono::steady_clock::now();
    require(worker.reserve(16),"Second job refused");worker.post(16,[&]{order.push_back(2);});
    require(!worker.reserve(1),"In-flight bytes escaped queue accounting");
    require(std::chrono::steady_clock::now()-begin<std::chrono::milliseconds(50),"Blocked sink held the producer");
    release.set_value();worker.finish();worker.finish();
    require(order==std::vector<int>({1,2}),"FIFO lost work/order");
    require(worker.peak()==32 && worker.outstanding()==0,"Queue accounting did not drain");
    require(!worker.reserve(1),"Closed queue admitted work");
    DiagnosticWorker failed(16,1);require(failed.reserve(16),"Failure reservation");
    failed.post(16,[]{throw std::runtime_error("controlled sink failure");});failed.finish();
    require(failed.error()=="controlled sink failure" && failed.outstanding()==0,"Sink failure was lost");
    DiagnosticWorker orphan(16,1);require(orphan.reserve(16),"Pending reservation");
    bool bad=false;try{orphan.post(8,[]{});}catch(const std::logic_error&){bad=true;}
    require(bad,"Mismatched reservation accepted");orphan.finish();
    require(orphan.outstanding()==16,"Unposted reservation hidden");
    std::cout<<"{\"success\":true,\"blocked_sink_nonblocking\":true,\"fifo\":true,\"failure_retained\":true}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
