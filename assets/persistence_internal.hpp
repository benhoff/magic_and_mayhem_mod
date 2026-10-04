#pragma once
#include "persistence.hpp"
#include <algorithm>
#include <new>
#include <stdexcept>

namespace mnm::assets::persistence_detail {
[[noreturn]] inline void fail(PersistenceErrorCode code, std::size_t offset, std::string detail) {
    throw PersistenceError{code,offset,std::move(detail),std::nullopt};
}
inline void cap(std::size_t size, std::uint32_t limit, std::size_t offset=0) {
    if (size>limit) fail(PersistenceErrorCode::limitExceeded,offset,"configured size limit exceeded");
}
inline std::uint32_t word(const Bytes& b, std::size_t p) {
    if(p>b.size() || b.size()-p<4) fail(PersistenceErrorCode::malformedData,p,"truncated DWORD");
    return std::uint32_t(b[p]) | (std::uint32_t(b[p+1])<<8) | (std::uint32_t(b[p+2])<<16) | (std::uint32_t(b[p+3])<<24);
}
inline std::int32_t signedWord(const Bytes& b,std::size_t p) {
    const auto v=word(b,p);
    return v<=0x7fffffffU ? static_cast<std::int32_t>(v) : static_cast<std::int32_t>(static_cast<std::int64_t>(v)-0x100000000LL);
}
struct Cursor {
    const Bytes& b; std::size_t p=0, diagnosticBase=0;
    void require(std::size_t n) const { if(p>b.size() || n>b.size()-p) fail(PersistenceErrorCode::malformedData,diagnosticBase+p,"truncated block"); }
    Bytes take(std::size_t n) { require(n); Bytes r(b.begin()+p,b.begin()+p+n); p+=n; return r; }
    std::uint32_t u32() {require(4);auto v=word(b,p);p+=4;return v;}
    std::uint8_t u8() {require(1);return b[p++];}
};
inline std::string fixedString(const Bytes& b,std::size_t p,std::size_t n) {
    if(p>b.size() || n>b.size()-p) fail(PersistenceErrorCode::malformedData,p,"truncated string");
    auto start=b.begin()+p, end=std::find(start,start+n,0);
    return std::string(start,end); // Bounded even without a NUL; raw bytes remain available.
}
template<class T,class F> PersistenceResult<T> guarded(F f) {
    try { return f(); }
    catch(const PersistenceError& e) {return e;}
    catch(const std::bad_alloc&) {return PersistenceError{PersistenceErrorCode::limitExceeded,0,"allocation failed",std::nullopt};}
    catch(const std::length_error&) {return PersistenceError{PersistenceErrorCode::limitExceeded,0,"allocation size exceeded",std::nullopt};}
}
template<class T,class F> PersistenceResult<T> load(AssetFile& file,const PersistenceLimits& limits,F f) {
    auto bytes=readWhole(file,limits.inputBytes);
    if(auto* e=std::get_if<Error>(&bytes)) return PersistenceError{PersistenceErrorCode::assetInput,0,"asset read failed",*e};
    return f(std::get<Bytes>(bytes));
}
}
