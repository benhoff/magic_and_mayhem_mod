  unsigned installed=0,expired=0,disabledSlices=0,pendingSlices=0;
  for(unsigned n=0;n<32;++n){
    memory.reset();const unsigned seed=17+n*137;
#ifdef MNM_BRAIN_REFERENCE
    m::AutonomousWorld world;auto v=fixture(0,world);
    v.sources.graph.resize(1);v.sources.information.resize(1);v.manager.nodes={0};v.pending={};v.pendingRoots.clear();
    v.sources.graph[0].parent.reset();v.sources.graph[0].children.clear();v.sources.graph[0].activity=3;
    RawSources raw(v);auto* b=raw.brain.data();auto* source=raw.nodes[0].data();
    global(0x6e1f64,0);global(0x6e1f54,0);global(0x6e1f58,0);global(0x643a80,1);global(0x643a80+12,0);
    global(0x6def58,address(raw.actors.data()));global(0x6def5c,3);global(0x6df180,3);
    put(raw.actors.data(),0xe4,100);raw.brain[0x19]=raw.brain[0x11dd3]=raw.brain[0x11fe0]=1;
    put(b,0x14,99);put(b,0x11ddd,bits(-2.f));put(b,0x11dd8,UINT32_MAX);
    std::array<unsigned,1> evaluators{{memory.make()}};put(b,8,address(evaluators.data()));put(b,12,address(evaluators.data()+1));
    auto rng=m::seedRandom(seed);global(0x6a29d8,rng.left);global(0x6a29dc,rng.right);std::memcpy(reinterpret_cast<void*>(0x6a29e0),rng.words.data(),1000);
    global(0x5d8650,0);global(0x5d8654,bits(m::OwnerReplacementPolicy{}.improvement));global(0x5d8658,bits(m::OwnerReplacementPolicy{}.expiry));
#else
    memory.set32(Borrowed+5,0);
    m::OwnerFallbackOwner<Storage> owner(memory,Borrowed,seed);
#endif
    unsigned beforeSerial=0;
    for(unsigned frame=0;frame<96;++frame){
      const int tick=100+int(frame),total=frame<8?int((frame+n)%4):int((frame+n)%13);
      const bool paused=(frame+n)%23==0;
      const bool invalidate=frame%29==17;
#ifdef MNM_BRAIN_REFERENCE
      if(invalidate)if(unsigned a=get(source,5))memory.set8(a+0x15,1);
      need(get(b,0x14)+1==unsigned(tick),"authored caller clock");
      global(0x6e1f54,paused);
      reinterpret_cast<void(__attribute__((thiscall)) *)(void*,int)>(0x415c40)(b,total);
      const unsigned a=get(source,5),serial=get(b,0xca);
      std::vector<unsigned> state{get(b,0x14),get(b,0x11dab)!=0,raw.brain[0x11dd3],get(b,0x11ddd),serial,unsigned(a!=0)};
      if(a){for(unsigned o:{0xcu,0x10u,0x1bu,0x1fu,0x27u,0x2bu,0x2fu,0x33u})state.push_back(memory.get32(a+o));state.push_back(memory.get8(a+0x14));state.push_back(memory.get8(a+0x15));}
      if(a){auto fields=memory.taskFields(a,address(source),true);state.insert(state.end(),fields.begin(),fields.end());}
      {auto fields=memory.taskFields(evaluators[0],address(source));state.insert(state.end(),fields.begin(),fields.end());}
      const unsigned rngLeft=*reinterpret_cast<unsigned*>(0x6a29d8),rngRight=*reinterpret_cast<unsigned*>(0x6a29dc);
      const auto* rngWords=reinterpret_cast<const unsigned*>(0x6a29e0);
#else
      if(invalidate)owner.invalidate();
      owner.advance(tick,total,paused);
      const unsigned a=owner.active,serial=owner.nextSerial;
      need(memory.get32(Borrowed+5)==a,"native task/source backlink");
      std::vector<unsigned> state{unsigned(tick),owner.scheduler.planningPending,owner.selection.reset,bits(owner.selection.value),serial,unsigned(a!=0)};
      if(a){for(unsigned o:{0xcu,0x10u,0x1bu,0x1fu,0x27u,0x2bu,0x2fu,0x33u})state.push_back(memory.get32(a+o));state.push_back(owner.scheduler.executionComplete);state.push_back(memory.get8(a+0x15));}
      if(a){auto fields=memory.taskFields(a,Borrowed,true);state.insert(state.end(),fields.begin(),fields.end());}
      {auto fields=memory.taskFields(owner.evaluator,Borrowed);state.insert(state.end(),fields.begin(),fields.end());}
      const unsigned rngLeft=owner.random.left,rngRight=owner.random.right;const auto* rngWords=owner.random.words.data();
#endif
      if(std::getenv("MNM_TASK_TRACE")){std::cerr<<n<<' '<<frame<<':';for(unsigned value:state)std::cerr<<' '<<value;std::cerr<<'\n';}
      for(unsigned value:state)record(value,value,"caller state");
      record(rngLeft,rngLeft,"caller RNG left");record(rngRight,rngRight,"caller RNG right");for(unsigned j=0;j<250;++j)record(rngWords[j],rngWords[j],"caller RNG word");
      for(unsigned g:{0x5faf2cu,0x5faf34u,0x5faf3cu,0x5faf44u}){unsigned count=memory.get32(g+4);record(count,count,"caller shared nil reference count");need(count==1+unsigned(a!=0),"caller fallback ownership conservation");}
      unsigned liveObjects=0;for(auto block:memory.blocks)if(block.n==0x14b && block.alive)++liveObjects;
      record(liveObjects,1+unsigned(a!=0),"one evaluator plus installed task");memory.guards();
      installed+=serial-beforeSerial;beforeSerial=serial;pendingSlices+=state[1];disabledSlices+=paused;++slices;
    }
#ifdef MNM_BRAIN_REFERENCE
    unsigned a=get(source,5);expired+=a!=0;put(b,0x14,256);
    reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x428b40)(b);
    need(!get(source,5),"original retirement unbinds source");
    memory.destroy(evaluators[0]);memory.free(evaluators[0]);
#else
    expired+=owner.active!=0;owner.expireAt(256);need(!owner.active,"native retirement unbinds source");owner.close();need(!memory.get32(Borrowed+5),"native close unbinds borrowed source");
#endif
    for(unsigned g:{0x5faf2cu,0x5faf34u,0x5faf3cu,0x5faf44u}){record(memory.get32(g),0,"last shared nil released");record(memory.get32(g+4),0,"last shared nil reference retired");}
    for(auto block:memory.blocks)if(block.n==0x14b)need(!block.alive,"no task objects after close");
    // Remaining original manager task-map/work-vector allocations belong to the
    // caller fixture. Explicit fixture teardown is separate from task ownership.
    for(auto block:memory.blocks)if(block.alive)memory.free(block.p);
    need(!memory.alive(),"fixture storage fully released");
  }
  need(installed && expired && pendingSlices && disabledSlices,"caller branch witnesses");
  record(installed,installed,"caller installations");record(expired,expired,"caller expiry drains");record(pendingSlices,pendingSlices,"caller pending slices");record(disabledSlices,disabledSlices,"caller disabled slices");
