#include "effect_animation_metadata.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Metadata assertion failed");}
int main(){
    require(effectAnimationFileCount("")==1 && effectAnimationFileCount("-2")==1 && effectAnimationFileCount("0")==1);
    require(effectAnimationFileCount("1")==1 && effectAnimationFileCount("+13tail")==13 && effectAnimationFileCount("4294967297")==1);
    const EffectAnimationMetadataWords initial{{7,8,9,10}};
    EffectAnimationMetadataFields fields{{"","UNKNOWN","unknown",""}};
    require(decodeEffectAnimationMetadata(fields,13,initial)==initial);
    fields={{"-1junk","eFfEcTs12","tRaNsPaReNt75","4294967297"}};
    require(decodeEffectAnimationMetadata(fields,13,initial)==EffectAnimationMetadataWords{{0xffffffffu,12,5,1}});
    auto refused=[&](const EffectAnimationMetadataFields& f,unsigned files,const EffectAnimationMetadataWords& seed){bool failed=false;try{decodeEffectAnimationMetadata(f,files,seed);}catch(const std::invalid_argument&){failed=true;}require(failed && initial==EffectAnimationMetadataWords{{7,8,9,10}});};
    refused(fields,0,initial);refused(fields,65,initial);refused(fields,13,{});refused({},13,{});
    fields[0][0]=std::string(256,'0');refused(fields,13,initial);fields[0][0]=std::string("a\0b",3);refused(fields,13,initial);
    fields[0][0]=std::string(1,char(128));refused(fields,13,initial);
}
