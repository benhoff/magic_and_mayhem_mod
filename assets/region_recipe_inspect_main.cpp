#include "region_recipe.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);if(argc!=3)throw std::runtime_error("Usage: mnm-region-recipe-inspect ROOT CFG_PATH");
 auto created=AssetStore::create(argv[1]);if(auto* e=std::get_if<Error>(&created))throw std::runtime_error(e->detail);
 auto opened=std::get<AssetStore>(created).open(argv[2]);if(auto* e=std::get_if<Error>(&opened))throw std::runtime_error(e->detail);
 auto decoded=loadRegionRecipes(*std::get<std::unique_ptr<AssetFile>>(opened));if(auto* e=std::get_if<PersistenceError>(&decoded))throw std::runtime_error(e->detail);
 QJsonArray regions;for(const auto& r:std::get<std::vector<RegionRecipe>>(decoded)){
  QJsonArray specific,random;for(const auto& s:r.specific)specific.append(QJsonArray{int(s.section),s.rotation,s.column,s.row});for(const auto& s:r.random)random.append(QJsonArray{int(s.section),int(s.occurrences)});
  regions.append(QJsonObject{{"id",int(r.id)},{"name",QString::fromStdString(r.name)},{"path",QString::fromStdString(r.path)},{"sprite_path",QString::fromStdString(r.spritePath)},{"prefix",QString::fromStdString(r.prefix)},{"columns",int(r.columns)},{"rows",int(r.rows)},{"specific",specific},{"random",random}});
 }
 std::cout<<QJsonDocument(regions).toJson(QJsonDocument::Compact).constData()<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
