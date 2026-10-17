#include "command_copy_diagnostics.hpp"
#include "commands.hpp"
#include "../protocols/include/mnm/render_stream_v4.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QtEndian>
#include <set>
#include <stdexcept>
namespace mnm::render {
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
QByteArray bytes(const Image& image){
    QByteArray out;out.resize(qsizetype(image.pixels.size()*4));
    for(std::size_t i=0;i<image.pixels.size();++i)qToLittleEndian<quint32>(image.pixels[i],out.data()+i*4);
    return out;
}
QString hash(const QByteArray& b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
}
struct CommandCopyDiagnostics::Impl {
    QDir directory;QFile records;
    std::set<unsigned> ancestry;
    std::set<std::array<std::uint32_t,13>> retained;
    Image source,before;
    unsigned pending=0,seen=0,checked=0,ended=0,truncated=0,saved=0;
    std::uint64_t pixels=0,readbacks=0,readbackBytes=0;
    QString error;
    explicit Impl(const QString& path):directory(path),records(directory.filePath("copies.jsonl")){
        require(!path.isEmpty() && QDir().mkpath(path),"Cannot create copy diagnostics directory");
        require(records.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Refusing copy diagnostic overwrite");
    }
    void report(){
        records.flush();QSaveFile file(directory.filePath("report.json"));
        require(file.open(QIODevice::WriteOnly),"Cannot open copy diagnostic report");
        const QJsonObject out{{"schema",1},{"success",bool(ended && checked && !pending && !truncated && error.isEmpty())},
          {"ended",bool(ended)},{"truncated",bool(truncated)},{"error",error},{"copies_seen",qint64(seen)},{"copies_checked",qint64(checked)},
          {"retained_pixel_cases",qint64(saved)},{"pixels_compared",qint64(pixels)},{"diagnostic_native_readbacks",qint64(readbacks)},
          {"diagnostic_readback_bytes",qint64(readbackBytes)},{"original_pixels_used_as_native_inputs",false},
          {"scope","Every admitted COPY with resident World ancestry until8192copies/32GiB diagnostic readbacks. Independent CPU copy transform and complete destination comparison; no original World or presented-session equivalence."}};
        require(file.write(QJsonDocument(out).toJson())>0 && file.commit(),"Cannot commit copy diagnostic report");
    }
};
CommandCopyDiagnostics::CommandCopyDiagnostics(const QString& path):impl_(std::make_unique<Impl>(path)){}
CommandCopyDiagnostics::~CommandCopyDiagnostics(){try{impl_->report();}catch(const std::exception&) {}}
bool CommandCopyDiagnostics::before(const SurfaceCommand& c,GlBlitter& renderer,SurfaceId source,SurfaceId target){
    auto& p=*impl_;++p.seen;
    if(!p.ancestry.count(c.words[0]))return false;
    require(!p.pending,"Overlapping copy diagnostics");
    const auto s=renderer.dimensions(source),d=renderer.dimensions(target);
    const auto cost=(std::uint64_t(s.width())*s.height()+2ull*std::uint64_t(d.width())*d.height())*4;
    if(p.checked>=8192 || cost>32ull*1024*1024*1024-p.readbackBytes){p.truncated=1;return false;}
    p.source=renderer.read(source);p.before=renderer.read(target);p.readbacks+=2;
    p.readbackBytes+=(p.source.pixels.size()+p.before.pixels.size())*4;p.pending=c.sequence;return true;
}
void CommandCopyDiagnostics::after(const SurfaceCommand& c,GlBlitter& renderer,SurfaceId target){
    auto& p=*impl_;require(p.pending==c.sequence,"Copy diagnostic sequence mismatch");
    const auto actual=renderer.read(target);++p.readbacks;p.readbackBytes+=actual.pixels.size()*4;
    // Only after GPU execution and its output capture, compute an independent
    // CPU expectation. No expected image can influence that execution.
    auto expected=p.before;const auto& w=c.words;
    require(actual.width==expected.width && actual.height==expected.height,"Copy diagnostic destination changed");
    for(unsigned y=0;y<w[5]-w[3];++y)for(unsigned x=0;x<w[4]-w[2];++x){
        const auto pixel=p.source.pixels.at(std::size_t(y+w[3])*p.source.width+x+w[2]);
        if(!w[8] || pixel!=w[9])expected.pixels.at(std::size_t(y+w[7])*expected.width+x+w[6])=pixel;
    }
    std::uint64_t mismatches=0;
    for(std::size_t i=0;i<actual.pixels.size();++i)if(actual.pixels[i]!=expected.pixels.at(i))++mismatches;
    const auto s=bytes(p.source),b=bytes(p.before),a=bytes(actual),e=bytes(expected);
    QJsonArray words;for(auto word:c.words)words.append(qint64(word));
    QJsonObject row{{"sequence",int(c.sequence)},{"words",words},{"source_width",p.source.width},{"source_height",p.source.height},
      {"target_width",actual.width},{"target_height",actual.height},{"pixels",qint64(actual.pixels.size())},
      {"mismatches",qint64(mismatches)},{"source_sha256",hash(s)},{"before_sha256",hash(b)},{"actual_sha256",hash(a)},{"expected_sha256",hash(e)}};
    const bool retain=mismatches || (p.retained.size()<16 && p.retained.insert(c.words).second);
    if(retain){
        const auto base=QString("copy-%1").arg(c.sequence,8,16,QChar('0'));QJsonObject files;
        for(const auto& item:std::array<std::pair<const char*,QByteArray>,4>{{{"source",s},{"before",b},{"actual",a},{"expected",e}}}){
            const auto name=base+"-"+item.first+".u32";QFile file(p.directory.filePath(name));
            require(file.open(QIODevice::WriteOnly|QIODevice::NewOnly) && file.write(item.second)==item.second.size(),"Copy diagnostic pixel write failed");files[item.first]=name;
        }
        row["files"]=files;++p.saved;
    }
    const auto record=QJsonDocument(row).toJson(QJsonDocument::Compact)+'\n';
    require(p.records.write(record)==record.size(),"Copy diagnostic record write failed");
    ++p.checked;p.pixels+=actual.pixels.size();p.pending=0;p.source={};p.before={};
    if(mismatches){p.error="Independent complete destination copy mismatch";p.report();throw std::runtime_error(p.error.toStdString());}
}
void CommandCopyDiagnostics::committed(const SurfaceCommand& c){
    auto& p=*impl_;const auto& w=c.words;
    if(c.operation==MNM_RENDER_STREAM_V4_OPERATION_WORLD)p.ancestry.insert(w[0]);
    else if(c.operation==3 && p.ancestry.count(w[0]))p.ancestry.insert(w[1]);
    else if(c.operation==7)p.ancestry.erase(w[0]);
    else if(c.operation==11){const auto a=p.ancestry.count(w[0]),b=p.ancestry.count(w[1]);p.ancestry.erase(w[0]);p.ancestry.erase(w[1]);if(a)p.ancestry.insert(w[1]);if(b)p.ancestry.insert(w[0]);}
    else if(c.operation==8){p.ended=1;p.report();}
}
}
