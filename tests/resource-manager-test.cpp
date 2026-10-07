#include "resource-fixtures.hpp"
#include <QCoreApplication>
#include <iostream>
#include <thread>
using namespace mnm::assets;
using namespace resource_test;
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root failed");
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    ResourceManager resources(store(root));
    const ResourceId creature{ResourceKind::creature,"10"},effect{ResourceKind::effect,"36"},terrainId{ResourceKind::terrain,"celtic/forest"},ui{ResourceKind::ui,"menu/title"};
    auto body=spr();body.animation="body.ani";auto effectRecipe=body;effectRecipe.sequence=0;
    auto tiles=spr();tiles.terrainCatalog="tiles.ttd";
    resources.bind(creature,body);resources.bind(effect,effectRecipe);resources.bind(terrainId,tiles);resources.bind(ui,spr());
    require(resources.stats().bindings==4 && resources.stats().loads==0,"Binding eagerly loaded resources");
    resources.bind(creature,body);require(resources.stats().bindings==4,"Idempotent binding duplicated an ID");
    auto conflict=body;conflict.image="indexed.spr";rejected([&]{resources.bind(creature,conflict);});
    require(resources.recipe(creature).image=="body.spr","Conflicting recipe mutated binding");
    ResourceManager reversed(store(root));reversed.bind(ui,spr());reversed.bind(creature,body);
    require(creature.text()=="creature:10" && terrainId.text()=="terrain:celtic/forest" &&
            reversed.recipe(creature)==resources.recipe(creature),"Identity depends on binding order");
    for(const auto* name:{"","Upper","a//b","/a","a/","../a","a.b"})rejected([&]{resources.bind({ResourceKind::ui,name},spr());});
    rejected([&]{resources.bind({static_cast<ResourceKind>(100),"x"},spr());});
    rejected([&]{resources.bind({ResourceKind::ui,"x"},{static_cast<ResourceImageFormat>(100),"body.spr",{},{},{}});});
    rejected([&]{resources.bind({ResourceKind::creature,"x"},spr());});
    rejected([&]{resources.bind({ResourceKind::terrain,"x"},spr());});
    rejected([&]{resources.load({ResourceKind::ui,"unknown"});});
    auto sequence=spr();sequence.sequence=0;rejected([&]{resources.bind({ResourceKind::ui,"sequence"},sequence);});
    auto invalid=body;invalid.image="";rejected([&]{resources.bind(creature,invalid);});
    const auto& loaded=resources.load(creature);const auto revision=loaded.revision;
    require(loaded.animation && loaded.frameCount()==3,"Creature ANI/SPR recipe not loaded");
    require(&resources.load(creature)==&loaded && resources.stats().loads==1 && resources.stats().hits==1,"Decoded cache miss");
    require(std::get<Sprite>(loaded.image).frames[0].opaqueMask==Bytes({1,1,0}),"Owned SPR mask differs");
    const auto bytes=loaded.decodedBytes;
    std::filesystem::remove(root/"body.spr");std::filesystem::remove(root/"body.ani");
    require(resources.load(creature).animation->records[0].argument==0,"Decoded data retained a file dependency");
    rejected([&]{resources.load(effect);});
    require(resources.stats().loads==1 && resources.stats().residentResources==1 && resources.stats().decodedBytes==bytes,"Partial recipe published after failure");
    write(root,"body.spr",sprite());write(root,"body.ani",animation());
    resources.unload(creature);require(resources.stats().residentResources==0 && resources.stats().decodedBytes==0,"Decoded retirement leaked storage");
    resources.unload(creature);require(resources.load(creature).revision>revision,"Retirement reused a revision");
    require(resources.load(terrainId).terrainCatalog->records[0][4]==17,"Terrain binding lost TTD");
    require(resources.load(effect).animation.has_value(),"Effect binding failed");
    const std::pair<ResourceImageFormat,const char*> images[]={{ResourceImageFormat::bmp,"ui.bmp"},{ResourceImageFormat::pcx,"ui.pcx"},{ResourceImageFormat::jpeg,"ui.jpg"}};
    for(const auto& image:images){const ResourceId id{ResourceKind::ui,image.second==std::string("ui.bmp")?"bmp":image.second==std::string("ui.pcx")?"pcx":"jpeg"};
        resources.bind(id,{image.first,image.second,{},{},{}});require(resources.load(id).frameCount()==1,"UI bitmap not loaded");}
    resources.bind({ResourceKind::ui,"missing"},spr("missing.spr"));rejected([&]{resources.load({ResourceKind::ui,"missing"});});
    resources.bind({ResourceKind::ui,"escape"},spr("../escape.spr"));rejected([&]{resources.load({ResourceKind::ui,"escape"});});
    auto badPair=body;badPair.animation="bad.ani";write(root,"bad.ani",animation(3));
    resources.bind({ResourceKind::creature,"bad"},badPair);rejected([&]{resources.load({ResourceKind::creature,"bad"});});
    badPair.sequence=1;badPair.animation="body.ani";resources.bind({ResourceKind::effect,"bad-sequence"},badPair);
    rejected([&]{resources.load({ResourceKind::effect,"bad-sequence"});});
    ResourceLimits limits;limits.bindings=1;limits.residentResources=1;ResourceManager bounded(store(root),limits);
    bounded.bind(ui,spr());rejected([&]{bounded.bind(creature,body);});bounded.load(ui);
    limits.bindings=2;ResourceManager countBounded(store(root),limits);countBounded.bind(ui,spr());countBounded.bind(creature,body);
    countBounded.load(ui);rejected([&]{countBounded.load(creature);});countBounded.unload(ui);countBounded.load(creature);
    limits={};limits.decodedBytes=bytes-1;ResourceManager tooSmall(store(root),limits);tooSmall.bind(creature,body);
    rejected([&]{tooSmall.load(creature);});require(tooSmall.stats().loads==0 && !tooSmall.stats().decodedBytes,"Byte failure published partial state");
    limits.decodedBytes=bytes;ResourceManager exact(store(root),limits);exact.bind(creature,body);exact.load(creature);
    require(exact.stats().decodedBytes==bytes,"Exact decoded capacity refused");
    limits={};limits.sprite.inputBytes=1;ResourceManager decodeBounded(store(root),limits);decodeBounded.bind(ui,spr());rejected([&]{decodeBounded.load(ui);});
    bool crossThread=false;std::thread worker([&]{try{resources.stats();}catch(const std::exception&){crossThread=true;}});worker.join();
    require(crossThread,"Manager accepted cross-thread access");
    // A checkpoint supplies ANI bytes without an installed ANI path. The recipe
    // owns them across caller mutation and decoded unload/reload.
    ResourceManager checkpoint(store(root));auto owned=spr();owned.animationBytes=animation();
    checkpoint.bind(creature,owned);const auto encodedBytes=checkpoint.stats().recipeBytes;
    owned.animationBytes->assign(1,0);require(checkpoint.load(creature).animation->records[0].argument==0,"Recipe borrowed checkpoint ANI input");
    checkpoint.unloadAll();require(checkpoint.stats().recipeBytes==encodedBytes && checkpoint.load(creature).animation.has_value(),"Decoded retirement lost owned ANI recipe");
    auto conflicting=spr();conflicting.animation="body.ani";conflicting.animationBytes=animation();
    rejected([&]{checkpoint.bind(effect,conflicting);});
    ResourceLimits recipeLimits;recipeLimits.recipeBytes=animation().size()-1;
    ResourceManager recipeBounded(store(root),recipeLimits);owned.animationBytes=animation();
    rejected([&]{recipeBounded.bind(creature,owned);});require(recipeBounded.stats().bindings==0 && recipeBounded.stats().recipeBytes==0,"Failed recipe budget published state");
    recipeLimits.recipeBytes=animation().size();ResourceManager recipeExact(store(root),recipeLimits);
    recipeExact.bind(creature,owned);recipeExact.bind(creature,owned);
    require(recipeExact.stats().recipeBytes==animation().size(),"Owned recipe charged twice");
    rejected([&]{recipeExact.bind(effect,owned);});
    resources.unloadAll();require(resources.stats().bindings==11 && !resources.stats().residentResources && !resources.stats().decodedBytes,"UnloadAll lost bindings or retained decoded storage");
    std::cout<<"Stable bindings, complete owned SPR/ANI/TTD/BMP/PCX/JPEG loads, retirement, budgets and failure atomicity pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
