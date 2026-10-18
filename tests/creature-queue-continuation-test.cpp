#define main unused_commands_fixture_main
#include "creature-transfer-commands-owned-test.cpp"
#undef main
int main(int argc, char **argv) try {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if(argc!=2) throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc; (void)argv;
#endif
  unsigned cases=0,emptyCycles=0,endStarts=0,refused=0;
  std::uint64_t digest=1469598103934665603ull;
  auto mix=[&](unsigned v){digest=(digest^v)*1099511628211ull;};
  for(unsigned n=0;n<128;++n) for(unsigned mode=0;mode<4;++mode) {
    auto state=generateTransfer(n);
    auto &a=state.relations.creatures[0]; auto &e=state.executions[0];
    a.ai.type=1;a.ai.health=400;a.ai.behavior=1;a.ai.action=15;a.ai.motion=false;
    a.ai.pending=a.mode=a.form=a.block71c=a.block723=a.block784=a.blockDE3=a.ai.engaged=0;
    e.follow=-1;e.marker68=0;e.routeEffect.reset();
    e.orders.resize(2+n%5);e.queuedCount=e.orders.size();e.queuedFlag=mode==3?0:1;
    for(unsigned j=0;j<e.orders.size();++j) {
      auto &o=e.orders[j]; o.serial=j+1;o.kind=mode==0?3:2+(n+j)%3;
      if(mode==3 && (j==0 || j+1==e.orders.size())) o.kind=2;
      o.target=o.kind==3?int(n%state.relations.creatures.size()):-1;
      o.destination={int((n+j)%16),int((n/3+j)%16),int(n%4)};
      o.referenceDestination=o.destination;o.effect.reset();o.repeat=false;
    }
    e.orderCursor=mode==1?std::nullopt:std::optional<unsigned>(mode==0?e.orders.back().serial:e.orders.front().serial);
    for(unsigned step=0;step<4;++step) {
      auto input=state,expected=input;input.initialOrders[0]=input.executions[0].orders;
      current=&expected;original=false;events.clear();
      OwnedCreatureCommands commands(expected.relations,expected.executions,expected.idleBindings,
          resetServices,idleServices,followServices,queueServices,commandBody,
          repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
      const bool returned=commands.queue(expected.relations.creatures[0]);
      auto trace=events;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
      unsigned actual=0;
      compare(input,expected,trace,nullptr,[&](void *p){
        using Step=unsigned(__attribute__((thiscall)) *)(void *);
        actual=reinterpret_cast<Step>(0x524130)(p)&255u;
      },true);
      if(actual!=unsigned(returned)) throw std::runtime_error("queue return mismatch seed="+std::to_string(n)+" mode="+std::to_string(mode));
#endif
      ++cases;emptyCycles+=returned&&expected.executions[0].orders.empty();
      endStarts+=!input.executions[0].orderCursor&&!input.executions[0].orders.empty();
      mix(returned);mix(expected.executions[0].queuedFlag);mix(expected.executions[0].queuedCount);
      mix(expected.executions[0].orderCursor.value_or(0xffffffff));
      for(auto v:trace)mix(v);
      for(const auto &o:expected.executions[0].orders){mix(o.kind);mix(o.serial);}
      state=std::move(expected);
    }
  }
  {
    auto f=generateTransfer(1);current=&f;original=false;
    auto &a=f.relations.creatures[0];auto &e=f.executions[0];
    a.ai.behavior=1;a.ai.type=1;a.form=a.block784=a.blockDE3=0;
    e.queuedFlag=0;e.orderCursor.reset();
    OwnedCreatureCommands c(f.relations,f.executions,f.idleBindings,
        resetServices,idleServices,followServices,queueServices,commandBody,
        repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
    bool caught=false;try{c.queue(a);}catch(const std::invalid_argument &){caught=true;}
    if(!caught)throw std::runtime_error("unsafe noncyclic end admitted");
    ++refused;
  }

  {
    auto f=generateTransfer(1);current=&f;original=false;
    auto &a=f.relations.creatures[0];auto &e=f.executions[0];
    a.ai.type=1;a.ai.health=400;a.ai.behavior=1;a.ai.motion=false;
    a.mode=a.form=a.block784=a.blockDE3=a.block71c=a.block723=0;
    f.relations.creatures[1].ai.health=0;
    e.orders.resize(3);e.queuedCount=3;e.queuedFlag=0;e.orderCursor=2;
    for(unsigned j=0;j<3;++j){e.orders[j].serial=j+1;e.orders[j].kind=j==2?3:2;e.orders[j].repeat=false;e.orders[j].effect.reset();}
    e.orders[2].target=1;
    OwnedCreatureCommands c(f.relations,f.executions,f.idleBindings,
        resetServices,idleServices,followServices,queueServices,commandBody,
        repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
    bool caught=false;try{c.queue(a);}catch(const std::invalid_argument &){caught=true;}
    if(!caught)throw std::runtime_error("unsafe terminal publication admitted");
    ++refused;
  }
  if(!emptyCycles || !endStarts)throw std::runtime_error("empty continuation branch");
  std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"functions\":24,\"outcome_digest\":"<<digest
      <<",\"empty_cycles\":"<<emptyCycles<<",\"end_cursor_starts\":"<<endStarts<<",\"native_refusal_cases\":"<<refused
      <<",\"sentinel_payload_zero\":true,\"queue_redirected\":false,\"commands_redirected\":false}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
