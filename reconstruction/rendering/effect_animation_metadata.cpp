#include "effect_animation_metadata.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void bounded(const std::string& text){
    if(text.size()>255)throw std::invalid_argument("Effect metadata profile extent");
    for(unsigned char c:text)if(c==0 || c>=128)throw std::invalid_argument("Effect metadata ASCII profile boundary");
}
std::uint32_t atoiWord(const std::string& text){
    bounded(text);std::size_t at=0;
    while(at<text.size() && (text[at]==' ' || (text[at]>='\t' && text[at]<='\r')))++at;
    bool negative=false;if(at<text.size() && (text[at]=='+' || text[at]=='-'))negative=text[at++]=='-';
    std::uint32_t n=0;while(at<text.size() && text[at]>='0' && text[at]<='9')n=n*10+unsigned(text[at++]-'0');
    return negative?0u-n:n;
}
std::string upper(std::string s){for(auto& c:s)if(c>='a' && c<='z')c=char(c-'a'+'A');return s;}
}
std::uint32_t effectAnimationFileCount(const std::string& value){
    const auto n=atoiWord(value);return n>1 && n<=0x7fffffffu?n:1;
}
EffectAnimationMetadataWords decodeEffectAnimationMetadata(const EffectAnimationMetadataFields& fields,std::uint32_t files,const EffectAnimationMetadataWords& initial){
    if(files<1 || files>64 || fields.empty() || fields.size()>4096 || fields.size()!=initial.size())
        throw std::invalid_argument("Effect metadata fixture capacity");
    auto result=initial;
    for(std::size_t i=0;i<fields.size();++i){
        for(const auto& s:fields[i])bounded(s);
        const auto& f=fields[i];auto& r=result[i];
        if(!f[0].empty())r[0]=atoiWord(f[0]);
        const auto reference=upper(f[1]);
        for(std::uint32_t j=0;j<files;++j)if(reference=="EFFECTS"+std::to_string(j))r[1]=j;
        const auto printer=upper(f[2]);
        if(printer=="NORMAL")r[2]=31;
        if(printer=="TRANSPARENT")r[2]=2;
        if(printer=="TRANSPARENT25")r[2]=3;
        if(printer=="TRANSPARENT50")r[2]=4;
        if(printer=="TRANSPARENT75")r[2]=5;
        if(!f[3].empty())r[3]=atoiWord(f[3]);
    }
    return result;
}
}
