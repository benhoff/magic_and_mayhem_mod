#pragma once
#include "execution.hpp"
namespace mnm::reconstruction::combat {
// Per-frame fields not present in the bounded decision projection. Optional
// effects are owned serials; animation slots are logical, never PE offsets.
struct CreatureMaintenance {
  unsigned random98 = 0, upkeepDeadline = 0, status114 = 0, status118 = 0,
           status744 = 0, status748 = 0, status74c = 0, flight110 = 0,
           status778 = 0, status77c = 0, status780 = 0, animation7cc = 0,
           status821 = 0, status8c5 = 0, status78c = 0, status790 = 0,
           status819 = 0, status81d = 0, status8d5 = 0, status8d9 = 0,
           status8dd = 0, powerDeadline = 0, animation889 = 0, animation8bd = 0,
           flight809 = 0, timer94 = 0, marker750 = 0, block71d = 0,
           timer660 = 0, recent6b0 = 0;
  int lifespan = 0, suspension = 0, status8cd = 0;
  std::array<unsigned, 8> visibilityAge{};
  std::optional<unsigned> statusEffect, damageEffect, flightEffect;
};
struct MaintenanceEnvironment {
  unsigned tick = 0, upkeepInterval = 0, upkeepCost = 0, statusDamage = 0,
           statusInterval = 0, poisonInterval = 0, powerInterval = 0,
           powerMaximum = 0, restoreMode = 0, drainQuantity = 0;
  int localOwner = -1, drainDivisor = 1, effectDamage = 0;
  bool diagnostic = false;
};
enum class MaintenanceAnimation {
  bonus,
  form,
  flight,
  response,
  auxiliary,
  owner
};
enum class MaintenanceColor { add, clear, remove, tick };
struct MaintenanceServices {
  std::function<void(AutonomousCreature &)> repairAnimation, refresh,
      expireStatus, bonusWork, expireForm, repairStatus, removeFromWorld,
      remove, repairFlight, action, activate, diagnostic;
  std::function<unsigned()> random;
  std::function<void(AutonomousCreature &, MaintenanceColor, unsigned)> color;
  // Engine scoring and owner-resource consequences remain explicit services.
  // Implementations can mutate the actor; the original checks the result.
  std::function<void(AutonomousCreature &)> upkeep;
  std::function<void(AutonomousCreature &, int, int, bool)> health;
  std::function<void(AutonomousCreature &, unsigned)> drain, restoreMode,
      ownerEvent;
  std::function<int(AutonomousCreature &, MaintenanceAnimation)> poll;
  std::function<void(AutonomousCreature &, std::optional<unsigned> &, unsigned)>
      effect;
  std::function<void(unsigned)> releaseEffect, repairReference;
};
void maintainCreature(AutonomousCreature &, CreatureExecution &,
                      CreatureMaintenance &, AutonomousWorld &,
                      const MaintenanceEnvironment &,
                      const MaintenanceServices &);
bool rangedReady(const AutonomousCreature &, const CreatureExecution &,
                 unsigned tick);
} // namespace mnm::reconstruction::combat
