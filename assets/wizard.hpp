#pragma once
#include "persistence.hpp"

namespace mnm::assets {
struct WizardStats {
    std::string name;
    std::optional<std::string> animationFile;
    std::optional<std::int32_t> maxHealth, maxMana, intelligence, magicResistance,
        controlLimit, combatModifier, difficultyModifier;
    std::optional<std::int32_t> aggressiveAttack, cautiousAttack, occupyPowerPoint,
        collectMana, evadeDetection, scout, retreat, collectFood, protectWizard;
    std::optional<std::int32_t> startTalismansLaw, startTalismansNeutral, startTalismansChaos;
};
struct WizardItem {
    std::optional<bool> hasIt, researched;
};
struct WizardAction {
    std::optional<bool> valid;
    std::optional<std::int32_t> beginTime, endTime, dependency, waitingTime, rating,
        numberOfActions, health, mana, sectionId, spellType, spellTargetId,
        xPosition, yPosition, zPosition, xTargetPosition, yTargetPosition, zTargetPosition;
    std::optional<std::string> nodeType, targetNodeType, destination, spellTarget;
};
struct WizardDefinition {
    WizardStats stats;
    std::optional<bool> validWizardFile;
    std::map<std::uint32_t, WizardItem> items;
    // Absence and explicit false remain distinct. These are IDs, not asset paths.
    std::map<std::uint32_t, bool> startSpells, startObjects, startMagicItems;
    std::map<std::uint32_t, WizardAction> actions;
    Config config; // Comment-normalized properties, including unknown sections/keys.
    Bytes source; // Exact original bytes; owned independently of input/file.
};
struct WizardLimits {
    std::uint32_t inputBytes=256*1024, lines=8192, entries=4096,
        maximumIndex=65535, stringBytes=1024;
};
using WizardResult = PersistenceResult<WizardDefinition>;
WizardResult decodeWizard(const Bytes&, const WizardLimits& = {});
WizardResult loadWizard(AssetFile&, const WizardLimits& = {});
}
