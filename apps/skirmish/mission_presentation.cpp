#include "mission_presentation.hpp"
#include "terrain_submission.hpp"
#include <map>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
QPoint project(int x,int y,int z) {return {280+x-y,180+(x+y)/2-z};}
QImage pixels(const assets::Sprite &sprite,unsigned index) {
    const auto &f=sprite.frames.at(index);if(f.empty()) return {};
    QImage image(int(f.width),int(f.height),QImage::Format_ARGB32);
    if(image.isNull()) throw std::runtime_error("Campaign terrain image allocation failed");
    for(unsigned y=0;y<f.height;++y) for(unsigned x=0;x<f.width;++x) {
        const auto at=std::size_t(y)*f.width+x;QRgb color=0;
        if(f.opaqueMask.at(at)) {
            if(sprite.storage==assets::SpriteStorage::indexed8) {
                const auto &c=sprite.palettes.at(f.paletteIndex.value()).at(std::get<std::vector<std::uint8_t>>(f.pixels).at(at));
                color=qRgb(c.red,c.green,c.blue);
            } else {
                const auto v=std::get<std::vector<std::uint16_t>>(f.pixels).at(at);
                const auto r=(v>>11)&31,g=(v>>5)&63,b=v&31;
                color=qRgb((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
            }
        }
        image.setPixel(int(x),int(y),color);
    }
    return image;
}
}
MissionPresentation::MissionPresentation(const MapCampaignResources &resources) {
    standing_=resources.mission().standing;
    const auto &geometry=resources.map().geometry;const auto &sprite=resources.sprites();
    std::map<unsigned,QImage> images;
    for(unsigned z=0;z<geometry.layers;++z) for(unsigned y=0;y<geometry.height;++y) for(unsigned x=0;x<geometry.width;++x) {
        const auto &cell=geometry.cell(x,y,z);const auto anchor=project(int(x)*32,int(y)*32,int(z)*16);
        reconstruction::TerrainTile tile{int(y),int(x),int(z),anchor.x(),anchor.y(),0,cell.flags8,cell.flags10,0};
        const reconstruction::TerrainAdmission admission{0,32,1,0,0,true};
        const auto definition=reconstruction::decodeTerrainDefinition(resources.catalog().records.at(cell.definition));
        for(const auto &draw:reconstruction::submitTerrain(definition,tile,admission,unsigned(sprite.frames.size()-1))) {
            auto it=images.find(draw.frame);if(it==images.end()) it=images.emplace(draw.frame,pixels(sprite,draw.frame)).first;
            const auto &frame=sprite.frames.at(draw.frame);
            terrain_.push_back({it->second,{draw.anchorX-frame.originX,draw.anchorY-frame.originY},draw.key});
        }
    }
    std::vector<reconstruction::SpriteQueueEntry> queue;
    for(unsigned i=0;i<terrain_.size();++i) queue.push_back({terrain_[i].key,i});
    reconstruction::sortSpriteQueue(queue);std::vector<TerrainStamp> sorted;
    for(const auto &q:queue) sorted.push_back(terrain_.at(q.payload));
    terrain_=std::move(sorted);
}
QPoint MissionPresentation::center(game::Point p) const {return project(p.x*32+16,p.y*32+16,p.z*16);}
QPoint MissionPresentation::fineCenter(game::Point p) const {return project(p.x,p.y,p.z);}
std::vector<MissionLayer> MissionPresentation::layers(const std::vector<game::Point> &actors) const {
    if(actors.size()>32) throw std::invalid_argument("Campaign actor presentation budget exceeded");
    std::vector<MissionLayer> entries;std::vector<reconstruction::SpriteQueueEntry> queue;
    for(unsigned i=0;i<terrain_.size();++i) {queue.push_back({terrain_[i].key,unsigned(entries.size())});entries.push_back({false,i});}
    for(unsigned i=0;i<actors.size();++i) {
        const auto p=actors[i];queue.push_back({reconstruction::spriteDepthKey({p.x*32+16,p.y*32+16,p.z*16,6},0),unsigned(entries.size())});entries.push_back({true,i});
    }
    reconstruction::sortSpriteQueue(queue);std::vector<MissionLayer> ordered;
    for(const auto &q:queue) ordered.push_back(entries.at(q.payload));
    return ordered;
}
std::optional<game::Point> MissionPresentation::pick(QPoint point,const std::vector<game::Point> &actors) const {
    if(point.x()<0 || point.y()<0 || point.x()>=560 || point.y()>=560) return {};
    // Native glyph overlays take precedence over the explicit standing diamonds.
    const auto ordered=layers(actors);
    for(auto it=ordered.rbegin();it!=ordered.rend();++it) if(it->actor) {
        const auto delta=point-unitCenter(actors.at(it->index));
        if(std::abs(delta.x())<=16 && delta.y()>=-26 && delta.y()<=18) return actors[it->index];
    }
    std::optional<game::Point> hit;int key=-1;
    for(auto cell:standing_) {
        const auto delta=point-center(cell);
        const auto depth=reconstruction::spriteDepthKey({cell.x*32+16,cell.y*32+16,cell.z*16,0},0);
        if(std::abs(delta.x())+2*std::abs(delta.y())<=32 && depth>key) {hit=cell;key=depth;}
    }
    return hit;
}
}
