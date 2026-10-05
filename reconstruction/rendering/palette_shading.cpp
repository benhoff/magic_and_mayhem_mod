#include "palette_shading.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction {
PaletteShadingConfig applyPaletteLighting(PaletteShadingConfig config,const PaletteLightingOverrides& fields){
 if(fields.lightCurve)config.intensityLevel=std::clamp(*fields.lightCurve,1,1000);
 if(fields.colourFactor)config.saturationLevel=std::clamp(*fields.colourFactor,1,1000);
 const auto power=[](double value){if(!std::isfinite(value))throw std::invalid_argument("Nonfinite palette lighting power");return value<=0?0.:std::min(value,1000.);};
 if(fields.lightPower)config.intensityPower=power(*fields.lightPower);
 if(fields.colourPower)config.saturationPower=power(*fields.colourPower);
 return config;
}
std::size_t ShadedPalette::tableIndex(std::int32_t shade) const {
 if(shift>7 || tables.empty() || neutral>=tables.size())throw std::invalid_argument("Invalid shading tables");
 const auto divisor=std::int64_t(1)<<shift;
 // Portable arithmetic shift; the original adds one for every negative
 // shifted value when shift is nonzero, including exact negative multiples.
 auto offset=std::int64_t(shade)/divisor;
 if(shade<0 && std::int64_t(shade)%divisor) --offset;
 if(offset<0 && shift) ++offset;
 const auto index=std::int64_t(neutral)+offset;
 if(index<0 || index>=std::int64_t(tables.size()))throw std::out_of_range("Shade outside palette chain");
 return std::size_t(index);
}
ShadedPalette buildShadedPalette(const PaletteRgb& rgb,PaletteShadingConfig c,bool rgb555){
 if(c.count<2 || c.count>256 || (c.count&(c.count-1)) ||
    !std::isfinite(c.intensityPower) || !std::isfinite(c.saturationPower) ||
    c.intensityPower<0 || c.saturationPower<0 || c.intensityPower>1000 || c.saturationPower>1000 ||
    c.intensityLevel<1 || c.intensityLevel>1000 || c.saturationLevel<1 || c.saturationLevel>1000)
  throw std::invalid_argument("Unsupported palette shading configuration");
 const double intensity=std::pow(c.intensityLevel,c.intensityPower),saturation=std::pow(c.saturationLevel,c.saturationPower);
 if(!std::isfinite(intensity) || !std::isfinite(saturation) || intensity==0 || saturation==0)
  throw std::invalid_argument("Nonfinite palette shading powers");
 ShadedPalette result;result.neutral=(c.count-1)/2;result.tables.resize(c.count-1);
 for(unsigned n=c.count;n<256;n*=2)++result.shift;
 const auto pack=[&](int r,int g,int b){r=std::clamp(r,0,255);g=std::clamp(g,0,255);b=std::clamp(b,0,255);
  return std::uint16_t(rgb555?((r&248)<<7)|((g&248)<<2)|(b>>3):((r&248)<<8)|((g&252)<<3)|(b>>3));};
 const auto half=result.neutral;
 for(unsigned t=0;t<result.tables.size();++t){
  const auto factor=t<half?std::pow((long double)((127*half-127*t)/half),(long double)c.intensityPower):0.L;
  if(!std::isfinite(factor))throw std::invalid_argument("Nonfinite dark shading power");
  for(unsigned i=0;i<256;++i){const auto& p=rgb[i];std::array<int,3> colour{};
   // The x87 routine stores the weighted sum to double before channel math.
   const double luminance=double((long double)p[1]*double(.59)+(long double)p[0]*double(.3)+(long double)p[2]*double(.11));
   for(unsigned ch=0;ch<3;++ch){const auto v=p[ch];
    if(t<half){const long double blend=v+(static_cast<long double>(luminance)-v)*(factor/(factor+saturation));
     const auto scaled=blend*(intensity/(static_cast<long double>(intensity)+factor));
     colour[ch]=(t==0 || !v)?0:int(std::clamp(scaled,0.L,255.L));
    }else if(t==half)colour[ch]=v;
    else colour[ch]=v+((255-v)*(t-half))/half;
   }
   result.tables[t][i]=pack(colour[0],colour[1],colour[2]);
  }
 }
 return result;
}
}
