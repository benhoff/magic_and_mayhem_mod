#include "palette_lighting.hpp"
#include "persistence_internal.hpp"
#include <charconv>
#include <cmath>
#include <string_view>
namespace mnm::assets {
PersistenceResult<PaletteLightingFields> decodePaletteLightingFields(const Config& config){
 return persistence_detail::guarded<PaletteLightingFields>([&]{
  PaletteLightingFields result;
  const auto parse=[&](const char* key,auto& output){if(const auto* text=config.find("GLOBAL_OPTIONS",key)){
   std::string_view number=*text;number=number.substr(0,number.find(';'));
   const auto first=number.find_first_not_of(" \t\r\n");number=first==number.npos?std::string_view(text->data(),0):number.substr(first,number.find_last_not_of(" \t\r\n")-first+1);
   using T=typename std::decay_t<decltype(output)>::value_type;T value{};auto begin=number.data(),end=begin+number.size();if(begin!=end && *begin=='+')++begin;
   const auto parsed=std::from_chars(begin,end,value);
   if(parsed.ec!=std::errc{} || parsed.ptr!=end || begin==end || !std::isfinite(double(value)))
    persistence_detail::fail(PersistenceErrorCode::malformedData,0,std::string("Invalid palette lighting field: ")+key);
   output=value;
  }};
  parse("LightCurve",result.lightCurve);parse("ColourFactor",result.colourFactor);parse("LightPower",result.lightPower);parse("ColourPower",result.colourPower);return result;
 });
}
PersistenceResult<PaletteLightingFields> loadPaletteLightingFields(AssetFile& file,bool packed,const PersistenceLimits& limits){
 auto config=loadConfig(file,packed,limits);if(auto* error=std::get_if<PersistenceError>(&config))return *error;
 return decodePaletteLightingFields(std::get<Config>(config));
}
}
