#include "../reconstruction/casting/projectile.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_PROJECTILE_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
using namespace mnm::reconstruction::casting;
namespace {
void check(bool b, const char *why) { if (!b) throw std::runtime_error(why); }
std::int32_t sign(std::uint32_t n) {return n<=INT32_MAX?int(n):-1-int(~n);}
struct Fixture {
  ProjectileEffect effect;
  std::vector<ProjectileActor> actors;
  unsigned number=0, motion=3, animation=0;
  int magnitude=0, owner=0;
};
Fixture *current=nullptr;
std::vector<unsigned> trace;
unsigned animationCalls=0;
void event(unsigned n, std::initializer_list<unsigned> args={}) {
  trace.push_back(n);trace.push_back(unsigned(args.size()));trace.insert(trace.end(),args);
}
unsigned index(const ProjectileActor &a) {return unsigned(&a-current->actors.data());}
unsigned health(unsigned old) {
  switch(current->number%4) {case 0:return old;case 1:return old-1;case 2:return 0;default:return old+1;}
}
ProjectileServices services() {
  ProjectileServices s;
  s.motion=[](auto &e) {event(1,{current->motion});if(current->motion==0)e.target=current->number%3;return current->motion;};
  s.damage=[](auto &a,int m,unsigned owner,const auto &credits) {event(2,{index(a),unsigned(m),owner,credits[0],credits[1]});a.health=health(a.health);};
  s.bounce=[](auto &e,bool relink) {event(3,{unsigned(relink)});e.audio^=0x19;};
  s.release=[](auto &e) {event(4);e.type=0;};
  s.animationTick=[] {auto result=animationCalls++<current->animation?1:current->number%2?0:2;event(5,{unsigned(result)});return result;};
  s.animationRestart=[] {event(6);};
  s.visibleDamage=[](auto &a) {event(7,{index(a)});};
  s.startAudio=[](auto &e) {event(8);e.audio=0x1234;};
  s.followAudio=[](auto &e) {event(9,{e.audio});};
  return s;
}
Fixture generate(unsigned n) {
  Fixture f; f.number=n;f.motion=n%6;f.animation=(n/6)%4;
  f.magnitude=sign(n*0x9e3779b9u); f.owner=std::array<int,6>{-1,0,1,7,8,INT32_MIN}[(n/24)%6];
  f.actors.resize(3);
  for(unsigned i=0;i<3;++i) {auto &a=f.actors[i];a.health=std::array<unsigned,5>{0,1,24,0x80000000u,UINT32_MAX}[(n+i)%5];a.recentAttacker=n+3;a.recentTime=n+7;for(unsigned j=0;j<8;++j)a.visible[j]=(n+i+j)%3;}
  auto &e=f.effect;e.spell=n%2?73:72;e.target=n%7?0:UINT32_MAX;e.source=n%3;e.attacker=n*17;e.attribution=~n;e.remaining=std::array<unsigned,7>{0,1,2,100,0x80000000u,0x80000001u,UINT32_MAX}[(n/3)%7];e.audio=n%3?0:n+19;e.contributions={n,n+5};if(n%2)e.cachedActor=n%3;
  return f;
}
#ifdef MNM_PROJECTILE_REFERENCE
Bytes actors;
unsigned char *effect=nullptr;
unsigned address(const void *p) {return unsigned(reinterpret_cast<std::uintptr_t>(p));}
void put(void *p,unsigned at,unsigned n) {std::memcpy(static_cast<unsigned char*>(p)+at,&n,4);}
unsigned load(const void *p,unsigned at) {unsigned n;std::memcpy(&n,static_cast<const unsigned char*>(p)+at,4);return n;}
unsigned rawIndex(const void *p) {
  const auto d=address(p)-address(actors.data()+32);check(d%0xe4b==0 && d/0xe4b<3,"projectile actor ABI");return d/0xe4b;
}
extern "C" unsigned __attribute__((thiscall)) motion(void *p,unsigned relink) {
  check(p==effect && relink==1,"projectile motion ABI");event(1,{current->motion});if(current->motion==0)put(p,0x48,current->number%3);return current->motion;
}
extern "C" void __attribute__((thiscall)) damage(void *p,int m,unsigned owner,const void *credits) {
  check(credits==effect+0x216,"projectile credit ABI");event(2,{rawIndex(p),unsigned(m),owner,load(credits,0),load(credits,4)});put(p,0xe4,health(load(p,0xe4)));
}
extern "C" void __attribute__((thiscall)) bounce(void *p,unsigned relink) {check(p==effect,"bounce ABI");event(3,{relink});put(p,0x19c,load(p,0x19c)^0x19);}
extern "C" void __attribute__((thiscall)) release(void *p) {check(p==effect,"release ABI");event(4);put(p,0x28,0);}
extern "C" unsigned __attribute__((thiscall)) animation(void *p) {check(p==effect+0x160,"animation ABI");auto result=animationCalls++<current->animation?1:current->number%2?0:2;event(5,{unsigned(result)});return result;}
extern "C" void __attribute__((thiscall)) restart(void *p) {check(p==effect+0x160,"restart ABI");event(6);}
extern "C" void __attribute__((thiscall)) feedback(void *p,unsigned enabled) {check(enabled==1,"feedback ABI");event(7,{rawIndex(p)});}
extern "C" void __attribute__((thiscall)) startAudio(void *p,unsigned code,int x,int y,int z,unsigned zero,void *out,unsigned one) {check(address(p)==0x658178 && code==0x2c6 && x==1 && y==2 && z==1 && zero==0 && out==effect+0x19c && one==1,"audio start ABI");event(8);put(effect,0x19c,0x1234);}
extern "C" void __attribute__((thiscall)) followAudio(void *p,void *audio,int x,int y,int z) {check(address(p)==0x658178 && audio==effect+0x19c && x==1 && y==2 && z==1,"audio follow ABI");event(9,{load(effect,0x19c)});}
void redirect(unsigned a,void *target) {auto *p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;put(p,1,address(target)-a-5);}
struct Fs {
  std::array<unsigned,8> data{};unsigned short saved=0;user_desc d{};
  Fs() {data.fill(0xa5a5a5a5);data[0]=UINT32_MAX;d.entry_number=unsigned(-1);d.base_addr=address(data.data());d.limit=31;d.seg_32bit=1;d.useable=1;check(syscall(SYS_set_thread_area,&d)==0,"private FS");asm volatile("mov %%fs,%0":"=rm"(saved));unsigned short sel=(d.entry_number<<3)|3;asm volatile("mov %0,%%fs"::"rm"(sel):"memory");}
  ~Fs(){asm volatile("mov %0,%%fs"::"rm"(saved):"memory");user_desc empty{};empty.entry_number=d.entry_number;empty.seg_not_present=1;empty.read_exec_only=1;(void)syscall(SYS_set_thread_area,&empty);}
  void verify(){check(data[0]==UINT32_MAX && std::all_of(data.begin()+1,data.end(),[](auto n){return n==0xa5a5a5a5;}),"FS chain");}
};
void packActor(void *p,const ProjectileActor &a) {put(p,0xe4,a.health);put(p,0x80d,a.recentAttacker);put(p,0x811,a.recentTime);for(unsigned i=0;i<8;++i)put(p,0x24+i*4,a.visible[i]);}
void packEffect(void *p,const ProjectileEffect &e) {for(auto f:{std::pair<unsigned,unsigned>{0x28,e.type},{0x4c,e.spell},{0x44,e.source},{0x48,e.target},{0x64,e.attacker},{0x212,e.attribution},{0x216,e.contributions[0]},{0x21a,e.contributions[1]},{0x1d2,e.remaining},{0x19c,e.audio},{0x198,e.cachedActor?address(actors.data()+32+*e.cachedActor*0xe4b):0}})put(p,f.first,f.second);}
Bytes originalBody,originalImpact,originalDispatch;
void install(const char *path) {
  map_image(read(path));
  for(auto item:{std::pair<unsigned,unsigned>{0x4970e0,0x2d44},{0x48ecf0,0x148},{0x499e24,89*4}}) {auto *p=reinterpret_cast<unsigned char*>(item.first);auto &b=item.first==0x4970e0?originalBody:item.first==0x48ecf0?originalImpact:originalDispatch;b.assign(p,p+item.second);}
  check(load(reinterpret_cast<void*>(0x499e24),12)==0x4973f7,"type3 dispatch");
  for(auto p:{std::pair<unsigned,void*>{0x4883f0,(void*)motion},{0x514860,(void*)damage},{0x48ee40,(void*)bounce},{0x4943f0,(void*)release},{0x464ec0,(void*)animation},{0x464d70,(void*)restart},{0x51fba0,(void*)feedback},{0x571310,(void*)startAudio},{0x571460,(void*)followAudio}})redirect(p.first,p.second);
}
void compare(Fixture input,const Fixture &expected,const std::vector<unsigned> &expectedTrace) {
  current=&input;trace.clear();animationCalls=0;
  actors.assign(3*0xe4b+64,0x9d);for(unsigned i=0;i<3;++i)packActor(actors.data()+32+i*0xe4b,input.actors[i]);auto expectedActors=actors;
  Bytes storage(0x22e + 64,0x85);effect=storage.data()+32;packEffect(effect,input.effect);put(effect,4,1);put(effect,8,1);put(effect,12,2);put(effect,16,1);put(effect,0x222,0);auto expectedEffect=storage;
  global(0x6def58,address(actors.data()+32));global(0x6def5c,3);global(0x644520,unsigned(input.owner));global(0x5e1460,0);
  global(0x6b13c5+input.effect.spell*721,unsigned(input.magnitude));
  static Fs fs;using Update=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Update>(0x4970e0)(effect);fs.verify();
  if(trace!=expectedTrace) {std::cerr<<"trace fixture "<<input.number<<"\n";for(auto n:trace)std::cerr<<n<<',';std::cerr<<" expected ";for(auto n:expectedTrace)std::cerr<<n<<',';std::cerr<<'\n';}
  check(trace==expectedTrace,"projectile call trace");
  packEffect(expectedEffect.data()+32,expected.effect);for(unsigned i=0;i<3;++i)packActor(expectedActors.data()+32+i*0xe4b,expected.actors[i]);
  check(storage==expectedEffect && actors==expectedActors,"projectile guarded state");
  for(auto item:{std::pair<unsigned,const Bytes*>{0x4970e0,&originalBody},{0x48ecf0,&originalImpact},{0x499e24,&originalDispatch}})check(!std::memcmp(reinterpret_cast<void*>(item.first),item.second->data(),item.second->size()),"original text/table changed");
}
#endif
std::uint64_t digest=14695981039346656037ULL;
void hash(unsigned n){digest=(digest^n)*1099511628211ULL;}
}
int main(int argc,char **argv) try {
#ifdef MNM_PROJECTILE_REFERENCE
  check(argc==2,"expected executable");install(argv[1]);
#else
  (void)argv;check(argc==1,"unexpected argument");
#endif
  for(unsigned n=0;n<32768;++n) {
    auto input=generate(n),expected=input;current=&expected;trace.clear();animationCalls=0;
    updateProjectile(expected.effect,expected.actors,expected.magnitude,expected.owner,services());
    for(auto v:{expected.effect.type,expected.effect.source,expected.effect.remaining,expected.effect.audio,expected.effect.cachedActor.value_or(UINT32_MAX)})hash(v);
    for(const auto &a:expected.actors){hash(a.health);hash(a.recentAttacker);hash(a.recentTime);}for(auto v:trace)hash(v);
#ifdef MNM_PROJECTILE_REFERENCE
    auto expectedTrace=trace;compare(input,expected,expectedTrace);
#endif
  }
  // Native preflight and unsafe motion output are explicit refusals. Caller stages.
  auto f=generate(0);f.effect.type=4;bool refused=false;try{updateProjectile(f.effect,f.actors,1,0,services());}catch(const std::invalid_argument&){refused=true;}check(refused,"wrong-type refusal");
  std::cout<<"{\"success\":true,\"cases\":32768,\"functions\":2,\"outcome_digest\":\""<<std::hex<<digest<<"\"}\n";
} catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
