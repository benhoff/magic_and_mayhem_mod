#include "palette_shading.hpp"
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool value){if(!value)throw std::runtime_error("Palette shading boundary mismatch");}
int main(){
 PaletteRgb rgb{};for(unsigned i=0;i<256;++i)rgb[i]={std::uint8_t(i),std::uint8_t(255-i),std::uint8_t(i/2)};
 const auto p=buildShadedPalette(rgb);require(p.shift==4 && p.neutral==7 && p.tables.size()==15);
 require(p.tableIndex(-16)==7 && p.tableIndex(-17)==6 && p.tableIndex(-1)==7 && p.tableIndex(16)==8);
 require(p.tableIndex(-127)==0 && p.tableIndex(127)==14);
 for(auto c:p.tables.front())require(c==0);
 for(auto c:p.tables.back())require(c==65535);
 for(auto shade:{std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}){bool refused=false;try{p.tableIndex(shade);}catch(const std::out_of_range&){refused=true;}require(refused);}
 for(unsigned count:{0,1,257,512}){bool refused=false;try{buildShadedPalette(rgb,{count,1,1,2,2});}catch(const std::invalid_argument&){refused=true;}require(refused);}
 for(unsigned count=2;count<=256;++count){const auto chain=buildShadedPalette(rgb,{count,1,1,2,2});
  require(chain.tables.size()==(count==3?2:2*((count-1)/2)+1) && chain.neutral==(count-1)/2);
  if(count&(count-1))require(chain.shift==0);
  require(chain.tableIndex(0)==chain.neutral);
 }
 const auto flat=buildShadedPalette(rgb,{2,1,1,2,2});require(flat.tableIndex(-127)==0 && flat.tableIndex(127)==0);
 // Native selector admission is checked for every signed shade around every
 // possible shift. Derive negative expected indices from positive magnitudes, independently
 // of the implementation's signed division/remainder correction.
 for(unsigned shift=0;shift<=7;++shift){
  ShadedPalette selector;selector.shift=shift;selector.neutral=130;selector.tables.resize(261);
  const int step=1<<shift;
  for(int shade=-32768;shade<=32768;++shade){
   int offset=0;
   if(!shift)offset=shade;
   else if(shade>=0)offset=shade/step;
   else offset=-((-shade-1)/step);
   const int expected=130+offset;
   bool refused=false;std::size_t actual=0;
   try{actual=selector.tableIndex(shade);}catch(const std::out_of_range&){refused=true;}
   require(refused==(expected<0 || expected>=261));
   if(!refused)require(actual==std::size_t(expected));
  }
 }
 for(unsigned invalid=0;invalid<3;++invalid){
  ShadedPalette p;p.shift=invalid==0?8:0;p.neutral=invalid==1?1:0;
  if(invalid!=2)p.tables.resize(1);
  bool refused=false;try{p.tableIndex(0);}catch(const std::invalid_argument&){refused=true;}require(refused);
 }
 for(double power:{-1.,1001.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}){
  for(bool saturation:{false,true}){PaletteShadingConfig c;if(saturation)c.saturationPower=power;else c.intensityPower=power;
   bool refused=false;try{buildShadedPalette(rgb,c);}catch(const std::invalid_argument&){refused=true;}require(refused);
  }
 }
 for(int level:{0,1001,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}){
  for(bool saturation:{false,true}){PaletteShadingConfig c;if(saturation)c.saturationLevel=level;else c.intensityLevel=level;
   bool refused=false;try{buildShadedPalette(rgb,c);}catch(const std::invalid_argument&){refused=true;}require(refused);
  }
 }

}
