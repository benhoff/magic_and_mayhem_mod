#define main unused_commands_fixture_main
#include "creature-transfer-commands-owned-test.cpp"
#undef main
namespace {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
unsigned allocationSerial=1;
#endif
void ingressDisplay(unsigned slot) { event(37,slot); }
ExecutionServices ingressServices(unsigned slot) {
  auto s=resetServices(slot);
  s.deliverQueue=[slot](auto &){ingressDisplay(slot);};
  s.adjustDuplicate=[](Cell &){throw std::runtime_error("duplicate geometry outside ingress domain");return false;};
  s.duplicateMove=[](auto &,Cell p){throw std::runtime_error("duplicate move outside ingress domain");return p;};
  return s;
}
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
extern "C" void *ingressAllocate(unsigned size) {
  if(size!=48||allocationSerial>16)throw std::runtime_error("ingress allocation domain");
  auto *p=static_cast<unsigned char *>(orderPointer(0,allocationSerial++));
  std::memset(p,0,45);
  return p;
}
extern "C" void __attribute__((thiscall)) ingressDisplayStub(void *p) {
  importActor();ingressDisplay(actorSlot(p));
}
#endif
} // namespace
int main(int argc,char **argv) try {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if(argc!=2)throw std::runtime_error("pinned PE expected");
  install(argv[1]);
  for(auto r:{std::pair<unsigned,unsigned>{0x523be0,0x5240ae},{0x5243b0,0x524460},{0x524460,0x5244ea},{0x52d110,0x52d1a5}})
    bodies.push_back({r.first,Bytes(reinterpret_cast<unsigned char *>(r.first),reinterpret_cast<unsigned char *>(r.second))});
  redirect(0x597850,reinterpret_cast<void *>(ingressAllocate));
  redirect(0x512220,reinterpret_cast<void *>(ingressDisplayStub));
#else
  (void)argc;(void)argv;
#endif
  std::uint64_t digest=1469598103934665603ull;auto mix=[&](unsigned v){digest=(digest^v)*1099511628211ull;};
  unsigned cases=0,appends=0,clears=0,follows=0;
  for(unsigned n=0;n<512;++n)for(unsigned op=0;op<3;++op) {
    auto input=generateTransfer(n);auto &a=input.relations.creatures[0];auto &e=input.executions[0];
    a.ai.type=1;a.ai.health=400;a.ai.behavior=n%2?2:1;a.ai.action=15;a.ai.pending=0;
    a.ai.motion=n%5==0;a.ai.engaged=a.mode=a.form=a.block784=a.blockDE3=a.block71c=a.block723=a.flag104=0;
    e.follow=-1;e.orders.resize(n%4);e.queuedCount=e.orders.size();e.queuedFlag=n%3==0;e.nextOrderSerial=e.orders.size()+1;
    e.orderCursor=e.orders.empty()?std::nullopt:std::optional<unsigned>(1);
    for(unsigned j=0;j<e.orders.size();++j) {
      auto &o=e.orders[j];o.serial=j+1;o.kind=2;o.destination={int((n+j)%16),2,0};
      o.referenceDestination=o.destination;o.target=-1;o.effect.reset();o.repeat=false;
    }
    input.initialOrders[0]=e.orders;
    CreatureOrder order;order.kind=2+n%3;order.destination={int(n%16),int(n/7%16),int(n%4)};
    order.target=order.kind==3?int(n%input.relations.creatures.size()):-1;
    auto expected=input;current=&expected;original=false;events.clear();
    OwnedCreatureCommands commands(expected.relations,expected.executions,expected.idleBindings,
        ingressServices,idleServices,followServices,queueServices,commandBody,
        repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
    auto &receiver=expected.relations.creatures[0];
    const int target=n%7?int(n%input.relations.creatures.size()):-1;
    if(op==0){commands.append(receiver,order);++appends;}
    else if(op==1){commands.clear(receiver);++clears;}
    else {commands.follow(receiver,target);++follows;}
    const auto trace=events;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
    allocationSerial=input.executions[0].nextOrderSerial;
    compare(input,expected,trace,nullptr,[&](void *p){
      if(op==0){using Append=void(__attribute__((thiscall)) *)(void *,unsigned,int,int,int,int);
        reinterpret_cast<Append>(0x523be0)(p,order.kind,order.destination.x,order.destination.y,order.destination.z,
            order.target);
      }else if(op==1){using Clear=void(__attribute__((thiscall)) *)(void *);reinterpret_cast<Clear>(0x5243b0)(p);}
      else {using Follow=void(__attribute__((thiscall)) *)(void *,int);reinterpret_cast<Follow>(0x524460)(p,target);}
    });
#endif
    ++cases;mix(expected.executions[0].orders.size());mix(expected.executions[0].nextOrderSerial);
    mix(expected.executions[0].queuedFlag);mix(expected.executions[0].orderCursor.value_or(0xffffffff));
    mix(unsigned(expected.executions[0].follow));mix(receiver.ai.behavior);mix(receiver.ai.pending);mix(receiver.ai.motion);
    mix(unsigned(expected.idleBindings[0]));for(auto v:trace)mix(v);
    for(const auto &o:expected.executions[0].orders){mix(o.serial);mix(o.kind);mix(unsigned(o.target));mix(unsigned(o.destination.x));mix(unsigned(o.destination.y));}
  }
  std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"functions\":28,\"outcome_digest\":"<<digest
      <<",\"append_cases\":"<<appends<<",\"clear_cases\":"<<clears<<",\"follow_cases\":"<<follows<<",\"ingress_redirected\":false}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
