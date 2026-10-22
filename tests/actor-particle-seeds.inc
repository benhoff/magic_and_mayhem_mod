// Authored independently of original execution. Full-prefix cases deliberately
// include initialized null holes, a branch supported by original5458b0; these
// do not claim1792 active emitter generations or update/emission validation.
static constexpr U ParticleRoots[]={0x546c20,0x546cb0,0x493f20,0x5458b0};
static constexpr U ParticleManager=0x6cbb8a, ParticleEffect=Base+0x8000;
static U directParticle=0;
static void needParticle(bool value) {
  if(!value) throw std::runtime_error("Particle ownership assertion");
}
static U liveParticles(const ParticleMemory &m) {
  U n=0;for(const auto &b:m.blocks) n+=b.alive;return n;
}
static U newParticle(ParticleMemory &m,U seed) {
  U p=m.invoke(0x597850,0,{100});needParticle(p!=0);
  for(U k=0;k<100;++k)m.set8(p+k,(k*37+seed*19)&255);
  U before=m.allocations;
  U width=m.get32(0x5eb480),height=m.get32(0x5eb484),top=m.get32(0x5eb488)*2;
  const U xs[]={0,width-1,width,width+1,U(-1),0-width,0-width-1,width*3+1};
  const U ys[]={height,height+1,0,0-height-1,height-1,U(-1),height*3+1,0-height};
  const U zs[]={0,top,top+1,U(-1),top+19,1};
  U result=m.invoke(0x546c20,p,{xs[seed%8],ys[(seed/8)%8],zs[(seed/64)%6]});
  needParticle(result==p&&m.allocations==before&&m.get32(p)==0x5c7438);
  return p;
}
static void initializeParticleCase(ParticleMemory &m,U seed) {
  directParticle=0;
  m.set32(0x5eb480,1+seed%193);m.set32(0x5eb484,1+(seed*7)%197);
  m.set32(0x5eb488,seed%41);
  const U periods[]={0,1,23,0xffffffff,0x80000000,0x7fffffff};
  m.set32(0x5e147c,periods[seed%6]);
  std::memset(m.pointer(ParticleManager,0x1c08),0,0x1c08);
}
static U executeParticle(ParticleMemory &m,U root,U seed) {
  if(root==0x546c20) {
    directParticle=newParticle(m,seed);needParticle(liveParticles(m)==1);
    return directParticle;
  }
  if(root==0x546cb0) {
    directParticle=newParticle(m,seed);U flags=seed%4;
    needParticle(m.invoke(root,directParticle,{flags})==directParticle);
    needParticle(liveParticles(m)==(flags&1?0u:1u));
    needParticle(m.get32(directParticle)==0x5c73a8);
    return directParticle;
  }
  U count=seed%3==0?0:seed%3==1?0x6ff:0x700;
  // Retain distinct actual emitter allocations at both ends of the prefix.
  if(count) {
    m.set32(ParticleManager,newParticle(m,seed+1));
    m.set32(ParticleManager+(count-1)*4,newParticle(m,seed+2));
  }
  m.set32(ParticleManager+0x1c00,count);
  m.set32(ParticleManager+0x1c04,seed&1?0xffffffff:0x400);
  U existing=liveParticles(m);
  if(root==0x5458b0) {
    m.invoke(root,ParticleManager,{});
    needParticle(liveParticles(m)==0&&m.get32(ParticleManager+0x1c00)==0&&m.get32(ParticleManager+0x1c04)==0);
    for(U k=0;k<count;++k)needParticle(m.get32(ParticleManager+k*4)==0);
    return 0;
  }
  needParticle(root==0x493f20);
  m.set32(Manager,ParticleEffect);m.set32(Manager+4,4);
  m.set32(0x689498,ParticleEffect);m.set32(0x68949c,4);
  for(U k=0;k<4;++k) {
    U p=ParticleEffect+k*0x22e;
    m.set32(p,k);m.set32(p+36,Manager);m.set16(p+0x1a0,0xffff);
    m.set32(p+8,2);m.set32(p+12,2);m.set32(p+16,1);
    m.set32(p+20,seed&1?U(-7):387);m.set32(p+24,seed&2?U(-9):401);
  }
  U self=ParticleEffect+0x22e,source=Buffer+256;
  std::memset(m.pointer(source,252),0,252);
  m.set32(source,seed&1?U(-3):67);m.set32(source+4,81);m.set32(source+8,21);
  m.set32(source+12,98);m.set32(source+16,66);m.set32(source+20,35);
  m.set32(source+24,seed&16?1:~0u); // sender owner, valid when selected
  m.set32(source+28,seed&32?(seed&64?99u:2u):~0u);
  U profile=seed%3;m.set32(source+32,profile);
  m.set32(0x6b13cd+profile*0x2d1,seed%3);
  m.set32(0x6b1270+profile*0x2d1,seed&128?0xfffffffe:7+seed);
  for(U k=0;k<9;++k)m.set32(0x5e143c+k*4,k*17+seed);
  bool enabled=seed%8!=0&&seed%8!=1,fail=seed%8==2;
  m.set32(0x5e1478,enabled?1+seed%101:(seed%8==0?0:0xffffffff));
  U previous=m.allocations;if(fail)m.failAllocation=previous+1;
  m.invoke(root,self,{30,source});
  needParticle(m.allocations==previous+(enabled?1:0));
  U admitted=enabled&&!fail&&count<0x700?1:0;
  needParticle(m.get32(ParticleManager+0x1c00)==count+admitted);
  needParticle(liveParticles(m)==existing+admitted);
  if(admitted) {
    U particle=m.get32(ParticleManager+count*4);
    needParticle(m.get32(particle)==0x5c7438);
    if(seed&32) {
      U expectedOwner=seed&64?0:m.get32(0x6def58)+2*0xe4b;
      needParticle(m.get32(particle+84)==expectedOwner&&m.get32(particle+88)==1&&m.get32(particle+92)==1);
    }
  }
  std::fprintf(stderr,"particle_outcome %u enabled %u failed %u admitted %u rejected %u\n",seed,U(enabled),U(enabled&&fail),admitted,U(enabled&&!fail&&count==0x700));
  return 0; //493f20 and5458b0 have no defined EAX result.
}
static void releaseParticleCase(ParticleMemory &m,U) {
  m.invoke(0x5458b0,ParticleManager,{});
  if(directParticle) {
    for(const auto &b:m.blocks) if(b.at==directParticle&&b.alive) {
      m.invoke(0x546cb0,directParticle,{1});break;
    }
  }
  needParticle(liveParticles(m)==0&&m.get32(ParticleManager+0x1c00)==0&&m.get32(ParticleManager+0x1c04)==0);
  // Empty retirement is idempotent and cannot issue another free.
  auto events=m.events.size();m.invoke(0x5458b0,ParticleManager,{});
  needParticle(m.events.size()==events);
}
