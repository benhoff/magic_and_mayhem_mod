#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace mnm::reconstruction {
struct FinalizerRectangle { std::uint32_t left,top,right,bottom; };
struct WorldFinalizerState {
    bool enabled{},ready{},blueBackground{},largeVideo{};
    std::uint32_t lineHeight{},rowCount{};
    std::vector<std::uint32_t> widths,locations;
    std::vector<FinalizerRectangle> rectangles;
};
// 4d8ab0: callbacks supply owned fill/map/font/unmap services. The original
// postlude's redundant Unlock and its ignored result are retained explicitly.
// Admission of owned metadata precedes effects; malformed original pointers
// and out-of-bounds arrays have no portable execution semantics to emulate.
template<class Backend>
void finalizeWorld(WorldFinalizerState& state,std::int32_t value,Backend&& backend) {
    if(!state.enabled||!state.ready)return;
    if(state.rowCount<2||state.rowCount-2>=state.widths.size()||
       state.rowCount-2>=state.locations.size()||state.rowCount-2>=state.rectangles.size())
        throw std::invalid_argument("Unowned finalizer metadata extent");
    const auto index=state.rowCount-2;
    FinalizerRectangle rectangle{0,state.lineHeight*index,404,state.lineHeight*(index+1)};
    backend.fill(rectangle,state.blueBackground?31:0);
    if(!backend.map())return;
    backend.bind();backend.clip(rectangle);
    const auto width=backend.font(std::string("Sprites printed: ")+std::to_string(value),rectangle);
    state.widths[index]=100;
    backend.unmap();
    rectangle.right=(width+3u)&~3u;
    state.rectangles[index]=rectangle;
    state.locations[index]=(((state.largeVideo?800u:640u)/2-(width>>1))+3u)&~3u;
}
}
