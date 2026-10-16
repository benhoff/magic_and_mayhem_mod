#include "campaign_session.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
#include <type_traits>

namespace mnm::game {
namespace {
void require(bool value,const char* reason) {if(!value) throw std::invalid_argument(reason);}
struct Mutation {
    bool& active;
    explicit Mutation(bool& flag):active(flag) {require(!active,"recursive campaign mutation");active=true;}
    ~Mutation() {active=false;}
};
bool same(const CampaignMission& m,const std::string& realm,const std::string& region) {
    return m.realm==realm && m.region==region;
}
CampaignRegion& region(CampaignState& p,const CampaignMission& m) {
    for(auto& r:p.realms) if(r.id==m.realm)
        for(auto& q:r.regions) if(q.id==m.region) return q;
    throw std::invalid_argument("campaign mission region is missing");
}
}
CampaignSession::CampaignSession(CampaignDefinition definition,std::shared_ptr<const CampaignResources> resources)
    : definition_(std::move(definition)),resources_(std::move(resources)),owned_{definition_.initial,{}} {
    static_assert(std::is_nothrow_move_assignable<Owned>::value,"campaign publication must not throw");
    require(bool(resources_),"campaign resources are required");
    const auto& initial=definition_.initial;encodeCampaign(initial);
    require(!initial.battle && !initial.lastResult && !initial.completed && initial.transition==0,
            "campaign definition must start before its first attempt");
    std::size_t count=0;
    for(const auto& r:initial.realms) count+=r.regions.size();
    require(count && definition_.missions.size()==count,"campaign requires exactly one mission per region");
    std::size_t index=0;
    for(const auto& realm:initial.realms) for(const auto& r:realm.regions) {
        const auto& m=definition_.missions[index++];
        require(!r.completed && same(m,realm.id,r.id),"campaign mission order differs from realm/region order");
        // Reuse the persistence validator for map, opponent and participant IDs.
        auto probe=initial;probe.realm=m.realm;probe.region=m.region;
        probe.battle=CampaignBattle{m.realm,m.region,m.map,{probe.player,m.opponent},0};
        encodeCampaign(probe);
        require(m.opponent!=probe.player,"campaign opponent must differ from player");
    }
    require(same(definition_.missions.front(),initial.realm,initial.region),"campaign must select its first mission");
    validate(owned_);
}
std::size_t CampaignSession::missionIndex(const CampaignState& p) const {
    for(std::size_t i=0;i<definition_.missions.size();++i)
        if(same(definition_.missions[i],p.realm,p.region)) return i;
    throw std::invalid_argument("campaign selected mission is not in content");
}
void CampaignSession::validateProgress(const CampaignState& p) const {
    encodeCampaign(p);
    const auto& initial=definition_.initial;
    require(p.identity.id==initial.identity.id && p.identity.contentRevision==initial.identity.contentRevision,
            "campaign content identity mismatch");
    require(p.player==initial.player && p.wizards.size()==initial.wizards.size() &&
            p.realms.size()==initial.realms.size(),"campaign content roster mismatch");
    for(std::size_t i=0;i<p.wizards.size();++i)
        require(p.wizards[i].id==initial.wizards[i].id,"campaign wizard roster differs from content");
    const auto selected=missionIndex(p);std::size_t index=0;
    for(std::size_t i=0;i<p.realms.size();++i) {
        const auto& r=p.realms[i];const auto& expected=initial.realms[i];
        require(r.id==expected.id && r.regions.size()==expected.regions.size(),"campaign realm content mismatch");
        for(std::size_t j=0;j<r.regions.size();++j,++index) {
            const auto& q=r.regions[j];
            require(q.id==expected.regions[j].id,"campaign region content mismatch");
            const bool completed=p.completed || index<selected;
            require(q.completed==completed,"campaign completed regions are not a linear prefix");
            require(q.owner==(completed ? std::optional<std::string>(p.player) : expected.regions[j].owner),
                    "campaign region ownership differs from results");
        }
    }
    require(!p.completed || selected+1==definition_.missions.size(),"campaign completed before final mission");
    if(p.lastResult) {
        const auto& result=*p.lastResult;
        require(result.attempt%2==1 && result.attempt<std::numeric_limits<std::uint64_t>::max(),
                "campaign result attempt is invalid");
        require(result.outcome!=CampaignOutcome::victory || p.completed || selected>0,
                "campaign victory has no completed predecessor");
        const auto previous=result.outcome==CampaignOutcome::victory && !p.completed ? selected-1 : selected;
        require(previous<definition_.missions.size() && same(definition_.missions[previous],result.realm,result.region),
                "campaign result does not match progress");
        require(p.transition-result.attempt==(p.battle ? 2U : 1U),"campaign result was replayed or skipped");
        require(!p.completed || result.outcome==CampaignOutcome::victory,"campaign completion lacks victory");
    } else {
        require(!p.completed && selected==0 && p.transition==(p.battle ? 1U : 0U),
                "campaign progress lacks its preceding result");
    }
    if(p.battle) {
        const auto& b=*p.battle;const auto& m=definition_.missions[selected];
        require(b.attempt==p.transition && b.attempt%2==1 &&
                b.attempt<std::numeric_limits<std::uint64_t>::max(),"campaign battle attempt is unbound or exhausted");
        require(b.map==m.map && b.participants==std::vector<std::string>{p.player,m.opponent},
                "campaign battle differs from mission content");
    }
}
void CampaignSession::validate(const Owned& s) const {
    validateProgress(s.progress);
    require(bool(s.battle)==bool(s.progress.battle),"campaign battle ownership mismatch");
    if(s.battle) {
        require(s.battle->state().outcome==Outcome::running,"campaign retains a settled battle");
        require(s.battle->world().state().map==s.progress.battle->map,"campaign battle map mismatch");
    }
    encodeCampaignSnapshot({s.battle ? s.battle->world().state() : State{},s.progress},definition_.initial.identity);
}
void CampaignSession::startMission() {
    Mutation mutation(changing_);
    require(!owned_.battle && !progress().completed,"campaign has an active battle or is complete");
    require(progress().transition<=std::numeric_limits<std::uint64_t>::max()-2,"campaign attempt counter exhausted");
    auto candidate=owned_;const auto& m=currentMission();
    candidate.battle=resources_->create(m);
    require(candidate.battle->world().state().tick==0 && candidate.battle->world().state().sequence==0,
            "campaign factory did not create a fresh battle");
    const auto attempt=++candidate.progress.transition;
    candidate.progress.battle=CampaignBattle{m.realm,m.region,m.map,{progress().player,m.opponent},attempt};
    validate(candidate);owned_=std::move(candidate);
}
void CampaignSession::enqueue(SkirmishOrder order) {
    Mutation mutation(changing_);
    require(bool(owned_.battle),"campaign has no active battle");
    auto candidate=owned_;candidate.battle->enqueue(std::move(order));
    validate(candidate);owned_=std::move(candidate);
}
CampaignStepReport CampaignSession::step(TickInput input) {
    Mutation mutation(changing_);
    CampaignStepReport report;
    if(!owned_.battle || !input.admitted) return report;
    auto candidate=owned_;report.battle=candidate.battle->step(input);
    const auto outcome=candidate.battle->state().outcome;
    if(outcome!=Outcome::running) {
        auto& p=candidate.progress;const auto b=*p.battle;
        const auto result=outcome==Outcome::victory ? CampaignOutcome::victory :
                          outcome==Outcome::defeat ? CampaignOutcome::defeat : CampaignOutcome::draw;
        p.lastResult=CampaignResult{b.attempt,b.realm,b.region,result};
        const auto selected=missionIndex(p);
        if(result==CampaignOutcome::victory) {
            auto& r=region(p,definition_.missions[selected]);r.completed=true;r.owner=p.player;
            if(selected+1==definition_.missions.size()) p.completed=true;
            else {const auto& next=definition_.missions[selected+1];p.realm=next.realm;p.region=next.region;}
        }
        ++p.transition;p.battle.reset();candidate.battle.reset();report.result=p.lastResult;
    }
    validate(candidate);owned_=std::move(candidate);return report;
}
CampaignCheckpoint CampaignSession::checkpoint() const {
    return {owned_.battle ? owned_.battle->world().state() : State{},progress()};
}
CommitResult CampaignSession::save(const std::filesystem::path& path,bool overwrite) const {
    return writeCampaignSnapshot(path,checkpoint(),definition_.initial.identity,overwrite);
}
void CampaignSession::restore(CampaignCheckpoint checkpoint) {
    Mutation mutation(changing_);
    validateProgress(checkpoint.campaign);Owned candidate{std::move(checkpoint.campaign),{}};
    if(candidate.progress.battle) {
        const auto& m=definition_.missions[missionIndex(candidate.progress)];
        require(checkpoint.world.map==m.map,"campaign restored map differs from content");
        const auto original=encodeSnapshot(checkpoint.world);
        candidate.battle=resources_->restore(m,std::move(checkpoint.world));
        require(encodeSnapshot(candidate.battle->world().state())==original,"campaign resource restore altered saved world");
    } else require(encodeSnapshot(checkpoint.world)==encodeSnapshot(State{}),"idle campaign retains a battle world");
    validate(candidate);owned_=std::move(candidate);
}
void CampaignSession::restore(const std::filesystem::path& path) {
    require(!changing_,"recursive campaign mutation");
    restore(readCampaignSnapshot(path,definition_.initial.identity));
}
}
