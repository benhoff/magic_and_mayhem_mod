#include "runtime.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
#include <tuple>
#include <type_traits>
namespace mnm::command_session {
namespace {
void need(bool ok,const char *why){if(!ok)throw std::invalid_argument(why);}
auto fields(model::Cell &v){return std::tie(v.x,v.y,v.z);}
auto fields(model::Extent &v){return std::tie(v.x,v.y,v.z);}
auto fields(model::InventoryEntry &v){return std::tie(v.kind,v.magnitude,v.count,v.available);}
auto fields(model::AiCreature &v){return std::tie(v.slot,v.type,v.kind,v.health,v.maximumHealth,v.behavior,v.pending,v.action,v.history,v.recent,v.engaged,v.incoming,v.flying,v.ranged,v.physical,v.corpseA,v.corpseB,v.corpseClaim,v.gateFC,v.flag704,v.countdown,v.flag815,v.flag80d,v.dirty932,v.readiness,v.marker,v.owner,v.age,v.bravery,v.target,v.active,v.motion,v.bloodLust,v.position,v.home,v.visible,v.hth,v.enemyRadius,v.allyRadius,v.retaliateThreshold);}
auto fields(model::AutonomousCreature &v){return std::tie(v.ai,v.width,v.height,v.animationKind,v.inventoryCount,v.block7c,v.configFlags,v.hazardSensitive,v.protection8c9,v.mode,v.form,v.formCapability,v.block784,v.blockDE3,v.block71c,v.block723,v.specialVision,v.routePhase,v.substate,v.movementKind,v.flag104,v.selectedSpell,v.targetMovingAttacker,v.routeMode,v.castSpell,v.alternateTarget,v.rangedDistance,v.region,v.range,v.castSource,v.castTarget,v.cooldown,v.destination,v.chosenDestination,v.finePosition,v.aimXY,v.aimZ,v.inventory);}
auto fields(model::CreatureOrder &v){return std::tie(v.kind,v.serial,v.destination,v.target,v.effect,v.referenceDestination,v.repeat);}
auto fields(model::CreatureExecution &v){return std::tie(v.orders,v.orderCursor,v.routeEffect,v.follow,v.castAim,v.displacementDestination,v.roamHome,v.roamRange,v.roamKind,v.roamAnnulus,v.roamInterval,v.roamCooldown,v.fearBias,v.combatValue,v.actionRouteCounters,v.actionCallback,v.queuedCount,v.queuedFlag,v.absorptionTimer,v.block962,v.block963,v.block722,v.markerB73,v.marker6a0,v.routeRenewal,v.routeChanged,v.mode10c,v.marker68,v.terrainRelocation,v.dirtyValue,v.commandMode,v.nextOrderSerial,v.targetDistance,v.valueScale8ed,v.bonusMode124,v.power84,v.mitigation90,v.rangedDeadline,v.rangedLatch,v.animationOffset,v.animationFacing,v.previousMove,v.previousMoveVariant,v.moveVariant,v.block855,v.strategyGuard,v.displacementActive);}
auto fields(model::AutonomousWorld &v){return std::tie(v.dimensions,v.creatures,v.relation,v.human,v.ownerEligible,v.localOwner);}
auto fields(model::CreatureScheduler &v){return std::tie(v.budget,v.cursor,v.phase20,v.phase90,v.continuation);}
auto fields(model::SchedulerEnvironment &v){return std::tie(v.suppressRoutes,v.updateRegions,v.network,v.localOwner,v.ownerController);}
auto fields(model::CreatureMaintenance &v){return std::tie(v.random98,v.upkeepDeadline,v.status114,v.status118,v.status744,v.status748,v.status74c,v.flight110,v.status778,v.status77c,v.status780,v.animation7cc,v.status821,v.status8c5,v.status78c,v.status790,v.status819,v.status81d,v.status8d5,v.status8d9,v.status8dd,v.powerDeadline,v.animation889,v.animation8bd,v.flight809,v.timer94,v.marker750,v.block71d,v.timer660,v.recent6b0,v.lifespan,v.suspension,v.status8cd,v.visibilityAge,v.statusEffect,v.damageEffect,v.flightEffect);}
auto fields(model::MovementBehaviorState &v){return std::tie(v.renewedOrder,v.landingKind,v.nextRouteCell);}
auto fields(model::ActionCallbackState &v){return std::tie(v.sequence,v.contributors,v.animationBreak);}
auto fields(model::MotionActionState &v){return std::tie(v.accumulator,v.duration,v.progress,v.travelX,v.travelY,v.heightDelta,v.heightStep,v.heightOrigin,v.step,v.deltaX,v.deltaY,v.direction,v.sampleCursor,v.initialCursor,v.samples,v.initialResidualX,v.initialResidualY,v.vertical,v.displacementPause,v.visualSprite);}
auto fields(model::SegmentRoutePoint &v){return std::tie(v.position,v.remainder);}
auto fields(model::SegmentContributionNode &v){return std::tie(v.parent,v.owner);}
auto fields(model::SegmentCompletionState &v){return std::tie(v.route,v.cursor,v.count,v.landing110,v.overlayFrame809,v.landingOverlay71d,v.scorePendingFc,v.regionPending,v.overlayLink805,v.contributionRoots,v.contributionNodes,v.contributionTotals,v.regionPath,v.regionCursor);}
auto fields(model::ActionObject &v){return std::tie(v.kind,v.next);}
auto fields(model::ObjectActionWorld &v){return std::tie(v.dimensions,v.heads,v.objects);}
auto fields(model::CommandActionBodyState &v){return std::tie(v.callbacks,v.motion,v.segments,v.objects);}
auto fields(reconstruction::AnimationState &v){return std::tie(v.pc,v.displayedRecord,v.active,v.delay,v.elapsed,v.repeats,v.breakFlag);}
auto fields(reconstruction::AnimationControllerState &v){return std::tie(v.forward,v.reverse,v.reverseCursor,v.reverseLength);}
auto fields(model::RandomState &v){return std::tie(v.left,v.right,v.words);}
auto fields(NativeAnimationChannel &v){return std::tie(v.asset,v.sequence,v.bound,v.state);}
auto fields(NativeActorAnimation &v){return std::tie(v.body,v.overlay,v.auxiliary);}
auto fields(NativeGeometryCell &v){return std::tie(v.kind,v.occupant,v.flags,v.links);}
auto fields(NativeOpenNode &v){return std::tie(v.priority,v.node);}
auto fields(reconstruction::MovementPayload &v){return std::tie(v.category,v.unknown_04);}
auto fields(reconstruction::NodeRecord &v){return std::tie(v.priority,v.predecessor,v.movement);}
auto fields(NativeSearchNode &v){return std::tie(v.node,v.record);}
auto fields(NativeSearchContinuation &v){return std::tie(v.open,v.records,v.bestNode,v.bestHeuristic,v.initialized,v.destination,v.context);}
auto fields(model::SegmentInitializationState &v){return std::tie(v.vertical,v.slow778,v.fast77c,v.specialRate73c,v.previousDirection,v.sampleBank,v.initialSampleBank,v.inlineSample);}
auto fields(model::EnvironmentPassState &v){return std::tie(v.cursor,v.filter,v.receiverAdjustment);}
auto fieldsV4(NativeProviderState &v){return std::tie(v.randomSeeded,v.alternate,v.profileStamp,v.tick,v.sequence,v.random,v.animation,v.worldAnimation,v.cells,v.search,v.initialization,v.environment);}
auto fields(model::ActorRouteSnapshot &v){return std::tie(v.destination,v.cursor,v.count,v.points,v.motionWords);}
auto fields(model::ActorRouteMetadata &v){return std::tie(v.regionDestination,v.marker69c);}
auto fields(model::ActorRouteContext &v){return std::tie(v.active,v.displacementContinuation,v.mode,v.budget,v.requestBudget,v.destination,v.origin,v.scratch,v.scratchDirty,v.useRegions,v.diagnostic);}
auto fieldsV5(NativeProviderState &v){return std::tie(v.randomSeeded,v.alternate,v.profileStamp,v.tick,v.sequence,v.random,v.animation,v.worldAnimation,v.cells,v.search,v.initialization,v.environment,v.route,v.routeMetadata);}
auto fields(NativeProviderState &v){return std::tie(v.randomSeeded,v.alternate,v.profileStamp,v.tick,v.sequence,v.random,v.animation,v.worldAnimation,v.cells,v.search,v.initialization,v.environment,v.route,v.routeMetadata,v.displacementOwners);}
auto fieldsV1(Continuation &v){return std::tie(v.world,v.execution,v.bindings,v.generations,v.scheduler,v.environment,v.seed,v.backend);}
auto fieldsV2(Continuation &v){return std::tie(v.world,v.execution,v.bindings,v.generations,v.scheduler,v.environment,v.seed,v.backend,v.maintenance,v.movement);}
auto fieldsV3(Continuation &v){return std::tie(v.world,v.execution,v.bindings,v.generations,v.scheduler,v.environment,v.seed,v.backend,v.maintenance,v.movement,v.originalBehaviorBindings,v.actions);}
auto fields(Continuation &v){return std::tie(v.world,v.execution,v.bindings,v.generations,v.scheduler,v.environment,v.seed,v.backend,v.maintenance,v.movement,v.originalBehaviorBindings,v.actions,v.native);}
template<class T>struct optional_type:std::false_type{};
template<class T>struct optional_type<std::optional<T>>:std::true_type{};
template<class T>struct vector_type:std::false_type{};
template<class T,class A>struct vector_type<std::vector<T,A>>:std::true_type{};
template<class T>struct array_type:std::false_type{};
template<class T,std::size_t N>struct array_type<std::array<T,N>>:std::true_type{};
struct Writer {
  unsigned version=6;
  game::Bytes bytes;
  void word(std::uint32_t v){for(unsigned i=0;i<4;++i)bytes.push_back(std::uint8_t(v>>(8*i)));}
  template<class T>void value(T &v){
    if constexpr(std::is_same_v<T,NativeProviderState>){word(v.present);if(v.present){if(version<5)std::apply([&](auto &...x){(value(x),...);},fieldsV4(v));else if(version==5)std::apply([&](auto &...x){(value(x),...);},fieldsV5(v));else std::apply([&](auto &...x){(value(x),...);},fields(v));}}
    else if constexpr(std::is_integral_v<T> || std::is_enum_v<T>){
      if constexpr(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T)>4)need(v<=UINT32_MAX,"native command word overflow");
      word(std::uint32_t(v));
    }
    else if constexpr(optional_type<T>::value){word(v.has_value());if(v)value(*v);}
    else if constexpr(vector_type<T>::value){word(std::uint32_t(v.size()));for(auto &x:v)value(x);}
    else if constexpr(array_type<T>::value){for(auto &x:v)value(x);}
    else std::apply([&](auto &...x){(value(x),...);},fields(v));
  }
};
struct Reader {
  const game::Bytes &bytes;std::size_t pos=0;unsigned version=6;
  std::uint32_t word(){need(bytes.size()-pos>=4,"truncated native command continuation");std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(bytes[pos++])<<(8*i);return v;}
  template<class T>void value(T &v){
    if constexpr(std::is_same_v<T,NativeProviderState>){auto tag=word();need(tag<=1,"invalid native provider tag");v.present=tag!=0;if(v.present){if(version<5)std::apply([&](auto &...x){(value(x),...);},fieldsV4(v));else if(version==5)std::apply([&](auto &...x){(value(x),...);},fieldsV5(v));else std::apply([&](auto &...x){(value(x),...);},fields(v));}}
    else if constexpr(std::is_same_v<T,bool>){auto w=word();need(w<=1,"invalid native command boolean");v=w!=0;}
    else if constexpr(std::is_same_v<T,std::byte>){auto w=word();need(w<=255,"native command opaque byte overflow");v=std::byte(w);}
    else if constexpr(std::is_enum_v<T>) v=static_cast<T>(word());
    else if constexpr(std::is_integral_v<T>){
      auto w=word();
      if constexpr(std::is_signed_v<T>){
        const std::int64_t n=w<=0x7fffffff?std::int64_t(w):-1-std::int64_t(~w);
        need(n>=std::numeric_limits<T>::min()&&n<=std::numeric_limits<T>::max(),"native command signed field overflow");v=T(n);
      }else{need(w<=std::numeric_limits<T>::max(),"native command unsigned field overflow");v=T(w);}
    }else if constexpr(optional_type<T>::value){auto tag=word();need(tag<=1,"invalid native command optional");if(tag){v.emplace();value(*v);}else v.reset();}
    else if constexpr(vector_type<T>::value){
      auto count=word();
      constexpr unsigned limit=std::is_same_v<typename T::value_type,std::uint8_t>?65536:
          std::is_same_v<typename T::value_type,std::uint16_t>?65536:
          (std::is_same_v<typename T::value_type,NativeGeometryCell> || std::is_same_v<typename T::value_type,NativeOpenNode> || std::is_same_v<typename T::value_type,NativeSearchNode>)?65536:
          std::is_same_v<typename T::value_type,model::CreatureOrder>?15:128;
      need(count<=limit,"native command continuation vector limit");v.resize(count);for(auto &x:v)value(x);
    }else if constexpr(array_type<T>::value){for(auto &x:v)value(x);}
    else std::apply([&](auto &...x){(value(x),...);},fields(v));
  }
};
} // namespace
void validate(const Continuation &c){
  const auto n=c.world.creatures.size();
  need(n>0&&n<=128&&c.execution.size()==n&&c.bindings.size()==n&&c.generations.size()==n,"native command continuation pool differs");
  need((c.maintenance.empty()&&c.movement.empty()) ||
       (c.maintenance.size()==n&&c.movement.size()==n),
       "native command behavior continuation pool differs");
  const auto d=c.world.dimensions;
  need(d.x>0&&d.x<=4096&&d.y>0&&d.y<=4096&&d.z>0&&d.z<=256,"native command dimensions out of bounds");
  const auto &a=c.actions;
  need((a.callbacks.empty()&&a.motion.empty()&&a.segments.empty()&&a.objects.heads.empty()&&a.objects.objects.empty()) ||
       (a.callbacks.size()==n&&a.motion.size()==n&&a.segments.size()==n&&c.maintenance.size()==n),
       "native command action continuation pool differs");
  if(!a.callbacks.empty()) {
    need(a.objects.dimensions.x==d.x&&a.objects.dimensions.y==d.y&&a.objects.dimensions.z==d.z&&
         a.objects.heads.size()==std::uint64_t(d.x)*d.y*d.z&&a.objects.heads.size()<=65536&&a.objects.objects.size()<=128,
         "native command object action world differs");
    for(auto h:a.objects.heads)need(h==0xffff||h<a.objects.objects.size(),"native command object head differs");
    for(const auto &o:a.objects.objects)need(o.next==0xffff||o.next<a.objects.objects.size(),"native command object link differs");
    for(const auto &s:a.segments)need(s.count<=s.route.size()&&s.cursor<=s.count&&s.regionCursor<s.regionPath.size(),"native command segment cursor differs");
  }
  const auto &native=c.native;
  if(native.present){
    need(c.originalBehaviorBindings&&a.callbacks.size()==n&&native.animation.size()==n&&native.search.size()==n&&native.initialization.size()==n,
         "native provider actor roots differ");
    need(native.cells.size()==std::uint64_t(d.x)*d.y*d.z&&native.cells.size()<=65536&&native.random.left<250&&native.random.right<250,
         "native provider geometry/random roots differ");
    need(native.displacementOwners.size()==n,"native displacement owner storage differs");for(auto owner:native.displacementOwners)need(owner>=0&&unsigned(owner)<n,"native displacement owner reference differs");
    need(native.routeMetadata.empty()||native.routeMetadata.size()==n,"native actor route metadata differs");
    need(native.route.scratch.count<=16&&native.route.scratch.cursor<=native.route.scratch.count&&!native.route.useRegions&&native.route.active<=1&&native.route.mode<=1&&native.route.displacementContinuation<=1,"native actor route context differs");
    need((native.route.active!=0)==c.scheduler.continuation,"native shared request continuation alias differs");
    for(const auto &cell:native.cells)need(cell.occupant==0xffff||(cell.occupant>=1&&cell.occupant<=n),"native provider occupant differs");
    auto channel=[](const NativeAnimationChannel &ch){const auto &s=ch.state;need(s.forward.pc<=65536&&s.forward.elapsed<=s.forward.delay&&s.forward.breakFlag<=1&&s.reverseCursor<=s.reverseLength&&s.reverseLength<=65536,"native ANI continuation differs");};
    for(const auto &ani:native.animation){channel(ani.body);channel(ani.overlay);for(const auto &aux:ani.auxiliary)channel(aux);}
    for(const auto &ch:native.worldAnimation)channel(ch);
  }else need(native.animation.empty()&&native.cells.empty()&&native.search.empty()&&native.initialization.empty(),"absent native provider roots differ");
  need(c.scheduler.cursor>=0&&unsigned(c.scheduler.cursor)<n&&c.scheduler.phase20>=0&&c.scheduler.phase20<20&&c.scheduler.phase90>=0&&c.scheduler.phase90<90,"native command scheduler out of bounds");
  need(c.backend.size()<=65536,"native command backend continuation limit");
  auto reference=[&](int v){need(v==-1||(v>=0&&unsigned(v)<n),"native command actor reference out of bounds");};
  for(unsigned i=0;i<n;++i){
    const auto &a=c.world.creatures[i];const auto &e=c.execution[i];
    need(a.ai.slot==i&&c.generations[i]!=0&&a.ai.owner>=0&&a.ai.owner<8&&unsigned(c.bindings[i])<=(c.originalBehaviorBindings?16u:4u),"native command actor identity out of bounds");
    need(a.ai.position.x>=0&&a.ai.position.x<d.x&&a.ai.position.y>=0&&a.ai.position.y<d.y&&a.ai.position.z>=0&&a.ai.position.z<d.z,"native command actor position out of bounds");
    reference(a.ai.target);reference(a.alternateTarget);reference(a.castSource);reference(a.castTarget);reference(e.follow);
    need(e.orders.size()<=15&&e.queuedCount==e.orders.size(),"native command queue count differs");
    std::set<unsigned>serials;
    for(const auto &o:e.orders){
      need(o.serial!=0&&serials.insert(o.serial).second,"native command queue serial differs");reference(o.target);
      if(o.kind==3)need(o.target>=0,"native command target order is missing its actor");
    }
    need(e.nextOrderSerial!=0&&(serials.empty()||e.nextOrderSerial>*serials.rbegin()),"native command next serial is exhausted");
    if(!e.orders.empty())need((e.orderCursor&&serials.count(*e.orderCursor))||(!e.orderCursor&&e.queuedFlag),"unsafe native command queue continuation");
  }
}
game::Bytes encode(const Continuation &input, unsigned version){
  validate(input);need(version>=1&&version<=6,"unsupported native command write version");
  if(version<4)need(!input.native.present,"legacy command write loses native providers");
  if(version<3)need(!input.originalBehaviorBindings&&input.actions.callbacks.empty(),"legacy command write loses action state");
  if(version==1)need(input.maintenance.empty()&&input.movement.empty(),"legacy command write loses behavior state");
  if(version<6&&input.native.present){need(!input.native.route.displacementContinuation,"legacy command write loses displacement continuation");for(auto owner:input.native.displacementOwners)need(!owner,"legacy command write loses displacement owners");}
  if(version<5&&input.native.present){auto q=input.native.route;model::ActorRouteContext empty;Writer actual,zero;actual.value(q);zero.value(empty);need(actual.bytes==zero.bytes&&input.native.routeMetadata.empty(),"legacy command write loses actor request");for(const auto &s:input.native.search)need(!s.initialized&&s.open.empty()&&s.records.empty(),"legacy actor search cannot migrate to shared request");for(const auto &a:input.world.creatures)need(!a.ai.motion,"legacy relative sample cursor cannot migrate during motion");}
  auto c=input;Writer w;w.version=version;w.bytes={'C','M','D','1'};w.word(version);
  if(version==1)std::apply([&](auto &...x){(w.value(x),...);},fieldsV1(c));
  else if(version==2)std::apply([&](auto &...x){(w.value(x),...);},fieldsV2(c));
  else if(version==3)std::apply([&](auto &...x){(w.value(x),...);},fieldsV3(c));
  else w.value(c);
  need(w.bytes.size()<=1024*1024,"native command encoded continuation limit");return w.bytes;
}
Continuation decode(const game::Bytes &bytes){
  need(bytes.size()>=8&&bytes.size()<=1024*1024&&game::Bytes(bytes.begin(),bytes.begin()+4)==game::Bytes{'C','M','D','1'},"unknown native command continuation");
  Reader r{bytes,4};const auto version=r.word();need(version>=1&&version<=6,"unsupported native command continuation version");Continuation c;r.version=version;
  if(version==1)std::apply([&](auto &...x){(r.value(x),...);},fieldsV1(c));
  else if(version==2)std::apply([&](auto &...x){(r.value(x),...);},fieldsV2(c));
  else if(version==3)std::apply([&](auto &...x){(r.value(x),...);},fieldsV3(c));
  else r.value(c);
  need(r.pos==bytes.size(),"trailing native command continuation");
  if(version<6&&c.native.present)c.native.displacementOwners.resize(c.world.creatures.size());
  if(version==4&&c.native.present){for(const auto &s:c.native.search)need(!s.initialized&&s.open.empty()&&s.records.empty(),"legacy actor search cannot migrate to shared request");for(const auto &a:c.world.creatures)need(!a.ai.motion,"legacy relative sample cursor cannot migrate during motion");}
  validate(c);return c;
}
} // namespace mnm::command_session
