#pragma once
#include "actor_particle_provider.hpp"
#include "particle_children.hpp"
#include <map>

namespace mnm::reconstruction::actor_services {
// Original PE32 value addresses remain behind the storage/call boundary.
// Kernels use private byte objects; publish before every observable callback.
template<class M> class EmissionComposition {
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p,U v) { m.set32(p,v); }
  ParticleWorld world() { return {particleSigned(r(0x5eb480)),particleSigned(r(0x5eb484)),particleSigned(r(0x5eb488))}; }
  ParticleObject load(U p) {
    ParticleObject object;
    for(U k=0;k<100;++k)object.bytes[k]=m.get8(p+k);
    return object;
  }
  void save(U p,const ParticleObject &object) {
    for(U k=0;k<100;++k)m.set8(p+k,object.bytes[k]);
  }
  ParticleOwnerFacts owner(U token) {
    ParticleOwnerFacts facts;facts.present=token!=0;
    if(token) {
      facts.active=r(token+0xe4)!=0;
      {
        for(U k=0;k<3;++k)facts.position[k]=particleSigned(r(token+0x14+k*4));
        U descriptor=r(token+0xac);
        facts.horizontalOffset=particleSigned(r(descriptor+0x198));
        facts.verticalOffset=particleSigned(r(descriptor+0x19c));
      }
    }
    return facts;
  }
  struct Backend {
    EmissionComposition &bridge;
    U parentToken;
    ParticleObject &parent;
    Kind8Random &random;
    std::map<U,ParticleObject> children;
    void flush() {
      bridge.save(parentToken,parent);
      for(const auto &entry:children)bridge.save(entry.first,entry.second);
      bridge.w(0x6574c8,random.first);bridge.w(0x6574cc,random.second);
      for(U k=0;k<250;++k)bridge.w(0x6574d0+k*4,random.words[k]);
    }
    U allocate(U size) { flush();return bridge.m.invoke(0x597850,0,{size}); }
    void construct(U token,ParticleChildKind kind,const std::array<std::int32_t,3> &p,const std::array<std::int32_t,3> &v) {
      if(kind!=ParticleChildKind::ownerAdditive)throw std::invalid_argument("Selected emitter child kind");
      flush();auto child=bridge.load(token);
      ParticleConstructorConfig config;config.rgb555=bridge.r(0x6e1f88)!=0;
      Kind8Random unused(0);
      //546ce0 inlines the child factory around545650/545b70.
      constructEmittedParticle(kind,child,bridge.world(),p,v,config,{},unused);
      bridge.save(token,child);children.emplace(token,child);
    }
    ParticleObject &object(U token) { return children.at(token); }
    bool insert(U token) { flush();return bridge.m.invoke(0x5457e0,0x6cbb8a,{token})!=0; }
    void destroy(U token) {
      flush();children.erase(token);
      bridge.m.invoke(bridge.r(bridge.r(token)),token,{1});
    }
  };
public:
  explicit EmissionComposition(M &memory):m(memory) {}
  std::optional<U> tryRun(U entry,U self,std::initializer_list<U> args={},U second=0) {
    switch(entry) {
    case 0x545910: {
      if(args.size())throw std::invalid_argument("Particle tick arguments");
      w(self+0x1c04,0);
      for(U i=r(self+0x1c00)-1;signed32(i)>=0;--i) {
        U token=r(self+i*4);
        if(m.invoke(r(r(token)+16),token,{}))continue;
        token=r(self+i*4);
        if(token)m.invoke(r(r(token)),token,{1});
        U last=r(self+0x1c00)-1;
        if(signed32(i)<signed32(last)) {
          w(self+0x1c00,last);w(self+i*4,r(self+last*4));w(self+last*4,0);
        } else { w(self+i*4,0);w(self+0x1c00,last); }
      }
      return 0;
    }
    case 0x546ce0: {
      if(args.size())throw std::invalid_argument("Emitter tick arguments");
      auto object=load(self);Kind8Random random(0);
      random.first=r(0x6574c8);random.second=r(0x6574cc);
      for(U k=0;k<250;++k)random.words[k]=r(0x6574d0+k*4);
      ParticleEmissionConfig config;config.rgb555=r(0x6e1f88)!=0;
      auto facts=owner(U(object.get(0x54)));
      Backend backend{*this,self,object,random,{}};
      U result=updateInterpolatingEmitter(object,world(),facts,config,random,backend);
      backend.flush();return result;
    }
    case 0x545650: {
      if(args.size()!=6)throw std::invalid_argument("Moving particle constructor arguments");
      auto object=load(self);auto a=args.begin();
      std::array<std::int32_t,3> position{particleSigned(a[0]),particleSigned(a[1]),particleSigned(a[2])},velocity{particleSigned(a[3]),particleSigned(a[4]),particleSigned(a[5])};
      constructParticleMotion(object,world(),position,velocity,true);
      save(self,object);return self;
    }
    case 0x5456c0:case 0x546120:case 0x546350: {
      if(args.size())throw std::invalid_argument("Particle update arguments");
      auto object=load(self);bool result;
      if(entry==0x5456c0)result=updateParticleMotion(object,world());
      else if(entry==0x546120)result=updateFadingParticle(object,world(),r(0x6e1f88)!=0);
      else {auto facts=owner(U(object.get(0x60)));result=updateFadingParticle(object,world(),r(0x6e1f88)!=0,&facts);}
      save(self,object);return U(result);
    }
    case 0x546270:
      if(args.size()!=1)throw std::invalid_argument("Child delete flags");
      m.invoke(0x546290,self,{});
      if(*args.begin()&1)m.invoke(0x597870,0,{self});
      return self;
    case 0x546290:
      if(args.size())throw std::invalid_argument("Child destructor arguments");
      w(self,0x5c73a8);return 0;
    default:return ParticleComposition<M>(m).tryRun(entry,self,args,second);
    }
  }
};
}
