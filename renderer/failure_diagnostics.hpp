#pragma once
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <array>
#include <algorithm>
#include <cstring>
#include <string>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <stdexcept>

namespace mnm::render {
// Always-on, owner-thread metadata only. No pixels, pointers, hashing, JSON or
// I/O on successful calls. Fixed storage bounds history even in long sessions.
// The compressed envelope survives adapters that forward only exception.what().
class DiagnosticCall final {
    struct Call {
        const char* name=nullptr;
        std::uint64_t serial=0;
        std::array<std::uint64_t,32> words{};
        unsigned count=0;
        std::array<char,160> label{};
        std::size_t payloadBytes=0;
        bool failed=false;
    };
    struct Active {Call call;const void* payload=nullptr;};
    struct Trace {
        std::array<Call,64> recent{};
        std::array<Active,16> active{};
        std::uint64_t serial=0;
        unsigned depth=0;
    };
    static Trace& trace(){static thread_local Trace value;return value;}
    static QJsonObject json(const Call& call){
        QJsonArray words;for(unsigned i=0;i<call.count;++i)words.append(QString::number(call.words[i]));
        return {{"call",call.name},{"serial",QString::number(call.serial)},{"words",words},
            {"payload_bytes",QString::number(call.payloadBytes)},{"label",QString::fromUtf8(call.label.data())},{"failed",call.failed}};
    }
    unsigned depth_=0;
    int exceptions_=0;
    std::uint64_t serial_=0;
public:
    explicit DiagnosticCall(const char* name,std::initializer_list<std::uint64_t> words={}) noexcept {
        auto& t=trace();depth_=t.depth++;exceptions_=std::uncaught_exceptions();serial_=++t.serial;
        Call call;call.name=name;call.serial=serial_;
        for(auto word:words)if(call.count<call.words.size())call.words[call.count++]=word;
        t.recent[serial_%t.recent.size()]=call;
        if(depth_<t.active.size())t.active[depth_]={call,nullptr};
    }
    template<class T,std::size_t N> void append(const std::array<T,N>& words) noexcept {
        auto& t=trace();auto& call=t.recent[serial_%t.recent.size()];
        for(auto word:words)if(call.count<call.words.size())call.words[call.count++]=std::uint64_t(word);
        if(depth_<t.active.size())t.active[depth_].call=call;
    }
    void label(const std::string& value) noexcept {
        auto& t=trace();auto& call=t.recent[serial_%t.recent.size()];
        std::memcpy(call.label.data(),value.data(),std::min(value.size(),call.label.size()-1));
        if(depth_<t.active.size())t.active[depth_].call=call;
    }
    void payload(const void* bytes,std::size_t size) noexcept {
        auto& t=trace();t.recent[serial_%t.recent.size()].payloadBytes=size;
        if(depth_<t.active.size()){t.active[depth_].call.payloadBytes=size;t.active[depth_].payload=bytes;}
    }
    ~DiagnosticCall(){
        auto& t=trace();auto& call=t.recent[serial_%t.recent.size()];
        if(call.serial==serial_)call.failed=std::uncaught_exceptions()>exceptions_;
        --t.depth;
    }
    DiagnosticCall(const DiagnosticCall&)=delete;
    DiagnosticCall& operator=(const DiagnosticCall&)=delete;
    static void reset(){trace()=Trace{};}
    static std::uint64_t serial(){return trace().serial;}
    static QJsonObject snapshot(){
        const auto& t=trace();QJsonArray recent,active;
        const auto begin=t.serial>t.recent.size()?t.serial-t.recent.size()+1:1;
        for(auto i=begin;i<=t.serial;++i)recent.append(json(t.recent[i%t.recent.size()]));
        for(unsigned i=0;i<std::min<unsigned>(t.depth,t.active.size());++i){
            const auto& a=t.active[i];auto value=json(a.call);
            // Borrowed bytes are inspected only while their scope is alive.
            // Hash the full input on failure; retain at most64 source bytes.
            if(a.payload && a.call.payloadBytes){
                const auto bytes=QByteArray::fromRawData(static_cast<const char*>(a.payload),qsizetype(a.call.payloadBytes));
                value["payload_sha256"]=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
                value["payload_prefix_hex"]=QString::fromLatin1(bytes.left(64).toHex());
            }
            active.append(value);
        }
        return {{"schema",1},{"recent_limit",64},{"active_limit",16},{"active_truncated",t.depth>t.active.size()},
            {"last_serial",QString::number(t.serial)},{"active",active},{"recent",recent}};
    }
};

inline constexpr const char* failureMarker="\nMNM_RENDER_FAILURE_V1:";
inline QJsonObject renderFailureDetails(const QString& message){
    const auto at=message.indexOf(QLatin1String(failureMarker));if(at<0)return {};
    const auto encoded=message.mid(at+int(std::char_traits<char>::length(failureMarker))).toLatin1();
    if(encoded.size()>128*1024)return {};
    const auto compressed=QByteArray::fromBase64(encoded);
    // qCompress prefixes the uncompressed byte count; bound before allocating.
    if(compressed.size()<4)return {};
    const auto* p=reinterpret_cast<const unsigned char*>(compressed.constData());
    const auto length=(std::uint32_t(p[0])<<24)|(std::uint32_t(p[1])<<16)|(std::uint32_t(p[2])<<8)|p[3];
    if(!length || length>256*1024)return {};
    const auto decoded=QJsonDocument::fromJson(qUncompress(compressed));
    return decoded.isObject() && decoded.object()["schema"].toInt()==1?decoded.object():QJsonObject{};
}
inline QString renderFailureSummary(const QString& message){
    const auto at=message.indexOf(QLatin1String(failureMarker));return at<0?message:message.left(at);
}
inline std::runtime_error renderFailure(const char* reason,QJsonObject detail={}){
    const auto message=QString::fromUtf8(reason);
    if(!renderFailureDetails(message).isEmpty())return std::runtime_error(reason);
    auto report=DiagnosticCall::snapshot();report["reason"]=message;report["detail"]=detail;
    const auto executablePath=QCoreApplication::applicationFilePath();report["executable"]=executablePath;
    QFile executable(executablePath);QCryptographicHash hash(QCryptographicHash::Sha256);
    const bool hashed=executable.open(QIODevice::ReadOnly)&&hash.addData(&executable);
    report["executable_hash_available"]=hashed;
    if(hashed)report["executable_sha256"]=QString::fromLatin1(hash.result().toHex());
    const auto bytes=QJsonDocument(report).toJson(QJsonDocument::Compact);
    return std::runtime_error((message+QLatin1String(failureMarker)+QString::fromLatin1(qCompress(bytes).toBase64())).toStdString());
}
[[noreturn]] inline void throwRenderFailure(const std::exception& error,const char* frozen=nullptr){
    const auto failure=frozen?std::runtime_error(frozen):renderFailure(error.what());
    if(dynamic_cast<const std::invalid_argument*>(&error))throw std::invalid_argument(failure.what());
    if(dynamic_cast<const std::out_of_range*>(&error))throw std::out_of_range(failure.what());
    if(dynamic_cast<const std::logic_error*>(&error))throw std::logic_error(failure.what());
    throw failure;
}

}
