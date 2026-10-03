#include "route_world.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
template<class T> void write(void* p,std::size_t at,T v) {
    std::memcpy(static_cast<std::byte*>(p)+at,&v,sizeof(v));
}
int main() {
    auto s=std::make_shared<RouteWorldSnapshot>();
    s->dimensions={6,6,3}; s->plane_stride=36; s->boundary=1; s->budget=300;
    s->rows={0,6,12,18,24,30}; s->layers={0,36,72}; s->cells.resize(108);
    s->terrain.resize(2); s->terrain[1].classification_94=16; s->terrain[1].flags_b0=8;
    for(int i=0;i<36;++i) write(&s->cells[i],0,std::uint16_t(1));
    s->object_token=0x1200000; s->cell_base=0x1000000;
    s->default_scalar=720; s->slope_global=0.969F; s->target={3,1,1};
    write(&s->object,8,1); write(&s->object,12,1); write(&s->object,16,1);
    write(s->scalar_type.data(),8,1); write(s->scalar_type.data(),12,1);
    write(s->scalar_type.data(),16,500);
    for(unsigned i=0;i<48;++i) write(s->scalar_type.data(),0xd8+i*4,std::uint32_t((i/12)%2?40:60)); write(s->generator_type.data(),0x589,720);
    RouteContextPrefix context{}; context.unknown_route_flag=1;
    SearchState state; auto budget=s->budget;
    const auto result=replay_route_world(s,context,state,budget);
    auto h=snapshot_neighbors(s);
    assert(result.path.size()==3 && context.route.waypoint_count==2);
    auto last=h.coordinates(result.path.back()); assert(last.x==3 && last.y==1 && last.z==1);
    assert(result.expansions>0 && budget==300-static_cast<int>(result.expansions));
    for(std::size_t i=1;i<result.path.size();++i) {
        auto m=neighbor_movement(state.records.at(result.path[i]).movement);
        assert(m.category==0 && m.scalar==720);
    }
    auto node=h.node_at({1,1,1});
    auto candidates=generate_neighbors(node,MovementPayload{},creature_descriptor(s->object,s->object_token,0,s->target),h);
    assert(candidates.size()==8);
    for(auto c:candidates) assert(c.edge_cost==32 || c.edge_cost==48);
    RouteSequenceReplay sequence;
    auto first=std::make_shared<RouteWorldSnapshot>(*s); first->budget=1;
    auto a=replay_route_sequence_call(sequence,0x690148,1,first);
    assert(a.replayed && a.search.stop==SearchStop::budget_exhausted);
    assert(a.search.expansions==1 && a.budget_remaining==0);
    auto second=std::make_shared<RouteWorldSnapshot>(*s); second->budget=2;
    auto other=std::make_shared<RouteWorldSnapshot>(*s); other->budget=1; other->target={4,1,1};
    auto b=replay_route_sequence_call(sequence,0x6c4bd0,1,other);
    assert(b.replayed && b.search.stop==SearchStop::budget_exhausted);
    auto resumed=replay_route_sequence_call(sequence,0x690148,0,second);
    assert(resumed.replayed && resumed.search.stop==SearchStop::budget_exhausted);
    assert(sequence.contexts.at(0x690148).state.best_node!=sequence.contexts.at(0x6c4bd0).state.best_node);
    second->object.route.unknown_14.back()=std::byte{0x7e}; // changed input route bytes on resume
    auto finished=replay_route_sequence_call(sequence,0x690148,0,second);
    assert(finished.replayed && finished.search.stop==SearchStop::target_bound);
    assert(sequence.contexts.at(0x690148).context.route.unknown_14.back()==std::byte{0x7e});
    assert(finished.search.path==result.path);
    assert(sequence.contexts.at(0x6c4bd0).context.unknown_route_flag==0);
    auto orphan=replay_route_sequence_call(sequence,0x222222,0,s);
    assert(!orphan.replayed && orphan.reason=="missing_initialization");
    auto flag_mismatch=replay_route_sequence_call(sequence,0x690148,0,s);
    assert(!flag_mismatch.replayed && flag_mismatch.reason=="initialization_flag_disagrees");
    assert(replay_route_sequence_call(sequence,0x690148,1,s).replayed);
    auto moved_map=std::make_shared<RouteWorldSnapshot>(*other); moved_map->cell_base+=0x10000;
    auto map_failure=replay_route_sequence_call(sequence,0x6c4bd0,0,moved_map);
    assert(!map_failure.replayed && map_failure.reason=="map_identity_changed");
    assert(!replay_route_sequence_call(sequence,0x6c4bd0,0,other).replayed);
    assert(replay_route_sequence_call(sequence,0x6c4bd0,1,moved_map).replayed);
    auto changed=std::make_shared<RouteWorldSnapshot>(*moved_map); changed->target={2,1,1};
    auto changed_request=replay_route_sequence_call(sequence,0x6c4bd0,0,changed);
    assert(!changed_request.replayed && changed_request.reason=="request_identity_changed");
    assert(replay_route_sequence_call(sequence,0x6c4bd0,1,changed).replayed);
    auto corrupt=std::make_shared<RouteWorldSnapshot>(*s); corrupt->layers[1]=0;
    bool refused=false; try { snapshot_neighbors(corrupt); }
    catch(const std::invalid_argument&) { refused=true; } assert(refused);
    // Callbacks must keep their input buffers alive after the caller releases ownership.
    std::weak_ptr<RouteWorldSnapshot> weak=s; s.reset();
    assert(!weak.expired()); assert(h.coordinates(node).x==1);
    std::cout<<"Frozen world full search, movement, cost and ownership checks passed\n";
}
