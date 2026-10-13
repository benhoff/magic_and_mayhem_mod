#pragma push_macro("main")
#undef main
#define main unused_lifecycle_fixture_main
#include "creature-execution-lifecycle-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/resource_contribution.hpp"
namespace {
struct ContributionFixture {
  LifeFixture lifecycle;
  ResourceContributionState state;
  unsigned grant = 0;
};
ContributionFixture *contribution;
unsigned floatBits(float v) { unsigned n; std::memcpy(&n, &v, 4); return n; }
int signedBits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1-int(~n); }
unsigned subjectFor(int owner) {
  return unsigned(owner) % contribution->lifecycle.base.world.creatures.size();
}
unsigned creditedWord(unsigned type) {
  return 0xfffffff0u + type + fixture->number;
}
ResourceContributionServices contributionServices() {
  ResourceContributionServices s;
  s.subject = [](int owner) {
    auto i=subjectFor(owner); record(145,i,unsigned(owner));
    return std::optional<unsigned>(i);
  };
  s.change = [](auto &a,unsigned delta) {
    record(146,a.ai.slot,delta);
    auto &v=contribution->state.resource.at(a.ai.slot);
    if (fixture->number % 5) v += delta;
    // Reentry changes inventory and source type before the original reloads.
    if (fixture->number % 7 == 0) {
      a.inventory[41].available=1; a.inventory[41].kind=19;
      a.inventory[41].magnitude=0xfffffffeu;
      fixture->world.creatures[0].ai.type=27;
    }
  };
  s.account = [](unsigned type,float amount,float delta) {
    record(147,0,type,{signedBits(floatBits(amount)),signedBits(floatBits(delta)),0});
    return creditedWord(type);
  };
  return s;
}
ContributionFixture generateContribution(unsigned n) {
  ContributionFixture f;
  f.lifecycle.base=generate(n);
  auto count=f.lifecycle.base.world.creatures.size();
  f.lifecycle.maintenance.resize(count); f.lifecycle.movement.resize(count);
  f.lifecycle.corpseFlags.assign(4096,0x80);f.lifecycle.corpseMarkers.assign(4096,0);
  f.state.resource.resize(count);f.state.roots.resize(count);
  f.state.totals={0xffffffffu,n,0x80000000u};
  f.state.ancestors={{{},0},{0,1},{1,2},{{},{}},{{},2},{4,0}};
  constexpr unsigned words[]={0,1,255,256,0x7fffffff,0x80000000,0xffffff00,0xffffffff};
  for (unsigned i=0;i<count;++i) {
    auto &a=f.lifecycle.base.world.creatures[i];
    a.ai.active=true;a.ai.health=400;a.ai.owner=int((n+i)%8);
    a.form=0;a.ai.flying=0;a.specialVision=0;a.protection8c9=0;
    f.lifecycle.base.execution[i].bonusMode124=0;
    for(unsigned j=0;j<42;++j)
      a.inventory[j]={(n+i+j)%3?19u:18u,words[(n/3+i+j)%8],0,(n+j)%4};
    f.state.resource[i]=words[(n+i)%8];
    f.state.roots[i]={n%4?std::optional<unsigned>((n+i)%6):std::nullopt,
                      n%5?std::optional<unsigned>((n/5+i)%6):std::nullopt};
  }
  for (unsigned i=0;i<8;++i) f.state.controllers[i]=(n+i)%4?-2:-1;
  auto &a=f.lifecycle.base.world.creatures[0];
  a.ai.gateFC=n%9?1:0;
  f.lifecycle.maintenance[0].upkeepDeadline=n%6==0?1000:n%6==1?1001:999;
  // Exercise deadline wrap independent of primitive grant arithmetic.
  f.grant=words[(n/11)%8];
  return f;
}
void nativeContribution(ContributionFixture &f) {
  contribution=&f;life=&f.lifecycle;fixture=&f.lifecycle.base;
  auto s=maintenanceServices();
  s.upkeep=[&](auto &a) { contributeCreatureResource(a,f.lifecycle.base.world,
                                 f.state,f.grant,contributionServices()); };
  maintainCreature(f.lifecycle.base.world.creatures[0],f.lifecycle.base.execution[0],
                   f.lifecycle.maintenance[0],f.lifecycle.base.world,
                   maintenanceEnvironment(),s);
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes contributionNodes, contributionRecipients;
extern "C" void *__attribute__((thiscall)) contributionSubjectStub(void *p,int owner) {
  if (reinterpret_cast<std::uintptr_t>(p)!=0x643a80u)
    throw std::runtime_error("contribution subject manager");
  unsigned i=subjectFor(owner);record(145,i,unsigned(owner));return actors.data()+i*0xe4b;
}
extern "C" void __attribute__((thiscall)) contributionChangeStub(void *p,unsigned delta) {
  auto i=slot(p);record(146,i,delta);
  if(fixture->number%5)write32(p,0xe8,read32(p,0xe8)+delta);
  if(fixture->number%7==0) {
    write32(p,0x17c+41*24,19);write32(p,0x17c+41*24+4,0xfffffffeu);
    write32(p,0x17c+41*24+20,1);write32(actors.data(),0xa8,27);
  }
}
extern "C" unsigned __attribute__((fastcall)) contributionAccountStub(
    unsigned type,unsigned second,unsigned zero,float amount,float delta,unsigned last) {
  if(second||zero||last)throw std::runtime_error("contribution account ABI");
  record(147,0,type,{signedBits(floatBits(amount)),signedBits(floatBits(delta)),0});
  return creditedWord(type);
}
void packContribution(Bytes &dst,const ContributionFixture &f) {
  for(unsigned i=0;i<f.lifecycle.base.world.creatures.size();++i) {
    auto *p=dst.data()+i*0xe4b;write32(p,0xe8,f.state.resource[i]);
    for(unsigned j=0;j<42;++j) {
      const auto &a=f.lifecycle.base.world.creatures[i].inventory[j];
      write32(p,0x17c+j*24,a.kind);write32(p,0x17c+j*24+4,a.magnitude);
      write32(p,0x17c+j*24+20,a.available);
    }
    for(unsigned j=0;j<2;++j)write32(p,0x678+j*4,f.state.roots[i][j]?
       reinterpret_cast<std::uintptr_t>(contributionNodes.data()+*f.state.roots[i][j]*64):0);
  }
}
void compareContribution(const ContributionFixture &input,const ContributionFixture &expected,
                         const std::vector<Event> &trace) {
  auto original=input;contribution=&original;
  contributionNodes.assign(input.state.ancestors.size()*64,0x3d);
  contributionRecipients.assign(input.state.totals.size()*64,0x2d);
  for(unsigned i=0;i<input.state.ancestors.size();++i) {
    auto *p=contributionNodes.data()+i*64;const auto &v=input.state.ancestors[i];
    write32(p,0x39,v.parent?reinterpret_cast<std::uintptr_t>(contributionNodes.data()+*v.parent*64):0);
    write32(p,5,v.recipient?reinterpret_cast<std::uintptr_t>(contributionRecipients.data()+*v.recipient*64):0);
  }
  for(unsigned i=0;i<input.state.totals.size();++i)
    write32(contributionRecipients.data()+i*64,0x2f,input.state.totals[i]);
  auto nodesBefore=contributionNodes,expectedRecipients=contributionRecipients;
  for(unsigned i=0;i<expected.state.totals.size();++i)
    write32(expectedRecipients.data()+i*64,0x2f,expected.state.totals[i]);
  Bytes ownerBefore,configBefore;
  LifeCompareExtension ext;
  ext.install=[&] {
    redirect(0x466870,reinterpret_cast<void *>(contributionSubjectStub));
    redirect(0x507f00,reinterpret_cast<void *>(contributionChangeStub));
    redirect(0x416d80,reinterpret_cast<void *>(contributionAccountStub));
    for(unsigned i=0;i<8;++i)global(0x643a80+i*340+12,unsigned(input.state.controllers[i]));
    global(0x5e1908,input.grant);
    ownerBefore=Bytes(reinterpret_cast<unsigned char *>(0x643a80),reinterpret_cast<unsigned char *>(0x643a80+8*340));
    configBefore=configs;
  };
  ext.pack=[&](Bytes &b,bool isExpected) { packContribution(b,isExpected?expected:input); };
  ext.verify=[&] {
    if(contributionNodes!=nodesBefore||contributionRecipients!=expectedRecipients||configs!=configBefore||
       std::memcmp(reinterpret_cast<void *>(0x643a80),ownerBefore.data(),ownerBefore.size()))
      throw std::runtime_error("contribution ownership guard");
    if(read32(reinterpret_cast<void *>(0x5e1908),0)!=input.grant)
      throw std::runtime_error("contribution grant guard");
  };
  compareLife(input.lifecycle,expected.lifecycle,0,false,trace,&ext);
}
#endif
void nativeContributionRefusals() {
  for(unsigned op=0;op<4;++op) {
    auto f=generateContribution(1);f.state.controllers.fill(1);contribution=&f;
    life=&f.lifecycle;fixture=&f.lifecycle.base;bool refused=false;
    if(op==0)f.lifecycle.base.world.creatures[0].ai.owner=-1;
    if(op==1)f.state.ancestors[0].parent=0;
    if(op==2)f.state.roots[0][0]=99;
    auto s=contributionServices();if(op==3)s.subject=[](int){return std::optional<unsigned>{};};
    try { contributeCreatureResource(f.lifecycle.base.world.creatures[0],f.lifecycle.base.world,f.state,2,s); }
    catch(const std::exception &){refused=true;}
    if(!refused)throw std::runtime_error("missing contribution refusal");
  }
}
} // namespace
int main(int argc,char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if(argc!=2)throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;(void)argv;
#endif
  unsigned grants=0,accounts=0,blocked=0,duplicate=0;
  for(unsigned n=0;n<8192;++n) {
    auto input=generateContribution(n),expected=input;events.clear();nativeContribution(expected);
    auto trace=events;
    for(const auto &e:trace) {
      mix(e.kind);mix(e.slot);mix(e.argument);mix(unsigned(e.position.x));mix(unsigned(e.position.y));
      grants+=e.kind==146;accounts+=e.kind==147;
    }
    blocked+=expected.lifecycle.maintenance[0].upkeepDeadline==input.lifecycle.maintenance[0].upkeepDeadline;
    const auto &roots=input.state.roots[0];duplicate+=roots[0]&&roots[1]&&roots[0]==roots[1];
    for(auto r:expected.state.resource)mix(r);
    for(auto r:expected.state.totals)mix(r);
    mix(expected.lifecycle.maintenance[0].upkeepDeadline);
#ifdef MNM_EXECUTION_REFERENCE
    compareContribution(input,expected,trace);
#endif
  }
  nativeContributionRefusals();
  if(!grants||grants!=accounts||!blocked||!duplicate)throw std::runtime_error("empty contribution branch");
  std::cout<<"{\"success\":true,\"cases\":8192,\"functions\":1,\"native_refusal_cases\":4,\"outcome_digest\":"<<digest
           <<",\"grant_calls\":"<<grants<<",\"account_calls\":"<<accounts<<",\"blocked_cases\":"<<blocked
           <<",\"duplicate_roots\":"<<duplicate<<"}\n";return 0;
} catch(const std::exception &e) { std::cerr<<e.what()<<'\n';return 1; }
