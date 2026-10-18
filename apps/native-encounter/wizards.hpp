#pragma once
#include "encounter.hpp"
namespace mnm::encounter {
// Staged native composition; no callback or host pointer enters a checkpoint.
void advanceWizardWorld(cs::Continuation &, const Profile &,
                        cs::model::OwnedCreatureCommands &,
                        const cs::NativeConsequences &);
game::Bytes loadWizardInputs(game::Bytes base, const std::filesystem::path &);
Profile decodeWizardInputs(game::Bytes);
} // namespace mnm::encounter
