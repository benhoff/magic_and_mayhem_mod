#include "world_frame.hpp"
#include "../../protocols/include/mnm/world_frame_v1.h"
#include <cstring>
#include <stdexcept>
namespace mnm::legacy {
namespace {
std::uint32_t word(const QByteArray& b,qsizetype at){if(at<0||at>b.size()-4)throw std::invalid_argument("Truncated World input");const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+at);return p[0]|std::uint32_t(p[1])<<8|std::uint32_t(p[2])<<16|std::uint32_t(p[3])<<24;}
std::int32_t signedWord(std::uint32_t w){std::int32_t v;std::memcpy(&v,&w,4);return v;}
}
std::optional<std::uint32_t> decodeWorldRefusal(const QByteArray& b){
    if(b.size()<MNM_WORLD_HEADER)throw std::invalid_argument("Truncated World input envelope");
    const auto failure=word(b,40);if(!failure)return {};
    if(b.size()!=MNM_WORLD_HEADER||b.first(8)!=MNM_WORLD_MAGIC||word(b,8)!=1||word(b,12)!=MNM_WORLD_HEADER||word(b,16)!=MNM_WORLD_HEADER||
       !word(b,20)||word(b,36)||word(b,44)!=MNM_WORLD_BUILD||word(b,72)||
       (failure!=MNM_WORLD_UNSUPPORTED_WAVE&&failure!=MNM_WORLD_UNSUPPORTED_KIND))throw std::invalid_argument("Malformed/unsupported World refusal envelope");
    const auto width=word(b,24),height=word(b,28),stride=word(b,32);
    if(!width||width>2048||!height||height>2048||stride<width||stride>4096)throw std::invalid_argument("Invalid World refusal dimensions");
    return failure;
}
WorldFrame decodeWorldFrame(const QByteArray& b){
    if(b.size()<MNM_WORLD_HEADER||b.size()>MNM_WORLD_MAX_BYTES||b.first(8)!=MNM_WORLD_MAGIC||word(b,8)!=1||word(b,12)!=MNM_WORLD_HEADER||word(b,16)!=std::uint32_t(b.size())||!word(b,20)||word(b,40)||word(b,44)!=MNM_WORLD_BUILD||word(b,72))throw std::invalid_argument("Unsupported/refused World input envelope");
    WorldFrame frame;frame.sequence=word(b,20);frame.width=word(b,24);frame.height=word(b,28);frame.stride=word(b,32);
    const auto count=word(b,36);if(!frame.width||frame.width>2048||!frame.height||frame.height>2048||frame.stride<frame.width||frame.stride>4096||!count||count>MNM_WORLD_MAX_DRAWS)throw std::invalid_argument("Invalid World dimensions/draw count");
    qsizetype at=MNM_WORLD_HEADER;
    for(std::uint32_t i=0;i<count;++i){
        const auto size=word(b,at),op=word(b,at+4),encoded=word(b,at+32),indexed=word(b,at+36),payload=word(b,at+40);
        if(size!=MNM_WORLD_RECORD+encoded+payload||encoded<40||encoded>MNM_WORLD_MAX_FRAME||indexed>1||op>5||size>std::uint32_t(b.size()-at)||word(b,at+52)>10)throw std::invalid_argument("Invalid World raster record");
        if(payload!=(op==MNM_WORLD_WAVE?64u:indexed?512u:0u)||(op!=MNM_WORLD_WAVE&&word(b,at+60)))throw std::invalid_argument("Invalid World raster payload");
        WorldDraw d;d.x=signedWord(word(b,at+8));d.y=signedWord(word(b,at+12));d.clip={signedWord(word(b,at+16)),signedWord(word(b,at+20)),signedWord(word(b,at+24)),signedWord(word(b,at+28))};d.backend=word(b,at+52);
        if(d.clip.left<0||d.clip.top<0||d.clip.right<=d.clip.left||d.clip.bottom<=d.clip.top||d.clip.right>int(frame.width)||d.clip.bottom>int(frame.height))throw std::invalid_argument("World clip outside viewport");
        d.frame={bool(indexed),b.mid(at+MNM_WORLD_RECORD,encoded)};if(word(d.frame.encoded,28))throw std::invalid_argument("World frame retains a legacy pointer");frameIdentity(d.frame.encoded,d.frame.indexed);
        d.composite.mode=render::CompositeMode(op);
        if(op==MNM_WORLD_WAVE){d.composite.rowPeriod=word(b,at+60);if(!d.composite.rowPeriod||d.composite.rowPeriod>16)throw std::invalid_argument("World displacement period outside bounds");for(unsigned j=0;j<16;++j){d.composite.rowOffsets[j]=signedWord(word(b,at+MNM_WORLD_RECORD+encoded+j*4));if(d.composite.rowOffsets[j]<0||d.composite.rowOffsets[j]>16)throw std::invalid_argument("World displacement offset outside bounds");}}
        else if(indexed){render::SpriteColourTable colours{};for(unsigned j=0;j<256;++j){const auto offset=at+MNM_WORLD_RECORD+encoded+j*2;const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+offset);colours[j]=std::uint16_t(p[0]|p[1]<<8);}d.colours=colours;}
        frame.draws.push_back(std::move(d));at+=size;
    }
    if(at!=b.size())throw std::invalid_argument("Trailing World raster inputs");
    return frame;
}
std::vector<render::SceneDraw> worldDisplay(const WorldFrame& frame,const SnapshotResources& resources){
    std::vector<render::SceneDraw> draws;draws.reserve(frame.draws.size());
    for(const auto& d:frame.draws){const auto bound=resources.resolve(d.frame,true);render::SceneDraw draw{bound.resource,bound.frame,d.x,d.y,true,true,d.colours,d.clip,d.composite};draws.push_back(std::move(draw));}return draws;
}
}
