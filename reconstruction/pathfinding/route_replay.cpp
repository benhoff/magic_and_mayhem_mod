#include "route_world.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
using namespace mnm::reconstruction;
namespace {
void report(std::shared_ptr<const RouteWorldSnapshot> s,const RouteContextPrefix& context,const SearchState& state,
    const SearchResult& result,int budget,std::uint32_t token=0) {
    auto helpers=snapshot_neighbors(s);
    const char* stop=result.stop==SearchStop::target_bound?"target_bound":
        result.stop==SearchStop::budget_exhausted?"budget_exhausted":"queue_empty";
    std::cout<<std::dec<<"{\"context\":"<<token<<",\"status\":\"replayed\",\"stop\":\""<<stop
        <<"\",\"expansions\":"<<result.expansions<<",\"budget_remaining\":"<<budget
        <<",\"waypoint_count\":"<<context.route.waypoint_count
        <<",\"best_heuristic\":"<<state.best_heuristic<<",\"best_node\":"<<state.best_node
        <<",\"best_priority\":"<<state.records.at(state.best_node).priority
        <<",\"queue_entries\":"<<state.open.size()<<",\"node_records\":"<<state.records.size()
        <<",\"path\":[";
    for(std::size_t i=0;i<result.path.size();++i) {
        if(i) std::cout<<',';
        const auto node=result.path[i]; const auto p=helpers.coordinates(node);
        const auto m=neighbor_movement(state.records.at(node).movement);
        std::cout<<"{\"xyz\":["<<p.x<<','<<p.y<<','<<p.z<<"],\"category\":"<<m.category
            <<",\"scalar\":"<<m.scalar<<",\"priority\":"<<state.records.at(node).priority<<'}';
    }
    std::cout<<"],\"flag\":"<<unsigned(context.unknown_route_flag)<<",\"snapshot_hex\":\"";
    const auto* raw=reinterpret_cast<const unsigned char*>(&context.route);
    for(std::size_t i=0;i<sizeof(context.route);++i)
        std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(raw[i]);
    std::cout<<std::dec<<"\"}\n";
}
}
int main(int argc,char** argv) {
    try {
        if(argc==3 && std::string(argv[1])=="--sequence") {
            std::ifstream index(argv[2]);
            if(!index) throw std::invalid_argument("cannot read sequence index");
            RouteSequenceReplay replay;
            std::uint32_t token; unsigned flag; std::string path;
            while(index>>token) {
                if(!(index>>flag>>std::quoted(path)) || !token || flag>255)
                    throw std::invalid_argument("invalid sequence index row");
                const auto s=read_route_world(path);
                auto result=replay_route_sequence_call(replay,token,static_cast<std::uint8_t>(flag),s);
                if(!result.replayed) {
                    std::cout<<"{\"context\":"<<token<<",\"status\":\"inconclusive\",\"reason\":\""
                        <<result.reason<<"\"}\n";
                } else {
                    const auto& session=replay.contexts.at(token);
                    report(s,session.context,session.state,result.search,result.budget_remaining,token);
                }
            }
            if(!index.eof()) throw std::invalid_argument("invalid sequence context token");
            return 0;
        }
        if(argc!=2) throw std::invalid_argument("usage: route-replay SNAPSHOT.bin | --sequence INDEX");
        const auto s=read_route_world(argv[1]);
        RouteContextPrefix context{}; context.unknown_route_flag=1;
        SearchState state; auto budget=s->budget;
        const auto result=replay_route_world(s,context,state,budget);
        report(s,context,state,result,budget);
    } catch(const std::exception& e) { std::cerr<<"Replay refused: "<<e.what()<<'\n'; return 1; }
}
