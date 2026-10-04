#include "animation.hpp"
#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
AnimationResult decodeAnimation(const std::vector<std::uint8_t>& b,const AnimationLimits& l)try{
    auto fail=[](AnimationErrorCode code,std::size_t at,const char* detail)->AnimationResult{return AnimationError{code,at,detail,{}};};
    if(b.size()>l.inputBytes)return fail(AnimationErrorCode::limitExceeded,0,"ANI input limit exceeded");
    if(b.size()<44)return fail(AnimationErrorCode::malformedData,0,"ANI header is truncated");
    auto u32=[&](std::size_t at){return std::uint32_t(b[at])|(std::uint32_t(b[at+1])<<8)|(std::uint32_t(b[at+2])<<16)|(std::uint32_t(b[at+3])<<24);};
    if(u32(0)!=0x00494e41)return fail(AnimationErrorCode::invalidFormat,0,"Expected ANI signature");
    if(u32(12)!=5)return fail(AnimationErrorCode::unsupportedVersion,12,"Only ANI version 5 is supported");
    if(u32(4)!=b.size())return fail(AnimationErrorCode::malformedData,4,"ANI declared size differs");
    const auto count=u32(8),offsets=u32(20);
    if(offsets<2)return fail(AnimationErrorCode::malformedData,20,"ANI needs a sequence and terminal offset");
    if(count>l.records || offsets-1>l.sequences)return fail(AnimationErrorCode::limitExceeded,8,"ANI count limit exceeded");
    const auto base=44+std::uint64_t(offsets)*4;
    if(base+std::uint64_t(count)*44!=b.size())return fail(AnimationErrorCode::malformedData,8,"ANI record/table extent differs");
    Animation result;result.opaqueHeader=u32(16);std::copy_n(b.begin()+24,20,result.spriteName.begin());
    result.starts.reserve(offsets);result.records.reserve(count);
    for(std::uint32_t i=0;i<offsets;++i){const auto start=u32(44+std::size_t(i)*4);
        if(start>count || (i && start<=result.starts.back()))return fail(AnimationErrorCode::malformedData,44+std::size_t(i)*4,"ANI offsets must increase within records");
        result.starts.push_back(start);}
    if(result.starts.front()!=0 || result.starts.back()!=count)return fail(AnimationErrorCode::malformedData,44,"ANI terminal offsets differ from record extent");
    for(std::uint32_t i=0;i<count;++i){const auto at=std::size_t(base)+std::size_t(i)*44;const auto arg=u32(at+4);
        AnimationRecord r;r.opcode=u32(at);r.argument=static_cast<std::int32_t>(arg<=INT32_MAX?std::int64_t(arg):std::int64_t(arg)-0x100000000LL);
        for(unsigned k=0;k<9;++k){r.metadata[k]=u32(at+8+k*4);}
        result.records.push_back(r);}
    for(std::size_t i=1;i<result.starts.size();++i){const auto end=result.starts[i];
        if(result.records[end-1].opcode!=6)return fail(AnimationErrorCode::malformedData,std::size_t(base)+std::size_t(end-1)*44,"ANI sequence lacks terminal stop");}
    return result;
}catch(const std::bad_alloc&){return AnimationError{AnimationErrorCode::limitExceeded,0,"ANI allocation failed",{}};
}catch(const std::length_error&){return AnimationError{AnimationErrorCode::limitExceeded,0,"ANI allocation extent unsupported",{}};}
AnimationResult loadAnimation(AssetFile& file,const AnimationLimits& limits){
    if(limits.inputBytes>INT64_MAX)return AnimationError{AnimationErrorCode::invalidArgument,0,"ANI input cap exceeds AssetFile range",{}};
    auto read=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&read))return AnimationError{error->code==ErrorCode::limitExceeded?AnimationErrorCode::limitExceeded:AnimationErrorCode::assetInput,0,error->detail,*error};
    return decodeAnimation(std::get<std::vector<std::uint8_t>>(read),limits);
}
}
