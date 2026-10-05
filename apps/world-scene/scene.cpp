#include "scene.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::scene {
namespace {
int narrow(std::int64_t n) {
    if(n<std::numeric_limits<int>::min() || n>std::numeric_limits<int>::max())
        throw std::invalid_argument("Scene coordinate overflow");
    return int(n);
}
}
reconstruction::AnimationOffset project(game::Point p,const Camera& c) {
    if(c.view>3) throw std::invalid_argument("Scene view outside 0..3");
    std::int64_t x=std::int64_t(p.x)-c.origin.x,y=std::int64_t(p.y)-c.origin.y;
    const auto old=x;
    switch(c.view) {case 1:x=y;y=-old;break;case 2:x=-x;y=-y;break;case 3:x=-y;y=old;break;}
    // Diagnostic policy: cell-centred isometric projection, truncation toward zero.
    return {narrow(c.x+x-y),narrow(c.y+(x+y)/2-(std::int64_t(p.z)-c.origin.z))};
}
std::optional<assets::AnimationRecord> displayed(const assets::Animation& a,const game::Entity& e) {
    if(!e.motion) return {};
    const auto& m=*e.motion;
    const game::AnimationCursor* cursor=nullptr;
    if(m.fine && m.fine->animation) cursor=&*m.fine->animation;
    else if(m.previous && m.previous->motion.animation) cursor=&*m.previous->motion.animation;
    if(!cursor || !cursor->displayed) return {};
    if(std::uint64_t(cursor->sequence)+1>=a.starts.size()) throw std::invalid_argument("Scene ANI sequence out of bounds");
    const auto first=a.starts[cursor->sequence],end=a.starts[cursor->sequence+1];
    const auto index=std::uint64_t(first)+*cursor->displayed;
    if(first>=end || end>a.records.size() || index>=end || a.records[index].opcode!=0)
        throw std::invalid_argument("Scene ANI display out of bounds");
    return a.records[index];
}
std::vector<Draw> compose(const game::MovementSession& session,const assets::Animation& animation,
                         const assets::TerrainCatalog& catalog,const std::vector<Tile>& tiles,const Camera& camera,
                         std::size_t terrainFrames,std::size_t creatureFrames) {
    if(tiles.empty() || tiles.size()>4096 || !terrainFrames || !creatureFrames)
        throw std::invalid_argument("Scene requires 1..4096 terrain tiles and nonempty sprites");
    std::vector<Draw> draws;std::vector<reconstruction::SpriteQueueEntry> queue;
    const auto add=[&](Draw d){queue.push_back({d.key,std::uint32_t(draws.size())});draws.push_back(d);};
    for(const auto& tile:tiles) {
        if(tile.definition>=catalog.records.size() || tile.surface.z<16 || tile.surface.x<0 || tile.surface.y<0 ||
           tile.surface.x%32 || tile.surface.y%32) throw std::invalid_argument("Invalid diagnostic terrain tile");
        auto anchor=project({tile.surface.x,tile.surface.y,tile.surface.z-16},camera);
        reconstruction::TerrainTile state{tile.surface.y/32,tile.surface.x/32,(tile.surface.z-16)/16,anchor.x,anchor.y,0,tile.flags8,tile.flags10,0};
        reconstruction::TerrainAdmission admission{camera.view,32,1,0,0,true};
        for(auto d:reconstruction::submitTerrain(reconstruction::decodeTerrainDefinition(catalog.records[tile.definition]),state,admission,std::uint32_t(terrainFrames-1))) {
            // The visual fixture's exact fine height can differ from whole layers.
            d.depth.height=tile.surface.z-16;d.key=reconstruction::spriteDepthKey(d.depth,camera.view);
            add({false,d.frame,d.anchorX,d.anchorY,d.key});
        }
    }
    unsigned actors=0;
    for(std::uint32_t i=0;i<session.world().state().slots.size();++i) {
        const auto& slot=session.world().state().slots[i];
        if(!slot.entity || slot.entity->cleaned) continue;
        const auto& e=*slot.entity;
        if(e.family!=game::Family::creature || !e.motion) continue;
        if(!e.motion->terrainMotion || ++actors>32)
            throw std::invalid_argument("Scene supports up to 32 ordinary terrain-motion creatures");
        const auto record=displayed(animation,e);if(!record) continue;
        if(record->argument<0 || std::uint32_t(record->argument)>=creatureFrames)
            throw std::invalid_argument("Scene ANI frame outside paired creature SPR");
        const auto fine=session.finePosition(e);
        const game::Point centre{narrow(std::int64_t(fine.x)+16),narrow(std::int64_t(fine.y)+16),fine.z};
        const auto anchor=project(centre,camera),offset=reconstruction::spriteOffset(*record,1,camera.view);
        const auto key=reconstruction::spriteDepthKey({centre.x,centre.y,centre.z,6},camera.view);
        add({true,std::uint32_t(record->argument),narrow(std::int64_t(anchor.x)+offset.x),narrow(std::int64_t(anchor.y)+offset.y),key,game::Handle{i,slot.generation}});
    }
    reconstruction::sortSpriteQueue(queue);std::vector<Draw> ordered;
    for(const auto& item:queue) ordered.push_back(draws.at(item.payload));
    return ordered;
}
Frame render(render::GlBlitter& renderer,const assets::Sprite& terrain,const assets::Sprite& creature,
             const std::vector<Draw>& draws) {
    if(draws.size()>12288+32) throw std::invalid_argument("Scene draw budget exceeded");
    const auto canvas=renderer.create({512,256,std::vector<std::uint32_t>(512*256,0x2124)},render::spriteFormat);
    try {
        // Bounded sequential uploads keep the surface budget independent of ANI length.
        for(const auto& d:draws) {
            render::UploadedSpriteFrame frame(renderer,d.creature?creature:terrain,d.frame);
            frame.drawClipped(canvas,d.x,d.y,{0,0,512,256});
        }
        Frame result{renderer.read(canvas),renderer.present(canvas),draws};
        renderer.destroy(canvas);return result;
    } catch(...) {renderer.destroy(canvas);throw;}
}
}
