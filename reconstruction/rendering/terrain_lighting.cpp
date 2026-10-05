#include "terrain_lighting.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
// Finite integer subset of the original scaled CRT hypot path (0059d646).
// Preserve double stores around extended products; integer boundaries matter.
int truncatedDistance(int a,int b){
 const auto scale=double(std::max(std::abs(a),std::abs(b)));if(scale==0)return 0;
 const double x=std::abs(a)/scale,y=std::abs(b)/scale;
 const double sum=double(static_cast<long double>(x)*x+static_cast<long double>(y)*y);
 const double root=std::sqrt(sum);
 const double distance=double(static_cast<long double>(root)*scale);
 return int(distance);
}
}
TerrainLightingConfig applyTerrainLighting(TerrainLightingConfig c,std::optional<int> ambient,std::optional<int> ramp){
 if(ambient)c.ambient=-std::clamp(*ambient,-127,127);
 if(ramp)c.ramp=std::clamp(*ramp,-127,127);
 return c;
}
TerrainLightField::TerrainLightField(unsigned width,unsigned height,unsigned layers,TerrainLightingConfig c):width_(width),height_(height),mapLayers_(layers),config_(c){
 if(!width || !height || !layers || width>128 || height>128 || layers>32 || c.ambient< -127 || c.ambient>127 || c.ramp< -127 || c.ramp>127)throw std::invalid_argument("Unsupported terrain light field configuration");
 const auto count=std::size_t(width)*height*((layers+1)/2);
 for(auto& buffer:buffers_)buffer.assign(count,-127);
 // The original truncates the first 2D distance before forming the 3D distance.
 for(unsigned n=2;n<=17;++n){auto& kernel=kernels_[n];kernel.resize(n*n*n);
  for(unsigned z=0;z<n;++z)for(unsigned y=0;y<n;++y)for(unsigned x=0;x<n;++x){
   const auto a=int(x)*c.ramp,b=int(y)*c.ramp,d=int(z)*c.ramp;
   const auto plane=truncatedDistance(a,b);
   const auto distance=truncatedDistance(plane,d);
   kernel[(z*n+y)*n+x]=std::int8_t(-std::clamp(distance+127-c.ramp*int(n),0,127));
  }
 }
}
void TerrainLightField::stamp(TerrainLightSource s){
 if(s.column>=width_ || s.row>=height_ || s.layer>=mapLayers_ || s.size<2 || s.size>17 || width_<s.size || height_<s.size)throw std::invalid_argument("Unsupported terrain light source bounds/size");
 const auto wrap=[](int v,unsigned limit){if(v<0)v+=int(limit);else if(v>=int(limit))v-=int(limit);return unsigned(v);};
 const auto n=s.size;
 for(unsigned dz=0;dz<n;++dz)for(unsigned dy=0;dy<n;++dy)for(unsigned dx=0;dx<n;++dx){
  const auto value=kernels_[n][(dz*n+dy)*n+dx];
  for(int zs:{-1,1}){const int z=int(s.layer/2)+zs*int(dz);if(z<0 || 2*z>=int(mapLayers_))continue;
   for(int ys:{-1,1})for(int xs:{-1,1}){
    const auto x=wrap(int(s.column)+xs*int(dx),width_),y=wrap(int(s.row)+ys*int(dy),height_);
    const auto index=(std::size_t(z)*height_+y)*width_+x;
    buffers_[3][index]=std::max(buffers_[3][index],value);
    buffers_[4][index]=std::max(buffers_[4][index],std::int8_t(std::min(int(value),config_.ambient)));
   }
  }
 }
}
void TerrainLightField::publish(){buffers_[0]=buffers_[3];}
std::int8_t TerrainLightField::at(unsigned x,unsigned y,unsigned z) const{
 if(x>=width_ || y>=height_ || z>=mapLayers_)throw std::out_of_range("Terrain light cell outside field");
 return buffers_[0][(std::size_t(z/2)*height_+y)*width_+x];
}
}
