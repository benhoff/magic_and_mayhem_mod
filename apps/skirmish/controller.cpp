#include "controller.hpp"
#include "campaign_view.hpp"
#include "cure_runtime.hpp"
#include "creature_runtime.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
QString refusal(game::SkirmishRefusal r) {
    switch (r) {
    case game::SkirmishRefusal::busy:
        return "Your wizard is already casting.";
    case game::SkirmishRefusal::mana:
        return "Not enough mana.";
    case game::SkirmishRefusal::placement:
        return "That cell is blocked or too far from the wizard.";
    case game::SkirmishRefusal::capacity:
        return "Creature limit reached.";
    case game::SkirmishRefusal::target:
        return "That target is no longer available.";
    default:
        return "That order could not be applied.";
    }
}
} // namespace
Controller::Controller(std::filesystem::path root, std::shared_ptr<const game::CombatDriver> driver) : driver_(std::move(driver)), session_(newSession(driver_)), mapRoot_(std::move(root)), playback_([this] { advance(); }) {
    selected_ = session().state().wizards[0].actor;
    feedback_ = driver_ ? "Summon a Crocodile near your wizard, then attack the rival." : "Summon a Sentinel near your wizard, then attack the rival.";
    playback_.onRefresh = [this] { changed(); };
    playback_.onPlaying = [this](bool) { changed(); };
    playback_.onError = [this](const std::string &e) {
        feedback_ = QString::fromStdString(e);
        changed();
    };
}
void Controller::changed() {
    if (onChanged)
        onChanged();
}
ArenaView Controller::view() const {
    ArenaView v;
    if(campaign_) presentCampaign(*campaign_,mapResources_?&mapResources_->definition():nullptr,v);
    if(mapPresentation_ && mapResources_ && campaign_) {
        bool terrain=campaign_->currentMission().map==mapResources_->mission().map;
        if(!campaign_->progress().graph && !hasBattle() && campaign_->progress().lastResult) {
            const auto &result=*campaign_->progress().lastResult;
            for(const auto &mission:mapResources_->definition().missions)
                terrain|=mission.realm==result.realm && mission.region==result.region && mission.map==mapResources_->mission().map;
        }
        if(terrain) v.mission=mapPresentation_;
    }
    if (!hasBattle()) {
        const auto &progress = campaign_->progress();
        v.resultHelp =
            progress.completed ? "Campaign complete. Save your progress."
            : progress.lastResult && progress.lastResult->outcome == game::CampaignOutcome::victory
                ? progress.graph ? "Choose a mission, then Continue." : "Continue to the next duel, or save your progress."
                : progress.lastResult ? "Retry this mission, or save your progress." : "Continue to start this mission, or save your progress.";
        v.selection = progress.completed ? "Campaign complete" : "Ready for the next attempt";
        if (progress.lastResult) {
            switch (progress.lastResult->outcome) {
            case game::CampaignOutcome::victory:
                v.outcome = game::Outcome::victory;
                break;
            case game::CampaignOutcome::defeat:
                v.outcome = game::Outcome::defeat;
                break;
            case game::CampaignOutcome::draw:
                v.outcome = game::Outcome::draw;
                break;
            }
        }
        v.feedback = feedback_;
        return v;
    }
    const auto s = session().state();
    const auto c = session().combat().combat();
    v.creatureName = session().combat().driver() ? "Crocodile" : "Sentinel";
    const auto creatureViews=creatureBattleViews(session().combat());
    v.outcome = s.outcome;
    v.projectiles = s.projectiles;
    v.cureEnabled = bool(session().effectsRuntime()) && (!campaign_ || campaign_->spellAvailable(game::Spell::cure));
    v.cureMana = cureCost;
    v.sphereEnabled = session().effectsRuntime() && session().effectsRuntime()->supports(game::Spell::sphere) && (!campaign_ || campaign_->spellAvailable(game::Spell::sphere));
    for (const auto &effect : sphereViews(session().combat())) v.spheres.push_back(effect.fine);
    for (const auto &effect : cureViews(session().combat())) v.cures.push_back(effect.position);
    v.selected = selected_;
    v.feedback = feedback_;
    for (unsigned i = 0; i < 2; ++i) {
        v.mana[i] = s.wizards[i].mana;
        const auto f =
            std::find_if(c.fighters.begin(), c.fighters.end(),
                         [&](const game::Fighter &f) { return f.actor == s.wizards[i].actor; });
        v.wizardHealth[i] = f->health;
    }
    for (const auto &r : s.units) {
        const auto &e = *session().world().find(r.actor);
        const auto &f = *std::find_if(c.fighters.begin(), c.fighters.end(),
                                      [&](const game::Fighter &f) { return f.actor == r.actor; });
        QString action = "Ready";
        if (!f.health)
            action = "Defeated";
        else if (r.role == game::UnitRole::wizard) {
            const auto &w = s.wizards[e.owner];
            if (w.spell != game::Spell::none)
                action = w.spell == game::Spell::summon ? "Summoning" : "Casting Arc Bolt";
        } else if (f.action == game::CombatAction::windingUp)
            action = "Attacking";
        else if (f.action == game::CombatAction::recovering)
            action = "Recovering";
        else if (game::active(e.motion->action))
            action = "Moving";
        else if (f.control == game::CombatControl::hold)
            action = "Holding";
        if(r.role==game::UnitRole::sentinel && f.health) {
            auto found=std::find_if(creatureViews.begin(),creatureViews.end(),[&](const auto &a){return a.actor==r.actor;});
            if(found!=creatureViews.end()) {
                switch(found->phase) {
                case game::CreaturePhase::attacking:action="Attacking";break;
                case game::CreaturePhase::stunned:action="Stunned";break;
                case game::CreaturePhase::knockback:action="Knocked back";break;
                case game::CreaturePhase::ready:action="Recovering";break;
                case game::CreaturePhase::blocked:action="Blocked";break;
                case game::CreaturePhase::pursuing:action="Pursuing";break;
                default:break;
                }
            }
        }
        v.units.push_back(
            {r.actor, {e.x, e.y, e.z}, r.role, e.owner, f.health, f.stats.maximumHealth, action,
             session().combat().navigation()->wizardType()!=session().combat().navigation()->creatureType()
                 ? unsigned(session().combat().navigation()->actorFootprint(e).x)
                 : (session().mission().map==terrainBattleMap || (r.role==game::UnitRole::sentinel && session().combat().driver()))?2U:1U});
        if (selected_ == std::optional<game::Handle>(r.actor)) {
            v.selection = QString("%1 %2 · %3/%4 HP · %5")
                              .arg(e.owner ? "Rival" : "Your",
                                   r.role == game::UnitRole::wizard ? QString("wizard") : v.creatureName)
                              .arg(f.health)
                              .arg(f.stats.maximumHealth)
                              .arg(action);
            const bool eligible = f.health && !e.owner && s.outcome == game::Outcome::running;
            v.canCast = eligible && r.role == game::UnitRole::wizard;
            v.canOrder = eligible && r.role == game::UnitRole::sentinel;
            v.busy = v.canCast && s.wizards[0].spell != game::Spell::none;
        }
    }
    if (v.selection.isEmpty())
        v.selection = QString("Select your wizard or a %1.").arg(v.creatureName);
    return v;
}
void Controller::advance() {
    if (!hasBattle())
        return;
    if (campaign_) {
        const auto before=campaign_->progress().mission;
        auto report = campaign_->step();
        if(before && campaign_->progress().mission) {
            const auto& after=*campaign_->progress().mission;
            if(after.version==2) {
                for(std::size_t i=0;i<after.objectives.size();++i) if(after.objectives[i].complete && !before->objectives[i].complete && campaign_->missionPlan()->nodes.at(i).kind!=game::ObjectiveKind::protect)
                    feedback_=QString::fromStdString(campaign_->missionPlan()->nodes.at(i).text)+" — complete.";
                for(std::size_t i=0;i<after.groups.size();++i) if(!after.groups[i].empty() && before->groups[i].empty()) feedback_="Enemy reinforcements have arrived.";
            } else if(after.stage!=before->stage) feedback_=QString::fromStdString(campaign_->missionPlan()->objectives[after.stage]);
            else if(after.reinforcement && !before->reinforcement) feedback_="Enemy reinforcements have arrived.";
        }
        last_ = report.battle.value_or(game::SkirmishReport{});
        if (report.result) {
            selected_.reset();
            mode_ = CursorMode::select;
            feedback_ = campaign_->progress().completed ? "Campaign complete. Save your progress."
                        : report.result->outcome == game::CampaignOutcome::victory
                            ? "Victory. Choose the next mission or Continue."
                            : "Attempt ended. Retry this mission.";
        }
    } else
        last_ = session_.step();
    for (const auto &e : last_.events)
        if (hasBattle() && e.actor == session().state().wizards[0].actor &&
            e.kind == game::SkirmishEventKind::refused)
            feedback_ = refusal(e.refusal);
    if (!hasBattle() || session().state().outcome != game::Outcome::running)
        playback_.pause();
    if (afterTick)
        afterTick();
}
void Controller::play() {
    if (hasBattle() && session().state().outcome == game::Outcome::running)
        playback_.play();
}
void Controller::pause() { playback_.pause(); }
void Controller::step() {
    if (hasBattle())
        playback_.step();
}
void Controller::restart() {
    pause();
    auto fresh = newSession(driver_);
    session_ = std::move(fresh);
    campaign_.reset();
    mapResources_.reset();mapPresentation_.reset();
    selected_ = session().state().wizards[0].actor;
    mode_ = CursorMode::select;
    last_ = {};
    feedback_ = "New skirmish. Your wizard is selected.";
    changed();
}
void Controller::setMode(CursorMode m) {
    if (m != CursorMode::select && !view().canCast)
        throw std::invalid_argument("Select your wizard to cast");
    mode_ = m;
    feedback_ = m == CursorMode::summon ? "Click an empty cell within two cells of your wizard."
                : m == CursorMode::cure ? "Click a living friendly creature to cast Cure."
                : m == CursorMode::sphere ? "Click a living enemy to cast Magic Sphere."
                : m == CursorMode::bolt ? "Click a living enemy to cast Arc Bolt."
                                        : "Select a unit.";
    changed();
}
void Controller::cell(game::Point p, bool right) {
    if (!hasBattle())
        return;
    const auto v = view();
    const auto hit = std::find_if(v.units.begin(), v.units.end(),
                                  [&](const UnitView &u) { return u.health && p.z==u.cell.z && p.x>=u.cell.x && p.x<u.cell.x+int(u.width) && p.y>=u.cell.y && p.y<u.cell.y+int(u.width); });
    if(hit==v.units.end() && !game::missionCell(session().mission(),p))
        throw std::invalid_argument("Choose a supported mission cell");
    std::optional<game::SkirmishOrder> order;
    if (mode_ == CursorMode::summon) {
        if (!selected_)
            throw std::invalid_argument("Select your wizard");
        order = game::SkirmishOrder{game::SkirmishOrderKind::summon, *selected_, {}, p};
    } else if (mode_ == CursorMode::cure) {
        if (!selected_ || !v.cureEnabled || hit == v.units.end() || hit->owner || hit->role != game::UnitRole::sentinel)
            throw std::invalid_argument("Choose a living friendly creature");
        order = game::SkirmishOrder{game::SkirmishOrderKind::cure, *selected_, hit->actor, {}};
    } else if ((mode_ == CursorMode::bolt || mode_ == CursorMode::sphere)) {
        if (!selected_ || hit == v.units.end() || !hit->owner)
            throw std::invalid_argument("Choose a living enemy");
        order = game::SkirmishOrder{mode_ == CursorMode::sphere ? game::SkirmishOrderKind::sphere : game::SkirmishOrderKind::bolt, *selected_, hit->actor, {}};
    } else if (right) {
        if (!selected_)
            throw std::invalid_argument("Select a friendly unit first");
        if (v.canCast && hit != v.units.end() && hit->owner)
            order = game::SkirmishOrder{mode_ == CursorMode::sphere ? game::SkirmishOrderKind::sphere : game::SkirmishOrderKind::bolt, *selected_, hit->actor, {}};
        else if (v.canOrder)
            order = game::SkirmishOrder{
                hit != v.units.end() && hit->owner ? game::SkirmishOrderKind::attack
                                                   : game::SkirmishOrderKind::move,
                *selected_,
                hit != v.units.end() && hit->owner ? std::optional<game::Handle>(hit->actor)
                                                   : std::nullopt,
                hit != v.units.end() && hit->owner ? std::nullopt : std::optional<game::Point>(p)};
        else
            throw std::invalid_argument("Select a creature to move or attack");
    } else
        selected_ = hit == v.units.end() ? std::nullopt : std::optional<game::Handle>(hit->actor);
    if (order) {
        enqueue(*order);
        if (onOrder)
            onOrder(*order);
        mode_ = CursorMode::select;
        feedback_ = "Order queued for the next tick.";
    }
    changed();
}
void Controller::stop() {
    if (!selected_ || !view().canOrder)
        throw std::invalid_argument("Select a friendly creature");
    game::SkirmishOrder o{game::SkirmishOrderKind::stop, *selected_, {}, {}};
    enqueue(o);
    if (onOrder)
        onOrder(o);
    feedback_ = "Stop queued.";
    changed();
}
void Controller::save(const std::filesystem::path &p, bool overwrite) {
    if (campaign_)
        throw std::invalid_argument("Use Save campaign for campaign progress");
    pause();
    const auto r = game::writeSnapshot(p, session().world().state(), overwrite);
    if (!r.durable)
        throw std::runtime_error(r.detail);
    feedback_ = "Skirmish saved. Playback is paused.";
    changed();
}
void Controller::load(const std::filesystem::path &p) {
    pause();
    auto candidate = skirmish::load(p);
    session_ = std::move(candidate);
    campaign_.reset();
    mapResources_.reset();mapPresentation_.reset();
    selected_ = session().state().wizards[0].actor;
    mode_ = CursorMode::select;
    last_ = {};
    feedback_ = "Skirmish loaded. Playback is paused.";
    changed();
}
const game::SkirmishSession &Controller::session() const {
    if (!campaign_)
        return session_;
    if (!campaign_->battle())
        throw std::logic_error("Campaign has no active battle");
    return *campaign_->battle();
}
void Controller::enqueue(game::SkirmishOrder order) {
    if (campaign_)
        campaign_->enqueue(std::move(order));
    else
        session_.enqueue(std::move(order));
}
void Controller::adopted(const QString &message) {
    selected_ = hasBattle() ? std::optional<game::Handle>(session().state().wizards[0].actor)
                            : std::nullopt;
    mode_ = CursorMode::select;
    last_ = {};
    feedback_ = message;
    changed();
}
void Controller::newMapCampaign(std::filesystem::path root, bool stagedMission) {startMapCampaign(std::move(root),stagedMission,false);}
void Controller::newCrossroadsCampaign(std::filesystem::path root) {startMapCampaign(std::move(root),false,true);}
void Controller::startMapCampaign(std::filesystem::path root, bool stagedMission, bool crossroads) {
    pause();
    if(root.empty()) root=mapRoot_;
    if(root.empty()) throw std::invalid_argument("Select the installed game data directory");
    auto resources=mapCampaignResources(root,true,true,bool(driver_) || stagedMission || crossroads,bool(driver_) || stagedMission || crossroads,stagedMission,false,crossroads);
    auto presentation=std::make_shared<const MissionPresentation>(*resources);
    auto candidate=std::make_unique<game::CampaignSession>(skirmish::newMapCampaign(resources));
    candidate->startMission();
    mapRoot_=std::move(root);mapResources_=std::move(resources);mapPresentation_=std::move(presentation);campaign_=std::move(candidate);
    adopted("Plains campaign started. Move and cast on the installed terrain. Playback is paused.");
}
void Controller::newCampaign() {
    pause();
    auto candidate = std::make_unique<game::CampaignSession>(skirmish::newCampaign(true,true,driver_));
    candidate->startMission();
    mapPresentation_.reset();mapResources_.reset();
    campaign_ = std::move(candidate);
    adopted("New campaign. First duel is paused.");
}
void Controller::loadCampaign(const std::filesystem::path &path) {
    pause();
    const auto outer=game::readSnapshot(path);
    const auto progress=game::decodeCampaign(outer.campaign);
    std::unique_ptr<game::CampaignSession> candidate;
    std::shared_ptr<const MissionPresentation> presentation;
    std::shared_ptr<const MapCampaignResources> resources;
    if(progress.identity.id=="native-plains-campaign" || progress.identity.id=="native-plains-creature-campaign" || progress.identity.id=="native-plains-species-campaign" || progress.identity.id=="native-plains-objectives-campaign" || progress.identity.id=="native-plains-survival-campaign" || progress.identity.id=="native-plains-crossroads-campaign") {
        if(mapRoot_.empty()) throw std::invalid_argument("Load this campaign with --asset-root GAME_DIRECTORY, or start a map campaign first");
        const bool crossroads=progress.identity.id=="native-plains-crossroads-campaign";
        const bool objectives=progress.identity.id=="native-plains-survival-campaign";
        const bool staged=crossroads || objectives || progress.identity.id=="native-plains-objectives-campaign";
        const bool enabled=std::any_of(progress.wizards.begin(),progress.wizards.end(),[&](const auto &w){return w.id==progress.player && std::find(w.spells.begin(),w.spells.end(),"cure")!=w.spells.end();});
        const bool spheres=std::any_of(progress.wizards.begin(),progress.wizards.end(),[&](const auto &w){return w.id==progress.player && std::find(w.spells.begin(),w.spells.end(),"magic-sphere")!=w.spells.end();});
        resources=mapCampaignResources(mapRoot_,enabled || staged,spheres || staged,staged || progress.identity.id=="native-plains-creature-campaign" || progress.identity.id=="native-plains-species-campaign",staged || progress.identity.id=="native-plains-species-campaign",staged,objectives,crossroads);
        presentation=std::make_shared<const MissionPresentation>(*resources);
        candidate=std::make_unique<game::CampaignSession>(skirmish::loadMapCampaign(resources,path));
    } else candidate=std::make_unique<game::CampaignSession>(skirmish::loadCampaign(path,driver_));
    mapResources_=std::move(resources);mapPresentation_=std::move(presentation);campaign_=std::move(candidate);
    adopted("Campaign loaded. Playback is paused.");
}
void Controller::saveCampaign(const std::filesystem::path &path, bool overwrite) {
    if (!campaign_)
        throw std::invalid_argument("Start or load a campaign first");
    pause();
    const auto result = campaign_->save(path, overwrite);
    if (!result.durable)
        throw std::runtime_error(result.detail);
    feedback_ = "Campaign saved. Playback is paused.";
    changed();
}
void Controller::selectCampaignMission(const QString &realm,const QString &region) {
    if(!campaign_) throw std::invalid_argument("Start or load a campaign first");
    pause();campaign_->selectMission(realm.toStdString(),region.toStdString());
    adopted("Mission selected. Continue starts it and commits the route. Playback is paused.");
}
void Controller::continueCampaign() {
    if (!campaign_)
        throw std::invalid_argument("Start or load a campaign first");
    pause();
    campaign_->startMission();
    adopted("Campaign mission started. Playback is paused.");
}
} // namespace mnm::skirmish
