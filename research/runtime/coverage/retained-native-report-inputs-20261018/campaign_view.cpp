#include "campaign_view.hpp"
#include <QStringList>
#include <algorithm>
namespace mnm::skirmish {
namespace {
QString name(const std::string &id) {
    auto words=QString::fromStdString(id).split('-');
    for(auto &word:words) if(!word.isEmpty()) word[0]=word[0].toUpper();
    return words.join(' ');
}
QString spell(const std::string &id) {return id=="magic-sphere" ? "Magic Sphere" : name(id);}
}
void presentCampaign(const game::CampaignSession &session,const game::CampaignDefinition *definition,ArenaView &v) {
    const auto &p=session.progress();
    if(const auto *plan=session.missionPlan()) {
        v.briefing=QString::fromStdString(plan->briefing);
        if(p.mission) {
            const auto &m=*p.mission;
            if(plan->version==1) {
                v.objective=QString("Objective %1 / 3: %2").arg(m.stage+1).arg(QString::fromStdString(plan->objectives.at(m.stage)));
                if(m.stage==1 && !m.reinforcement) v.objective+=" Reinforcements awaiting a clear arrival area.";
            } else {
                QStringList lines;
                for(std::size_t i=0;i<plan->nodes.size();++i) {
                    const auto &node=plan->nodes[i];const auto &state=m.objectives.at(i);
                    // Protection remains a live condition even after its saved flag completes.
                    QString status=node.kind==game::ObjectiveKind::protect ? "Keep alive throughout" : state.complete ? "Complete" : "Active";
                    if(node.kind!=game::ObjectiveKind::protect && !state.complete && !std::all_of(node.after.begin(),node.after.end(),[&](auto d){return m.objectives.at(d).complete;})) status="Waiting";
                    if(node.kind==game::ObjectiveKind::survive) status+=QString(" · %1/%2 ticks").arg(state.elapsed).arg(node.ticks);
                    if(node.kind==game::ObjectiveKind::eliminate && node.target==game::MissionTarget::group && m.groups.at(node.group).empty()) {
                        const auto &group=plan->groups.at(node.group);
                        const bool ready=std::all_of(group.after.begin(),group.after.end(),[&](auto d){return m.objectives.at(d).complete;});
                        status=ready ? "Awaiting a clear arrival area" : "Waiting for reinforcements";
                    }
                    lines << QString::fromStdString(node.text)+" — "+status;
                }
                v.objective=lines.join('\n');
            }
        }
    }
    if(p.graph && definition && definition->graph) {
        const auto &graph=*definition->graph;const auto available=session.availableMissions();QStringList earned;
        for(auto i:p.graph->completed) for(const auto &reward:graph.nodes.at(i).rewards)
            if(!earned.contains(spell(reward))) earned << spell(reward);
        if(!earned.isEmpty()) v.reward="Earned spells: "+earned.join(", ");
        for(std::size_t i=0;i<definition->missions.size();++i) {
            const auto &mission=definition->missions[i];const auto &node=graph.nodes.at(i);
            CampaignChoiceView choice;choice.realm=QString::fromStdString(mission.realm);choice.region=QString::fromStdString(mission.region);
            choice.selected=mission.realm==p.realm && mission.region==p.region;
            choice.enabled=std::any_of(available.begin(),available.end(),[&](const auto &a){return a.realm==mission.realm && a.region==mission.region;});
            const bool completed=std::find(p.graph->completed.begin(),p.graph->completed.end(),i)!=p.graph->completed.end();
            const bool other=std::any_of(p.graph->choices.begin(),p.graph->choices.end(),[&](auto j){return j!=i && !node.choice.empty() && graph.nodes.at(j).choice==node.choice;});
            QString status=completed?"Complete":other?"Other route chosen":p.completed?"Campaign complete":choice.selected && session.battle()?"In progress":choice.enabled?"Available":"Locked";
            QStringList rewards;for(const auto &reward:node.rewards) rewards << spell(reward);
            choice.text=name(mission.region)+" — "+status;
            if(!rewards.isEmpty()) choice.text+=" · Reward: "+rewards.join(", ");
            QStringList all,any;for(auto j:node.all) all << name(definition->missions.at(j).region);for(auto j:node.any) any << name(definition->missions.at(j).region);
            if(!all.isEmpty()) choice.detail="Requires: "+all.join(", ");
            if(!any.isEmpty()) choice.detail+=(choice.detail.isEmpty()?QString():"; ")+QString("Requires either: ")+any.join(" or ");
            if(!node.choice.empty()) choice.detail+=(choice.detail.isEmpty()?QString():". ")+QString("Starting this mission commits its route, including retries.");
            v.choices.push_back(std::move(choice));
        }
    } else if(p.identity.id=="native-plains-objectives-campaign" && p.lastResult && p.lastResult->region=="plains-crossing" && p.lastResult->outcome==game::CampaignOutcome::victory)
        v.reward="Reward earned: Cure unlocked for the arena finale.";
}
}
