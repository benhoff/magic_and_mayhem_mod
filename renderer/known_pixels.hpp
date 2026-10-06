#pragma once
#include "blit.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
// Native validity policy: undefined bytes are never exposed as captured output.
// The common fully-known state allocates no per-pixel map. Invalidated storage
// uses one byte per pixel, bounded by the existing surface/pixel budgets.
class KnownPixels {
    int width_=0,height_=0;
    std::size_t unknown_=0;
    std::vector<std::uint8_t> known_;
    void validate(Rect r) const {
        if(r.left<0 || r.top<0 || r.left>=r.right || r.top>=r.bottom || r.right>width_ || r.bottom>height_)
            throw std::runtime_error("Validity rectangle is out of bounds");
    }
public:
    KnownPixels()=default;
    KnownPixels(int width,int height):width_(width),height_(height){
        if(width<1 || height<1 || width>2048 || height>2048)throw std::runtime_error("Invalid validity dimensions");
    }
    void invalidate(){known_.assign(std::size_t(width_)*height_,0);unknown_=known_.size();}
    bool known(Rect r) const {
        validate(r);if(!unknown_)return true;
        for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x)
            if(!known_[std::size_t(y)*width_+x])return false;
        return true;
    }
    void require(Rect r) const {if(!known(r))throw std::runtime_error("Surface pixels are undefined; reload or overwrite required");}
    void define(Rect r){
        validate(r);if(!unknown_)return;
        for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x){
            auto& v=known_[std::size_t(y)*width_+x];if(!v){v=1;--unknown_;}
        }
        if(!unknown_)known_.clear();
    }
};
}
