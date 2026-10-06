#include "frozen_navigation.hpp"
#include "simulation/occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <filesystem>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>

using namespace mnm::game;
static void check(bool ok) {if(!ok) throw std::runtime_error("native occupancy assertion");}
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
static Bytes bytes(const MovementSession& s) {return encodeSnapshot(s.world().state());}
static MovementSession session(const std::string& map,std::shared_ptr<const Navigation> nav) {
    World w(8);auto state=w.state();state.map=map;state.navigation=nav->binding();w.restore(state);return MovementSession(std::move(w),nav);
}
static Entity entity(int x,int y=1,int z=1) {Entity e;e.x=x;e.y=y;e.z=z;return e;}
static void volumes() {
    World world(4);auto e=entity(1,1,1);e.type=10;auto h=world.spawn(e);
    const auto state=world.state();Occupancy grid(state,{8,8,8},10,{2,2,3});
    for(int z=0;z<8;++z) for(int y=0;y<8;++y) for(int x=0;x<8;++x) {
        const bool inside=x>=1 && x<=2 && y>=1 && y<=2 && z>=1 && z<=3;
        check(bool(grid.owner({x,y,z}))==inside);
        check(!grid.blocked({x,y,z},h));
        check(grid.blocked({x,y,z},{h.slot,h.generation+1})==inside);
    }
    auto bad=state;bad.slots[1].entity=e;refused([&]{Occupancy g(bad,{8,8,8},10,{2,2,3});});
    bad.slots[1].entity->cleaned=true;Occupancy cleaned(bad,{8,8,8},10,{2,2,3});check(cleaned.owner({1,1,1})==std::optional<Handle>(h));
    bad=state;bad.slots[0].entity->z=7;refused([&]{Occupancy g(bad,{8,8,8},10,{2,2,3});});
    bad=state;bad.slots[0].entity->type=12;refused([&]{Occupancy g(bad,{8,8,8},10,{2,2,3});});
    refused([&]{Occupancy g(state,{16,16,32},10,{1,1,1});});
    refused([&]{grid.owner({-1,0,0});});
}
int main(int argc,char** argv) try {
    check(argc==3);volumes();const auto map=std::filesystem::absolute(argv[1]).string();const std::filesystem::path root=argv[2];
    auto nav=mnm::sandbox::loadFrozenNavigation(map,{},true);
    auto legacy=mnm::sandbox::loadFrozenNavigation(map);check(!(nav->binding()==legacy->binding()));
    auto old=session(map,legacy);refused([&]{old.spawnBlocker(entity(3));});
    auto s=session(map,nav);auto self=s.spawn(entity(1));auto blocker=s.spawnBlocker(entity(3));
    const auto before=bytes(s);refused([&]{s.spawnBlocker(entity(1));});check(bytes(s)==before);
    refused([&]{s.spawnBlocker(entity(-1));});check(bytes(s)==before);
    s.move(self,{3,1,1});s.step();check(s.world().find(self)->motion->action==Action::blocked);
    s.enqueue({Operation::release,blocker,{}});s.move(self,{3,1,1});s.step();
    check(s.world().find(self)->motion->action==Action::moving);
    for(int i=0;i<8;++i) {s.step();}
    check(s.world().find(self)->x==3 && s.world().find(self)->motion->action==Action::arrived);
    auto replacement=s.spawnBlocker(entity(5));check(replacement.slot==blocker.slot && replacement.generation!=blocker.generation);
    s.enqueue({Operation::release,blocker,{}});check(s.step().rejected==1 && s.world().find(replacement));
    s.move(self,{5,1,1});s.step();check(s.world().find(self)->motion->action==Action::blocked);
    s.enqueue({Operation::cleanup,replacement,{}});s.move(self,{5,1,1});s.step();
    for(int i=0;i<8;++i) {s.step();}
    check(s.world().find(self)->x==5);
    // Width-two self exclusion, footprint edge rejection, and nonwrapping bounds.
    std::ifstream input(map,std::ios::binary);std::vector<char> raw((std::istreambuf_iterator<char>(input)),{});
    std::vector<std::byte> profile(raw.size());std::memcpy(profile.data(),raw.data(),raw.size());
    std::uint32_t scalarOffset=92;
    for(unsigned i=0;i<5;++i){std::uint32_t size;std::memcpy(&size,profile.data()+64+4*i,4);scalarOffset+=size;}
    const std::int32_t width=2;std::memcpy(profile.data()+scalarOffset+8,&width,4);
    auto wideNav=mnm::sandbox::loadFrozenNavigationBytes(profile,{},true);auto wide=session(map,wideNav);
    auto wideSelf=wide.spawn(entity(1));wide.spawnBlocker(entity(4));
    check(wideNav->acceptsInWorld(wide.world().state(),wideSelf,{{2,1,1},2,0,0,720}));
    check(!wideNav->acceptsInWorld(wide.world().state(),wideSelf,{{3,1,1},2,0,0,720}));
    auto seam=session(map,wideNav);auto seamSelf=seam.spawn(entity(10));
    check(!wideNav->acceptsInWorld(seam.world().state(),seamSelf,{{11,1,1},2,0,0,720}));
    auto stale=wideSelf;++stale.generation;refused([&]{wideNav->planInWorld(wide.world().state(),stale,{5,1,1},300);});
    refused([&]{wideNav->acceptsInWorld(wide.world().state(),stale,{{2,1,1},2,0,0,720});});
    refused([&]{wideNav->acceptsInWorld(wide.world().state(),{7,1},{{2,1,1},2,0,0,720});});
    std::uint32_t cellsOffset=92;
    for(unsigned i=0;i<2;++i){std::uint32_t size;std::memcpy(&size,profile.data()+64+4*i,4);cellsOffset+=size;}
    profile.at(cellsOffset+10)=std::byte{1};
    check(bool(mnm::sandbox::loadFrozenNavigationBytes(profile,{},true))); // Marker with empty sentinel has no occupant.
    profile.at(cellsOffset+4)=std::byte{0};profile.at(cellsOffset+5)=std::byte{0};
    refused([&]{mnm::sandbox::loadFrozenNavigationBytes(profile,{},true);});
    // A late obstruction is checked before committing the next waypoint.
    auto dynamic=session(map,nav);auto actor=dynamic.spawn(entity(1));dynamic.move(actor,{5,1,1});dynamic.step();
    check(dynamic.world().find(actor)->motion->action==Action::moving);
    const auto next=dynamic.world().find(actor)->motion->route.front().position;
    auto obstruction=entity(next.x,next.y,next.z);dynamic.spawnBlocker(obstruction);
    const auto checkpoint=root/"obstructed.mnms";check(writeSnapshot(checkpoint,dynamic.world().state()).durable);
    auto restored=session(map,nav);restored.restore(checkpoint,[&](const std::string&){return nav;});check(bytes(restored)==bytes(dynamic));
    const auto prior=bytes(restored);refused([&]{restored.restore(checkpoint,[&](const std::string&){return legacy;});});check(bytes(restored)==prior);
    for(int i=0;i<4;++i){dynamic.step();restored.step();check(bytes(dynamic)==bytes(restored));}
    check(dynamic.world().find(actor)->x==1 && dynamic.world().find(actor)->motion->action==Action::blocked);
    check(writeSnapshot(root/"uninterrupted.mnms",dynamic.world().state()).durable);
    // An intra-cell obstruction cancels/reset fine state at the logical origin.
    auto fine=session(map,nav);auto moving=fine.spawn(entity(1),true,true);fine.move(moving,{5,1,1});fine.step();fine.step();
    check(fine.world().find(moving)->motion->fine.has_value());
    const auto target=fine.world().find(moving)->motion->route.at(fine.world().find(moving)->motion->next).position;
    fine.spawnBlocker(entity(target.x,target.y,target.z));fine.step();
    check(fine.world().find(moving)->x==1 && fine.world().find(moving)->motion->action==Action::blocked && !fine.world().find(moving)->motion->fine);
    check(writeSnapshot(root/"fine-blocked.mnms",fine.world().state()).durable);
    std::cout<<"Owned occupancy volumes, self/generation identity, blocking, cleanup/release, rollback, mode identity, checkpoint reconstruction and intra-cell obstruction pass.\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
