#include "palette_effects.hpp"
#include "terrain_selection.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
long double effectPower(double base,double power){
#if defined(__i386__) || defined(__x86_64__)
 // Positive finite branch of59bcc0/5a05e0: binary64 operands, x87
 // logarithm/exponential reduction, extended result until the caller spills.
 // libm powl can round an integer power exactly where this sequence does not.
 long double result;
 asm volatile("fldl %2; fldl %1; fyl2x; fld %%st(0); frndint; fxch; "
              "fsub %%st(1), %%st(0); f2xm1; fld1; faddp; fscale; fstp %%st(1); fstpt %0"
              : "=m"(result) : "m"(base), "m"(power)
              : "st", "st(1)", "st(2)");
 return result;
#else
 return std::pow(static_cast<long double>(base),static_cast<long double>(power));
#endif
}
int integer(long double v){
 // The original __ftol returns integer-indefinite for unordered/out-of-range.
 if(!std::isfinite(v)||v<std::numeric_limits<int>::min()||v>=2147483648.L)return std::numeric_limits<int>::min();
 return int(v);
}
void profileBounds(const PaletteEffectProfile& p){
 for(double v:{p.stoneReducer,p.stoneSpecalValue,p.stoneDarkest,p.metalReducer,p.metalSpecalValue,p.metalDarkest,p.metalBlueTint,p.metalContrast,p.redTintReducer})
  if(!std::isfinite(v)||v < -1000||v>1000)throw std::invalid_argument("Palette effect profile bounds");
}
std::array<int,3> desaturate(std::array<int,3> c){
 const long double mean=static_cast<long double>(c[0]+c[1]+c[2])*double(1./3.);
 for(auto& v:c){const long double ch=v;v=integer(ch<=mean?(mean-ch)*double(.5)+ch:ch-(ch-mean)*double(.5));}
 return c;
}
std::array<int,3> tint(std::array<int,3> c,unsigned mode,double reducer){
 const auto before=c;const long double sum=c[0]+c[1]+c[2];
 if(mode==1||mode==9){
  const auto red=sum*double(-.4),green=sum*double(-.1);const double blue=double(sum*double(.7));
  c[0]=before[0]?integer(red*(static_cast<long double>(before[0])/sum)+before[0]):0;
  c[1]=before[1]?integer((static_cast<long double>(before[1])/sum)*green+before[1]):0;
  if(before[0]||before[1])c[2]=integer((static_cast<long double>(before[2])/sum)*blue+before[2]);
 }else if(mode==2||mode==3){
  const auto half=sum*double(.5);
  c[2]=before[2]?integer(before[2]-(static_cast<long double>(before[2])/sum)*half):0;
  if(mode==2){
   c[1]=before[1]?integer(before[1]-(static_cast<long double>(before[1])/sum)*half):0;
   if(before[1]||before[2]){const auto gain=(static_cast<long double>(before[0])/sum)*half;c[0]=integer(gain+gain+before[0]);}
  }else{
   if(before[1])c[0]=integer(before[0]-(static_cast<long double>(before[0])/sum)*half);
   if(before[1]||before[2]){const auto gain=(static_cast<long double>(before[1])/sum)*half;c[1]=integer(gain+gain+before[1]);}
  }
 }else if(mode==7){
  const auto gain=sum*reducer;
  c[0]=integer((static_cast<long double>(before[0])/sum)*gain+before[0]);
  c[1]=integer(before[1]-(static_cast<long double>(before[1])/sum)*gain);
  c[2]=integer(before[2]-(static_cast<long double>(before[2])/sum)*gain);
 }
 return c;
}
}
PaletteEffectResult buildPaletteEffect(const PaletteRgb& rgb,unsigned mode,PaletteShadingConfig c,
 const PaletteEffectProfile& p,bool rgb555){
 if(mode>9||c.count<2||c.count>256||(mode!=0&&mode!=8&&(c.count&(c.count-1))))throw std::invalid_argument("Palette effect mode/count");
 profileBounds(p);PaletteEffectResult result;result.mode=mode;result.originalEntryBytes=mode>=8?4:2;result.source=rgb;
 if(mode==0||mode==8){result.palette=buildShadedPalette(rgb,c,rgb555);if(mode==8)result.palette.tables.resize(result.palette.neutral+1);return result;}
 if(!std::isfinite(c.intensityPower)||!std::isfinite(c.saturationPower)||c.intensityPower<0||c.intensityPower>1000||c.saturationPower<0||c.saturationPower>1000||c.intensityLevel<1||c.intensityLevel>1000||c.saturationLevel<1||c.saturationLevel>1000)throw std::invalid_argument("Palette effect lighting bounds");
 const double intensity=std::pow(c.intensityLevel,c.intensityPower),saturation=std::pow(c.saturationLevel,c.saturationPower);
 if(!std::isfinite(intensity)||!std::isfinite(saturation)||intensity<=0||saturation<=0)throw std::invalid_argument("Palette effect lighting powers");
 std::array<int,256> noise{};
 if(mode==4||mode==5){
  TerrainRegionRandom random(2000);const auto spec=integer(mode==4?p.stoneSpecalValue:p.metalSpecalValue);
  for(auto& v:noise)v=spec==1?1:int(std::int64_t(random.next()&0x7fffffffu)%(std::int64_t(spec)-1)+1);
 }
 if(mode==5){
  int maximum=0;for(const auto& ch:result.source)maximum=std::max(maximum,int(ch[0])+ch[1]+ch[2]);
  const long double middle=maximum*double(.5),contrast=static_cast<long double>(p.metalContrast)*double(.01);
  for(auto& ch:result.source){const int sum=int(ch[0])+ch[1]+ch[2];
   // The original equality path consumes an iteration without advancing its
   // RGB pointer. It revisits this unmodified triple for all remaining slots.
   if(sum==middle)break;
   for(unsigned channel=0;channel<3;++channel){const long double value=ch[channel];const auto delta=value*contrast;ch[channel]=std::uint8_t(std::uint32_t(integer(sum<middle?delta+value:value-delta)));}
  }
 }
 auto& out=result.palette;out.neutral=(c.count-1)/2;for(unsigned n=c.count;n<256;n*=2)++out.shift;
 const unsigned half=out.neutral;out.tables.resize(mode==9?half+1:2*half+1);
 const auto pack=[&](std::array<int,3> values){for(auto& v:values)v=std::clamp(v,0,255);return std::uint16_t(rgb555?((values[0]&248)<<7)|((values[1]&248)<<2)|(values[2]>>3):((values[0]&248)<<8)|((values[1]&252)<<3)|(values[2]>>3));};
 for(unsigned table=0;table<out.tables.size();++table){
  const auto factor=table<half?effectPower((127*half-127*table)/half,c.intensityPower):0.L;
  if(!std::isfinite(factor))throw std::invalid_argument("Palette dark power");
  for(unsigned colour=0;colour<256;++colour){const auto& bytes=result.source[colour];std::array<int,3> input{{bytes[0],bytes[1],bytes[2]}},value=input;
   if(table<half){
    const long double r=bytes[0],g=bytes[1],b=bytes[2];// Preserve the x87 addition order before the original double spill.
    // Metal starts with blue/green; the other effect builders start red/blue.
    const double luminance=double(mode==5?(b*double(.11)+g*double(.59))+r*double(.3):(r*double(.3)+b*double(.11))+g*double(.59));
    if(mode==6)value=desaturate(value);
    for(auto& v:value)v=v?integer((intensity/(static_cast<long double>(intensity)+factor))*((static_cast<long double>(luminance)-v)*(factor/(static_cast<long double>(saturation)+factor))+v)):0;
   }else if(table>half){for(unsigned channel=0;channel<3;++channel)value[channel]=input[channel]+((255-input[channel])*(table-half))/half;}
   if(mode==6&&table>=half)value=desaturate(value);
   if(mode==4||mode==5){
    const long double sum=value[0]+value[1]+value[2],mean=sum*double(1./3.);const double reducer=mode==4?p.stoneReducer:p.metalReducer,darkest=mode==4?p.stoneDarkest:p.metalDarkest;
    const auto adjustment=sum*reducer*(mean/sum);const auto gray=mode==4?mean-adjustment+noise[colour]:adjustment+mean+noise[colour];
    const auto clamped=std::min(255.L,std::max(static_cast<long double>(darkest),gray));const int word=integer(clamped);value={{word,word,mode==5?integer(std::min(255.L,static_cast<long double>(word)+p.metalBlueTint)):word}};
   }else value=tint(value,mode,p.redTintReducer);
   out.tables[table][colour]=pack(value);
  }
 }
 return result;
}
}
