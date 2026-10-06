#pragma once
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
enum class SurfaceWrapper {PartialFill,FullFill,OpaqueCopy,KeyedCopy};
enum class RecoverySurface {Source=1,Destination=2};
enum class SurfaceReloadRoute {None,Callback,CursorBitmap};
struct SurfaceRetryInput {
    SurfaceWrapper wrapper=SurfaceWrapper::OpaqueCopy;
    bool fast=false,noWait=false,keyEnabled=false,sourceReload=false,destinationReload=false;
    std::uint16_t sourceKey=0,destinationKey=0;
    std::uint32_t color=0;
    SurfaceReloadRoute sourceRoute=SurfaceReloadRoute::None,destinationRoute=SurfaceReloadRoute::None;
};
struct SurfaceRetryAdapter {
    virtual ~SurfaceRetryAdapter()=default;
    virtual std::uint32_t draw(bool fast,bool fill,std::uint32_t flags,std::uint16_t color)=0;
    virtual std::uint32_t restore(RecoverySurface)=0;
    virtual std::uint32_t setSourceKey(RecoverySurface,std::uint16_t)=0;
    virtual void reload(RecoverySurface)=0;
    virtual void report(std::uint32_t)=0;
    // The -1 route names a global resource, independent of the triggering surface.
    virtual void reloadCursorBitmap(){throw std::runtime_error("Cursor bitmap binding is not installed");}
};
struct SurfaceRetryResult {unsigned draws=0;bool returned=false,budgetExhausted=false;};
// Original rectangle wrappers can loop forever. Budget exhaustion is an explicit
// native stop, never a synthetic successful original return or HRESULT.
inline SurfaceRetryResult runSurfaceRetry(const SurfaceRetryInput& in,SurfaceRetryAdapter& adapter,unsigned budget){
    if(!budget || budget>65536)throw std::runtime_error("Invalid recovery draw budget");
    const auto route=[](SurfaceReloadRoute explicitRoute,bool callback){
        if(explicitRoute!=SurfaceReloadRoute::None && explicitRoute!=SurfaceReloadRoute::Callback &&
           explicitRoute!=SurfaceReloadRoute::CursorBitmap)throw std::runtime_error("Unknown surface reload route");
        if(callback && explicitRoute!=SurfaceReloadRoute::None)throw std::runtime_error("Ambiguous surface reload route");
        return callback?SurfaceReloadRoute::Callback:explicitRoute;
    };
    const auto sourceRoute=route(in.sourceRoute,in.sourceReload),destinationRoute=route(in.destinationRoute,in.destinationReload);
    const bool fill=in.wrapper==SurfaceWrapper::PartialFill || in.wrapper==SurfaceWrapper::FullFill;
    if(in.wrapper!=SurfaceWrapper::PartialFill && in.wrapper!=SurfaceWrapper::FullFill &&
       in.wrapper!=SurfaceWrapper::OpaqueCopy && in.wrapper!=SurfaceWrapper::KeyedCopy)
        throw std::runtime_error("Unknown original surface wrapper");
    const bool fast=in.fast && !fill,keyed=in.wrapper==SurfaceWrapper::KeyedCopy;
    const auto flags=fill?0x01000400u:((keyed?(fast?1u:0x8000u):0u)|(in.noWait?0u:fast?0x10u:0x01000000u));
    const auto recover=[&](RecoverySurface s){
        const auto h=adapter.restore(s);
        if(!h){
            if(in.keyEnabled)(void)adapter.setSourceKey(s,s==RecoverySurface::Source?in.sourceKey:in.destinationKey);
            const auto selected=s==RecoverySurface::Source?sourceRoute:destinationRoute;
            if(selected==SurfaceReloadRoute::Callback)adapter.reload(s);
            else if(selected==SurfaceReloadRoute::CursorBitmap)adapter.reloadCursorBitmap();
        }
        return h;
    };
    SurfaceRetryResult result;
    while(result.draws<budget){
        ++result.draws;const auto h=adapter.draw(fast,fill,flags,std::uint16_t(in.color));
        if(h || in.wrapper==SurfaceWrapper::FullFill)adapter.report(h);
        if(fill){
            if(h==0x887601c2u){
                const auto restored=recover(RecoverySurface::Destination);
                if(in.wrapper==SurfaceWrapper::FullFill){
                    adapter.report(restored);
                    if(result.draws==budget){result.budgetExhausted=true;return result;}
                    ++result.draws;adapter.report(adapter.draw(false,true,flags,std::uint16_t(in.color)));
                }
            }
            result.returned=true;return result;
        }
        if(!h){result.returned=true;return result;}
        if(h==0x887601c2u){recover(RecoverySurface::Source);recover(RecoverySurface::Destination);}
    }
    result.budgetExhausted=true;return result;
}
}
