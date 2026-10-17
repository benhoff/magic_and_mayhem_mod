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
Spell orderSpell(SkirmishOrderKind kind) {
    return kind==SkirmishOrderKind::summon?Spell::summon:kind==SkirmishOrderKind::bolt?Spell::bolt:kind==SkirmishOrderKind::cure?Spell::cure:kind==SkirmishOrderKind::sphere?Spell::sphere:Spell::none;
}
bool ownsSpell(const CampaignState& p,Spell spell) {
    const char* name=spell==Spell::summon ? "summon-crocodile" : spell==Spell::bolt ? "arc-bolt" : spell==Spell::cure ? "cure" : spell==Spell::sphere ? "magic-sphere" : "";
    for(const auto& w:p.wizards) if(w.id==p.player) return std::find(w.spells.begin(),w.spells.end(),name)!=w.spells.end();
    return false;
}
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
    require(definition_.plans.empty() || definition_.plans.size()==count,"mission plan roster mismatch");
    require(!definition_.spellLoadout || definition_.plans.size()==count,"loadout policy requires mission plans");
    require(bool(initial.graph)==bool(definition_.graph),"campaign graph definition/state mismatch");
    if(definition_.graph) {
        validateCampaignGraph(*definition_.graph);
        require(definition_.graph->nodes.size()==count && initial.graph->completed.empty() && initial.graph->choices.empty(),"campaign graph initial roster/progress");
        require(definition_.spellLoadout,"graph campaigns require derived spell loadout");
        for(const auto& wizard:initial.wizards) for(const auto& spell:wizard.spells) require(campaignSpell(spell),"unsupported graph initial spell");
        for(const auto& plan:definition_.plans) if(plan) require(plan->version==2 && plan->rewardSpell.empty(),"graph rewards belong to campaign nodes");
    }
    std::size_t index=0;
    for(const auto& realm:initial.realms) for(const auto& r:realm.regions) {
        const auto& m=definition_.missions[index++];
        require(!r.completed && same(m,realm.id,r.id),"campaign mission order differs from realm/region order");
        // Reuse the persistence validator for map, opponent and participant IDs.
        auto probe=initial;probe.realm=m.realm;probe.region=m.region;
        probe.battle=CampaignBattle{m.realm,m.region,m.map,{probe.player,m.opponent},0};
        if(probe.graph) {probe.transition=1;probe.battle->attempt=1;}
        encodeCampaign(probe);
        require(m.opponent!=probe.player,"campaign opponent must differ from player");
    }
    require(same(definition_.missions.front(),initial.realm,initial.region),"campaign must select its first mission");
    validate(owned_);
}
std::size_t CampaignSession::missionIndex(const CampaignState& p) const {return missionIndex(p.realm,p.region);}
std::size_t CampaignSession::missionIndex(const std::string& realm,const std::string& region) const {
    for(std::size_t i=0;i<definition_.missions.size();++i)
        if(same(definition_.missions[i],realm,region)) return i;
    throw std::invalid_argument("campaign selected mission is not in content");
}
void CampaignSession::validateGraphProgress(const CampaignState& p) const {
    const auto& graph=*definition_.graph;const auto& state=*p.graph;
    const auto selected=missionIndex(p);std::set<std::string> choices;
    for(auto i:state.choices) {
        require(i<graph.nodes.size() && !graph.nodes[i].choice.empty() && choices.insert(graph.nodes[i].choice).second,"invalid/exclusive campaign choice");
    }
    for(auto i:state.choices) {
        auto probe=state;probe.completed.erase(std::remove_if(probe.completed.begin(),probe.completed.end(),[&](auto n){return n>=i;}),probe.completed.end());
        require(campaignNodeAvailable(graph,probe,i),"committed route lacks prerequisites");
        if(std::find(state.completed.begin(),state.completed.end(),i)==state.completed.end())
            require(i==selected && (p.battle || (p.lastResult && p.lastResult->outcome!=CampaignOutcome::victory && same(definition_.missions[i],p.lastResult->realm,p.lastResult->region))),"route commitment has no attempt");
    }
    CampaignGraphProgress replay;replay.choices=state.choices;
    for(auto i:state.completed) {
        require(i<graph.nodes.size() && campaignNodeAvailable(graph,replay,i),"campaign victory history violates prerequisites/choice");
        require(graph.nodes[i].choice.empty() || std::find(state.choices.begin(),state.choices.end(),i)!=state.choices.end(),"completed route lacks commitment");
        replay.completed.push_back(i);
        require(i+1!=graph.nodes.size() || replay.completed.size()==state.completed.size(),"campaign continued after finale");
    }
    const bool terminal=std::find(state.completed.begin(),state.completed.end(),graph.nodes.size()-1)!=state.completed.end();
    require(p.completed==terminal && (!terminal || selected+1==graph.nodes.size()),"campaign terminal progress mismatch");
    require(p.completed || campaignNodeAvailable(graph,state,unsigned(selected)),"selected mission is locked or completed");
    require(p.transition/2>=state.completed.size(),"campaign victories exceed attempts");
    if(p.battle && !graph.nodes[selected].choice.empty())
        require(std::find(state.choices.begin(),state.choices.end(),selected)!=state.choices.end(),"active route lacks commitment");
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
    require(bool(p.graph)==bool(definition_.graph),"campaign graph continuation mismatch");
    const auto selected=missionIndex(p);std::size_t index=0;
    if(p.graph) validateGraphProgress(p);
    for(std::size_t i=0;i<p.realms.size();++i) {
        const auto& r=p.realms[i];const auto& expected=initial.realms[i];
        require(r.id==expected.id && r.regions.size()==expected.regions.size(),"campaign realm content mismatch");
        for(std::size_t j=0;j<r.regions.size();++j,++index) {
            const auto& q=r.regions[j];
            require(q.id==expected.regions[j].id,"campaign region content mismatch");
            const bool completed=p.graph ? std::find(p.graph->completed.begin(),p.graph->completed.end(),index)!=p.graph->completed.end() : p.completed || index<selected;
            require(q.completed==completed,"campaign completed regions differ from progress");
            require(q.owner==(completed ? std::optional<std::string>(p.player) : expected.regions[j].owner),
                    "campaign region ownership differs from results");
        }
    }
    require(!p.completed || selected+1==definition_.missions.size(),"campaign completed before final mission");
    if(p.lastResult) {
        const auto& result=*p.lastResult;
        require(result.attempt%2==1 && result.attempt<std::numeric_limits<std::uint64_t>::max(),
                "campaign result attempt is invalid");
        if(p.graph) {
            const auto previous=missionIndex(result.realm,result.region);
            if(result.outcome==CampaignOutcome::victory) require(!p.graph->completed.empty() && p.graph->completed.back()==previous,"campaign result differs from victory history");
            else require(!p.completed && selected==previous && std::find(p.graph->completed.begin(),p.graph->completed.end(),previous)==p.graph->completed.end(),"campaign loss must retain retry mission");
        } else {
        require(result.outcome!=CampaignOutcome::victory || p.completed || selected>0,
                "campaign victory has no completed predecessor");
        const auto previous=result.outcome==CampaignOutcome::victory && !p.completed ? selected-1 : selected;
        require(previous<definition_.missions.size() && same(definition_.missions[previous],result.realm,result.region),
                "campaign result does not match progress");
        }
        require(p.transition-result.attempt==(p.battle ? 2U : 1U),"campaign result was replayed or skipped");
        require(!p.completed || result.outcome==CampaignOutcome::victory,"campaign completion lacks victory");
    } else {
        require((!p.graph || p.graph->completed.empty()) && !p.completed && selected==0 && p.transition==(p.battle ? 1U : 0U),
                "campaign progress lacks its preceding result");
    }
    if(definition_.spellLoadout) {
        auto expected=initial.wizards;
        if(p.graph) {
            for(auto i:p.graph->completed) for(auto& wizard:expected) if(wizard.id==p.player)
                for(const auto& spell:definition_.graph->nodes[i].rewards)
                    if(std::find(wizard.spells.begin(),wizard.spells.end(),spell)==wizard.spells.end()) wizard.spells.push_back(spell);
        } else for(std::size_t i=0;i<definition_.missions.size();++i) if(p.completed || i<selected) {
            if(definition_.plans[i]) for(auto& wizard:expected) if(wizard.id==p.player) {
                const auto& spell=definition_.plans[i]->rewardSpell;
                if(!spell.empty() && std::find(wizard.spells.begin(),wizard.spells.end(),spell)==wizard.spells.end()) wizard.spells.push_back(spell);
            }
        }
        for(std::size_t i=0;i<expected.size();++i)
            require(p.wizards[i].spells==expected[i].spells && p.wizards[i].experience==expected[i].experience &&
                    p.wizards[i].attributes==expected[i].attributes && p.wizards[i].inventory==expected[i].inventory,"campaign loadout/reward differs from completed missions");
    }
    require(bool(p.mission)==(bool(p.battle) && !definition_.plans.empty() && bool(definition_.plans[selected])),"campaign mission continuation mismatch");
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
        if(s.progress.mission) validateMissionProgress(*definition_.plans[missionIndex(s.progress)],*s.progress.mission,*s.battle);
        else require(!s.battle->state().missionControlled,"unexpected scripted battle");
        if(definition_.spellLoadout) for(const auto& order:s.battle->state().pending) {
            const auto spell=orderSpell(order.kind);
            require(spell==Spell::none || ownsSpell(s.progress,spell),"checkpoint queues a locked spell");
        }
    }
    encodeCampaignSnapshot({s.battle ? s.battle->world().state() : State{},s.progress},definition_.initial.identity);
}
const MissionPlan* CampaignSession::missionPlan() const {
    return definition_.plans.empty() ? nullptr : (definition_.plans[missionIndex(progress())] ? &*definition_.plans[missionIndex(progress())] : nullptr);
}
bool CampaignSession::spellAvailable(Spell spell) const {
    if(!definition_.spellLoadout) return true;
    return ownsSpell(progress(),spell);
}
std::vector<CampaignMission> CampaignSession::availableMissions() const {
    std::vector<CampaignMission> out;
    if(owned_.battle || progress().completed) return out;
    if(!definition_.graph) return {currentMission()};
    const auto& p=progress();
    for(unsigned i=0;i<definition_.missions.size();++i)
        if(campaignNodeAvailable(*definition_.graph,*p.graph,i) &&
           (!p.lastResult || p.lastResult->outcome==CampaignOutcome::victory || same(definition_.missions[i],p.lastResult->realm,p.lastResult->region))) out.push_back(definition_.missions[i]);
    return out;
}
void CampaignSession::selectMission(const std::string& realm,const std::string& region) {
    Mutation mutation(changing_);const auto available=availableMissions();
    require(std::any_of(available.begin(),available.end(),[&](const auto& m){return same(m,realm,region);}),"mission is unavailable");
    auto candidate=owned_;candidate.progress.realm=realm;candidate.progress.region=region;
    validate(candidate);owned_=std::move(candidate);
}
void CampaignSession::startMission() {
    Mutation mutation(changing_);
    require(!owned_.battle && !progress().completed,"campaign has an active battle or is complete");
    require(progress().transition<=std::numeric_limits<std::uint64_t>::max()-2,"campaign attempt counter exhausted");
    auto candidate=owned_;const auto& m=currentMission();
    candidate.battle=resources_->create(m);
    require(candidate.battle->world().state().tick==0 && candidate.battle->world().state().sequence==0,
            "campaign factory did not create a fresh battle");
    if(const auto* plan=missionPlan()) {
        validateMissionPlan(*plan,candidate.battle->mission());candidate.battle->beginMission();
        if(plan->version==2) candidate.progress.mission=startObjectives(*plan,*candidate.battle);
        else {
            auto group=candidate.battle->spawnMissionGroup(plan->initial);
            require(bool(group),"initial mission group blocked");
            MissionProgress progress;progress.initial=*group;
            candidate.progress.mission=std::move(progress);
        }
    }
    if(candidate.progress.graph) {
        const auto index=missionIndex(candidate.progress);const auto& node=definition_.graph->nodes[index];
        if(!node.choice.empty() && std::find(candidate.progress.graph->choices.begin(),candidate.progress.graph->choices.end(),index)==candidate.progress.graph->choices.end()) candidate.progress.graph->choices.push_back(unsigned(index));
    }
    const auto attempt=++candidate.progress.transition;
    candidate.progress.battle=CampaignBattle{m.realm,m.region,m.map,{progress().player,m.opponent},attempt};
    validate(candidate);owned_=std::move(candidate);
}
void CampaignSession::enqueue(SkirmishOrder order) {
    Mutation mutation(changing_);
    require(bool(owned_.battle),"campaign has no active battle");
    const auto spell=orderSpell(order.kind);
    require(spell==Spell::none || spellAvailable(spell),"Spell is locked until its mission reward is earned");
    auto candidate=owned_;candidate.battle->enqueue(std::move(order));
    validate(candidate);owned_=std::move(candidate);
}
CampaignStepReport CampaignSession::step(TickInput input) {
    Mutation mutation(changing_);
    CampaignStepReport report;
    if(!owned_.battle || !input.admitted) return report;
    auto candidate=owned_;
    if(candidate.progress.mission && !input.alternateMode) {
        auto& mission=*candidate.progress.mission;
        if(mission.version==2) admitMissionGroups(*missionPlan(),mission,*candidate.battle);
        else if(mission.stage==1 && !mission.reinforcement) {
            auto wave=candidate.battle->spawnMissionGroup(missionPlan()->reinforcement);
            if(wave) {mission.wave=*wave;mission.reinforcement=true;}
        }
    }
    report.battle=candidate.battle->step(input,candidate.progress.mission ? std::optional<bool>(candidate.progress.mission->stage==2) : std::nullopt);
    if(candidate.progress.mission) {
        auto& mission=*candidate.progress.mission;
        if(mission.version==2) {
            if(!input.alternateMode) advanceObjectives(*missionPlan(),mission,*candidate.battle);
        } else if(mission.stage==0 && missionGroupDead(*candidate.battle,mission.initial)) mission.stage=1;
        else if(mission.stage==1 && mission.reinforcement && missionGroupDead(*candidate.battle,mission.wave)) mission.stage=2;
    }
    auto outcome=candidate.battle->state().outcome;
    if(candidate.progress.mission && candidate.progress.mission->version==2) {
        // Combat deaths take precedence; a protection failure beats objective success.
        if(outcome==Outcome::running) {
            if(protectionFailed(*missionPlan(),*candidate.battle)) outcome=Outcome::defeat;
            else if(objectivesComplete(*candidate.progress.mission)) outcome=Outcome::victory;
            if(outcome!=Outcome::running) report.battle->events.push_back({SkirmishEventKind::finished,candidate.battle->state().wizards[0].actor,{}});
        }
    }
    if(outcome!=Outcome::running) {
        auto& p=candidate.progress;const auto b=*p.battle;
        const auto result=outcome==Outcome::victory ? CampaignOutcome::victory :
                          outcome==Outcome::defeat ? CampaignOutcome::defeat : CampaignOutcome::draw;
        p.lastResult=CampaignResult{b.attempt,b.realm,b.region,result};
        const auto selected=missionIndex(p);
        if(result==CampaignOutcome::victory) {
            if(p.graph) {
                for(auto& w:p.wizards) if(w.id==p.player) for(const auto& spell:definition_.graph->nodes[selected].rewards)
                    if(std::find(w.spells.begin(),w.spells.end(),spell)==w.spells.end()) w.spells.push_back(spell);
            } else if(!definition_.plans.empty() && definition_.plans[selected]) for(auto& w:p.wizards) if(w.id==p.player) {
                const auto& spell=definition_.plans[selected]->rewardSpell;
                if(!spell.empty() && std::find(w.spells.begin(),w.spells.end(),spell)==w.spells.end()) w.spells.push_back(spell);
            }
            auto& r=region(p,definition_.missions[selected]);r.completed=true;r.owner=p.player;
            if(p.graph) p.graph->completed.push_back(unsigned(selected));
            if(selected+1==definition_.missions.size()) p.completed=true;
            else {
                auto next=selected+1;
                if(p.graph) {
                    next=0;while(next<definition_.missions.size() && !campaignNodeAvailable(*definition_.graph,*p.graph,unsigned(next))) ++next;
                    require(next<definition_.missions.size(),"campaign graph has no successor");
                }
                p.realm=definition_.missions[next].realm;p.region=definition_.missions[next].region;
            }
        }
        ++p.transition;p.mission.reset();p.battle.reset();candidate.battle.reset();report.result=p.lastResult;
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
