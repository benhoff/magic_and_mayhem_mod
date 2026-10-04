#include "attachment_recipe.hpp"
#include <stdexcept>
#include <iostream>
static void require(bool ok){if(!ok)throw std::runtime_error("Recipe contract differs");}
int main()try{
    mnm::assets::Config config;config.sections["ani_36"]={{"animationno","40"},{"animationfileref","effects2"},{"spriteprinter","NORMAL"}};
    const auto recipe=mnm::preview::modeOneRecipe(config,7);config.sections.clear();
    require(recipe.ani=="Sprites/effects2.ani" && recipe.selection.sequence==47 && recipe.selection.assetIndex==2);
    for(const auto& reference:{"EFFECTS../file","EFFECTS13","EFFECTS-1","EFFECTS","EFFECTS2/evil"}){
        config.sections["ani_36"]={{"animationno","40"},{"animationfileref",reference},{"spriteprinter","NORMAL"}};
        bool failed=false;try{mnm::preview::modeOneRecipe(config,0);}catch(const std::runtime_error&){failed=true;}require(failed);}
    config.sections["ani_36"]={{"animationno","40"},{"animationfileref","EFFECTS2"},{"spriteprinter","TRANSPARENT"}};
    bool failed=false;try{mnm::preview::modeOneRecipe(config,0);}catch(const std::runtime_error&){failed=true;}require(failed);
    std::cout<<"Owned configuration recipe and unsupported reference/printer rejection pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
