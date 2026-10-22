#pragma once
#include "canvas_release.hpp"

namespace mnm::reconstruction {
struct OriginalDrawingSession {
    std::uint32_t draw1{},draw2{},draw4{},clipper{},fullscreen{};
    CanvasRegistry canvases;
};
// Original 58acf0 rereads globals across external calls. A callback can change
// later cleanup decisions and interfaces; successful return is not a proof of
// physical retirement or of every external COM credit being released.
template<class Provider,class Resolve>
void releaseOriginalDrawingSession(OriginalDrawingSession& session,Provider& provider,Resolve&& resolve){
    if(!session.draw2)return;
    while(session.canvases.entries[0]){
        const auto token=session.canvases.entries[0];
        if(!resolve(token).words[2])throw std::invalid_argument("Original shutdown would make no registry progress");
        releaseCanvas(token,resolve(token),session.canvases,
            [&](ReleaseOperation operation,std::uint32_t interface){return provider.surface(operation,interface);},resolve);
    }
    if(!session.fullscreen){
        if(session.clipper){provider.release(session.clipper);session.clipper=0;}
        else goto devices;
        if(!session.fullscreen)goto devices;
    }
    provider.restore(session.draw2);session.fullscreen=0;
devices:
    provider.release(session.draw1);provider.release(session.draw2);provider.release(session.draw4);
    session.draw4=session.draw2=session.draw1=0;session.canvases.primary=0;
}
} // namespace mnm::reconstruction
