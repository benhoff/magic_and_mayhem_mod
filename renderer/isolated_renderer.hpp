#pragma once
#include "isolated_wire.hpp"
#include <QThread>
#include <sys/types.h>
namespace mnm::render {
class GraphicsFailure:public std::runtime_error {public:using std::runtime_error::runtime_error;};
// Host-owned committed state; no GL calls or pointers on this side of the pipe.
class IsolatedRenderer final {
public:
    IsolatedRenderer();
    ~IsolatedRenderer();
    static bool enabled();
    QByteArray call(const QByteArray&,bool mutation=false);
    void restart();
    void checkpoint();
    std::uint64_t recoveries() const{return recoveries_;}
    void faultNext(unsigned mode){fault_=mode;}
private:
    void start();
    void retire();
    void restore();
    QByteArray exchange(const QByteArray&);
    QThread* owner_=QThread::currentThread();
    pid_t pid_=-1;
    int input_=-1,output_=-1,timeout_=1500;
    bool software_=false;
    unsigned fault_=0;
    std::uint64_t recoveries_=0,journalBytes_=0;
    QByteArray checkpoint_;
    std::vector<QByteArray> journal_;
};
}
