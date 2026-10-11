#include "resident_profile_capture.hpp"
#include <QByteArray>
#include <QElapsedTimer>
#include <QFile>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <algorithm>
struct ResidentProfileCapture::State {
    struct Job {std::uint64_t generation;unsigned source,request;QByteArray bytes;};
    QString path,error;
    mutable std::mutex mutex;std::condition_variable ready;
    std::deque<Job> jobs;std::thread worker;
    std::uint64_t limit,queued=0,peakQueued=0,accepted=0,written=0,rawBytes=8,fileBytes=8;
    double workerMs=0;bool stopping=false,opened=false,truncated=false,finished=false;
    explicit State(QString p,std::uint64_t bytes):path(std::move(p)),limit(bytes){
        if(bytes<128 || bytes>256ull*1024*1024)throw std::runtime_error("Resident capture queue bound");
        worker=std::thread([this]{run();});
        std::unique_lock<std::mutex> lock(mutex);ready.wait(lock,[&]{return opened || !error.isEmpty();});
    }
    void run(){
        try {
            QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly) || file.write("MNMPROF2",8)!=8)
                throw std::runtime_error("Cannot create compressed resident corpus");
            {std::lock_guard<std::mutex> lock(mutex);opened=true;ready.notify_all();}
            for(;;){
                Job job;
                {std::unique_lock<std::mutex> lock(mutex);ready.wait(lock,[&]{return stopping || !jobs.empty();});
                    if(jobs.empty()){break;}
                    job=std::move(jobs.front());jobs.pop_front();}
                QElapsedTimer timer;timer.start();const auto compressed=qCompress(job.bytes,1);
                if(compressed.isEmpty() || std::uint64_t(compressed.size())>std::uint64_t(job.bytes.size())+std::uint64_t(job.bytes.size())/1000+128)
                    throw std::runtime_error("Resident capture compression extent");
                {std::lock_guard<std::mutex> lock(mutex);if(20+std::uint64_t(compressed.size())>16ull*1024*1024*1024-fileBytes){truncated=true;jobs.clear();queued=0;break;}}
                QByteArray header(20,0);for(unsigned i=0;i<8;++i)header[int(i)]=char(job.generation>>(i*8));
                const unsigned fields[]={job.source,job.request,unsigned(compressed.size())};
                for(unsigned k=0;k<3;++k)for(unsigned i=0;i<4;++i)header[int(8+k*4+i)]=char(fields[k]>>(i*8));
                if(file.write(header)!=20 || file.write(compressed)!=compressed.size())throw std::runtime_error("Resident capture worker write");
                {std::lock_guard<std::mutex> lock(mutex);queued-=std::uint64_t(job.bytes.size());++written;fileBytes+=20+std::uint64_t(compressed.size());workerMs+=double(timer.nsecsElapsed())/1000000.;}
            }
            if(!file.flush())throw std::runtime_error("Resident capture worker flush");
            {std::lock_guard<std::mutex> lock(mutex);finished=true;ready.notify_all();}
        }catch(const std::exception& e){std::lock_guard<std::mutex> lock(mutex);error=e.what();jobs.clear();queued=0;finished=true;ready.notify_all();}
    }
    void finish(){ {std::lock_guard<std::mutex> lock(mutex);stopping=true;ready.notify_all();}if(worker.joinable())worker.join(); }
    ~State(){finish();}
};
ResidentProfileCapture::ResidentProfileCapture(const QString& path,std::uint64_t queueBytes):state_(std::make_unique<State>(path,queueBytes)){}
ResidentProfileCapture::~ResidentProfileCapture()=default;
bool ResidentProfileCapture::enqueue(std::uint64_t generation,const std::vector<std::uint8_t>& source,const std::vector<std::uint8_t>& request){
    auto& s=*state_;const auto size=std::uint64_t(source.size())+request.size();
    std::lock_guard<std::mutex> lock(s.mutex);
    if(s.stopping || s.truncated || !s.error.isEmpty())return false;
    if(generation!=s.accepted+1 || source.size()<16 || source.size()>16*1024*1024 || request.size()<64 || request.size()>197184){s.error="Resident capture generation/extent";return false;}
    if(generation>8192 || size+20>16ull*1024*1024*1024-s.rawBytes || size>s.limit-s.queued){s.truncated=true;return false;}
    State::Job job{generation,unsigned(source.size()),unsigned(request.size()),{}};
    job.bytes.reserve(qsizetype(size));job.bytes.append(reinterpret_cast<const char*>(source.data()),qsizetype(source.size()));job.bytes.append(reinterpret_cast<const char*>(request.data()),qsizetype(request.size()));
    s.jobs.push_back(std::move(job));++s.accepted;s.rawBytes+=size+20;s.queued+=size;s.peakQueued=std::max(s.peakQueued,s.queued);s.ready.notify_all();return true;
}
void ResidentProfileCapture::finish(){state_->finish();}
QJsonObject ResidentProfileCapture::report() const {
    const auto& s=*state_;std::lock_guard<std::mutex> lock(s.mutex);
    return {{"path",s.path},{"format",2},{"error",s.error},{"accepted_worlds",double(s.accepted)},{"written_worlds",double(s.written)},
        {"raw_bytes",double(s.rawBytes)},{"file_bytes",double(s.fileBytes)},{"queue_bytes",double(s.queued)},
        {"peak_queue_bytes",double(s.peakQueued)},{"queue_limit_bytes",double(s.limit)},{"truncated",s.truncated},
        {"finished",s.finished},{"complete",s.finished && s.error.isEmpty() && !s.truncated && s.accepted==s.written},
        {"compression_write_cpu_wall_ms",s.workerMs}};
}
