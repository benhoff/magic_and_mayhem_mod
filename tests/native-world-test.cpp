#include "persistence/snapshot.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unistd.h>

using namespace mnm::game;
static void check(bool value) {if(!value) throw std::runtime_error("native world assertion");}
template<class F> static void rejected(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
static Bytes bytes(const World& w) {return encodeSnapshot(w.state(),w.limits());}
int main() try {
    World empty(0);check(decodeSnapshot(bytes(empty)).slots.empty());
    empty.enqueue({Operation::release,{0,0},{}});check(empty.step().rejected==1);
    World world(3);Entity e;e.type=7;e.x=std::numeric_limits<std::int32_t>::min();e.state={0,1,255};
    auto a=world.spawn(e);e.family=Family::missileEffect;e.target=a;auto b=world.spawn(e);
    e.family=Family::mapLinked;e.target=b;auto c=world.spawn(e);
    auto full=bytes(world);rejected([&]{world.spawn(e);});check(bytes(world)==full);
    world.enqueue({Operation::cleanup,a,{}});auto pending=bytes(world);
    auto gate=world.step({false,false,false});check(!gate.advanced && bytes(world)==pending);
    auto report=world.step();check(report.advanced && report.applied==1 && report.rejected==0);
    check(report.phases==std::vector<Phase>({Phase::maintenance,Phase::decisions,Phase::secondaryCreatures,Phase::effects,Phase::map,Phase::queuedMap,Phase::audio}));
    check(world.find(a)->cleaned && world.find(b)->target==std::optional<Handle>(a));
    world.enqueue({Operation::release,a,{}});world.step();check(!world.find(a) && !world.find(b)->target);
    auto replacement=world.spawn(Entity{});check(replacement.slot==a.slot && replacement.generation!=a.generation);
    world.enqueue({Operation::target,c,a});world.enqueue({Operation::release,a,{}});
    report=world.step();check(report.rejected==2 && world.find(replacement) && world.find(c)->target==std::optional<Handle>(b));
    auto state=world.state();state.sequence=0xffffffffU;state.tick=0xffffffffU;state.phase20=19;state.phase90=89;
    state.campaign={9,8,7};state.systems={6,5,4};state.map="owned/map";world.restore(state);
    world.step();check(world.state().tick==0 && world.state().sequence==0 && world.state().phase20==0 && world.state().phase90==0);
    auto tick=world.state().tick;auto p20=world.state().phase20;
    report=world.step({true,true,false});check(world.state().tick==tick && world.state().phase20==p20 && report.phases.size()==4);
    world.step({true,false,true});check(world.state().expansionBudget==0);
    auto checkpoint=bytes(world);
    bool observed=false;
    world.step({},[&](Phase phase,const State& staged,std::vector<Command>& out) {
        if(phase==Phase::maintenance) out.push_back({Operation::target,replacement,c});
        if(phase==Phase::decisions) observed=staged.slots[replacement.slot].entity->target==std::optional<Handle>(c);
    });check(observed && world.find(replacement)->target==std::optional<Handle>(c));
    world.restore(decodeSnapshot(checkpoint));
    // Tick systems see committed prior-phase commands, and exceptions roll back the entire tick.
    rejected([&]{world.step({},[&](Phase p,const State&,std::vector<Command>& out) {
        if(p==Phase::maintenance) out.push_back({Operation::release,b,{}});
        if(p==Phase::decisions) throw std::runtime_error("fixture system failure");
    });});check(bytes(world)==checkpoint);
    rejected([&]{world.step({},[&](Phase,const State&,std::vector<Command>&){world.step();});});check(bytes(world)==checkpoint);
    rejected([&]{world.step({},[&](Phase,const State&,std::vector<Command>&){world.enqueue({Operation::cleanup,b,{}});});});check(bytes(world)==checkpoint);
    rejected([&]{world.step({},[&](Phase,const State&,std::vector<Command>& out){out.push_back({static_cast<Operation>(99),b,{}});});});check(bytes(world)==checkpoint);
    // A pending operation survives restoration and produces identical continued states.
    world.enqueue({Operation::release,b,{}});World resumed(0);resumed.restore(decodeSnapshot(bytes(world)));
    for(unsigned i=0;i<200;++i) {auto x=world.step();auto y=resumed.step();check(x.applied==y.applied && x.rejected==y.rejected && bytes(world)==bytes(resumed));}
    check(!world.find(c)->target);
    checkpoint=bytes(world);
    for(std::size_t n=0;n<checkpoint.size();++n) {Bytes cut(checkpoint.begin(),checkpoint.begin()+n);rejected([&]{resumed.restore(decodeSnapshot(cut));});check(bytes(resumed)==checkpoint);}
    auto corrupt=checkpoint;corrupt.back()^=1;rejected([&]{resumed.restore(decodeSnapshot(corrupt));});check(bytes(resumed)==checkpoint);
    state=world.state();state.slots[c.slot].entity->target=a;rejected([&]{world.restore(state);});check(bytes(world)==checkpoint);
    state=world.state();state.phase90=90;rejected([&]{world.restore(state);});check(bytes(world)==checkpoint);
    // Generation exhaustion retires storage instead of wrapping into an old identity.
    World exhausted(1);state=exhausted.state();state.slots[0].generation=0xffffffffU;exhausted.restore(state);
    auto last=exhausted.spawn(Entity{});exhausted.enqueue({Operation::release,last,{}});exhausted.step();
    check(exhausted.state().slots[0].generation==0);rejected([&]{exhausted.spawn(Entity{});});
    Limits limits;limits.slots=1;rejected([&]{decodeSnapshot(checkpoint,limits);});
    limits={};limits.bytes=10;rejected([&]{decodeSnapshot(checkpoint,limits);});
    rejected([&]{World tooLarge(1,limits);});
    limits={};limits.bytes=checkpoint.size()-24;
    // Encoded empty slots can be compact; decoded allocation charges still apply.
    World sparse(100);auto sparseBytes=bytes(sparse);limits.bytes=sparseBytes.size()-24;
    rejected([&]{decodeSnapshot(sparseBytes,limits);});
    limits={};limits.commands=1;World bounded(1,limits);auto one=bounded.spawn(Entity{});bounded.enqueue({Operation::cleanup,one,{}});
    auto before=bytes(bounded);rejected([&]{bounded.enqueue({Operation::release,one,{}});});check(bytes(bounded)==before);
    rejected([&]{bounded.step({},[&](Phase,const State&,std::vector<Command>& out){out.push_back({Operation::release,one,{}});});});check(bytes(bounded)==before);
    // File commit/refusal/error paths and staged load preserve previously valid state.
    char directory[]="/tmp/mnm-world-test-XXXXXX";check(::mkdtemp(directory)!=nullptr);
    struct Cleanup {std::filesystem::path path;~Cleanup(){std::filesystem::remove_all(path);}} cleanup{directory};
    auto path=cleanup.path/"world.mnw";check(writeSnapshot(path,world.state()).durable);
    rejected([&]{writeSnapshot(path,bounded.state());});check(encodeSnapshot(readSnapshot(path))==checkpoint);
    check(writeSnapshot(path,bounded.state(),true).durable);check(encodeSnapshot(readSnapshot(path))==before);
    auto live=bytes(world);restoreSnapshot(resumed,path);check(bytes(resumed)==before);
    rejected([&]{writeSnapshot(cleanup.path/"missing"/"state.mnw",world.state());});check(bytes(world)==live);
    auto invalid=cleanup.path/"bad.mnw";{std::ofstream f(invalid,std::ios::binary);f<<"broken";}
    rejected([&]{restoreSnapshot(world,invalid);});check(bytes(world)==live);
    // A directory destination makes publication fail; temporary files are cleaned up.
    rejected([&]{writeSnapshot(cleanup.path,world.state(),true);});
    std::filesystem::path nulPath(path.string()+std::string("\0suffix",7));
    rejected([&]{writeSnapshot(nulPath,world.state(),true);});rejected([&]{readSnapshot(nulPath);});
    std::size_t files=0;for(const auto& entry:std::filesystem::directory_iterator(cleanup.path)) {check(entry.path().filename().string().front()!='.');++files;}check(files==2);
    std::cout<<"native ownership, tick transactions, continuation, bounded snapshots and atomic store passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
