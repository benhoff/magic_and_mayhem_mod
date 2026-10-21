#include "palette_binding.hpp"
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string_view>
namespace mnm::scene {
reconstruction::PaletteEffectProfile paletteEffectProfile(const assets::ProfileSnapshot& profile){
 reconstruction::PaletteEffectProfile result;
 const char* sections[]={"STONE_EFFECT","STONE_EFFECT","STONE_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","REDTINT_EFFECT"};
 const char* keys[]={"Reducer","SpecalValue","Darkest","Reducer","SpecalValue","Darkest","BlueTint","Contrast","Reducer"};
 double* fields[]={&result.stoneReducer,&result.stoneSpecalValue,&result.stoneDarkest,&result.metalReducer,&result.metalSpecalValue,&result.metalDarkest,&result.metalBlueTint,&result.metalContrast,&result.redTintReducer};
 for(unsigned i=0;i<9;++i){const auto owned=profile.value(sections[i],keys[i],256);if(owned.empty())continue;
  std::string_view text=owned;const auto start=text.find_first_not_of(" \t\r\n\v\f");text=start==text.npos?text.substr(text.size()):text.substr(start);if(!text.empty()&&text.front()=='+')text.remove_prefix(1);
  double value=0;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);
  if(parsed.ec==std::errc::invalid_argument)value=0;
  else if(parsed.ec!=std::errc{}||!std::isfinite(value)||value< -1000||value>1000)throw std::invalid_argument(std::string("Palette effect profile bounds: ")+sections[i]+"/"+keys[i]);
  *fields[i]=value;
 }
 return result;
}
std::vector<reconstruction::PaletteRgb> paletteSources(const assets::Sprite& sprite){
 if(sprite.storage!=assets::SpriteStorage::indexed8||sprite.palettes.empty()||sprite.palettes.size()>4)throw std::invalid_argument("Palette effects require1..4 indexed sprite palettes");
 std::vector<reconstruction::PaletteRgb> result(sprite.palettes.size());
 for(std::size_t p=0;p<result.size();++p)for(unsigned c=0;c<256;++c){const auto& colour=sprite.palettes[p][c];result[p][c]={{colour.red,colour.green,colour.blue}};}
 return result;
}
}
