// Authored independently of original execution. Live prefixes contain distinct
// nonnull allocations. Only545910 callback-control cases use a supplied updater.
static constexpr U EmissionRoots[]={0x546ce0,0x545910,0x493f20,0x546350};
static constexpr U EmissionManager=0x6cbb8a,EmissionEffect=Base+0x8000;
static U heldEmission=0;
static void needEmission(bool value) {if(!value)throw std::runtime_error("Emitter ownership assertion");}
static bool emissionLive(const EmissionMemory &m,U token) {
  for(const auto &b:m.blocks)if(b.at==token)return b.alive;
  return false;
}
static U newEmissionChild(EmissionMemory &m,U seed,U level=100,U delay=0) {
  U token=m.invoke(0x597850,0,{100});needEmission(token!=0);
  m.invoke(0x545650,token,{seed%128,(seed*7)%96,31,1,U(-1),0});
  m.set32(token,0x5c7408);
  const U fields[][2]={{0x1c,128},{0x20,128},{0x24,32},{0x28,level},{0x2c,5},{0x30,delay},{0x34,2},{0x38,2},{0x3c,16},{0x40,m.get32(0x6e1f88)?16u:32u},{0x44,4},{0x54,0},{0x58,0},{0x5c,0},{0x60,0}};
  for(const auto &f:fields)m.set32(token+f[0],f[1]);
  return token;
}
static void initializeEmissionCase(EmissionMemory &m,U seed) {
  heldEmission=0;m.set32(0x5eb480,128);m.set32(0x5eb484,96);m.set32(0x5eb488,64);
  m.set32(0x6e1f88,(seed>>3)&1);m.set32(0x5e147c,2+seed%3);
  std::memset(m.pointer(EmissionManager,0x1c08),0,0x1c08);
  mnm::reconstruction::Kind8Random random(seed*37+1);
  m.set32(0x6574c8,random.first);m.set32(0x6574cc,random.second);
  for(U k=0;k<250;++k)m.set32(0x6574d0+k*4,random.words[k]);
  for(U k=0;k<4;++k) {
    U actor=Pool+k*0xe4b;m.set32(actor+0xac,Catalog);m.set32(actor+0xe4,1);
    m.set32(actor+0x14,U(-12));m.set32(actor+0x18,97);m.set32(actor+0x1c,7);
  }
  m.set32(Catalog+0x198,3);m.set32(Catalog+0x19c,5);
}
static void checkEmissionOwnership(EmissionMemory &m) {
  U count=m.get32(EmissionManager+0x1c00);needEmission(count<=0x700);
  std::set<U> owned;
  for(U k=0;k<count;++k) {
    U token=m.get32(EmissionManager+k*4);
    needEmission(token&&owned.insert(token).second&&emissionLive(m,token));
  }
  if(heldEmission)needEmission(owned.insert(heldEmission).second&&emissionLive(m,heldEmission));
  U pending=m.get32(Other+0x1d00);if(pending)needEmission(owned.insert(pending).second&&emissionLive(m,pending));
  for(const auto &b:m.blocks) {
    needEmission(b.alive==(owned.count(b.at)!=0));
    for(U k=0;k<16;++k)needEmission(m.get8(b.at-16+k)==0xa7&&m.get8(b.at+b.size+k)==0x7a);
  }
}
static U executeEmission(EmissionMemory &m,U root,U seed) {
  auto w=[&](U p,U v){m.set32(p,v);};
  bool full=seed%128==127,control=root==0x545910&&(seed&64);
  U count=root==0x546350?0:(full?0x700:seed%3),callbackToken=0;
  for(U k=0;k<count;++k)w(EmissionManager+k*4,newEmissionChild(m,k+seed,50,10000));
  w(EmissionManager+0x1c00,count);w(EmissionManager+0x1c04,0x1234);
  U parent=0;
  if(control) {
    if(!count) {count=1;w(EmissionManager,newEmissionChild(m,seed,50,10000));w(EmissionManager+0x1c00,count);}
    callbackToken=m.get32(EmissionManager);U table=Other+0x1c40;
    w(table,0x546270);w(table+16,0x1001f6c0);w(table+20,0x54aa80);w(callbackToken,table);
    w(Other+0x1d00,newEmissionChild(m,seed+1,200));w(Other+0x1d04,(seed>>7)&1);
  } else if(root==0x546350) {
    heldEmission=newEmissionChild(m,seed,seed%4==0?5:100,seed%3);
    if(seed&4) {
      w(heldEmission+0x60,Pool+2*0xe4b);
      for(U k=0;k<3;++k)w(heldEmission+0x54+k*4,(seed>>k)&1);
      w(Pool+2*0xe4b+0xe4,(seed>>4)&1);
    }
  } else if(root==0x493f20) {
    w(Manager,EmissionEffect);w(Manager+4,4);w(0x689498,EmissionEffect);w(0x68949c,4);
    for(U k=0;k<4;++k) {
      U p=EmissionEffect+k*0x22e;w(p,k);w(p+36,Manager);m.set16(p+0x1a0,0xffff);
      w(p+8,2);w(p+12,2);w(p+16,1);w(p+20,67);w(p+24,81);
    }
    U source=Buffer+256;std::memset(m.pointer(source,252),0,252);
    const U data[]={67,81,21,98,66,35,~0u,2,0};
    for(U k=0;k<9;++k)w(source+k*4,data[k]);
    w(0x5e1478,100);w(0x6b1270,1+seed%3);w(0x6b13cd,0);
    m.invoke(root,EmissionEffect+0x22e,{30,source});
    if(!full)parent=m.get32(EmissionManager+count*4);
    needEmission(m.get32(EmissionManager+0x1c00)==count+(full?0:1));
  } else {
    parent=m.invoke(0x597850,0,{100});needEmission(parent!=0);
    m.invoke(0x546c20,parent,{seed%128,seed%96,31});
    w(parent+0x14,seed%3);w(parent+0x18,1+(seed/3)%3);w(parent+0x1c,seed%2);
    w(parent+0x20,1+seed%3);w(parent+0x24,(seed>>3)&1);
    w(parent+0x34,seed&2?4:0);w(parent+0x44,seed&2?4:0);w(parent+0x48,seed&2?4:0);w(parent+0x4c,seed&2?3:0);w(parent+0x50,seed%3);
    if(seed&4) {
      w(parent+0x54,Pool+2*0xe4b);
      for(U k=0;k<3;++k)w(parent+0x58+k*4,(seed>>k)&1);
      w(Pool+2*0xe4b+0xe4,(seed>>4)&1);
    }
    if(root==0x546ce0)heldEmission=parent;
    else {
      U accepted=m.invoke(0x5457e0,EmissionManager,{parent});
      if(!accepted){m.invoke(0x546cb0,parent,{1});parent=0;}
    }
  }
  U before=m.allocations,failedBefore=m.allocationFailures;
  if(seed%4==0)m.failAllocation=m.allocations+1;
  if(seed%4==1)m.failAllocation=m.allocations+2;
  checkEmissionOwnership(m);m.snapshotProviders(root,0,2);
  for(U frame=1;frame<=32;++frame) {
    if(frame==2){w(Pool+2*0xe4b+0x14,129);w(Catalog+0x198,5);}
    if(frame==3)w(Pool+2*0xe4b+0xe4,0);
    if(frame==4)w(Pool+2*0xe4b+0xe4,1);
    std::vector<unsigned char> borrowed(m.pointer(Pool,4*0xe4b),m.pointer(Pool,4*0xe4b)+4*0xe4b),descriptor(m.pointer(Catalog,0x200),m.pointer(Catalog,0x200)+0x200);
    if(root==0x546ce0&&heldEmission) {
      if(!m.invoke(root,heldEmission,{})){m.invoke(0x546cb0,heldEmission,{1});heldEmission=0;}
    } else if(root==0x546350&&heldEmission) {
      if(!m.invoke(root,heldEmission,{})){m.invoke(0x546270,heldEmission,{1});heldEmission=0;}
    }
    // The real manager drives all adopted child updates, including newly
    // admitted children on the following tick, never in their birth callback.
    m.invoke(0x545910,EmissionManager,{});
    needEmission(!std::memcmp(borrowed.data(),m.pointer(Pool,4*0xe4b),borrowed.size())&&!std::memcmp(descriptor.data(),m.pointer(Catalog,0x200),descriptor.size()));
    checkEmissionOwnership(m);
    if(frame<=4)m.snapshotProviders(root,frame,frame+2);
    if(frame==1&&control&&full)needEmission(!emissionLive(m,callbackToken)&&m.get32(Other+0x1d0c)==1);
  }
  U failed=m.allocationFailures-failedBefore,created=m.allocations-before-failed;
  std::fprintf(stderr,"emission_outcome %x %u created %u failed %u full %u control %u nested %u evicted %u\n",root,seed,created,failed,U(full),U(control),m.get32(Other+0x1d0c),U(control&&full&&!emissionLive(m,callbackToken)));
  return 0;
}
static void releaseEmissionCase(EmissionMemory &m,U) {
  m.invoke(0x5458b0,EmissionManager,{});
  if(heldEmission) {m.invoke(m.get32(m.get32(heldEmission)),heldEmission,{1});heldEmission=0;}
  U pending=m.get32(Other+0x1d00);if(pending){m.invoke(0x546270,pending,{1});m.set32(Other+0x1d00,0);}
  checkEmissionOwnership(m);auto count=m.events.size();m.invoke(0x5458b0,EmissionManager,{});needEmission(m.events.size()==count);
}
