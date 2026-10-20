#include "palette_effects.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool b){if(!b)throw std::runtime_error("Palette effect unit mismatch");}
int main()try{
 PaletteRgb rgb{};rgb[1]={255,0,0};rgb[2]={0,255,0};rgb[3]={0,0,255};rgb[4]={10,20,30};const auto input=rgb;
 for(unsigned mode=0;mode<10;++mode){const auto a=buildPaletteEffect(rgb,mode,{16,1,1,2,2}),b=buildPaletteEffect(rgb,mode,{16,1,1,2,2});check(a.palette.tables==b.palette.tables&&a.source==b.source);check(a.palette.neutral==7&&a.palette.shift==4&&a.palette.tables.size()==(mode>=8?8u:15u));check(a.originalEntryBytes==(mode>=8?4u:2u));}
 check(rgb==input);
 check(buildPaletteEffect(rgb,2,{2,1,1,2,2}).palette.tables[0][1]==0xf800);
 check(buildPaletteEffect(rgb,3,{2,1,1,2,2}).palette.tables[0][1]==0xf800);
 check(buildPaletteEffect(rgb,1,{2,1,1,2,2}).palette.tables[0][3]==0x001f);
 for(unsigned count=2;count<=256;++count)for(unsigned mode:{0u,8u}){auto p=buildPaletteEffect(rgb,mode,{count,1,1,2,2});check(!p.palette.tables.empty());}
 PaletteEffectProfile metal;metal.metalContrast=.5;auto changed=buildPaletteEffect(rgb,5,{2,1,1,2,2},metal);check(changed.source!=input&&rgb==input);
 unsigned refused=0;
 for(unsigned mode=1;mode<10;++mode)if(mode!=8){try{(void)buildPaletteEffect(rgb,mode,{17,1,1,2,2});}catch(const std::invalid_argument&){++refused;}}
 check(refused==8);
 for(unsigned mode=0;mode<10;++mode){auto bad=metal;bad.redTintReducer=std::numeric_limits<double>::quiet_NaN();bool rejected=false;try{(void)buildPaletteEffect(rgb,mode,{},bad);}catch(const std::invalid_argument&){rejected=true;}check(rejected);}
 std::cout<<"Palette effect unit policies passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
