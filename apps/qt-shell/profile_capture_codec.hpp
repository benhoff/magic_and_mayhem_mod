#pragma once
#include <QByteArray>
#include <QCryptographicHash>
#include <QLibrary>
#include <QString>
#include <QtEndian>
#include <zstd.h>
#include <algorithm>
#include <memory>
#include <stdexcept>

// Optional diagnostic file codec. The library is loaded only for MNMPROF4;
// older capture consumers keep their existing QtCore-only link contract.
namespace mnm::profile {
constexpr unsigned HeaderSize=80,KeyInterval=128;
constexpr qsizetype MaxBody=16*1024*1024+197184;
inline void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}

class Zstd final {
    QLibrary library_{QStringLiteral("zstd"),1};
    template<class T>T symbol(const char* name){
        auto p=library_.resolve(name);check(p!=nullptr,"Campaign source codec symbol missing");return reinterpret_cast<T>(p);
    }
public:
    decltype(&ZSTD_createCCtx) createC;
    decltype(&ZSTD_freeCCtx) freeC;
    decltype(&ZSTD_CCtx_setParameter) setC;
    decltype(&ZSTD_CCtx_refPrefix) prefixC;
    decltype(&ZSTD_compress2) compress;
    decltype(&ZSTD_compressBound) bound;
    decltype(&ZSTD_createDCtx) createD;
    decltype(&ZSTD_freeDCtx) freeD;
    decltype(&ZSTD_DCtx_setParameter) setD;
    decltype(&ZSTD_DCtx_refPrefix) prefixD;
    decltype(&ZSTD_decompressDCtx) decompress;
    decltype(&ZSTD_findFrameCompressedSize) frameSize;
    decltype(&ZSTD_getFrameContentSize) contentSize;
    decltype(&ZSTD_isError) isError;
    decltype(&ZSTD_versionNumber) version;
    Zstd(){
        check(library_.load(),"Campaign source capture requires libzstd");
        createC=symbol<decltype(createC)>("ZSTD_createCCtx");freeC=symbol<decltype(freeC)>("ZSTD_freeCCtx");
        setC=symbol<decltype(setC)>("ZSTD_CCtx_setParameter");prefixC=symbol<decltype(prefixC)>("ZSTD_CCtx_refPrefix");
        compress=symbol<decltype(compress)>("ZSTD_compress2");bound=symbol<decltype(bound)>("ZSTD_compressBound");
        createD=symbol<decltype(createD)>("ZSTD_createDCtx");freeD=symbol<decltype(freeD)>("ZSTD_freeDCtx");
        setD=symbol<decltype(setD)>("ZSTD_DCtx_setParameter");prefixD=symbol<decltype(prefixD)>("ZSTD_DCtx_refPrefix");
        decompress=symbol<decltype(decompress)>("ZSTD_decompressDCtx");frameSize=symbol<decltype(frameSize)>("ZSTD_findFrameCompressedSize");
        contentSize=symbol<decltype(contentSize)>("ZSTD_getFrameContentSize");isError=symbol<decltype(isError)>("ZSTD_isError");
        version=symbol<decltype(version)>("ZSTD_versionNumber");check(version()>=10400,"Campaign source codec version");
    }
    void valid(size_t result)const{check(!isError(result),"Campaign source codec failure");}
};

struct Encoded {QByteArray packed,digest;quint64 ordinal,base;unsigned flags;};

