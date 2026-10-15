#pragma once
#include "../../protocols/include/mnm/canvas_producers_v4.h"
#include <cmath>
#include <cstdint>
namespace mnm::legacy {
inline bool sceneDeclarationNumberAdmitted(double value,unsigned version,unsigned minimum=1){
 const unsigned maximum=version==4?MNM_PRODUCER_V4_MAX_RECORDS:262144u;
 return std::isfinite(value)&&value>=minimum&&value<=maximum&&value==std::uint32_t(value);
}
}
