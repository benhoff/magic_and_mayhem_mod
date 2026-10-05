#include "palette_shading.hpp"
#include <cassert>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
int main(){
 PaletteRgb rgb{};for(unsigned i=0;i<256;++i)rgb[i]={std::uint8_t(i),std::uint8_t(255-i),std::uint8_t(i/2)};
 const auto p=buildShadedPalette(rgb);assert(p.shift==4 && p.neutral==7 && p.tables.size()==15);
 assert(p.tableIndex(-16)==7 && p.tableIndex(-17)==6 && p.tableIndex(-1)==7 && p.tableIndex(16)==8);
 assert(p.tableIndex(-127)==0 && p.tableIndex(127)==14);
 for(auto c:p.tables.front())assert(c==0);for(auto c:p.tables.back())assert(c==65535);
 for(auto shade:{std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}){bool refused=false;try{p.tableIndex(shade);}catch(const std::out_of_range&){refused=true;}assert(refused);}
 for(unsigned count:{0,1,3,17,512}){bool refused=false;try{buildShadedPalette(rgb,{count,1,1,2,2});}catch(const std::invalid_argument&){refused=true;}assert(refused);}
 const auto flat=buildShadedPalette(rgb,{2,1,1,2,2});assert(flat.tableIndex(-127)==0 && flat.tableIndex(127)==0);
}
