#include "controller.hpp"
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
Controller::Controller() : session_(newSession()), playback_([this] { advance(); }) {
    selected_ = session_.state().wizards[0].actor;
    feedback_ = "Summon a Sentinel near your wizard, then attack the rival.";
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
    const auto s = session_.state();
    const auto c = session_.combat().combat();
    v.outcome = s.outcome;
    v.projectiles = s.projectiles;
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
        const auto &e = *session_.world().find(r.actor);
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
        v.units.push_back(
            {r.actor, {e.x, e.y, e.z}, r.role, e.owner, f.health, f.stats.maximumHealth, action});
        if (selected_ == std::optional<game::Handle>(r.actor)) {
            v.selection = QString("%1 %2 · %3/%4 HP · %5")
                              .arg(e.owner ? "Rival" : "Your",
                                   r.role == game::UnitRole::wizard ? "wizard" : "Sentinel")
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
        v.selection = "Select your wizard or a Sentinel.";
    return v;
}
void Controller::advance() {
    last_ = session_.step();
    for (const auto &e : last_.events)
        if (e.actor == session_.state().wizards[0].actor &&
            e.kind == game::SkirmishEventKind::refused)
            feedback_ = refusal(e.refusal);
    if (session_.state().outcome != game::Outcome::running)
        playback_.pause();
    if (afterTick)
        afterTick();
}
void Controller::play() {
    if (session_.state().outcome == game::Outcome::running)
        playback_.play();
}
void Controller::pause() { playback_.pause(); }
void Controller::step() { playback_.step(); }
void Controller::restart() {
    pause();
    auto fresh = newSession();
    session_ = std::move(fresh);
    selected_ = session_.state().wizards[0].actor;
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
                : m == CursorMode::bolt ? "Click a living enemy to cast Arc Bolt."
                                        : "Select a unit.";
    changed();
}
void Controller::cell(game::Point p, bool right) {
    if (!game::arenaCell(p))
        throw std::invalid_argument("Choose a cell inside the arena");
    const auto v = view();
    const auto hit = std::find_if(v.units.begin(), v.units.end(),
                                  [&](const UnitView &u) { return u.health && u.cell == p; });
    std::optional<game::SkirmishOrder> order;
    if (mode_ == CursorMode::summon) {
        if (!selected_)
            throw std::invalid_argument("Select your wizard");
        order = game::SkirmishOrder{game::SkirmishOrderKind::summon, *selected_, {}, p};
    } else if (mode_ == CursorMode::bolt) {
        if (!selected_ || hit == v.units.end() || !hit->owner)
            throw std::invalid_argument("Choose a living enemy");
        order = game::SkirmishOrder{game::SkirmishOrderKind::bolt, *selected_, hit->actor, {}};
    } else if (right) {
        if (!selected_)
            throw std::invalid_argument("Select a friendly unit first");
        if (v.canCast && hit != v.units.end() && hit->owner)
            order = game::SkirmishOrder{game::SkirmishOrderKind::bolt, *selected_, hit->actor, {}};
        else if (v.canOrder)
            order = game::SkirmishOrder{
                hit != v.units.end() && hit->owner ? game::SkirmishOrderKind::attack
                                                   : game::SkirmishOrderKind::move,
                *selected_,
                hit != v.units.end() && hit->owner ? std::optional<game::Handle>(hit->actor)
                                                   : std::nullopt,
                hit != v.units.end() && hit->owner ? std::nullopt : std::optional<game::Point>(p)};
        else
            throw std::invalid_argument("Select a Sentinel to move or attack");
    } else
        selected_ = hit == v.units.end() ? std::nullopt : std::optional<game::Handle>(hit->actor);
    if (order) {
        session_.enqueue(*order);
        if (onOrder)
            onOrder(*order);
        mode_ = CursorMode::select;
        feedback_ = "Order queued for the next tick.";
    }
    changed();
}
void Controller::stop() {
    if (!selected_ || !view().canOrder)
        throw std::invalid_argument("Select a friendly Sentinel");
    game::SkirmishOrder o{game::SkirmishOrderKind::stop, *selected_, {}, {}};
    session_.enqueue(o);
    if (onOrder)
        onOrder(o);
    feedback_ = "Stop queued.";
    changed();
}
void Controller::save(const std::filesystem::path &p, bool overwrite) {
    pause();
    const auto r = game::writeSnapshot(p, session_.world().state(), overwrite);
    if (!r.durable)
        throw std::runtime_error(r.detail);
    feedback_ = "Skirmish saved. Playback is paused.";
    changed();
}
void Controller::load(const std::filesystem::path &p) {
    pause();
    auto candidate = skirmish::load(p);
    session_ = std::move(candidate);
    selected_ = session_.state().wizards[0].actor;
    mode_ = CursorMode::select;
    last_ = {};
    feedback_ = "Skirmish loaded. Playback is paused.";
    changed();
}
} // namespace mnm::skirmish
