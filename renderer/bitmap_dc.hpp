#pragma once
#include "blit.hpp"
#include "surface_copy.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
// Owned raster context modeled on bounded Surface2 observations. This is not a
// COM HDC, global surface lock, or wire protocol. The caller supplies environment
// palette context and associates a renderer-owned raster target when reloading.
class BitmapDcState final {
public:
    using Palette=std::array<Rgb,256>;
private:
    int width_,height_;
    unsigned bits_;
    bool active_=false;
    std::optional<Palette> defaults_,binding_;
    Palette colors_{};
    std::optional<std::vector<Rect>> regions_;
    void active() const {if(!active_)throw std::runtime_error("Bitmap DC is not acquired");}
    void indexed() const {if(bits_!=8)throw std::runtime_error("Bitmap DC palette requires indexed8");}
    static void range(unsigned first,std::size_t count){
        if(!count || first>=256 || count>256-first)throw std::runtime_error("Bitmap DC palette range outside table");
    }
public:
    BitmapDcState(int width,int height,unsigned bits,std::optional<Palette> defaults=std::nullopt)
        :width_(width),height_(height),bits_(bits),defaults_(std::move(defaults)){
        if(width<1 || height<1 || width>2048 || height>2048 || (bits!=8 && bits!=16 && bits!=24 && bits!=32))
            throw std::runtime_error("Unsupported bitmap DC dimensions/format");
        if(bits!=8 && defaults_)throw std::runtime_error("RGB bitmap DC has no indexed default table");
    }
    BitmapDcState(const BitmapDcState&)=delete;
    BitmapDcState& operator=(const BitmapDcState&)=delete;
    void bindPalette(std::optional<Palette> palette){indexed();binding_=std::move(palette);}
    void updateBoundPalette(unsigned first,const std::vector<Rgb>& colors){
        indexed();range(first,colors.size());
        if(!binding_)throw std::runtime_error("Bitmap DC has no bound surface palette");
        std::copy(colors.begin(),colors.end(),binding_->begin()+first);
    }
    void acquire(){
        if(active_)throw std::runtime_error("Bitmap DC is already acquired");
        if(bits_==8){
            if(!binding_ && !defaults_)throw std::runtime_error("Missing bitmap DC default palette context");
            colors_=binding_?*binding_:*defaults_;
        }
        regions_.reset();active_=true;
    }
    void release(){active();active_=false;}
    const Palette& palette() const {active();indexed();return colors_;}
    const std::optional<std::vector<Rect>>& regions() const {active();return regions_;}
    void setDcColors(unsigned first,const std::vector<Rgb>& colors){
        active();indexed();range(first,colors.size());std::copy(colors.begin(),colors.end(),colors_.begin()+first);
    }
    void selectRegions(std::optional<std::vector<Rect>> regions){
        active();
        if(regions){
            if(regions->size()>maxClipRegions)throw std::runtime_error("Bitmap DC region budget exceeded");
            for(auto r:*regions)if(r.left<0 || r.top<0 || r.left>=r.right || r.top>=r.bottom || r.right>width_ || r.bottom>height_)
                throw std::runtime_error("Bitmap DC region outside raster target");
        }
        regions_=std::move(regions);
    }
    void reload(GlBlitter& renderer,SurfaceId target,const DibInput& dib) const {
        active();
        // This target presents DC colors, independent of the deferred binding.
        // It is an owned headless raster adapter, not live COM palette identity.
        if(bits_==8)renderer.setPalette(target,0,{colors_.begin(),colors_.end()});
        renderer.reloadDib(target,dib,regions_);
    }
};
}
