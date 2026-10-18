#pragma once
#include "../../reconstruction/animation/controller.hpp"
#include "../../reconstruction/casting/admission.hpp"
#include "../../reconstruction/casting/cure_effect.hpp"
#include "../../reconstruction/casting/projectile.hpp"
#include "../../reconstruction/creature-ai/owner_planner.hpp"
#include "../../reconstruction/creature-ai/wizard_projectile_task.hpp"
#include "../../reconstruction/rendering/effect_creature_collision.hpp"
namespace mnm::command_session {
struct NativeWizardSpell {
  int magnitude = 0, duration = 0, range = 0, speed = 0;
};
struct NativeWizardPlan {
  unsigned spell = 0, target = 0, serial = 0, started = 0, expires = 0;
  reconstruction::combat::Cell stand, aim;
  float score = -1;
  bool issued = false;
};
struct NativeWizardController {
  unsigned actor = 0, maximumMana = 0, cursor = 0, nextThink = 0;
  bool producing = false;
  std::array<bool, 104> learned{};
  reconstruction::combat::OwnerTaskSelection selection;
  std::optional<NativeWizardPlan> candidate, best, active;
};
struct NativeWizardEffect {
  bool active = false;
  unsigned generation = 0, spell = 0, animationAsset = 0, animationSequence = 0;
  reconstruction::casting::CureEffect cure;
  reconstruction::casting::ProjectileEffect projectile;
  reconstruction::EffectPlacementRecord placement;
  reconstruction::EffectCreatureTransitionState movement;
  reconstruction::AnimationControllerState animation;
};
struct NativeWizardState {
  unsigned tick = 0, nextSerial = 1, casts = 0, heals = 0, impacts = 0,
           summons = 0, retirements = 0, decisions = 0, manaUses = 0,
           regeneration = 0, refusedCasts = 0, bounces = 0, expirations = 0;
  unsigned planningAllocation = 4, regenerationRate = 1,
           regenerationPeriod = 20, effectCapacity = 16;
  std::vector<NativeWizardController> controllers;
  std::vector<NativeWizardSpell> spells;
  reconstruction::casting::CastAdmissionState admission;
  reconstruction::combat::RandomState random;
  std::vector<NativeWizardEffect> effects;
  std::array<bool, 8> usedSummonSlots{};
  unsigned cureAsset = 0, cureSequence = 0, projectileAsset = 0,
           projectileSequence = 0;
};
} // namespace mnm::command_session
