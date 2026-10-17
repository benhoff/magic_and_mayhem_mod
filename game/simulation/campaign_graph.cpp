#include "campaign_graph.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
namespace mnm::game {
namespace {
void need(bool ok,const char* why) {if(!ok) throw std::invalid_argument(why);}
std::uint32_t mask(const std::vector<std::uint32_t>& values) {
    std::uint32_t out=0;for(auto i:values) {need(i<16,"campaign graph index limit");out|=1U<<i;}return out;
}
bool prerequisites(const CampaignNode& node,std::uint32_t done) {
    const auto all=mask(node.all),any=mask(node.any);
    return (done&all)==all && (!any || (done&any));
}
bool available(const CampaignGraph& graph,std::uint32_t done,std::uint32_t i) {
    const auto& node=graph.nodes.at(i);
    if((done&(1U<<i)) || !prerequisites(node,done)) return false;
    if(!node.choice.empty()) for(unsigned j=0;j<graph.nodes.size();++j)
        if((done&(1U<<j)) && graph.nodes[j].choice==node.choice) return false;
    return true;
}
}
bool campaignSpell(const std::string& spell) {
    return spell=="summon-crocodile" || spell=="arc-bolt" || spell=="cure" || spell=="magic-sphere";
}
bool campaignNodeAvailable(const CampaignGraph& graph,const CampaignGraphProgress& p,std::uint32_t i) {
    need(!graph.nodes.empty() && graph.nodes.size()<=16,"campaign graph node budget");
    if(i>=graph.nodes.size()) return false;
    const auto done=mask(p.completed);
    if(done&(1U<<(graph.nodes.size()-1))) return false;
    const auto& choice=graph.nodes[i].choice;
    for(auto selected:p.choices) {
        need(selected<graph.nodes.size(),"unknown committed campaign choice");
        if(selected!=i && !choice.empty() && graph.nodes[selected].choice==choice) return false;
    }
    return available(graph,done,i);
}
void validateCampaignGraph(const CampaignGraph& graph) {
    const auto n=graph.nodes.size();need(n>=2 && n<=16,"campaign graph node budget");
    for(unsigned i=0;i<n;++i) {
        const auto& node=graph.nodes[i];std::set<unsigned> seen;
        need(node.all.size()<=16 && node.any.size()<=16,"campaign prerequisite budget");
        for(const auto* list:{&node.all,&node.any}) for(auto d:*list)
            need(d<i && seen.insert(d).second,"invalid/cyclic campaign prerequisite");
        need((i==0)==(node.all.empty() && node.any.empty()),"campaign requires one entry node");
        need(node.choice.size()<=64 && node.choice.find('\0')==std::string::npos,"campaign choice ID");
        need((i!=0 && i+1!=n) || node.choice.empty(),"entry/finale cannot be exclusive alternatives");
        std::set<std::string> spells;need(node.rewards.size()<=4,"campaign reward budget");
        for(const auto& spell:node.rewards) need(campaignSpell(spell) && spells.insert(spell).second,"invalid campaign spell reward");
        if(!node.choice.empty()) {
            unsigned peers=0;
            for(const auto& other:graph.nodes) if(other.choice==node.choice) {
                ++peers;need(node.all==other.all && node.any==other.any,"route alternatives need identical prerequisites");
            }
            need(peers>=2,"exclusive campaign group needs alternatives");
        }
    }
    // Enumerate the bounded completion state space. Reject definitions that
    // contain unreachable missions or permit a route to dead-end before finale.
    const unsigned terminal=1U<<(n-1);std::vector<bool> seen(1U<<n);
    std::vector<unsigned> queue{0};seen[0]=true;unsigned reached=0;
    for(std::size_t at=0;at<queue.size();++at) {
        const auto done=queue[at];if(done&terminal) continue;
        bool advanced=false;
        for(unsigned i=0;i<n;++i) if(available(graph,done,i)) {
            advanced=true;reached|=1U<<i;const auto next=done|(1U<<i);
            if(!seen[next]) {seen[next]=true;queue.push_back(next);}
        }
        need(advanced,"campaign route dead end");
    }
    need(reached==((1U<<n)-1),"campaign has unreachable missions");
}
}
