#include "frozen_navigation.hpp"
#include "route_world.hpp"
#include "route_cell_support.hpp"
#include "simulation/occupancy.hpp"
#include "creature_motion.hpp"
#include "segment_setup.hpp"
#include "no_cd.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace mnm::sandbox {
namespace {
namespace r=mnm::reconstruction;
namespace g=mnm::game;
template<class T> T read(const void* p,std::size_t offset) {T value;std::memcpy(&value,static_cast<const std::byte*>(p)+offset,sizeof(value));return value;}
template<class T> void put(void* p,std::size_t offset,T value) {std::memcpy(static_cast<std::byte*>(p)+offset,&value,sizeof(value));}
r::Coordinates coordinate(g::Point p) {return {p.x,p.y,p.z};}
g::Point point(r::Coordinates p) {return {p.x,p.y,p.z};}
class FrozenNavigation final:public g::Navigation {
    std::shared_ptr<const r::RouteWorldSnapshot> frozen_;
    g::NavigationBinding binding_;
    bool stationaryOccupancy_=false,multiMovement_=false;
    std::optional<g::AnimationBinding> animation_;
    std::array<std::vector<assets::AnimationRecord>,8> sequences_;
    static r::AnimationState controllerState(const g::AnimationCursor& c) {
        r::AnimationState s;s.pc=c.pc;if(c.displayed) s.displayedRecord=*c.displayed;
        s.active=c.active;s.delay=c.delay;s.elapsed=c.elapsed;s.repeats=c.repeats;s.breakFlag=c.breakFlag;return s;
    }
    static g::AnimationCursor cursor(std::uint32_t sequence,const r::AnimationState& s) {
        g::AnimationCursor c;c.sequence=sequence;c.pc=s.pc;if(s.displayedRecord) c.displayed=*s.displayedRecord;
        c.active=s.active;c.delay=s.delay;c.elapsed=s.elapsed;c.repeats=s.repeats;c.breakFlag=s.breakFlag;return c;
    }
    r::NoCdAnimationPlayer player(const g::AnimationCursor& c) const {
        if(!animation_ || !c.active || c.sequence<animation_->sequenceBase || c.sequence>=animation_->sequenceBase+8)
            throw std::invalid_argument("saved animation sequence outside movement profile");
        r::NoCdAnimationPlayer result(sequences_.at(c.sequence-animation_->sequenceBase));result.restore(controllerState(c));return result;
    }
    bool advance(g::FineMotion& f,r::MotionInputs p) const {
        auto state=motionState(f);
        if(!animation_) {auto complete=r::advance_creature_motion(state,p);store(f,state);return complete;}
        if(!f.animation) throw std::invalid_argument("missing animation continuation");
        auto staged=player(*f.animation);
        r::MotionAnimation events{[&]{return staged.tick();},[&]{staged.start();}};
        const auto complete=r::advance_creature_motion(state,p,&events);
        store(f,state);f.animation=cursor(f.animation->sequence,staged.state());return complete;
    }
    std::shared_ptr<r::RouteWorldSnapshot> inputs(const g::Entity& e,g::Point target) const {
        if(e.type!=creatureType() || !g::contains({e.x,e.y,e.z},binding_) || !g::contains(target,binding_))
            throw std::invalid_argument("frozen navigation profile/coordinates mismatch");
        auto copy=std::make_shared<r::RouteWorldSnapshot>(*frozen_);
        put(&copy->object,8,e.x);put(&copy->object,12,e.y);put(&copy->object,16,e.z);
        copy->target=coordinate(target);return copy;
    }
    r::NeighborHelpers checkedNeighbors(std::shared_ptr<const r::RouteWorldSnapshot> snapshot) const {
        auto neighbors=r::snapshot_neighbors(std::move(snapshot));
        if(stationaryOccupancy_) {
            const auto accept=neighbors.accept;const auto dims=binding_.dimensions;
            const auto width=read<int>(frozen_->scalar_type.data(),8),height=read<int>(frozen_->scalar_type.data(),12);const auto multi=multiMovement_;
            // Native blocker policy: even the requested goal must pass occupancy.
            // The original selected-goal special flag otherwise bypasses it.
            neighbors.accept=[accept,dims,width,height,multi](const r::NeighborDescriptor& desc,r::Coordinates from,r::Coordinates to,
                const r::NeighborRecord& record,bool,std::int32_t& category) {
                if(multi && (std::llabs(std::int64_t(to.x)-from.x)>1 || std::llabs(std::int64_t(to.y)-from.y)>1 || std::llabs(std::int64_t(to.z)-from.z)>1)) {category=5;return false;}
                if(to.x<0 || to.y<0 || to.z<0 || to.x>dims.x-width || to.y>dims.y-width || to.z>dims.z-height) {category=5;return false;}
                return accept(desc,from,to,record,false,category);
            };
        }
        return neighbors;
    }
    g::Occupancy occupancy(const g::State& state) const {
        return g::Occupancy(state,binding_.dimensions,creatureType(),
            {read<int>(frozen_->scalar_type.data(),8),read<int>(frozen_->scalar_type.data(),8),read<int>(frozen_->scalar_type.data(),12)});
    }
    std::shared_ptr<r::RouteWorldSnapshot> occupiedInputs(const g::State& state,g::Handle self,g::Point target) const {
        const auto& slot=state.slots.at(self.slot);
        if(slot.generation!=self.generation || !slot.entity || !slot.entity->motion)
            throw std::invalid_argument("stale/nonmoving occupancy actor");
        auto copy=inputs(*slot.entity,target);const auto owned=occupancy(state);
        std::optional<g::MovementReservations> reservations;
        if(multiMovement_) reservations.emplace(state,binding_.dimensions,creatureType(),creatureFootprint());
        put(&copy->object,0,self.slot); // Deliberate slot-token projection; generation checked above.
        const auto d=binding_.dimensions;
        for(int z=0;z<d.z;++z) for(int y=0;y<d.y;++y) for(int x=0;x<d.x;++x) {
            const auto h=reservations?reservations->owner({x,y,z}):owned.owner({x,y,z});if(!h) continue;
            auto& cell=copy->cells.at(std::size_t(copy->layers.at(z))+copy->rows.at(y)+x);
            cell.flags_0a|=1;cell.occupant_04=static_cast<std::uint16_t>(h->slot);
        }
        return copy;
    }
public:
    FrozenNavigation(std::shared_ptr<const r::RouteWorldSnapshot> snapshot,std::uint64_t hash,const std::optional<g::AnimationBinding>& animation,bool stationaryOccupancy,bool multiMovement):frozen_(std::move(snapshot)),stationaryOccupancy_(stationaryOccupancy),multiMovement_(multiMovement),animation_(animation) {
        if(multiMovement_ && !stationaryOccupancy_) throw std::invalid_argument("multi-mover policy requires occupancy");
        if(animation_) {
            auto decoded=assets::decodeAnimation(animation_->data);
            if(auto* error=std::get_if<assets::AnimationError>(&decoded)) throw std::invalid_argument(error->detail);
            auto data=std::get<assets::Animation>(std::move(decoded));
            if(data.version!=5 || animation_->sequenceBase>4088 || animation_->sequenceBase+8>=data.starts.size())
                throw std::invalid_argument("ANI movement needs eight version-5 sequences at the selected base");
            for(unsigned direction=0;direction<8;++direction) {
                const auto index=animation_->sequenceBase+direction;
                sequences_[direction]={data.records.begin()+data.starts[index],data.records.begin()+data.starts[index+1]};
                for(const auto& record:sequences_[direction]) if((record.opcode==5 && record.argument!=0 && record.argument!=2) || (record.opcode==0 && record.argument<0))
                    throw std::invalid_argument("unsupported movement ANI event/sprite profile");
            }
        }
        const auto d=frozen_->dimensions;
        if(std::uint64_t(d.x)*d.y*d.z>4096 || frozen_->cells.size()>4096 || !std::isfinite(frozen_->slope_global))
            throw std::invalid_argument("movement sandbox frozen map exceeds bounded slice");
        const auto width=read<std::int32_t>(frozen_->scalar_type.data(),8);
        const auto height=read<std::int32_t>(frozen_->scalar_type.data(),12);
        if(width<1 || width>2 || height<1 || height>d.z || frozen_->boundary<0 || frozen_->boundary>d.z)
            throw std::invalid_argument("unsupported frozen creature/map geometry");
        if(stationaryOccupancy_) {
            for(const auto& cell:frozen_->cells) if((cell.flags_0a&3) && cell.occupant_04!=0xffff)
                throw std::invalid_argument("stationary overlay requires an occupancy-free frozen baseline");
            // Version the native occupancy policy in the existing resource identity.
            for(unsigned char c:std::string("stationary-occupancy-v1")) {hash^=c;hash*=1099511628211ULL;}
        }
        if(multiMovement_) for(unsigned char c:std::string("multi-creature-occupancy-v1")) {hash^=c;hash*=1099511628211ULL;}
        binding_={point({d.x,d.y,d.z}),hash};
        (void)r::snapshot_neighbors(frozen_); // Validate full row/layer ownership at resource admission.
    }
    g::NavigationBinding binding() const override {return binding_;}
    int terrainOffset(g::Point at) const {
        if(!g::contains(at,binding_)) throw std::invalid_argument("terrain height coordinate outside map");
        // Generator type +8 selects the special 004f3260 snap branch, not modeled here.
        if(read<int>(frozen_->generator_type.data(),8)==2) throw std::invalid_argument("special creature terrain height profile unavailable");
        const auto index=std::int64_t(frozen_->layers.at(at.z))+frozen_->rows.at(at.y)+at.x;
        if(index<0 || std::uint64_t(index)>=frozen_->cells.size()) throw std::invalid_argument("terrain height cell outside owned storage");
        const auto id=read<std::uint16_t>(&frozen_->cells.at(index),0);
        if(id>=frozen_->terrain.size()) throw std::invalid_argument("terrain height definition unavailable");
        const auto height=id?frozen_->terrain.at(id).classification_94:0;
        if(height<-16 || height>16) throw std::invalid_argument("terrain height outside bounded ordinary profile");
        return height;
    }
    void validateSpawnPosition(g::Point p) const override {
        if(!g::contains(p,binding_) || p.z==0) throw std::invalid_argument("Spawn cell outside standing terrain");
        const r::CellValidityMapView view{{frozen_->dimensions,frozen_->cells.data(),frozen_->cells.size(),frozen_->rows.data(),frozen_->rows.size(),frozen_->layers.data(),frozen_->layers.size(),frozen_->plane_stride,frozen_->dimensions.z},frozen_->terrain.data(),frozen_->terrain.size()};
        const r::CreatureMovementParameters profile{read<int>(frozen_->scalar_type.data(),12),read<int>(frozen_->scalar_type.data(),8),0,read<int>(frozen_->scalar_type.data(),0x44),0,int(creatureType()),0};
        (void)terrainOffset(p);
        if(!r::test_cell_validity(coordinate(p),profile,view) || !r::test_cell_support(coordinate(p),profile,view))
            throw std::invalid_argument("Creature cannot stand in this terrain cell");
    }
    g::Point finePosition(const g::Entity& e) const override {
        return {e.x*32,e.y*32,e.motion && e.motion->terrainMotion?r::ordinary_creature_height(e.z,terrainOffset({e.x,e.y,e.z})):e.z*16};
    }
    std::uint32_t creatureType() const override {return read<std::uint32_t>(&frozen_->object,0xa8);}
    bool supportsStationaryOccupants() const override {return stationaryOccupancy_;}
    std::uint32_t maxMovingCreatures() const override {return multiMovement_?32:1;}
    g::Point creatureFootprint() const override {return {read<int>(frozen_->scalar_type.data(),8),read<int>(frozen_->scalar_type.data(),8),read<int>(frozen_->scalar_type.data(),12)};}
    void validateOccupants(const g::State& state) const override {
        if(stationaryOccupancy_) (void)occupancy(state);
        if(multiMovement_) (void)g::MovementReservations(state,binding_.dimensions,creatureType(),creatureFootprint());
    }
    g::RoutePlan plan(const g::Entity& e,g::Point destination,std::uint32_t budget) const override {
        return planSnapshot(inputs(e,destination),destination,budget);
    }
    g::RoutePlan planInWorld(const g::State& state,g::Handle self,g::Point destination,std::uint32_t budget) const override {
        if(!stationaryOccupancy_) return Navigation::planInWorld(state,self,destination,budget);
        return planSnapshot(occupiedInputs(state,self,destination),destination,budget);
    }
    g::RoutePlan planSnapshot(std::shared_ptr<r::RouteWorldSnapshot> snapshot,g::Point destination,std::uint32_t budget) const {
        r::RouteContextPrefix context{};context.unknown_route_flag=1;r::SearchState state;
        auto remaining=static_cast<std::int32_t>(budget);
        auto neighbors=checkedNeighbors(snapshot);
        auto result=r::route_search(context,snapshot->object,snapshot->unknown_argument,snapshot->target,remaining,state,
            r::neighbor_search_world(neighbors,snapshot->target,snapshot->object_token));
        if(result.stop==r::SearchStop::budget_exhausted) return {g::PlanStatus::budgetExhausted,{},static_cast<std::uint32_t>(result.expansions)};
        if(result.path.empty() || point(neighbors.coordinates(result.path.back()))!=destination) return {g::PlanStatus::unreachable,{},static_cast<std::uint32_t>(result.expansions)};
        g::RoutePlan out{g::PlanStatus::reachable,{},static_cast<std::uint32_t>(result.expansions)};
        for(std::uint32_t i=0;i<context.route.waypoint_count;++i) {
            r::Waypoint waypoint;std::memcpy(&waypoint,context.route.unknown_14.data()+i*sizeof(waypoint),sizeof(waypoint));
            const auto metadata=r::neighbor_movement(state.records.at(result.path.at(i+1)).movement);
            out.route.push_back({{waypoint.x,waypoint.y,waypoint.z},waypoint.direction,waypoint.vertical_delta,waypoint.category,metadata.scalar});
        }
        return out;
    }
    std::optional<g::AnimationBinding> animationBinding() const override {return animation_;}
    r::CreatureScalarState scalarState() const {
        r::CreatureScalarState s;s.type_3c=read<std::int32_t>(frozen_->scalar_type.data(),0x3c);
        s.type_10=read<std::int32_t>(frozen_->scalar_type.data(),0x10);
        std::memcpy(s.type_d8.data(),frozen_->scalar_type.data()+0xd8,sizeof(s.type_d8));
        s.object_778=read<std::int32_t>(&frozen_->object,0x778);s.object_77c=read<std::int32_t>(&frozen_->object,0x77c);
        s.default_scalar=frozen_->default_scalar;s.slope_global=frozen_->slope_global;return s;
    }
    static r::MotionState motionState(const g::FineMotion& f) {return {f.accumulator,f.progress,f.travelX,f.travelY,f.fine.x,f.fine.y,f.fine.z,f.residualX,f.residualY,f.frame,f.animationFrame,f.initialFrame,f.initialResidualX,f.initialResidualY};}
    static void store(g::FineMotion& f,const r::MotionState& s) {
        f.accumulator=s.accumulator;f.progress=s.progress;f.travelX=s.travelX;f.travelY=s.travelY;
        f.fine={s.fineX,s.fineY,s.fineZ};f.residualX=s.residualX;f.residualY=s.residualY;f.frame=s.frame;f.animationFrame=s.animationFrame;f.initialFrame=s.initialFrame;f.initialResidualX=s.initialResidualX;f.initialResidualY=s.initialResidualY;
    }
    std::optional<g::DisplayPose> initialDisplayPose() const override {
        if(!animation_) return {};
        const auto& records=sequences_[0];
        // Explicit native static pose: no player execution, events or elapsed time.
        for(std::uint32_t i=0;i<records.size();++i) if(records[i].opcode==0)
            return g::DisplayPose{animation_->sequenceBase,i};
        return {};
    }
    void validateDisplayPose(const g::DisplayPose& pose) const override {
        if(!animation_ || pose.sequence<animation_->sequenceBase || pose.sequence-animation_->sequenceBase>=8)
            throw std::invalid_argument("display pose outside bound movement ANI profile");
        const auto& records=sequences_.at(pose.sequence-animation_->sequenceBase);
        if(pose.displayed>=records.size() || records[pose.displayed].opcode!=0 || records[pose.displayed].argument<0)
            throw std::invalid_argument("display pose is not a bound ANI bitmap record");
    }
    void validateSegmentHistory(const g::Entity& e,const g::SegmentHistory& h) const override {
        constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
        const auto& f=h.motion;
        if(animation_) {
            if(!f.animation || f.animation->sequence!=animation_->sequenceBase+static_cast<unsigned>(h.direction)) throw std::invalid_argument("history animation/direction mismatch");
            (void)player(*f.animation);
        }
        const bool terrain=e.motion && e.motion->terrainMotion;
        if(h.direction<0 || h.direction>7 || (terrain?(h.vertical<-1 || h.vertical>1 || (h.category!=0 && h.category!=4)):(h.vertical!=0 || h.category!=0))) throw std::invalid_argument("unsupported segment history");
        const auto x=h.vertical?0:dx[h.direction],y=h.vertical?0:dy[h.direction];
        const auto px=terrain?h.origin.x:(e.x-x+binding_.dimensions.x)%binding_.dimensions.x,py=terrain?h.origin.y:(e.y-y+binding_.dimensions.y)%binding_.dimensions.y;
        const auto pz=terrain?h.origin.z:e.z;
        const r::Coordinates delta{r::wrapped_difference(e.x,px,binding_.dimensions.x),r::wrapped_difference(e.y,py,binding_.dimensions.y),e.z-pz};
        if(delta.x!=x || delta.y!=y || delta.z<-1 || delta.z>1 || (h.vertical && delta.z!=h.vertical) ||
           f.duration!=static_cast<int>(r::base_movement_scalar(h.category,delta,scalarState())) ||
           f.travelX!=x*f.progress || f.travelY!=y*f.progress || f.fine.x!=px*32+f.travelX/6 || f.fine.y!=py*32+f.travelY/6)
            throw std::invalid_argument("segment history disagrees with bound profile/position");
        if(terrain && (f.heightOrigin!=pz*16+terrainOffset({px,py,pz}) || f.heightDelta!=e.z*16+terrainOffset({e.x,e.y,e.z})-f.heightOrigin))
            throw std::invalid_argument("segment history terrain height disagreement");
    }
    g::FineMotion prepareContinuous(const g::Entity& e,const g::RoutePoint& point) const {
        const bool terrain=e.motion->terrainMotion;
        if(!terrain && (point.category || point.verticalDelta || point.position.z!=e.z)) throw std::invalid_argument("continuous driver requires planar category zero");
        r::SegmentPrevious previous;
        if(e.motion->previous) {
            const auto& h=*e.motion->previous;validateSegmentHistory(e,h);
            previous.state=motionState(h.motion);previous.rate=h.motion.rate;
            previous.action=2;previous.direction=h.direction;previous.vertical=h.vertical;previous.category=h.category;
        }
        const auto origin=finePosition(e);
        previous.state.fineX=origin.x;previous.state.fineY=origin.y;previous.state.fineZ=origin.z;
        r::SegmentRequest request;request.gridX=e.x;request.gridY=e.y;request.gridZ=e.z;request.heightOrigin=origin.z;request.direction=point.direction;
        request.category=point.category;request.vertical=point.verticalDelta;
        request.destinationTerrainHeight=terrain?terrainOffset(point.position):0;
        request.delta={r::wrapped_difference(point.position.x,e.x,binding_.dimensions.x),r::wrapped_difference(point.position.y,e.y,binding_.dimensions.y),point.position.z-e.z};
        const auto result=r::initialize_creature_segment(previous,request,scalarState());
        if(result.rate>1000000 || !result.duration || result.duration>1000000) throw std::invalid_argument("segment scalar outside bounded profile");
        g::FineMotion out;out.rate=result.rate;out.duration=result.duration;out.heightOrigin=result.heightOrigin;out.heightDelta=result.heightDelta;store(out,result.state);
        if(animation_) {
            if(result.carried) out.animation=e.motion->previous->motion.animation;
            else {r::NoCdAnimationPlayer fresh(sequences_.at(point.direction));fresh.start();if(!fresh.state().active) throw std::invalid_argument("movement ANI starts inactive");out.animation=cursor(animation_->sequenceBase+point.direction,fresh.state());}
        }
        return out;
    }
    r::MotionInputs motionInputs(const g::Entity& e,const g::RoutePoint& waypoint) const {
        if((waypoint.category!=0 && waypoint.category!=4) || e.type==12 ||
           read<std::uint8_t>(&frozen_->object,0x722)!=0 ||
           read<std::int32_t>(&frozen_->object,0x108)!=0)
            throw std::invalid_argument("sample driver requires ordinary forward movement profile");
        r::CreatureScalarState scalar;
        scalar.type_3c=read<std::int32_t>(frozen_->scalar_type.data(),0x3c);
        scalar.type_10=read<std::int32_t>(frozen_->scalar_type.data(),0x10);
        std::memcpy(scalar.type_d8.data(),frozen_->scalar_type.data()+0xd8,sizeof(scalar.type_d8));
        scalar.default_scalar=frozen_->default_scalar;scalar.slope_global=frozen_->slope_global;
        const auto delta=r::Coordinates{r::wrapped_difference(waypoint.position.x,e.x,binding_.dimensions.x),
            r::wrapped_difference(waypoint.position.y,e.y,binding_.dimensions.y),waypoint.position.z-e.z};
        const auto duration=r::base_movement_scalar(waypoint.category,delta,scalar);
        if(!waypoint.scalar || waypoint.scalar>1000000 || !duration || duration>1000000)
            throw std::invalid_argument("unsupported sample motion rate/duration");
        r::MotionInputs p;p.rate=waypoint.scalar;p.duration=duration;p.gridX=e.x;p.gridY=e.y;
        p.heightOrigin=finePosition(e).z;p.heightDelta=waypoint.position.z*16+(e.motion && e.motion->terrainMotion?terrainOffset(waypoint.position):0)-p.heightOrigin;p.direction=waypoint.direction;p.vertical=waypoint.verticalDelta!=0;
        if(e.motion && e.motion->continuousMotion) {
            p.separateCursor=true;
            for(unsigned i=0;i<48;++i) p.samples[i]=read<std::int32_t>(frozen_->scalar_type.data(),0xd8+i*4);
            return p;
        }
        const auto bank=p.vertical?0:(waypoint.direction&1);
        for(unsigned i=0;i<12;++i) p.samples[i]=read<std::int32_t>(frozen_->scalar_type.data(),0xd8+(bank*12+i)*4);
        return p;
    }
    g::FineMotion prepareFineMotion(const g::Entity& e,const g::RoutePoint& waypoint) const override {
        auto p=motionInputs(e,waypoint);
        if(e.motion && e.motion->continuousMotion) {
            auto f=prepareContinuous(e,waypoint);p.rate=f.rate;p.duration=f.duration;
            auto probe=f;(void)advance(probe,p);return f;
        }
        g::FineMotion f;
        f.rate=p.rate;f.duration=p.duration;f.heightOrigin=p.heightOrigin;f.heightDelta=p.heightDelta;
        f.fine={e.x*32,e.y*32,e.z*16};
        // Check sample/config admission even if no iteration is admitted yet.
        r::MotionState probe;probe.fineX=f.fine.x;probe.fineY=f.fine.y;probe.fineZ=f.fine.z;
        (void)r::advance_creature_motion(probe,p);return f;
    }
    void validateFineMotion(const g::Entity& e,const g::RoutePoint& waypoint,const g::FineMotion& f) const override {
        const auto expected=prepareFineMotion(e,waypoint);auto p=motionInputs(e,waypoint);
        if(e.motion->continuousMotion) {
            auto replay=expected;p.rate=replay.rate;p.duration=replay.duration;
            for(unsigned i=0;i<e.motion->segmentTicks;++i) if(advance(replay,p)) throw std::invalid_argument("saved ongoing segment already completed");
            if(bool(replay.animation)!=bool(f.animation)) throw std::invalid_argument("saved animation driver mismatch");
            if(replay.animation) {
                const auto& a=*replay.animation;const auto& b=*f.animation;
                if(a.sequence!=b.sequence || a.pc!=b.pc || a.displayed!=b.displayed || a.active!=b.active || a.delay!=b.delay ||
                   a.elapsed!=b.elapsed || a.repeats!=b.repeats || a.breakFlag!=b.breakFlag) throw std::invalid_argument("saved animation replay mismatch");
            }
            if(replay.rate!=f.rate || replay.duration!=f.duration || replay.heightOrigin!=f.heightOrigin || replay.heightDelta!=f.heightDelta ||
               replay.accumulator!=f.accumulator || replay.progress!=f.progress || replay.travelX!=f.travelX || replay.travelY!=f.travelY ||
               replay.fine!=f.fine || replay.residualX!=f.residualX || replay.residualY!=f.residualY || replay.frame!=f.frame || replay.animationFrame!=f.animationFrame || replay.initialFrame!=f.initialFrame || replay.initialResidualX!=f.initialResidualX || replay.initialResidualY!=f.initialResidualY)
                throw std::invalid_argument("saved segment initialization/replay mismatch");
            return;
        }
        int cycle=0,prefix=0;
        for(unsigned i=0;i<12;++i) {const auto step=p.samples[i]*(p.vertical?2:1);cycle+=step;if(i<f.frame) prefix+=step;}
        constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
        const auto x=p.vertical?0:dx[p.direction],y=p.vertical?0:dy[p.direction];
        if(f.accumulator<0 || f.accumulator>=p.duration || f.progress<prefix || (f.progress-prefix)%cycle ||
           f.travelX!=x*f.progress || f.travelY!=y*f.progress ||
           f.residualX!=x*((f.progress-prefix)/6-f.progress/6) ||
           f.residualY!=y*((f.progress-prefix)/6-f.progress/6))
            throw std::invalid_argument("saved sample cursor/displacement disagrees with profile");
        if(f.rate!=expected.rate || f.duration!=expected.duration || f.heightOrigin!=expected.heightOrigin || f.heightDelta!=expected.heightDelta)
            throw std::invalid_argument("saved sample motion profile disagrees with map");
    }
    bool advanceFineMotion(const g::Entity& e,const g::RoutePoint& waypoint,g::FineMotion& f) const override {
        auto inputs=motionInputs(e,waypoint);
        if(e.motion->continuousMotion) {
            const auto expected=prepareContinuous(e,waypoint);
            if(f.rate!=expected.rate || f.duration!=expected.duration) throw std::invalid_argument("segment scalar disagreement");
            inputs.rate=f.rate;inputs.duration=f.duration;
        } else validateFineMotion(e,waypoint,f);
        return advance(f,inputs);
    }
    bool accepts(const g::Entity& e,const g::RoutePoint& waypoint) const override {
        if(!e.motion) return false;
        return acceptsSnapshot(e,waypoint,inputs(e,e.motion->destination));
    }
    bool acceptsInWorld(const g::State& state,g::Handle self,const g::RoutePoint& waypoint) const override {
        if(!stationaryOccupancy_) return Navigation::acceptsInWorld(state,self,waypoint);
        const auto snapshot=occupiedInputs(state,self,waypoint.position);
        return acceptsSnapshot(*state.slots.at(self.slot).entity,waypoint,snapshot);
    }
    bool acceptsSnapshot(const g::Entity& e,const g::RoutePoint& waypoint,std::shared_ptr<r::RouteWorldSnapshot> snapshot) const {
        auto neighbors=checkedNeighbors(snapshot);
        const auto from=r::Coordinates{e.x,e.y,e.z};
        auto candidates=r::generate_neighbors(neighbors.node_at(from),r::MovementPayload{},
            r::creature_descriptor(snapshot->object,snapshot->object_token,snapshot->unknown_argument,snapshot->target),neighbors);
        for(const auto& c:candidates) if(point(neighbors.coordinates(c.node))==waypoint.position && c.movement.category==waypoint.category) {
            if(waypoint.position.x==e.x && waypoint.position.y==e.y)
                return waypoint.verticalDelta==waypoint.position.z-e.z;
            const auto dx=r::wrapped_difference(waypoint.position.x,e.x,binding_.dimensions.x);
            const auto dy=r::wrapped_difference(waypoint.position.y,e.y,binding_.dimensions.y);
            if(dx<-1 || dx>1 || dy<-1 || dy>1) return false;
            constexpr std::array<int,9> directions{{7,0,1,6,0,2,5,4,3}};
            return waypoint.verticalDelta==0 && waypoint.direction==directions.at(dx+3*dy+4);
        }
        return false;
    }
};
}
game::AnimationBinding loadMovementAnimation(const std::string& path,std::uint32_t sequenceBase) {
    if(path.find('\0')!=std::string::npos) throw std::invalid_argument("ANI path contains NUL");
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f) throw std::runtime_error("cannot open movement ANI");
    const auto size=f.tellg();if(size<44 || size>8*1024*1024) throw std::invalid_argument("invalid movement ANI size");
    f.seekg(0);game::AnimationBinding binding;binding.data.resize(static_cast<std::size_t>(size));binding.sequenceBase=sequenceBase;
    f.read(reinterpret_cast<char*>(binding.data.data()),size);if(!f) throw std::runtime_error("cannot read movement ANI");return binding;
}
std::shared_ptr<const game::Navigation> loadFrozenNavigation(const std::string& path,const std::optional<game::AnimationBinding>& animation,bool stationaryOccupancy,bool multiMovement) {
    if(path.find('\0')!=std::string::npos) throw std::invalid_argument("map path contains NUL");
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f) throw std::runtime_error("cannot open frozen movement map");
    const auto size=f.tellg();if(size<92 || size>64*1024*1024) throw std::invalid_argument("invalid frozen map size");
    f.seekg(0);std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    f.read(reinterpret_cast<char*>(bytes.data()),size);if(!f) throw std::runtime_error("cannot read frozen movement map");
    return loadFrozenNavigationBytes(bytes,animation,stationaryOccupancy,multiMovement);
}
std::shared_ptr<const game::Navigation> loadFrozenNavigationBytes(const std::vector<std::byte>& bytes,const std::optional<game::AnimationBinding>& animation,bool stationaryOccupancy,bool multiMovement) {
    std::uint64_t hash=14695981039346656037ULL;
    for(auto byte:bytes) {hash^=std::to_integer<std::uint8_t>(byte);hash*=1099511628211ULL;}
    return std::make_shared<FrozenNavigation>(reconstruction::decode_route_world(bytes),hash,animation,stationaryOccupancy,multiMovement);
}
}
