#include "command_copy_diagnostics.hpp"
#include "commands.hpp"
#include "diagnostic_worker.hpp"
#include "../protocols/include/mnm/render_stream_v4.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QElapsedTimer>
#include <QtEndian>
#include <set>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <future>
#include <atomic>
namespace mnm::render {
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
QByteArray bytes(const Image& image){
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    // The image owns these words until the worker hash/file writes finish.
    return QByteArray::fromRawData(reinterpret_cast<const char*>(image.pixels.data()),qsizetype(image.pixels.size()*4));
#else
    QByteArray out;out.resize(qsizetype(image.pixels.size()*4));
    for(std::size_t i=0;i<image.pixels.size();++i)qToLittleEndian<quint32>(image.pixels[i],out.data()+i*4);
    return out;
#endif
}
QString hash(const QByteArray& b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
}
namespace detail {
// Pure CPU results never touch the renderer, journal, retention sets or counters.
// The existing FIFO sink owns their reservation through the final write.
struct PreparedCopy {
    unsigned sequence=0;std::array<std::uint32_t,13> words{};bool ancestry=false;
    std::array<std::uint32_t,16> input{};Image source,before,actual,expected;
    QString sourceHash,beforeHash,actualHash,expectedHash;
    std::uint64_t mismatches=0;double referenceMs=0,encodingHashMs=0;
};
std::unique_ptr<PreparedCopy> prepareCopy(unsigned sequence,const std::array<std::uint32_t,13>& words,
        bool ancestry,const std::array<std::uint32_t,16>& input,Image source,Image before,Image actual){
    auto value=std::make_unique<PreparedCopy>();auto& p=*value;
    p.sequence=sequence;p.words=words;p.ancestry=ancestry;p.input=input;
    p.source=std::move(source);p.before=std::move(before);p.actual=std::move(actual);
    QElapsedTimer timing;timing.start();p.expected=p.before;const auto& w=p.words;
    require(p.actual.width==p.expected.width && p.actual.height==p.expected.height,"Copy diagnostic destination changed");
    for(unsigned y=0;y<w[5]-w[3];++y)for(unsigned x=0;x<w[4]-w[2];++x){
        const auto pixel=p.source.pixels.at(std::size_t(y+w[3])*p.source.width+x+w[2]);
        if(!w[8] || pixel!=w[9])p.expected.pixels.at(std::size_t(y+w[7])*p.expected.width+x+w[6])=pixel;
    }
    for(std::size_t i=0;i<p.actual.pixels.size();++i)
        if(p.actual.pixels[i]!=p.expected.pixels.at(i))++p.mismatches;
    p.referenceMs=timing.nsecsElapsed()/1000000.;timing.restart();
    // Share digests only after exact complete canonical-word equality. All
    // four named fields and independent expected/actual mismatch bytes remain.
    p.actualHash=hash(bytes(p.actual));
    const bool beforeEqual=p.before.width==p.actual.width && p.before.height==p.actual.height && p.before.pixels==p.actual.pixels;
    p.beforeHash=beforeEqual?p.actualHash:hash(bytes(p.before));
    const bool sourceActual=p.source.width==p.actual.width && p.source.height==p.actual.height && p.source.pixels==p.actual.pixels;
    const bool sourceBefore=!sourceActual && p.source.width==p.before.width && p.source.height==p.before.height && p.source.pixels==p.before.pixels;
    p.sourceHash=sourceActual?p.actualHash:sourceBefore?p.beforeHash:hash(bytes(p.source));
    p.expectedHash=p.mismatches?hash(bytes(p.expected)):p.actualHash;
    p.encodingHashMs=timing.nsecsElapsed()/1000000.;return value;
}
class CopyPreparers final {
    using Result=std::unique_ptr<PreparedCopy>;using Task=std::packaged_task<Result()>;
public:
    static constexpr unsigned Count=8;
    CopyPreparers(){
        try{for(unsigned i=0;i<Count;++i)threads_.emplace_back([this]{
            for(;;){Task task;
                {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stopping_||!tasks_.empty();});
                    if(tasks_.empty())return;
                    task=std::move(tasks_.front());tasks_.pop_front();}
                const auto active=++active_;auto old=peak_.load();while(old<active&&!peak_.compare_exchange_weak(old,active)){}
                task();--active_;
            }
        });}catch(...){stop();throw;}
    }
    ~CopyPreparers(){stop();}
    template<class F> std::shared_ptr<std::future<Result>> submit(F&& work){
        Task task(std::forward<F>(work));auto result=std::make_shared<std::future<Result>>(task.get_future());
        {std::lock_guard<std::mutex> lock(mutex_);require(!stopping_,"COPY preparers stopped");tasks_.push_back(std::move(task));}
        wake_.notify_one();return result;
    }
    unsigned peak()const{return peak_.load();}
private:
    void stop(){
        {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}wake_.notify_all();
        for(auto& thread:threads_)if(thread.joinable())thread.join();
    }
    std::mutex mutex_;std::condition_variable wake_;bool stopping_=false;
    std::deque<Task> tasks_;std::vector<std::thread> threads_;std::atomic<unsigned> active_{0},peak_{0};
};

}
using detail::PreparedCopy;
using detail::CopyPreparers;
using detail::prepareCopy;
struct CommandCopyDiagnostics::Impl {
    using Input=std::array<std::uint32_t,16>;
    QDir directory;QFile records;
    std::set<unsigned> ancestry;
    Selection selection;std::array<unsigned,4> retainedClasses{};
    std::set<std::array<std::uint32_t,13>> retained;
    std::set<Input> inputs;
    std::map<unsigned,unsigned> births;
    std::array<unsigned,4> inputClasses{};
    Input pendingInput{};
    std::uint64_t duplicateSkipped=0,classSkipped=0,byteSkipped=0,inputBytes=0;
    static constexpr unsigned InputsPerClass=8;
    static constexpr std::uint64_t InputByteLimit=256ull*1024*1024;
    Image source,before;
    unsigned pending=0,seen=0,enqueued=0,checked=0,ended=0,truncated=0,saved=0;
    std::size_t pendingCharge=0;bool finalized=false;
    unsigned rowRefusals=0,readbackRefusals=0,workerRefusals=0;
    std::uint64_t admissionWaitNs=0,maxAdmissionWaitNs=0;
    std::uint64_t pixels=0,readbacks=0,readbackBytes=0;
    double readbackMs=0,referenceMs=0,encodingHashMs=0,writeMs=0;
    QString error;
    static constexpr std::size_t QueueLimit=512ull*1024*1024;
    std::unique_ptr<CopyPreparers> preparers;
    detail::DiagnosticWorker worker{QueueLimit,256};
    void process(std::unique_ptr<PreparedCopy>);
    void checkError(){const auto failure=worker.error();if(!failure.empty())throw std::runtime_error(failure);}
    explicit Impl(const QString& path,Selection choose):directory(path),records(directory.filePath("copies.jsonl")),selection(choose){
        require(!path.isEmpty() && QDir().mkpath(path),"Cannot create copy diagnostics directory");
        require(records.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Refusing copy diagnostic overwrite");
        if(selection==Selection::AllCopies)preparers=std::make_unique<CopyPreparers>();
    }
    void report(){
        records.flush();QSaveFile file(directory.filePath("report.json"));
        require(file.open(QIODevice::WriteOnly),"Cannot open copy diagnostic report");
        QJsonObject out{{"schema",1},{"selection",selection==Selection::DistinctInputs?"distinct-copy-inputs":selection==Selection::AllCopies?"all-copies":"world-ancestry"},{"success",bool(ended && checked && enqueued==checked && !pending && !truncated && error.isEmpty() && !worker.outstanding())},
          {"ended",bool(ended)},{"truncated",bool(truncated)},{"error",error},{"copies_seen",qint64(seen)},{"copies_checked",qint64(checked)},{"copies_enqueued",qint64(enqueued)},
          {"proof_processing","bounded-fifo-worker"},{"worker_queue_limit_bytes",qint64(QueueLimit)},{"worker_peak_bytes",qint64(worker.peak())},
          {"retained_pixel_cases",qint64(saved)},{"pixels_compared",qint64(pixels)},{"diagnostic_native_readbacks",qint64(readbacks)},
          {"diagnostic_readback_bytes",qint64(readbackBytes)},{"original_pixels_used_as_native_inputs",false},
          {"cpu_readback_ms",readbackMs},{"cpu_reference_compare_ms",referenceMs},
          {"cpu_encoding_hash_ms",encodingHashMs},{"cpu_witness_write_ms",writeMs},{"timing_scope","Readbacks on GL owner; bounded parallel CPU comparison/hashing and one FIFO witness writer. Per-job elapsed sums can overlap; no GPU busy-time, production FPS or scanout claim"},
          {"preparation_workers",preparers?int(CopyPreparers::Count):1},{"preparation_peak_jobs",preparers?int(preparers->peak()):1},
          {"worker_admission_wait_limit_ms",selection==Selection::AllCopies?50:0},{"worker_admission_wait_ns",qint64(admissionWaitNs)},
          {"worker_admission_wait_ns_max",qint64(maxAdmissionWaitNs)},{"copy_limit_refusals",int(rowRefusals)},
          {"readback_limit_refusals",int(readbackRefusals)},{"worker_reservation_refusals",int(workerRefusals)},
          {"scope","Selected native COPY commands until8192copies/32GiB diagnostic readbacks. Selection is explicit; independent CPU transform and complete destination comparison. No original World or presented-session equivalence."}};
        if(selection==Selection::DistinctInputs){
            out["copies_skipped_duplicate"]=qint64(duplicateSkipped);out["copies_skipped_class_budget"]=qint64(classSkipped);
            out["copies_skipped_byte_budget"]=qint64(byteSkipped);out["selected_input_signatures"]=qint64(inputs.size());
            out["input_limit_per_class"]=int(InputsPerClass);out["retained_plane_byte_limit"]=qint64(InputByteLimit);
            out["selected_plane_bytes"]=qint64(inputBytes);
            out["scope"]="At most8distinct COPY word/source-lifetime/target-lifetime/ancestry signatures per ancestry/key class and256MiB full retained planes. Every selected source/prior/actual/expected plane is compared and retained. Duplicates and budget exclusions still draw normally and receive no output credit; no whole-run or repeated-value proof.";
        }
        require(file.write(QJsonDocument(out).toJson())>0 && file.commit(),"Cannot commit copy diagnostic report");
    }
};
CommandCopyDiagnostics::CommandCopyDiagnostics(const QString& path,Selection selection):impl_(std::make_unique<Impl>(path,selection)){}
CommandCopyDiagnostics::~CommandCopyDiagnostics(){try{finish();}catch(const std::exception&) {}}
void CommandCopyDiagnostics::finish(){
    auto& p=*impl_;if(p.finalized)return;p.worker.finish();p.finalized=true;
    const auto failure=p.worker.error();if(p.error.isEmpty() && !failure.empty())p.error=QString::fromStdString(failure);
    p.report();p.checkError();
}
bool CommandCopyDiagnostics::before(const SurfaceCommand& c,GlBlitter& renderer,SurfaceId source,SurfaceId target){
    auto& p=*impl_;require(!p.finalized,"Copy diagnostics already finalized");p.checkError();++p.seen;
    if(p.selection==Selection::WorldAncestry && !p.ancestry.count(c.words[0]))return false;
    Impl::Input input{};const bool distinct=p.selection==Selection::DistinctInputs;
    const bool ancestry=p.ancestry.count(c.words[0]);const unsigned bucket=(ancestry?2u:0u)+(c.words[8]?1u:0u);
    if(distinct){
        std::copy(c.words.begin(),c.words.end(),input.begin());
        input[13]=p.births.at(c.words[0]);input[14]=p.births.at(c.words[1]);input[15]=ancestry;
        if(p.inputs.count(input)){++p.duplicateSkipped;return false;}
        if(p.inputClasses[bucket]>=Impl::InputsPerClass){++p.classSkipped;return false;}
    }
    require(!p.pending,"Overlapping copy diagnostics");
    const auto s=renderer.dimensions(source),d=renderer.dimensions(target);
    const auto cost=(std::uint64_t(s.width())*s.height()+2ull*std::uint64_t(d.width())*d.height())*4;
    const auto planeBytes=cost+std::uint64_t(d.width())*d.height()*4;
    if(distinct && planeBytes>Impl::InputByteLimit-p.inputBytes){++p.byteSkipped;return false;}
    if(p.enqueued>=8192){++p.rowRefusals;p.truncated=1;return false;}
    if(cost>32ull*1024*1024*1024-p.readbackBytes){++p.readbackRefusals;p.truncated=1;return false;}
    p.pendingCharge=std::size_t(cost+std::uint64_t(d.width())*d.height()*4+65536);
    QElapsedTimer admission;admission.start();
    const auto admitted=p.worker.reserve_for(p.pendingCharge,std::chrono::milliseconds(p.selection==Selection::AllCopies?50:0));
    const auto elapsed=std::uint64_t(admission.nsecsElapsed());p.admissionWaitNs+=elapsed;p.maxAdmissionWaitNs=std::max(p.maxAdmissionWaitNs,elapsed);
    if(!admitted){++p.workerRefusals;p.checkError();p.truncated=1;return false;}
    if(distinct){p.inputs.insert(input);++p.inputClasses[bucket];p.inputBytes+=planeBytes;p.pendingInput=input;}
    QElapsedTimer timing;timing.start();
    p.source=renderer.read(source);p.before=renderer.read(target);p.readbacks+=2;
    p.readbackMs+=timing.nsecsElapsed()/1000000.;
    p.readbackBytes+=(p.source.pixels.size()+p.before.pixels.size())*4;p.pending=c.sequence;return true;
}
void CommandCopyDiagnostics::after(const SurfaceCommand& c,GlBlitter& renderer,SurfaceId target){
    auto& p=*impl_;require(!p.finalized,"Copy diagnostics already finalized");require(p.pending==c.sequence,"Copy diagnostic sequence mismatch");
    QElapsedTimer timing;timing.start();
    auto actual=renderer.read(target);++p.readbacks;p.readbackBytes+=actual.pixels.size()*4;
    p.readbackMs+=timing.nsecsElapsed()/1000000.;timing.restart();

    const bool ancestry=p.ancestry.count(c.words[0]);
    auto prepare=[sequence=c.sequence,words=c.words,ancestry,input=p.pendingInput,source=std::move(p.source),before=std::move(p.before),actual=std::move(actual)]()mutable{
        return prepareCopy(sequence,words,ancestry,input,std::move(source),std::move(before),std::move(actual));
    };
    if(p.preparers){
        std::shared_ptr<std::future<std::unique_ptr<PreparedCopy>>> future;
        try{future=p.preparers->submit(std::move(prepare));}
        catch(...){p.worker.cancel(p.pendingCharge);p.pending=0;p.pendingCharge=0;throw;}
        p.worker.post(p.pendingCharge,[&p,future]{p.process(future->get());});
    }else p.worker.post(p.pendingCharge,[&p,prepare=std::move(prepare)]()mutable{p.process(prepare());});
    ++p.enqueued;p.pending=0;p.pendingCharge=0;
}
void CommandCopyDiagnostics::Impl::process(std::unique_ptr<PreparedCopy> prepared){
    auto& p=*this;auto& value=*prepared;const auto sequence=value.sequence;const auto& words=value.words;
    const auto ancestry=value.ancestry;const auto& input=value.input;const auto mismatches=value.mismatches;
    const auto& source=value.source;const auto& before=value.before;const auto& actual=value.actual;const auto& expected=value.expected;
    const auto s=bytes(source),b=bytes(before),a=bytes(actual),e=mismatches?bytes(expected):a;
    const auto& sourceHash=value.sourceHash;const auto& beforeHash=value.beforeHash;
    const auto& actualHash=value.actualHash;const auto& expectedHash=value.expectedHash;
    p.referenceMs+=value.referenceMs;p.encodingHashMs+=value.encodingHashMs;
    QElapsedTimer timing;timing.start();
    QJsonArray encodedWords;for(auto word:words)encodedWords.append(qint64(word));
    QJsonObject row{{"sequence",int(sequence)},{"source_world_ancestry",bool(ancestry)},{"words",encodedWords},{"source_width",source.width},{"source_height",source.height},
      {"target_width",actual.width},{"target_height",actual.height},{"pixels",qint64(actual.pixels.size())},
      {"mismatches",qint64(mismatches)},{"source_sha256",sourceHash},{"before_sha256",beforeHash},{"actual_sha256",actualHash},{"expected_sha256",expectedHash}};
    const unsigned bucket=(ancestry?2u:0u)+(words[8]?1u:0u);
    const bool retain=mismatches || p.selection==Selection::DistinctInputs || (p.selection==Selection::AllCopies?
        (p.retainedClasses[bucket]<4 && p.retained.insert(words).second):
        (p.retained.size()<16 && p.retained.insert(words).second));
    if(retain && !mismatches)++p.retainedClasses[bucket];
    if(p.selection==Selection::DistinctInputs){
        row["source_create_sequence"]=qint64(input[13]);row["target_create_sequence"]=qint64(input[14]);
        require(bool(input[15])==ancestry,"Diagnostic selection ancestry changed");
    }
    if(retain){
        const auto base=QString("copy-%1").arg(sequence,8,16,QChar('0'));QJsonObject files;
        for(const auto& item:std::array<std::pair<const char*,QByteArray>,4>{{{"source",s},{"before",b},{"actual",a},{"expected",e}}}){
            const auto name=base+"-"+item.first+".u32";QFile file(p.directory.filePath(name));
            require(file.open(QIODevice::WriteOnly|QIODevice::NewOnly) && file.write(item.second)==item.second.size(),"Copy diagnostic pixel write failed");files[item.first]=name;
        }
        row["files"]=files;++p.saved;
    }
    const auto record=QJsonDocument(row).toJson(QJsonDocument::Compact)+'\n';
    require(p.records.write(record)==record.size(),"Copy diagnostic record write failed");
    p.writeMs+=timing.nsecsElapsed()/1000000.;
    ++p.checked;p.pixels+=actual.pixels.size();
    if(mismatches){p.error="Independent complete destination copy mismatch";throw std::runtime_error(p.error.toStdString());}
}
void CommandCopyDiagnostics::committed(const SurfaceCommand& c){
    auto& p=*impl_;require(!p.finalized,"Copy diagnostics already finalized");p.checkError();const auto& w=c.words;
    if(p.selection==Selection::DistinctInputs){if(c.operation==1)p.births[w[0]]=c.sequence;else if(c.operation==7)p.births.erase(w[0]);}
    if(c.operation==MNM_RENDER_STREAM_V4_OPERATION_WORLD)p.ancestry.insert(w[0]);
    else if(c.operation==3 && p.ancestry.count(w[0]))p.ancestry.insert(w[1]);
    else if(c.operation==7)p.ancestry.erase(w[0]);
    else if(c.operation==11){const auto a=p.ancestry.count(w[0]),b=p.ancestry.count(w[1]);p.ancestry.erase(w[0]);p.ancestry.erase(w[1]);if(a)p.ancestry.insert(w[1]);if(b)p.ancestry.insert(w[0]);}
    else if(c.operation==8)p.ended=1;
}
}
