#include "effect_animation_catalog_binding.hpp"
#include "effect_animation_selection.hpp"
#include "sprite_loader.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool value){if(!value)throw std::runtime_error("Effect catalog test failed");}
static assets::Config config(const std::string& text){
    auto result=assets::decodeConfig({text.begin(),text.end()});
    require(std::holds_alternative<assets::Config>(result));return std::get<assets::Config>(result);
}
static const std::string fixture="[HEADER]\nNumberofAnimations=1 ; count\nNumberOfEffectsFile=1\n[ANI_0]\nAnimationNo=0\nAnimationFileRef=effects0\nSpritePrinter=normal\nData1=4294967295\n";
static void units(){
    auto recipes=assets::readEffectAnimationRecipes(config(fixture));
    require(recipes.fileCount==1 && recipes.entries.size()==1 && recipes.entries[0].printer==31 && recipes.entries[0].data1==0xffffffffu);
    for(const auto& item:std::vector<std::pair<std::string,unsigned>>{{"TRANSPARENT",2},{"TRANSPARENT25",3},{"TRANSPARENT50",4},{"TRANSPARENT75",5}}){
        auto c=config(fixture);c.sections["ani_0"]["spriteprinter"]=item.first;
        require(assets::readEffectAnimationRecipes(c).entries[0].printer==item.second);
    }
    auto refused=[](assets::Config c){bool failed=false;try{assets::readEffectAnimationRecipes(c);}catch(const std::invalid_argument&){failed=true;}require(failed);};
    for(const auto& item:std::vector<std::pair<std::string,std::string>>{{"animationno","-1"},{"animationno","1tail"},{"animationno","4294967296"},{"animationfileref","EFFECTS00"},{"animationfileref","EFFECTS1"},{"spriteprinter","UNKNOWN"},{"data1",""}}){
        auto c=config(fixture);c.sections["ani_0"][item.first]=item.second;refused(c);
    }
    auto c=config(fixture);c.sections["ani_0"].erase("data1");refused(c);
    c=config(fixture);c.sections["header"]["numberofanimations"]="4097";refused(c);
    c=config(fixture);c.sections["header"]["numberofeffectsfile"]="0";refused(c);
    c=config(fixture);c.sections["header"]["numberofeffectsfile"]="65";refused(c);
    assets::EffectAnimationCatalog catalog;catalog.recipes=recipes;
    assets::Animation a;a.starts={0,2};a.records={{0,7,{}},{6,0,{}}};catalog.animations={a};
    auto binding=reconstruction::bindEffectAnimationCatalog(0,catalog);catalog={};
    require(binding.entry.opaque==0xffffffffu && binding.player.sprite()==7);
    require(binding.player.tick()==0 && binding.player.tick()==1);binding.player.start();require(binding.player.sprite()==7);
    bool failed=false;try{reconstruction::bindEffectAnimationCatalog(1,catalog);}catch(const std::invalid_argument&){failed=true;}require(failed);
}
static void word(std::ostream& out,std::uint32_t n){for(unsigned i=0;i<4;++i)out.put(char(n>>(8*i)));}
int main(int argc,char** argv){try{
    units();if(argc==1)return 0;if(argc!=3)throw std::invalid_argument("root output");
    auto storeResult=assets::AssetStore::create(argv[1]);require(std::holds_alternative<assets::AssetStore>(storeResult));
    const auto store=std::get<assets::AssetStore>(std::move(storeResult));
    auto catalog=assets::loadEffectAnimationCatalog(store);std::ofstream out(argv[2],std::ios::binary);require(bool(out));
    word(out,catalog.recipes.fileCount);word(out,std::uint32_t(catalog.recipes.entries.size()));
    std::vector<assets::Sprite> sprites;
    for(const auto& path:catalog.spritePaths){
        auto opened=store.open(path);require(std::holds_alternative<std::unique_ptr<assets::AssetFile>>(opened));
        auto loaded=assets::loadSprite(*std::get<std::unique_ptr<assets::AssetFile>>(opened));
        if(const auto* e=std::get_if<assets::SpriteError>(&loaded))throw std::runtime_error(path+": "+e->detail);
        sprites.push_back(std::get<assets::Sprite>(std::move(loaded)));
    }
    const auto entryCount=catalog.recipes.entries.size();unsigned states=0,displayed=0;
    for(std::uint32_t i=0;i<catalog.recipes.entries.size();++i){
        const auto& r=catalog.recipes.entries[i];word(out,r.sequence);word(out,r.assetIndex);word(out,r.printer);word(out,r.data1);
        const auto& ani=catalog.animations[r.assetIndex];
        for(auto j=ani.starts[r.sequence];j<ani.starts[r.sequence+1];++j)
            if(ani.records[j].opcode==0)require(ani.records[j].argument>=0 && std::uint32_t(ani.records[j].argument)<sprites[r.assetIndex].frames.size());
        auto b=reconstruction::bindEffectAnimationCatalog(i,catalog);require(b.animationOrdinal==i && b.entry.property==r.printer && b.entry.opaque==r.data1);
        for(unsigned tick=0;tick<=64;++tick){++states;if(b.player.sprite()){++displayed;require(*b.player.sprite()<sprites[r.assetIndex].frames.size());}if(tick!=64)b.player.tick();}
    }
    auto b=reconstruction::bindEffectAnimationCatalog(reconstruction::selectEffectAnimation(3,71,{},0xffffffffu,{}),catalog);
    const auto initial=b.player.displayedRecord();catalog={};require(b.player.displayedRecord().has_value()==initial.has_value());b.player.tick();
    require(bool(out));std::cout<<"{\"entries\":"<<entryCount<<",\"states\":"<<states<<",\"displayed\":"<<displayed<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
