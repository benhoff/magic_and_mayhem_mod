#include "palette_effect_chain.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool b){if(!b)throw std::runtime_error("Palette chain unit mismatch");}
int main()try{
 PaletteRgb rgb{};rgb[1]={255,255,255};rgb[2]={100,120,140};rgb[3]={20,40,60};
 PaletteEffectChain chain({rgb,rgb});check(chain.paletteCount()==2&&chain.ordinals()==0);
 check(chain.append(0)==0);const auto old=chain.colours(0,0,0);auto before=chain.source(0);
 PaletteEffectProfile metal;metal.metalContrast=200;check(chain.append(5,{},metal)==1);check(chain.source(0)!=before);check(chain.append(0)==2);
 auto expected=buildPaletteEffect(chain.source(0),0);check(chain.colours(2,0,0)==expected.palette.tables.at(expected.palette.neutral));check(chain.colours(0,0,0)==old);
 const auto count=chain.ordinals();const auto bytes=chain.retainedBytes();before=chain.source(0);
 check(!chain.append(99,{1,0,0,0,0}));check(chain.ordinals()==count&&chain.retainedBytes()==bytes);
 bool refused=false;try{chain.append(2,{17,1,1,2,2});}catch(const std::invalid_argument&){refused=true;}check(refused&&chain.ordinals()==count&&chain.source(0)==before);
 auto bad=metal;bad.stoneReducer=std::numeric_limits<double>::quiet_NaN();refused=false;try{chain.append(5,{},bad);}catch(const std::invalid_argument&){refused=true;}check(refused&&chain.source(0)==before&&chain.ordinals()==count);
 PaletteEffectChain one({rgb},{1,1024*1024});check(one.append(0)==0);refused=false;try{one.append(0);}catch(const std::length_error&){refused=true;}check(refused&&one.ordinals()==1);
 PaletteEffectChain small({rgb,rgb},{8,8192});before=small.source(0);refused=false;try{small.append(5,{},metal);}catch(const std::length_error&){refused=true;}check(refused&&small.ordinals()==0&&small.source(0)==before);
 PaletteEffectChain empty({});check(!empty.append(5,{1,0,0,0,0},bad)&&empty.ordinals()==0);
 for(auto call:{0,1,2}){refused=false;try{if(call==0)(void)chain.colours(99,0,0);else if(call==1)(void)chain.colours(0,99,0);else (void)chain.colours(0,0,10000);}catch(const std::out_of_range&){refused=true;}check(refused);}
 // Exact-middle branch stalls the original RGB pointer before all later colours.
 PaletteRgb stalled{};stalled[0]={255,255,0};stalled[1]={255,0,0};stalled[2]={100,120,140};
 auto changed=buildPaletteEffect(stalled,5,{},metal);check(changed.source[0]!=stalled[0]&&changed.source[1]==stalled[1]&&changed.source[2]==stalled[2]);
 // Mode8 has deliberately retained a larger builder allocation than its
 // visible wing. A deep copy may shrink it and must charge its own capacity.
 PaletteEffectChain retained({rgb});const auto metadata=retained.retainedBytes();retained.append(8);
 PaletteEffectChain copied=retained;
 auto actual=metadata+sizeof(ShadedPalette)+copied.tableSet(0,0).tables.capacity()*sizeof(PaletteWords);
 check(copied.retainedBytes()==actual&&copied.colours(0,0,0)==retained.colours(0,0,0));
 for(unsigned i=1;i<64;++i){auto staged=copied;check(staged.append(0,{2,1,1,2,2})==i);copied=std::move(staged);
  actual=metadata;for(unsigned n=0;n<=i;++n)actual+=sizeof(ShadedPalette)+copied.tableSet(n,0).tables.capacity()*sizeof(PaletteWords);
  check(copied.retainedBytes()==actual);
 }
 refused=false;try{copied.append(0,{2,1,1,2,2});}catch(const std::length_error&){refused=true;}check(refused&&copied.ordinals()==64);
 std::cout<<"Palette chain ordering, no-op, propagation and atomic budgets passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
