#pragma once
#include "attachment.hpp"
#include "persistence.hpp"
namespace mnm::preview {
struct AttachmentRecipe {std::string ani;reconstruction::ModeOneSelection selection;};
AttachmentRecipe modeOneRecipe(const assets::Config&,std::uint32_t facing);
AttachmentRecipe loadModeOneRecipe(assets::AssetStore&,std::uint32_t facing);
}
