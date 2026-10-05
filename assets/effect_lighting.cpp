#include "effect_lighting.hpp"
#include <stdexcept>
namespace mnm::assets {
EffectLightingFields readEffectLightingFields(const ProfileSnapshot& profile){
    EffectLightingFields fields;
    const std::array<const char*,3> keys{"LightSourceDiameter","LightSourceAffected","ClippedToHeight"};
    for(unsigned i=0;i<fields.values.size();++i)
        for(unsigned k=0;k<keys.size();++k)
            fields.values[i][k]=profile.value("FXA_"+std::to_string(i),keys[k],256);
    return fields;
}
PersistenceResult<EffectLightingFields> loadEffectLightingFields(AssetFile& file,const PersistenceLimits& limits){
    auto packed=loadPackedContainer(file,limits);
    if(auto* error=std::get_if<PersistenceError>(&packed))return *error;
    try{return readEffectLightingFields(ProfileSnapshot::parse(std::get<PackedContainer>(packed).decoded));}
    catch(const std::invalid_argument& error){return PersistenceError{PersistenceErrorCode::malformedData,0,error.what(),{}};}
}
}
