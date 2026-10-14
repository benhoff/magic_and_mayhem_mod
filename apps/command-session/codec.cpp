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
auto fields(Continuation &v){return std::tie(v.world,v.execution,v.bindings,v.generations,v.scheduler,v.environment,v.seed,v.backend);}
template<class T>struct optional_type:std::false_type{};
template<class T>struct optional_type<std::optional<T>>:std::true_type{};
template<class T>struct vector_type:std::false_type{};
template<class T,class A>struct vector_type<std::vector<T,A>>:std::true_type{};
template<class T>struct array_type:std::false_type{};
template<class T,std::size_t N>struct array_type<std::array<T,N>>:std::true_type{};
struct Writer {
  game::Bytes bytes;
  void word(std::uint32_t v){for(unsigned i=0;i<4;++i)bytes.push_back(std::uint8_t(v>>(8*i)));}
  template<class T>void value(T &v){
    if constexpr(std::is_integral_v<T> || std::is_enum_v<T>) word(std::uint32_t(v));
    else if constexpr(optional_type<T>::value){word(v.has_value());if(v)value(*v);}
    else if constexpr(vector_type<T>::value){word(std::uint32_t(v.size()));for(auto &x:v)value(x);}
    else if constexpr(array_type<T>::value){for(auto &x:v)value(x);}
    else std::apply([&](auto &...x){(value(x),...);},fields(v));
  }
};
struct Reader {
  const game::Bytes &bytes;std::size_t pos=0;
  std::uint32_t word(){need(bytes.size()-pos>=4,"truncated native command continuation");std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(bytes[pos++])<<(8*i);return v;}
  template<class T>void value(T &v){
    if constexpr(std::is_same_v<T,bool>){auto w=word();need(w<=1,"invalid native command boolean");v=w!=0;}
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
  const auto d=c.world.dimensions;
  need(d.x>0&&d.x<=4096&&d.y>0&&d.y<=4096&&d.z>0&&d.z<=256,"native command dimensions out of bounds");
  need(c.scheduler.cursor>=0&&unsigned(c.scheduler.cursor)<n&&c.scheduler.phase20>=0&&c.scheduler.phase20<20&&c.scheduler.phase90>=0&&c.scheduler.phase90<90,"native command scheduler out of bounds");
  need(c.backend.size()<=65536,"native command backend continuation limit");
  auto reference=[&](int v){need(v==-1||(v>=0&&unsigned(v)<n),"native command actor reference out of bounds");};
  for(unsigned i=0;i<n;++i){
    const auto &a=c.world.creatures[i];const auto &e=c.execution[i];
    need(a.ai.slot==i&&c.generations[i]!=0&&a.ai.owner>=0&&a.ai.owner<8&&unsigned(c.bindings[i])<=4,"native command actor identity out of bounds");
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
game::Bytes encode(const Continuation &input){
  validate(input);auto c=input;Writer w;w.bytes={'C','M','D','1'};w.word(1);w.value(c);
  need(w.bytes.size()<=1024*1024,"native command encoded continuation limit");return w.bytes;
}
Continuation decode(const game::Bytes &bytes){
  need(bytes.size()>=8&&bytes.size()<=1024*1024&&game::Bytes(bytes.begin(),bytes.begin()+4)==game::Bytes{'C','M','D','1'},"unknown native command continuation");
  Reader r{bytes,4};need(r.word()==1,"unsupported native command continuation version");Continuation c;r.value(c);need(r.pos==bytes.size(),"trailing native command continuation");validate(c);return c;
}
} // namespace mnm::command_session
