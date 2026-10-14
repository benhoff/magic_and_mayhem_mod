#pragma once
#include <cstdint>
#include <stdexcept>
// Only limits heavy diagnostic/status construction. Never waits, advances a
// render clock, suppresses output leases or consumes source/command work.
class NativeWorkerStatusGate {
public:
    static constexpr std::int64_t PeriodUs=16000;
    explicit NativeWorkerStatusGate(bool everyTurn=false):everyTurn_(everyTurn){}
    bool due(std::int64_t now,bool immediate){
        if(initialized_&&now<last_)throw std::logic_error("Status publication clock regressed");
        if(everyTurn_||!initialized_||immediate||now-last_>=PeriodUs){last_=now;initialized_=true;return true;}
        return false;
    }
private:
    bool everyTurn_,initialized_=false;std::int64_t last_=0;
};