class Encoder final {
    Zstd api_;ZSTD_CCtx* context_=nullptr;QByteArray previous_;quint64 records_=0;
public:
    Encoder(){context_=api_.createC();check(context_,"Campaign source compressor allocation");}
    ~Encoder(){if(context_)api_.freeC(context_);}
    Encoder(const Encoder&)=delete;Encoder& operator=(const Encoder&)=delete;
    unsigned version()const{return api_.version();}
    Encoded encode(const QByteArray& body){
        check(body.size()>=16 && body.size()<=MaxBody && records_<8192,"Campaign source body bound");
        const bool key=records_%KeyInterval==0;
        api_.valid(api_.setC(context_,ZSTD_c_compressionLevel,3));
        api_.valid(api_.setC(context_,ZSTD_c_windowLog,26));
        api_.valid(api_.setC(context_,ZSTD_c_checksumFlag,1));
        api_.valid(api_.prefixC(context_,key?nullptr:previous_.constData(),key?0:size_t(previous_.size())));
        QByteArray packed(qsizetype(api_.bound(size_t(body.size()))),Qt::Uninitialized);
        auto size=api_.compress(context_,packed.data(),size_t(packed.size()),body.constData(),size_t(body.size()));api_.valid(size);packed.resize(qsizetype(size));
        Encoded result{std::move(packed),QCryptographicHash::hash(body,QCryptographicHash::Sha256),records_+1,key?0:records_,key?0u:1u};
        previous_=body;++records_;return result;
    }
};

class Decoder final {
    Zstd api_;ZSTD_DCtx* context_=nullptr;QByteArray previous_;quint64 records_=0;
public:
    Decoder(){context_=api_.createD();check(context_,"Campaign source decoder allocation");try{api_.valid(api_.setD(context_,ZSTD_d_windowLogMax,26));}catch(...){api_.freeD(context_);context_=nullptr;throw;}}
    ~Decoder(){if(context_)api_.freeD(context_);}
    Decoder(const Decoder&)=delete;Decoder& operator=(const Decoder&)=delete;
    QByteArray decode(const QByteArray& header,const QByteArray& packed,unsigned expected){
        check(header.size()==HeaderSize && expected>=16 && expected<=MaxBody && records_<8192,"Campaign source header/body bound");
        check(qFromLittleEndian<quint32>(header.constData()+24)==quint32(packed.size())
            && quint64(qFromLittleEndian<quint32>(header.constData()+16))+qFromLittleEndian<quint32>(header.constData()+20)==expected,"Campaign source declared extent");
        const auto flags=qFromLittleEndian<quint32>(header.constData()+28);
        const auto ordinal=qFromLittleEndian<quint64>(header.constData()+32),base=qFromLittleEndian<quint64>(header.constData()+40);
        const bool key=records_%KeyInterval==0;
        check(ordinal==records_+1 && flags==(key?0u:1u) && base==(key?0:records_),"Campaign source prefix identity");
        check(!packed.isEmpty() && packed.size()<=qsizetype(expected)+expected/128+1024,"Campaign source compressed extent");
        auto frame=api_.frameSize(packed.constData(),size_t(packed.size()));api_.valid(frame);
        check(frame==size_t(packed.size()) && api_.contentSize(packed.constData(),size_t(packed.size()))==expected,"Campaign source compressed frame");
        api_.valid(api_.prefixD(context_,key?nullptr:previous_.constData(),key?0:size_t(previous_.size())));
        QByteArray body(expected,Qt::Uninitialized);
        auto size=api_.decompress(context_,body.data(),expected,packed.constData(),size_t(packed.size()));api_.valid(size);
        check(size==expected && QCryptographicHash::hash(body,QCryptographicHash::Sha256)==header.mid(48,32),"Campaign source decoded hash");
        previous_=body;++records_;return body;
    }
};

inline QByteArray header(quint64 generation,quint64 chunk,unsigned source,unsigned request,const Encoded& encoded){
    QByteArray result(HeaderSize,0);
    qToLittleEndian<quint64>(generation,result.data());qToLittleEndian<quint64>(chunk,result.data()+8);
    qToLittleEndian<quint32>(source,result.data()+16);qToLittleEndian<quint32>(request,result.data()+20);
    qToLittleEndian<quint32>(quint32(encoded.packed.size()),result.data()+24);qToLittleEndian<quint32>(encoded.flags,result.data()+28);
    qToLittleEndian<quint64>(encoded.ordinal,result.data()+32);qToLittleEndian<quint64>(encoded.base,result.data()+40);
    check(encoded.digest.size()==32,"Campaign source digest extent");std::copy(encoded.digest.begin(),encoded.digest.end(),result.begin()+48);
    return result;
}
}
