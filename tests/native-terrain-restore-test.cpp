#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
static void check(bool value) {if(!value) throw std::runtime_error("terrain transaction/restoration mismatch");}
template<class Fn> static void refused(Fn fn) {bool caught=false;try{fn();}catch(const std::exception&){caught=true;}check(caught);}
class ControlledNavigation final: public Navigation {
    std::shared_ptr<const Navigation> bound_;
public:
    bool failPrepare=false,failAdvance=false;
    explicit ControlledNavigation(std::shared_ptr<const Navigation> bound):bound_(std::move(bound)){}
    NavigationBinding binding() const override {return bound_->binding();}
    std::uint32_t creatureType() const override {return bound_->creatureType();}
    RoutePlan plan(const Entity& e,Point p,std::uint32_t b) const override {return bound_->plan(e,p,b);}
    bool accepts(const Entity& e,const RoutePoint& p) const override {return bound_->accepts(e,p);}
    Point finePosition(const Entity& e) const override {return bound_->finePosition(e);}
    void validateSegmentHistory(const Entity& e,const SegmentHistory& h) const override {bound_->validateSegmentHistory(e,h);}
    FineMotion prepareFineMotion(const Entity& e,const RoutePoint& p) const override {
        if(failPrepare) throw std::runtime_error("controlled preparation failure");
        return bound_->prepareFineMotion(e,p);
    }
    bool advanceFineMotion(const Entity& e,const RoutePoint& p,FineMotion& f) const override {
        if(failAdvance) throw std::runtime_error("controlled advancement failure");
        return bound_->advanceFineMotion(e,p,f);
    }
    void validateFineMotion(const Entity& e,const RoutePoint& p,const FineMotion& f) const override {bound_->validateFineMotion(e,p,f);}
};
int main(int argc,char** argv) try {
    check(argc==2);
    auto navigation=std::make_shared<ControlledNavigation>(mnm::sandbox::loadFrozenNavigation(argv[1]));
    World world(8);auto state=world.state();state.map=argv[1];state.navigation=navigation->binding();world.restore(state);
    MovementSession session(std::move(world),navigation);Entity e;e.type=navigation->creatureType();e.x=e.y=e.z=1;
    auto actor=session.spawn(e,true,true,true);session.move(actor,{5,1,1});
    const auto pending=encodeSnapshot(session.world().state());navigation->failPrepare=true;
    refused([&]{session.step();});check(encodeSnapshot(session.world().state())==pending);navigation->failPrepare=false;
    for(unsigned i=0;i<6;++i) session.step();
    const auto before=encodeSnapshot(session.world().state());check(before.at(8)==6);
    check(session.world().find(actor)->motion->previous->origin==Point{1,1,1});
    navigation->failAdvance=true;refused([&]{session.step();});check(encodeSnapshot(session.world().state())==before);navigation->failAdvance=false;
    char folder[]="/tmp/mnm-terrain-restore-XXXXXX";check(mkdtemp(folder));
    struct Cleanup {std::filesystem::path folder;~Cleanup(){std::error_code error;std::filesystem::remove_all(folder,error);}} cleanup{folder};
    const auto checkpoint=cleanup.folder/"checkpoint";check(writeSnapshot(checkpoint,session.world().state()).durable);
    auto malformed=session.world().state();auto& f=*malformed.slots[actor.slot].entity->motion->fine;
    f.heightDelta=5;f.fine.z=f.heightOrigin+f.heightDelta*f.progress/192;
    const auto bad=cleanup.folder/"bad";check(writeSnapshot(bad,malformed).durable);
    refused([&]{session.restore(bad,[&](const std::string&){return navigation;});});check(encodeSnapshot(session.world().state())==before);
    refused([&]{session.restore(checkpoint,[](const std::string&)->std::shared_ptr<const Navigation>{throw std::runtime_error("missing map");});});
    check(encodeSnapshot(session.world().state())==before);
    World clone(0);clone.restore(session.world().state());MovementSession expected(std::move(clone),navigation);
    session.step();expected.step();check(encodeSnapshot(session.world().state())==encodeSnapshot(expected.world().state()));
    session.restore(checkpoint,[&](const std::string&){return navigation;});expected.restore(checkpoint,[&](const std::string&){return navigation;});
    for(unsigned i=0;i<18;++i){session.step();expected.step();check(encodeSnapshot(session.world().state())==encodeSnapshot(expected.world().state()));}
    session.move(actor,{3,1,1});session.step();
    check(session.world().find(actor)->motion->terrainMotion && !session.world().find(actor)->motion->previous);
    session.step();session.enqueue({Operation::cleanup,actor,{}});session.step();
    check(session.world().find(actor)->motion->terrainMotion && !session.world().find(actor)->motion->fine && !session.world().find(actor)->motion->previous);
    std::cout<<"terrain planning/tick rollback, failed in-place restore, continued execution, order and cleanup reset passed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
