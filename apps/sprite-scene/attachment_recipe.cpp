#include "attachment_recipe.hpp"
#include <charconv>
#include <stdexcept>
namespace mnm::preview {
AttachmentRecipe modeOneRecipe(const assets::Config& config,std::uint32_t facing){
    auto required=[&](const char* key){const auto* value=config.find("ANI_36",key);
        if(!value)throw std::runtime_error("Missing mode-one effect configuration field");
        return *value;};
    auto number=[](const std::string& text){std::uint32_t value=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
        if(result.ec!=std::errc{} || result.ptr!=text.data()+text.size())throw std::runtime_error("Invalid mode-one effect number");
        return value;};
    const auto base=number(required("AnimationNo"));auto reference=required("AnimationFileRef");
    for(auto& c:reference)if(c>='a' && c<='z')c=char(c-'a'+'A');
    if(reference.size()<8 || reference.compare(0,7,"EFFECTS")!=0 || required("SpritePrinter")!="NORMAL")
        throw std::runtime_error("Mode-one preview requires an EFFECTS reference and NORMAL printer");
    const auto index=number(reference.substr(7));
    if(index>=13)throw std::runtime_error("Mode-one effect file index outside supported registry");
    return {"Sprites/effects"+std::to_string(index)+".ani",reconstruction::modeOneSelection(base,index,facing)};
}
AttachmentRecipe loadModeOneRecipe(assets::AssetStore& store,std::uint32_t facing){
    auto opened=store.open("CFG/Encrypted/effectani.cfg");
    if(const auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
    auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));
    assets::PersistenceLimits limits;limits.inputBytes=limits.decodedBytes=1024*1024;
    auto loaded=assets::loadConfig(*file,true,limits);file.reset();
    if(const auto* e=std::get_if<assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);
    return modeOneRecipe(std::get<assets::Config>(loaded),facing);
}
}
