#include "sprite_visibility.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::uint32_t word(const std::vector<std::uint8_t>& b,std::size_t at){
    if(at>b.size() || b.size()-at<4)throw std::invalid_argument("Truncated SPR visibility plane");
    return std::uint32_t(b[at]) | std::uint32_t(b[at+1])<<8 | std::uint32_t(b[at+2])<<16 | std::uint32_t(b[at+3])<<24;
}
std::uint32_t reverse(std::uint32_t a){return (a>>24)|((a>>8)&0xff00u)|((a<<8)&0xff0000u)|(a<<24);}
std::int32_t bits(std::uint32_t a){std::int32_t b;std::memcpy(&b,&a,4);return b;}
}
std::optional<SpriteVisibilityShape> decodeSpriteVisibility(const std::vector<std::uint8_t>& cover,const std::vector<std::uint8_t>& test,std::int32_t x,std::int32_t y){
    if(test.empty())return {};
    if(cover.size()<4)throw std::invalid_argument("Missing SPR visibility header");
    const auto n=cover[1];if(cover.size()<4+std::size_t(n)*4 || test.size()<std::size_t(n)*4)throw std::invalid_argument("Short SPR visibility rows");
    SpriteVisibilityShape shape;shape.originX=bits(std::uint32_t(x)+cover[2]);shape.originY=bits(std::uint32_t(y)+cover[3]);
    for(unsigned i=0;i<n;++i){shape.coverRows.push_back(word(cover,4+i*4));shape.testRows.push_back(word(test,i*4));}
    return shape;
}
void SpriteVisibilityGrid::clear(){std::fill(data_.begin(),data_.end(),0);}
void SpriteVisibilityGrid::seed(std::vector<std::uint8_t> bytes){if(bytes.size()!=data_.size())throw std::invalid_argument("Visibility seed size");data_=std::move(bytes);}
bool SpriteVisibilityGrid::testAndCover(const SpriteVisibilityEntry& entry){
    if(entry.kind==8 || entry.kind==9 || !entry.shape || entry.kind==1)return false;
    const auto& shape=*entry.shape;const auto n=shape.testRows.size();
    if(n>255 || n!=shape.coverRows.size())throw std::invalid_argument("Visibility row extent");
    const auto x=bits(std::uint32_t(entry.x)-std::uint32_t(shape.originX));
    const auto y=bits(std::uint32_t(entry.y)-std::uint32_t(shape.originY));
    const int width=expanded_?800:640,height=expanded_?632:512;
    if(x< -32 || x>=width+32 || y< -32 || y>=height)return false;
    const auto column=(2*y+x)/8+24,row=(2*y-x+width+7)/8+24;
    const auto byte=column/8;const auto shift=unsigned(column%8)&31u;
    if(row<0 || byte<0 || byte+4>int(stride) || std::size_t(row)+n>rows)throw std::out_of_range("Visibility footprint outside recovered grid");
    bool hidden=true;
    for(std::size_t i=0;i<n;++i){const auto at=(std::size_t(row)+i)*stride+std::size_t(byte);const auto mask=reverse(shape.testRows[i]>>shift);
        if((word(data_,at)&mask)!=mask){hidden=false;break;}}
    if(entry.kind==0 || entry.kind==33)for(std::size_t i=0;i<n;++i){
        const auto at=(std::size_t(row)+i)*stride+std::size_t(byte);const auto value=word(data_,at)|reverse(shape.coverRows[i]>>shift);
        for(unsigned b=0;b<4;++b)data_[at+b]=std::uint8_t(value>>(b*8));
    }
    return hidden;
}
void applySpriteVisibility(std::vector<SpriteVisibilityEntry>& entries,SpriteVisibilityGrid& grid,std::vector<VisibilityOwner>& owners){
    if(entries.size()>65536)throw std::length_error("Visibility queue limit");
    for(const auto& entry:entries)if(entry.owner && *entry.owner>=owners.size())throw std::out_of_range("Visibility owner index");
    for(std::size_t i=entries.size();i>1;){auto& entry=entries[--i];const bool hidden=grid.testAndCover(entry);
        if(hidden)entry.kind=-2;
        if(!entry.owner)continue;
        auto& owner=owners[*entry.owner];const auto id=entry.spriteIdentity;
        if(id==owner.body){if(hidden)owner.flags10|=0x40;else owner.flags10&=0xffbf;}
        else if(id==owner.first){if(hidden)owner.flags8|=4;else owner.flags8&=0xfffb;}
        else if(id==owner.second){if(hidden)owner.flags8|=0x8000;else owner.flags8&=0x7fff;}
    }
}
}
