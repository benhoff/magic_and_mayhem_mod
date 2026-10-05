#include "animation.hpp"

namespace mnm::assets {
AnimationResult loadAnimation(AssetFile& file,const AnimationLimits& limits){
    if(limits.inputBytes>INT64_MAX)return AnimationError{AnimationErrorCode::invalidArgument,0,"ANI input cap exceeds AssetFile range",{}};
    auto read=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&read))return AnimationError{error->code==ErrorCode::limitExceeded?AnimationErrorCode::limitExceeded:AnimationErrorCode::assetInput,0,error->detail,*error};
    return decodeAnimation(std::get<std::vector<std::uint8_t>>(read),limits);
}
}
