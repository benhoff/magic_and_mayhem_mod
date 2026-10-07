#include "scene-resource-fixtures.hpp"
#include "picking.hpp"
#include "scene_canvas.hpp"
#include "movement_controls.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QMouseEvent>
#include <QPixmap>
#include <QSpinBox>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm;
static void check(bool b) {if(!b) throw std::runtime_error("Picking assertion failed");}
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
struct Navigation final:game::Navigation {
    game::NavigationBinding binding() const override {return {{12,12,3},14};}
    std::uint32_t creatureType() const override {return 7;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    bool accepts(const game::Entity& e,const game::RoutePoint& p) const override {return std::abs(e.x-p.position.x)==1 && e.y==p.position.y && e.z==p.position.z;}
    game::RoutePlan plan(const game::Entity& e,game::Point target,std::uint32_t) const override {
        if(e.y!=target.y || e.z!=target.z) return {};
        game::RoutePlan p{game::PlanStatus::reachable,{},1};auto x=e.x;
        while(x!=target.x) {auto d=x<target.x?1:-1;x+=d;p.route.push_back({{x,e.y,e.z},d>0?2:6,0,0,720});}return p;
    }
};
static game::MovementSession session(game::State state) {game::World w(0);w.restore(std::move(state));return game::MovementSession(std::move(w),std::make_shared<Navigation>());}
static game::MovementSession empty() {game::World w(8);auto s=w.state();s.map="synthetic-mouse-picking";s.navigation=Navigation().binding();return session(s);}
static game::Bytes bytes(const game::MovementSession& s) {return game::encodeSnapshot(s.world().state());}
static void mouse(scene::SceneCanvas& canvas,Qt::MouseButton button,double x,double y) {
    QMouseEvent e(QEvent::MouseButtonPress,QPointF(x,y),QPointF(x,y),button,button,Qt::NoModifier);QApplication::sendEvent(&canvas,&e);
}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);
    if(argc==5 && std::string(argv[1])=="resume") {
        auto s=session(game::readSnapshot(argv[2]));for(int i=0;i<std::stoi(argv[4]);++i) s.step();check(game::writeSnapshot(argv[3],s.world().state()).durable);return 0;
    }
    assets::Sprite terrain,creature;terrain.storage=creature.storage=assets::SpriteStorage::rgb565;
    assets::SpriteFrame f;f.width=3;f.height=3;f.originX=1;f.originY=1;f.pixels=std::vector<std::uint16_t>(9,0x07e0);std::get<std::vector<std::uint16_t>>(f.pixels)[4]=0;f.opaqueMask={1,0,1,1,1,0,0,1,1};terrain.frames={f};
    f.opaqueMask={0,1,1,1,1,0,1,0,1};creature.frames={f};
    std::uint64_t comparisons=0;
    for(int px:{-2,100,510}) for(int py:{-2,100,255}) for(bool cover:{false,true}) {
        std::vector<scene::Draw> q{{false,0,px,py,0,{},game::Point{1,1,1}},
            {true,0,px,py,1,game::Handle{2,0xfedcba98}},
            {true,0,px+1,py,1,game::Handle{3,9}},
            {false,0,px+1,py+1,2,{},game::Point{1,1,2}}};
        if(!cover) std::swap(q[1],q[3]); // Presented order, including equal depth, is authoritative.
        std::vector<std::optional<game::Handle>> actor(512*256);
        std::vector<std::optional<game::Point>> cell(512*256);
        for(const auto& d:q) {
            const auto& src=(d.creature?creature:terrain).frames[0];
            for(unsigned y=0;y<3;++y) for(unsigned x=0;x<3;++x) {
                const auto dx=d.x-1+int(x),dy=d.y-1+int(y);
                if(dx<0 || dy<0 || dx>=512 || dy>=256 || !src.opaqueMask[y*3+x]) continue;
                actor[dy*512+dx]=d.creature?d.actor:std::nullopt;
                if(!d.creature) cell[dy*512+dx]=d.standing;
            }
        }
        for(int y=0;y<256;++y) for(int x=0;x<512;++x) {
            check(scene::pickActor(q,terrain,creature,x,y)==actor[y*512+x]);
            check(scene::pickTerrain(q,terrain,x,y)==cell[y*512+x]);comparisons+=2;
        }
    }
    std::vector<scene::Draw> q{{false,0,100,100,0,{},game::Point{3,1,1}},{true,0,100,100,1,game::Handle{0,0xfedcba98}}};
    check(scene::pickActor(q,terrain,creature,100,100)==game::Handle{0,0xfedcba98});
    check(scene::pickTerrain(q,terrain,100,100)==game::Point{3,1,1}); // Body does not intercept terrain destinations.
    q.push_back({false,0,100,100,2});check(!scene::pickActor(q,terrain,creature,100,100) && !scene::pickTerrain(q,terrain,100,100));q.pop_back();
    for(auto point:{QPoint(-1,0),QPoint(512,0),QPoint(0,256)}) check(!scene::pickActor(q,terrain,creature,point.x(),point.y()) && !scene::pickTerrain(q,terrain,point.x(),point.y()));
    auto bad=terrain;bad.frames[0].opaqueMask.pop_back();refused([&] {scene::pickTerrain(q,bad,100,100);});
    bad=terrain;bad.frames[0].opaqueMask[4]=2;refused([&] {scene::pickTerrain(q,bad,100,100);});
    bad=terrain;bad.frames.clear();refused([&] {scene::pickTerrain(q,bad,100,100);});
    std::vector<scene::Draw> oversized(12321);refused([&] {scene::pickActor(oversized,terrain,creature,100,100);});
    auto extreme=q;extreme.back().x=std::numeric_limits<int>::min();check(!scene::pickActor(extreme,terrain,creature,100,100));
    // Actual composition keeps explicit standing layers, rather than guessing from fine height.
    auto blank=empty();assets::TerrainCatalog catalog;catalog.records.resize(1);
    for(unsigned view=0;view<4;++view) catalog.records[0][0x84+view*4]=1;
    for(unsigned view=0;view<4;++view) {
        auto draws=scene::compose(blank,{},catalog,{{0,{32,32,20},4,0,game::Point{1,1,2}}},{view,{},256,160},1,1);
        check(draws.size()==1 && draws[0].standing==game::Point{1,1,2});
        check(scene::pickTerrain(draws,terrain,draws[0].x,draws[0].y)==game::Point{1,1,2});
    }
    refused([&] {scene::compose(blank,{},catalog,{{0,{32,32,20},4,0,game::Point{2,1,1}}},{},1,1);});
    refused([&] {scene::compose(blank,{},catalog,{{0,{32,32,20},4,0,game::Point{1,1,3}}},{},1,1);});
    auto s=empty();game::Entity e;e.type=7;e.x=1;e.y=1;e.z=1;auto a=s.spawn(e);e.y=3;auto b=s.spawn(e);
    auto state=s.world().state();state.slots[a.slot].generation=0xfedcba98;a.generation=0xfedcba98;s=session(state);
    scene::Orders orders;scene::SceneCanvas canvas;scene::MovementControls controls;unsigned picks=0,moves=0;bool rejectedEvent=false;
    test::SceneResources resources;const auto terrainId=resources.add("terrain",terrain),creatureId=resources.add("creature",creature);
    render::GlBlitter renderer;render::SceneRenderer drawing(renderer,resources.manager,{512,256,std::vector<std::uint32_t>(512*256,0x2124)});const auto displayed=scene::render(drawing,terrainId,creatureId,q);canvas.present(QPixmap::fromImage(displayed.image));refused([&] {canvas.present(QPixmap(511,256));});
    auto refresh=[&] {orders.synchronize(s.world().state());controls.updateChoices(scene::creatureChoices(s.world().state()),orders.selected(),Navigation().binding().dimensions);};refresh();
    canvas.onSelectAt=[&](int x,int y) {++picks;try {orders.select(s.world().state(),scene::pickActor(q,terrain,creature,x,y));} catch(const std::exception&) {rejectedEvent=true;}refresh();};
    canvas.onMoveAt=[&](int x,int y) {const auto target=scene::pickTerrain(q,terrain,x,y);if(target) {try {orders.move(s,*target);controls.setTarget(*target);++moves;} catch(const std::exception&) {rejectedEvent=true;}refresh();}};
    auto before=bytes(s);mouse(canvas,Qt::LeftButton,100.9,100.8);check(orders.selected()==a && picks==1 && bytes(s)==before);
    mouse(canvas,Qt::MiddleButton,100,100);mouse(canvas,Qt::LeftButton,-0.1,100);mouse(canvas,Qt::LeftButton,512,100);check(picks==1 && bytes(s)==before);
    mouse(canvas,Qt::RightButton,100.2,100.7);check(moves==1 && s.world().state().tick==0 && s.world().state().pending.front().subject==a);
    check(controls.findChild<QSpinBox*>("targetX")->value()==3);
    before=bytes(s);refused([&] {controls.setTarget({12,1,1});});check(bytes(s)==before);
    if(argc==2) {std::filesystem::create_directories(argv[1]);check(game::writeSnapshot(std::filesystem::path(argv[1])/"pending.mnw",s.world().state()).durable);canvas.show();controls.show();app.processEvents();check(canvas.grab().save(QString::fromStdString((std::filesystem::path(argv[1])/"canvas.png").string())));}
    auto restarted=session(game::decodeSnapshot(bytes(s)));
    for(unsigned i=0;i<3;++i) {s.step();restarted.step();check(bytes(s)==bytes(restarted));}
    check(s.world().find(a)->x==3 && s.world().find(b)->x==1);
    if(argc==2) check(game::writeSnapshot(std::filesystem::path(argv[1])/"whole.mnw",s.world().state()).durable);
    before=bytes(s);mouse(canvas,Qt::RightButton,200,200);check(bytes(s)==before && moves==1);
    mouse(canvas,Qt::LeftButton,200,200);check(!orders.selected() && bytes(s)==before);
    mouse(canvas,Qt::RightButton,100,100);check(rejectedEvent && bytes(s)==before);rejectedEvent=false;
    orders.select(s.world().state(),a);s.enqueue({game::Operation::release,a,{}});s.step();e.y=1;auto reused=s.spawn(e);check(reused.slot==a.slot && reused.generation!=a.generation);
    before=bytes(s);mouse(canvas,Qt::LeftButton,100,100);check(rejectedEvent);rejectedEvent=false;mouse(canvas,Qt::RightButton,100,100);check(rejectedEvent && bytes(s)==before && !orders.selected());
    std::cout<<"{\"all_match\":true,\"pixel_pick_comparisons\":"<<comparisons<<",\"masked_occlusion\":true,\"terrain_under_actor\":true,\"explicit_layers_four_views\":true,\"actual_mouse_events\":true,\"fractional_and_bounds\":true,\"queued_until_step\":true,\"stale_frame_refusal\":true,\"live_validated\":false}"<<'\n';return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
