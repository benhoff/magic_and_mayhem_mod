#include "palette_lighting.hpp"
#include "persistence_internal.hpp"
#include <charconv>
#include <cmath>
#include <string_view>
namespace mnm::assets {
namespace {
template<class T> void parseField(const Config& config,const char* section,const char* key,std::optional<T>& output){if(const auto* text=config.find(section,key)){
   std::string_view number=*text;number=number.substr(0,number.find(';'));
   const auto first=number.find_first_not_of(" \t\r\n");number=first==number.npos?std::string_view(text->data(),0):number.substr(first,number.find_last_not_of(" \t\r\n")-first+1);
   T value{};auto begin=number.data(),end=begin+number.size();if(begin!=end && *begin=='+')++begin;
   const auto parsed=std::from_chars(begin,end,value);
   if(parsed.ec!=std::errc{} || parsed.ptr!=end || begin==end || !std::isfinite(double(value)))
    persistence_detail::fail(PersistenceErrorCode::malformedData,0,std::string("Invalid palette configuration field: ")+key);
   output=value;
  }}

}
PersistenceResult<TerrainPalettePreference> decodeTerrainPalettePreference(const Config& config){
 return persistence_detail::guarded<TerrainPalettePreference>([&]{TerrainPalettePreference result;parseField(config,"VIDEO","TerrainLightLevels",result.lightLevels);return result;});
}
PersistenceResult<TerrainPalettePreference> loadTerrainPalettePreference(AssetFile& file,const PersistenceLimits& limits){
 auto config=loadConfig(file,false,limits);if(auto* error=std::get_if<PersistenceError>(&config))return *error;
 return decodeTerrainPalettePreference(std::get<Config>(config));
}
PersistenceResult<PaletteLightingFields> decodePaletteLightingFields(const Config& config){
 return persistence_detail::guarded<PaletteLightingFields>([&]{
  PaletteLightingFields result;
  parseField(config,"GLOBAL_OPTIONS","LightCurve",result.lightCurve);parseField(config,"GLOBAL_OPTIONS","ColourFactor",result.colourFactor);parseField(config,"GLOBAL_OPTIONS","LightPower",result.lightPower);parseField(config,"GLOBAL_OPTIONS","ColourPower",result.colourPower);return result;
 });
}
PersistenceResult<PaletteLightingFields> loadPaletteLightingFields(AssetFile& file,bool packed,const PersistenceLimits& limits){
 auto config=loadConfig(file,packed,limits);if(auto* error=std::get_if<PersistenceError>(&config))return *error;
 return decodePaletteLightingFields(std::get<Config>(config));
}
}
