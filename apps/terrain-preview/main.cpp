#include "scene.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QLabel>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
#include <algorithm>
namespace {
void writeNew(const QString& path,const QByteArray& bytes){QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly) || file.write(bytes)!=bytes.size() || !file.flush())throw std::runtime_error("Cannot write new preview output");}
}
int main(int argc,char** argv)try{
 QApplication app(argc,argv);QCommandLineParser p;p.addHelpOption();
 p.addOption({"root","Installed asset root","directory"});p.addOption({"realm","Realm directory","path","Realms/Celtic/Forest"});
 p.addOption({"definitions","1..9 comma-separated TTD IDs","ids","5,9,13,17,21,25,29,33,37"});
 p.addOption({"map","Installed MAP request; replaces explicit definitions","path"});
 p.addOption({"region","MAP slice x,y,layer,width,height (width/height 1..3)","coordinates","0,0,0,3,3"});
 p.addOption({"world","Produce an orientation-zero terrain scene from MAP"});
 p.addOption({"camera","World column,row,span,cut-level","fields"});p.addOption({"pan","World base screen origin x,y","pixels","256,64"});
 p.addOption({"view","Raw view 0..3","index","0"});p.addOption({"visibility","Apply recovered visibility pass"});
 p.addOption({"overlap","Use one anchor for every tile"});p.addOption({"output","New output prefix; writes .565, .png and .json, then exits","prefix"});p.process(app);
 bool ok=false;const auto view=p.value("view").toUInt(&ok);if(!ok || view>3 || !p.isSet("root"))throw std::runtime_error("Specify --root and view 0..3");
 auto configured=mnm::assets::AssetStore::create(p.value("root").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
 auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
 const auto open=[&](const char* name){auto result=store.open((p.value("realm")+"/"+name).toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&result))throw std::runtime_error(e->detail);return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(result));};
 auto ttd=open("Terrain.ttd"),spr=open("Terrain.spr");auto catalog=mnm::assets::loadTerrainCatalog(*ttd);auto sprite=mnm::assets::loadSprite(*spr);
 if(auto* e=std::get_if<mnm::assets::TerrainCatalogError>(&catalog))throw std::runtime_error(e->detail);
 if(auto* e=std::get_if<mnm::assets::SpriteError>(&sprite))throw std::runtime_error(e->detail);
 std::vector<mnm::preview::TerrainPreviewTile> tiles;
 QJsonObject mapInfo;mnm::reconstruction::TerrainCamera camera;
 const bool world=p.isSet("world");
 if((world && (!p.isSet("map") || p.isSet("region") || p.isSet("overlap"))) || (!world && (p.isSet("camera") || p.isSet("pan"))))throw std::runtime_error("--world requires --map; camera/pan require world; region/overlap require slice mode");
 if(p.isSet("map")){
  if(p.isSet("definitions"))throw std::runtime_error("Choose --map or --definitions");
  const auto values=p.value("region").split(',');if(values.size()!=5)throw std::runtime_error("Region requires x,y,layer,width,height");
  std::array<std::uint32_t,5> fields{};for(unsigned i=0;i<5;++i){fields[i]=values[i].toUInt(&ok);if(!ok)throw std::runtime_error("Region coordinates must be unsigned integers");}
  auto opened=store.open(p.value("map").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::loadMap(*file);if(auto* e=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);
  const auto map=std::get<mnm::assets::MapAsset>(std::move(loaded));
  if(world){
   camera.view=view;camera.column=map.width/2;camera.row=map.height/2;camera.span=std::min<std::uint32_t>(20,std::min(map.width,map.height));camera.cutLevel=map.layers;
   if(p.isSet("camera")){const auto values=p.value("camera").split(',');if(values.size()!=4)throw std::runtime_error("Camera requires column,row,span,cut-level");std::array<std::uint32_t,4> f{};for(unsigned i=0;i<4;++i){f[i]=values[i].toUInt(&ok);if(!ok || f[i]>128)throw std::runtime_error("Invalid camera field");}camera.column=f[0];camera.row=f[1];camera.span=f[2];camera.cutLevel=f[3];}
   const auto pan=p.value("pan").split(',');if(pan.size()!=2)throw std::runtime_error("Pan requires x,y");camera.f11=pan[0].toInt(&ok);if(!ok || camera.f11< -4096 || camera.f11>4096)throw std::runtime_error("Invalid horizontal pan");camera.f15=pan[1].toInt(&ok);if(!ok || camera.f15< -4096 || camera.f15>4096)throw std::runtime_error("Invalid vertical pan");
   camera.diagonal=camera.span/2;camera.f41=2*camera.diagonal;tiles=mnm::preview::worldTerrainTiles(map,camera);
   mapInfo={{"path",p.value("map")},{"width",qint64(map.width)},{"height",qint64(map.height)},{"layers",qint64(map.layers)},{"camera",QJsonArray{camera.column,camera.row,int(camera.span),int(camera.cutLevel)}},{"pan",QJsonArray{camera.f11,camera.f15}}};
  }else{
   tiles=mnm::preview::mapRegionTiles(map,{fields[0],fields[1],fields[2],fields[3],fields[4]},p.isSet("overlap"));
   mapInfo={{"path",p.value("map")},{"width",qint64(map.width)},{"height",qint64(map.height)},{"layers",qint64(map.layers)},{"region",p.value("region")}};
  }
 }else{
 if(p.isSet("region"))throw std::runtime_error("--region requires --map");
 for(const auto& value:p.value("definitions").split(',')){const auto id=value.toUInt(&ok);if(!ok || tiles.size()>=9)throw std::runtime_error("Expected 1..9 definition IDs");const int row=tiles.size()/3,col=tiles.size()%3;
  tiles.push_back({id,{row,col,0,p.isSet("overlap")?256:256+32*(col-row),p.isSet("overlap")?160:96+16*(col+row),0,0,0,0}});}
 }
 mnm::render::GlBlitter renderer;const auto result=mnm::preview::renderTerrain(renderer,std::get<mnm::assets::TerrainCatalog>(catalog),std::get<mnm::assets::Sprite>(sprite),tiles,{view,world?camera.cutLevel:1},p.isSet("visibility"),world);
 if(p.isSet("output")){
  QByteArray raw;for(auto word:result.pixels.pixels){raw.append(char(word&255));raw.append(char((word>>8)&255));}
  const auto prefix=p.value("output");writeNew(prefix+".565",raw);QFile png(prefix+".png");if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly) || !result.image.save(&png,"PNG"))throw std::runtime_error("Cannot save new PNG");
  QJsonArray queue;for(const auto& item:result.queue){const auto& d=item.draw;queue.append(QJsonObject{{"tile",qint64(item.tile)},{"frame",qint64(d.frame)},{"role",qint64(d.role)},{"key",d.key},{"kind",d.kind},{"x",d.anchorX},{"y",d.anchorY},{"shade",d.shade}});}
  const auto rgba=result.image.convertToFormat(QImage::Format_RGBA8888);const auto rgbaHash=QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(rgba.constBits()),rgba.sizeInBytes()),QCryptographicHash::Sha256).toHex();
  QJsonArray owners;for(const auto& owner:result.owners)owners.append(QJsonArray{owner.flags8,owner.flags10});
  QJsonArray tileInfo;for(const auto& tile:tiles){const auto& state=tile.state;QJsonObject input{{"definition",qint64(tile.definition)},{"column",state.column},{"row",state.row},{"level",state.level},{"flags8",state.flags8},{"flags10",state.flags10}};if(world){input.insert("cell",qint64(tile.cell));input.insert("priority",state.priority);input.insert("x",state.anchorX);input.insert("y",state.anchorY);}tileInfo.append(input);}
  writeNew(prefix+".json",QJsonDocument(QJsonObject{{"view",qint64(view)},{"visibility",p.isSet("visibility")},{"map",mapInfo},{"tiles",tileInfo},{"queue",queue},{"owners",owners},{"rgba_sha256",QString::fromLatin1(rgbaHash)},{"live_validated",false},{"world",world},{"remaining_surfaces",qint64(renderer.stats().surfaces)}}).toJson());return 0;
 }
 QLabel widget;widget.setWindowTitle("Native terrain preview");widget.setPixmap(QPixmap::fromImage(result.image));widget.resize(512,256);widget.show();return app.exec();
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
