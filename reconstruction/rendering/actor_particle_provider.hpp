#pragma once
#include "actor_support_provider.hpp"
#include "particle_constructors.hpp"

namespace mnm::reconstruction::actor_services {
// PE32 value-address bridge to the unchanged recovered emitter constructor.
// Only the original 100-byte allocation is read/written; no host pointers escape.
template<class M> class ParticleComposition {
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p,U v) { m.set32(p,v); }
public:
  explicit ParticleComposition(M &memory):m(memory) {}
  std::optional<U> tryRun(U entry,U self,std::initializer_list<U> args={},U second=0) {
    switch(entry) {
    case 0x546c20: {
      if(args.size()!=3) throw std::invalid_argument("Emitter constructor arguments");
      ParticleObject object;
      for(U k=0;k<100;++k) object.bytes[k]=m.get8(self+k);
      auto a=args.begin();
      ParticleConstructorArguments input;
      input.position={particleSigned(a[0]),particleSigned(a[1]),particleSigned(a[2])};
      ParticleConstructorConfig config;
      config.emitterPeriod=particleSigned(r(0x5e147c));
      Kind8Random unused(0);
      constructParticleVariant(ParticleVariant::emitter,object,
        {particleSigned(r(0x5eb480)),particleSigned(r(0x5eb484)),particleSigned(r(0x5eb488))},
        input,config,{}, {},unused);
      for(U k=0;k<100;++k) m.set8(self+k,object.bytes[k]);
      return self;
    }
    case 0x546cb0: {
      if(args.size()!=1) throw std::invalid_argument("Emitter delete flags");
      // The inherited546cd0 body only resets the base vtable before free.
      w(self,0x5c73a8);
      if(*args.begin()&1) m.invoke(0x597870,0,{self});
      return self;
    }
    case 0x5458b0: {
      if(args.size()) throw std::invalid_argument("Particle manager clear arguments");
      // Reload the live count and publish each cleared slot before the next
      // callback, preserving original callback-visible ordering.
      for(U i=0;signed32(i)<signed32(r(self+0x1c00));++i) {
        U token=r(self+i*4);
        if(token) m.invoke(r(r(token)),token,{1});
        w(self+i*4,0);
      }
      w(self+0x1c00,0);w(self+0x1c04,0);
      return 0;
    }
    default:return SupportComposition<M>(m).tryRun(entry,self,args,second);
    }
  }
};
}
