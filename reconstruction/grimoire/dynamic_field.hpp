#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace mnm::reconstruction::grimoire {
// No original addresses or storage pointers cross this recovered interface.
enum class Property {ranged,combat,health,resistance,intelligence,weight,sight,fireRange,groundSpeed,flyingSpeed,bravery,spellRange,manaCost};
struct CreatureValues {std::int32_t health=0,resistance=0,intelligence=0,weight=0,sight=0,fireRange=0,groundSpeed=0,flyingSpeed=0,bravery=0;};
struct SpellValues {std::int32_t range=0,manaCost=0;};
struct Sources {
    std::array<std::int32_t,13> maxima{}; // Original consecutive 6e981c..6e9844 plus spell maxima.
    std::vector<CreatureValues> creatures;
    std::vector<SpellValues> spells;
    std::function<std::int32_t(Property,std::uint32_t,std::uint32_t)> combatValue;
    bool highResolution=true;
    std::int32_t originX=0,originY=0;
    std::uint32_t font=0; // Caller-supplied identity; never dereferenced here.
};
struct Bar {std::string label;Property property;std::int32_t x=0,y=0,width=0,height=0,minimum=0,maximum=400,value=200;std::uint32_t font=0;};
struct Descriptor {std::optional<Bar> bar;std::size_t cursor=0;};
// Native bounds are intentional guards, not claims about legacy malformed input.
inline Descriptor decodeDynamic(std::string_view source,const Sources& env) {
    struct Reader {
        std::string_view text;std::size_t at=0;
        char peek()const{return at<text.size()?text[at]:0;}
        static bool space(char c){return c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\v'||c=='\f';}
        void spaces(){while(space(peek()))++at;}
        void expect(char c){if(peek()!=c)throw std::invalid_argument("Dynamic descriptor delimiter");++at;}
        std::int32_t number(){std::int32_t n=0;const auto begin=at;while(peek()>='0'&&peek()<='9'){n=n*10+(peek()-'0');++at;if(n>65535)throw std::invalid_argument("Dynamic number bound");}if(at==begin)throw std::invalid_argument("Dynamic number missing");return n;}
        std::string token(){auto begin=at;while(peek()&&!space(peek()))++at;if(at-begin>32||at==begin)throw std::invalid_argument("Dynamic property bound");return std::string(text.substr(begin,at-begin));}
    } r{source};
    if(source.size()>4096||source.find('\0')!=source.npos)throw std::invalid_argument("Dynamic input bound");
    if(r.peek()!='D'&&r.peek()!='d')throw std::invalid_argument("Dynamic marker missing");
    ++r.at;r.spaces();r.expect('"');auto begin=r.at;while(r.peek()&&r.peek()!='"')++r.at;
    if(r.at-begin>128)throw std::invalid_argument("Dynamic label bound");
    Bar b;b.label=source.substr(begin,r.at-begin);r.expect('"');r.spaces();auto x=r.number();std::int32_t y=0;
    if(r.peek()=='x'){++r.at;y=r.number();}r.spaces();auto width=r.number();r.expect('x');auto height=r.number();r.spaces();
    // The original consumes all six supplied colour numbers but discards them.
    for(int group=0;group<2;++group){for(int i=0;i<3;++i){(void)r.number();if(i<2)r.expect('-');}r.spaces();}
    const auto key=r.token();r.spaces();auto id=std::uint32_t(r.number());
    const auto rangeMaximum=std::max(env.maxima[6],env.maxima[7]);
    const auto sharedRangeMaximum=std::max(rangeMaximum,env.maxima[12]);
    if(key=="LRC"||key=="HTH") {
        const auto creature=id;r.expect('v');id=std::uint32_t(r.number());
        if(creature>=env.creatures.size()||id>=env.creatures.size())throw std::invalid_argument("Dynamic combat index");
        b.property=key=="LRC"?Property::ranged:Property::combat;
        if(!env.combatValue)throw std::invalid_argument("Dynamic combat backend missing");
        const auto first=env.combatValue(b.property,creature,id);
        if(b.property==Property::ranged&&!first)return {std::nullopt,r.at};
        b.value=b.property==Property::ranged?env.combatValue(b.property,creature,id):first;b.maximum=std::max(env.maxima[0],env.maxima[1]);
    }else if(key=="MANA_COST"||key=="SPELL_RANGE") {
        if(id>=env.spells.size())throw std::invalid_argument("Dynamic spell index");
        b.property=key=="MANA_COST"?Property::manaCost:Property::spellRange;
        b.value=key=="MANA_COST"?env.spells[id].manaCost:env.spells[id].range;
        b.maximum=key=="MANA_COST"?env.maxima[11]:std::int32_t(std::max(std::uint32_t(b.value),std::uint32_t(sharedRangeMaximum)));
    }else{
        if(id>=env.creatures.size())throw std::invalid_argument("Dynamic creature index");
        const auto& c=env.creatures[id];
        if(key=="HEALTH"){b.property=Property::health;b.value=c.health;b.maximum=env.maxima[2];}
        else if(key=="MAGIC_RESIST"){b.property=Property::resistance;b.value=c.resistance;b.maximum=env.maxima[3];}
        else if(key=="INTELLIGENCE"){b.property=Property::intelligence;b.value=c.intelligence;b.maximum=env.maxima[4];}
        else if(key=="WEIGHT"){b.property=Property::weight;b.value=c.weight;b.maximum=env.maxima[5];}
        else if(key=="VIEW_RANGE"){b.property=Property::sight;b.value=c.sight;b.maximum=rangeMaximum;}
        else if(key=="FIRE_RANGE"){b.property=Property::fireRange;b.value=c.fireRange;b.maximum=sharedRangeMaximum;}
        else if(key=="GROUND_SPEED"){b.property=Property::groundSpeed;b.value=c.groundSpeed;b.maximum=std::max(env.maxima[8],env.maxima[9]);}
        else if(key=="FLYING_SPEED"){b.property=Property::flyingSpeed;b.value=c.flyingSpeed;if(!b.value)return {std::nullopt,r.at};b.maximum=std::max(env.maxima[8],env.maxima[9]);}
        else if(key=="BRAVERY"){b.property=Property::bravery;b.value=c.bravery;b.maximum=env.maxima[10];}
        else throw std::invalid_argument("Unsupported dynamic property");
    }
    if(!Reader::space(r.peek()))throw std::invalid_argument("Dynamic terminator missing");
    r.spaces();
    if(!env.highResolution){x=x*8/10;y=y*8/10;width=width*8/10;height=height*8/10;}
    b.x=env.originX+x;b.y=env.originY+y;b.width=width;b.height=height;b.font=env.font;
    // Original helper returns one byte before the next input; its caller advances.
    return {b,r.at-1};
}
}
