#include "scene.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);render::GlBlitter renderer;
    assets::Sprite terrain,creature;terrain.storage=creature.storage=assets::SpriteStorage::rgb565;
    assets::SpriteFrame tile;tile.width=7;tile.height=5;tile.originX=3;tile.originY=2;
    tile.pixels=std::vector<std::uint16_t>(35,0x07e0);tile.opaqueMask=std::vector<std::uint8_t>(35,1);
    terrain.frames={tile};tile.pixels=std::vector<std::uint16_t>(35,0xf800);tile.opaqueMask[17]=0;creature.frames={tile};
    unsigned cases=0;
    for(int x:{-3,0,3,255,510,515}) for(int y:{-2,0,2,128,255,258}) for(bool front:{false,true}) {
        std::vector<scene::Draw> draws{{front,0,x,y,-1},{!front,0,x+2,y+1,1},
            {true,0,x+1,y+2,1,game::Handle{7,2}},{true,0,x-1,y,1,game::Handle{9,4}}};
        const auto result=scene::render(renderer,terrain,creature,draws);
        std::vector<std::uint32_t> expected(512*256,0x2124);
        for(const auto& draw:draws) {
            const auto& source=(draw.creature?creature:terrain).frames[0];
            for(int row=0;row<5;++row) for(int col=0;col<7;++col) {
                const int dx=draw.x-3+col,dy=draw.y-2+row;
                if(dx>=0 && dx<512 && dy>=0 && dy<256 && source.opaqueMask[row*7+col])
                    expected[dy*512+dx]=std::get<std::vector<std::uint16_t>>(source.pixels)[row*7+col];
            }
        }
        check(result.queue.size()==4 && result.queue[2].actor==game::Handle{7,2} && result.queue[3].actor==game::Handle{9,4},"Scene lost actor identity at equal depth");
        check(result.pixels.pixels==expected,"Mixed sprite clipping/mask differs from CPU oracle");
        check(renderer.stats().surfaces==0,"Scene rendering leaked GPU surfaces");++cases;
    }
    const std::array<reconstruction::AnimationOffset,4> projected{{{272,56},{304,24},{240,8},{208,40}}};
    for(unsigned v=0;v<4;++v) {const auto a=scene::project({32,16,32},{v,{},256,64});check(a.x==projected[v].x && a.y==projected[v].y,"Diagnostic projection mismatch");}
    assets::Animation animation;animation.starts={0,2};animation.records={{0,0,{}},{6,0,{}}};
    game::Entity e;e.motion=game::CreatureMotion{};e.motion->fine=game::FineMotion{};
    e.motion->fine->animation=game::AnimationCursor{0,1,0,true,1,0,0,0};
    check(scene::displayed(animation,e)->argument==0,"Scene lost owned display");
    const auto before=e.motion->fine->animation->pc;
    scene::displayed(animation,e);check(before==e.motion->fine->animation->pc,"Scene advanced ANI clock");
    e.motion->previous=game::SegmentHistory{*e.motion->fine,0,0,0,{}};e.motion->fine.reset();
    check(bool(scene::displayed(animation,e)),"Boundary lost completed controller display");
    e.motion->pose=game::displayedPose(*e.motion);e.motion->previous.reset();
    check(bool(scene::displayed(animation,e)),"Stopped pose lost body");
    scene::displayed(animation,e);check(e.motion->pose->displayed==0,"Static display advanced");
    e.motion->fine=game::FineMotion{};e.motion->fine->animation=game::AnimationCursor{0,1,0,true,1,0,0,0};
    check(bool(scene::displayed(animation,e)),"Active display did not override retained pose");
    e.motion->fine.reset();e.motion->pose->displayed=1;bool rejected=false;
    try {scene::displayed(animation,e);} catch(const std::invalid_argument&) {rejected=true;}
    check(rejected,"Control opcode accepted as display");
    std::cout<<cases<<" mixed CPU/OpenGL pixel comparisons; four views and owned display boundaries pass\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
