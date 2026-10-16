#pragma once
#include "simulation/skirmish.hpp"
#include <QString>
namespace mnm::skirmish {
struct UnitView {
    game::Handle actor;
    game::Point cell;
    game::UnitRole role;
    unsigned owner, health, maximum;
    QString action;
    unsigned width = 1;
};
struct ArenaView {
    std::vector<UnitView> units;
    std::vector<game::Projectile> projectiles;
    std::vector<game::Point> cures, spheres;
    unsigned cureMana = 0;
    bool cureEnabled = false, sphereEnabled = false;
    std::array<unsigned, 2> mana{}, wizardHealth{};
    std::optional<game::Handle> selected;
    game::Outcome outcome = game::Outcome::running;
    QString creatureName = "Sentinel";
    QString selection, feedback, resultHelp = "Start a new skirmish or load a checkpoint.";
    bool canCast = false, canOrder = false, busy = false;
};
} // namespace mnm::skirmish
