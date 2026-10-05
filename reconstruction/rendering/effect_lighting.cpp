#include "effect_lighting.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::int64_t originalAtoi(const std::string& text){
    std::size_t i=0;
    while(i<text.size() && (text[i]==' ' || (text[i]>='\t' && text[i]<='\r')))++i;
    bool negative=false;
    if(i<text.size() && (text[i]=='+' || text[i]=='-'))negative=text[i++]=='-';
    std::uint32_t value=0;
    while(i<text.size() && text[i]>='0' && text[i]<='9')value=value*10+unsigned(text[i++]-'0');
    if(negative)value=0u-value;
    return value<=0x7fffffffu?std::int64_t(value):std::int64_t(value)-0x100000000ll;
}
bool isTrue(std::string value){
    for(auto& c:value)if(c>='A' && c<='Z')c=char(c-'A'+'a');
    return value=="true";
}
}
void EffectLightingTable::reload(const Fields& fields){
    auto candidate=entries_;
    for(unsigned i=0;i<fields.size();++i){
        for(const auto& value:fields[i])if(value.size()>255 || value.find('\0')!=value.npos)
            throw std::invalid_argument("Effect lighting requires bounded profile strings");
        const auto& values=fields[i];auto& entry=candidate[i];
        if(!values[0].empty())entry.diameter=unsigned(std::clamp(originalAtoi(values[0]),std::int64_t(0),std::int64_t(33)));
        if(isTrue(values[1]))entry.affected=1;
        if(isTrue(values[2]))entry.clippedToHeight=1;
    }
    entries_=candidate;
}
unsigned EffectLightingTable::lightIndex(unsigned type) const{
    if(type>=entries_.size())throw std::out_of_range("Effect type exceeds FXA_0..FXA_88");
    return entries_[type].diameter;
}
TerrainLightObject EffectLightingTable::source(unsigned type,TerrainLightObject positions) const{
    positions.lightIndex=lightIndex(type);return positions;
}
}
