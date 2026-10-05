#include "scene.hpp"
#include "terrain_camera.hpp"
#include "terrain_map.hpp"
#include "terrain_sections.hpp"
#include "region_loader.hpp"
#include "palette_lighting.hpp"
#include "terrain_lighting.hpp"
#include "light_fixture.hpp"
#include <sstream>
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
QString mapCellHash(const mnm::assets::MapAsset& map){QByteArray bytes;for(const auto& c:map.cells)for(auto v:std::array<std::uint16_t,6>{c.definition,c.references[0],c.references[1],c.references[2],c.flags8,c.flags10}){bytes.append(char(v&255));bytes.append(char(v>>8));}return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
}
int main(int argc,char** argv)try{
 QApplication app(argc,argv);QCommandLineParser p;p.addHelpOption();
 p.addOption({"root","Installed asset root","directory"});p.addOption({"realm","Realm directory","path","Realms/Celtic/Forest"});
 p.addOption({"definitions","1..9 comma-separated TTD IDs","ids","5,9,13,17,21,25,29,33,37"});
 p.addOption({"map","Installed MAP request; replaces explicit definitions","path"});
 p.addOption({"region-config","Realm CFG containing a region recipe","path"});
 p.addOption({"region-id","Region recipe numeric ID","id"});
 p.addOption({"generation-seed","Generate a region with an explicit unsigned 32-bit seed","seed"});
 p.addOption({"grid","Explicit complete section grid columns,rows,side","dimensions"});
 p.addOption({"section","MAP path,sourceX,sourceY,column,row,rotation; repeat for every grid slot","selection"});
 p.addOption({"region","MAP slice x,y,layer,width,height (width/height 1..3)","coordinates","0,0,0,3,3"});
 p.addOption({"world","Produce a four-orientation terrain scene from MAP"});
 p.addOption({"camera","World column,row,span,cut-level","fields"});p.addOption({"pan","World base screen origin x,y","pixels","256,64"});
 p.addOption({"initialize-terrain","Prepare ordinary terrain geometry; omit runtime entities"});
 p.addOption({"recovered-camera","Use recovered map binding, viewport and position setters"});
 p.addOption({"scroll","Recovered screen-direction scroll x,y; repeat in order","pixels"});
 p.addOption({"view","Raw view 0..3","index","0"});p.addOption({"visibility","Apply recovered visibility pass"});
 p.addOption({"palette-shading","Apply recovered mode 0 palettes with explicit count 16 and levels 1 and powers 2"});
 p.addOption({"preferences","Read terrain palette count from plain installed prefs CFG; requires palette-shading","path"});
 p.addOption({"lighting-config","Read global palette lighting from a packed installed CFG; requires palette-shading","path"});
 p.addOption({"terrain-lighting","Initialize an owned light field from global CFG and publish controlled source stamps; requires world/shading/lighting-config"});
 p.addOption({"light-source","Controlled column,row,layer,size (2..17); repeat in order; requires terrain-lighting","source"});
 p.addOption({"light-fixture","Local MNM_LIGHTING v1 object snapshots; requires terrain-lighting; excludes light-source","path"});
 p.addOption({"light-tick","Zero-based fixture tick to render (default last); requires light-fixture","index"});
 p.addOption({"light","Controlled uniform terrain light (-127..127); requires palette-shading","value","0"});
 p.addOption({"overlap","Use one anchor for every tile"});p.addOption({"output","New output prefix; writes .565, .png and .json, then exits","prefix"});p.process(app);
 bool ok=false;const auto view=p.value("view").toUInt(&ok);if(!ok || view>3 || !p.isSet("root"))throw std::runtime_error("Specify --root and view 0..3");
 auto configured=mnm::assets::AssetStore::create(p.value("root").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
 auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
 std::optional<std::uint32_t> generationSeed;
 if(p.isSet("generation-seed")){const auto seed=p.value("generation-seed").toUInt(&ok);if(!ok || !p.isSet("region-config"))throw std::runtime_error("Generation seed requires region config and an unsigned 32-bit integer");generationSeed=seed;}
 std::optional<mnm::assets::RegionRecipe> recipe;QString realm=p.value("realm");
 if(p.isSet("region-config")){
  if(!p.isSet("region-id") || !p.isSet("world") || !p.isSet("initialize-terrain") || p.isSet("map") || p.isSet("grid") || p.isSet("section") || p.isSet("realm"))throw std::runtime_error("Region config requires region-id, world and initialize-terrain; excludes map/grid/section/realm");
  const auto id=p.value("region-id").toUInt(&ok);if(!ok || id>99)throw std::runtime_error("Invalid region ID");
  recipe=mnm::preview::loadTerrainRecipe(store,p.value("region-config").toStdString(),id);realm=QString::fromStdString(recipe->spritePath);
 }else if(p.isSet("region-id"))throw std::runtime_error("Region ID requires region config");
 const auto open=[&](const char* name){auto result=store.open((realm+"/"+name).toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&result))throw std::runtime_error(e->detail);return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(result));};
 auto ttd=open("Terrain.ttd"),spr=open("Terrain.spr");auto catalog=mnm::assets::loadTerrainCatalog(*ttd);auto sprite=mnm::assets::loadSprite(*spr);
 if(auto* e=std::get_if<mnm::assets::TerrainCatalogError>(&catalog))throw std::runtime_error(e->detail);
 if(auto* e=std::get_if<mnm::assets::SpriteError>(&sprite))throw std::runtime_error(e->detail);
 std::vector<mnm::preview::TerrainPreviewTile> tiles;
 std::array<unsigned,3> lightShape{};
 QJsonObject mapInfo;mnm::reconstruction::TerrainCamera camera;
 const bool world=p.isSet("world");
 if((p.isSet("grid") && (!world || !p.isSet("initialize-terrain") || p.isSet("map"))) || (p.isSet("section") && !p.isSet("grid")))throw std::runtime_error("Grid requires world, initialize-terrain and sections; excludes map");
 if(p.isSet("initialize-terrain") && !world)throw std::runtime_error("Terrain initialization requires world mode");
 if((p.isSet("recovered-camera") && (!world || p.isSet("camera") || p.isSet("pan"))) || (p.isSet("scroll") && !p.isSet("recovered-camera")))throw std::runtime_error("Recovered camera requires world and excludes camera/pan; scroll requires recovered camera");
 if((world && ((!p.isSet("map") && !p.isSet("grid") && !recipe) || p.isSet("region") || p.isSet("overlap"))) || (!world && (p.isSet("camera") || p.isSet("pan"))))throw std::runtime_error("--world requires map, grid or region-config; camera/pan require world; region/overlap require slice mode");
 if(p.isSet("map") || p.isSet("grid") || recipe){
  if(p.isSet("definitions"))throw std::runtime_error("Choose map/grid/region-config or definitions");
  const auto values=p.value("region").split(',');if(values.size()!=5)throw std::runtime_error("Region requires x,y,layer,width,height");
  std::array<std::uint32_t,5> fields{};for(unsigned i=0;i<5;++i){fields[i]=values[i].toUInt(&ok);if(!ok)throw std::runtime_error("Region coordinates must be unsigned integers");}
  const auto load=[&](const QString& path){auto opened=store.open(path.toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::loadMap(*file);if(auto* e=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);return std::get<mnm::assets::MapAsset>(std::move(loaded));};
  mnm::assets::MapAsset map;QJsonObject assemblyInfo,recipeInfo;
  if(recipe){
   mnm::preview::LoadedFixedRegion loaded;QJsonObject generationInfo;
   if(generationSeed){
    auto generated=mnm::preview::loadGeneratedTerrainRegion(store,*recipe,std::get<mnm::assets::TerrainCatalog>(catalog),*generationSeed);
    const auto& state=generated.generated.generation;
    if(!generated.assembly)throw std::runtime_error("Region generation exhausted "+std::to_string(state.attempts.size())+" attempts; next seed "+std::to_string(state.nextSeed));
    QJsonArray attempts;for(const auto& attempt:state.attempts){QJsonArray notices;for(const auto& notice:attempt.diagnostics)notices.append(QJsonObject{{"descriptor",int(notice.descriptor)},{"notice",int(notice.notice)}});attempts.append(QJsonObject{{"seed",qint64(attempt.seed)},{"complete",attempt.complete},{"backtracks",int(attempt.backtracks)},{"multi_block_backtracks",int(attempt.multiBlockBacktracks)},{"notices",notices}});}
    generationInfo={{"seed",qint64(*generationSeed)},{"next_seed",qint64(state.nextSeed)},{"complete",state.complete},{"attempts",attempts}};
    loaded.assembly=std::move(*generated.assembly);loaded.plan=std::move(*generated.generated.plan);loaded.paths=std::move(generated.paths);
   }else loaded=mnm::preview::loadFixedTerrainRegion(store,*recipe,std::get<mnm::assets::TerrainCatalog>(catalog));
   QJsonArray blocks;for(const auto& b:loaded.plan.blocks)blocks.append(QJsonObject{{"path",QString::fromStdString(loaded.paths[b.source])},{"source_x",int(b.sourceX)},{"source_y",int(b.sourceY)},{"column",int(b.column)},{"row",int(b.row)},{"rotation",int(b.rotation)}});
   recipeInfo={{"config",p.value("region-config")},{"id",int(recipe->id)},{"name",QString::fromStdString(recipe->name)},{"sprite_path",realm},{"side",int(loaded.plan.side)},{"blocks",blocks},{"projected_source_objects",int(loaded.assembly.projectedObjects)},{"projected_source_references",int(loaded.assembly.projectedReferences)}};
   if(generationSeed)recipeInfo.insert("generation",generationInfo);
   map=std::move(loaded.assembly.map);
  }else if(p.isSet("grid")){
   const auto dimensions=p.value("grid").split(',');if(dimensions.size()!=3)throw std::runtime_error("Grid requires columns,rows,side");
   std::array<unsigned,3> grid{};for(unsigned i=0;i<3;++i){grid[i]=dimensions[i].toUInt(&ok);if(!ok || !grid[i] || grid[i]>128)throw std::runtime_error("Invalid grid field");}
   const auto selections=p.values("section");if(selections.size()>16384)throw std::runtime_error("Too many sections");
   std::vector<mnm::assets::MapAsset> sources;sources.reserve(selections.size());std::vector<mnm::reconstruction::TerrainSection> sections;
   qint64 projectedObjects=0,projectedReferences=0;std::size_t sourceCells=0;
   for(const auto& selection:selections){
    const auto parts=selection.split(',');if(parts.size()!=6)throw std::runtime_error("Section requires path,sourceX,sourceY,column,row,rotation");
    std::array<unsigned,5> f{};for(unsigned i=0;i<5;++i){f[i]=parts[i+1].toUInt(&ok);if(!ok || f[i]>128)throw std::runtime_error("Invalid section field");}
    sources.push_back(load(parts[0]));auto& source=sources.back();
    sourceCells+=source.cells.size();if(sourceCells>4u*128*128*32)throw std::runtime_error("Section source storage limit exceeded");
    // Explicit ordinary-terrain projection before rotation; derive geometry only
    // after assembling neighbors across section boundaries.
    for(auto& c:source.cells){projectedObjects+=bool(c.flags10&8);c.flags10&=0xfff7;for(auto& ref:c.references){projectedReferences+=ref!=0xffff;ref=0xffff;}}
    sections.push_back({&source,f[0],f[1],f[2],f[3],f[4]});
   }
   if(sources.empty())throw std::runtime_error("Grid requires sections");
   map=mnm::reconstruction::assembleTerrainRegion(grid[0],grid[1],grid[2],sources.front().layers,sections,std::get<mnm::assets::TerrainCatalog>(catalog));
   assemblyInfo={{"grid",p.value("grid")},{"sections",QJsonArray::fromStringList(selections)},{"projected_source_objects",projectedObjects},{"projected_source_references",projectedReferences}};
  }else map=load(p.value("map"));
  QJsonObject geometryInfo;
  if(p.isSet("initialize-terrain")){
   auto geometry=mnm::reconstruction::prepareTerrainGeometry(map,std::get<mnm::assets::TerrainCatalog>(catalog));
   geometryInfo={{"projected_objects",qint64(geometry.projectedObjects)},{"projected_references",qint64(geometry.projectedReferences)},{"changed_cells",qint64(geometry.changedCells)},{"removed_definitions",qint64(geometry.removedDefinitions)}};
   map=std::move(geometry.map);
  }
  lightShape={map.width,map.height,map.layers};
  if(world){
   camera.view=view;camera.column=map.width/2;camera.row=map.height/2;camera.span=std::min<std::uint32_t>(20,std::min(map.width,map.height));camera.cutLevel=map.layers;
   if(p.isSet("camera")){const auto values=p.value("camera").split(',');if(values.size()!=4)throw std::runtime_error("Camera requires column,row,span,cut-level");std::array<std::uint32_t,4> f{};for(unsigned i=0;i<4;++i){f[i]=values[i].toUInt(&ok);if(!ok || f[i]>128)throw std::runtime_error("Invalid camera field");}camera.column=f[0];camera.row=f[1];camera.span=f[2];camera.cutLevel=f[3];}
   const auto pan=p.value("pan").split(',');if(pan.size()!=2)throw std::runtime_error("Pan requires x,y");camera.f11=pan[0].toInt(&ok);if(!ok || camera.f11< -4096 || camera.f11>4096)throw std::runtime_error("Invalid horizontal pan");camera.f15=pan[1].toInt(&ok);if(!ok || camera.f15< -4096 || camera.f15>4096)throw std::runtime_error("Invalid vertical pan");
   camera.diagonal=camera.span/2;camera.f41=2*camera.diagonal;
   if(p.isSet("recovered-camera")){
    mnm::reconstruction::bindTerrainCamera(camera,map.width,map.height,map.layers);
    mnm::reconstruction::setTerrainCameraViewport(camera,{0,0,512,256});
    // Preview policy selects map center at height zero; setters are recovered.
    mnm::reconstruction::setTerrainCameraPosition(camera,map.width,map.height,map.layers,(map.width/2)*32,(map.height/2)*32,0);
    camera.view=view;
    for(const auto& value:p.values("scroll")){
     const auto pair=value.split(',');if(pair.size()!=2)throw std::runtime_error("Scroll requires x,y");
     const int dx=pair[0].toInt(&ok);if(!ok || dx< -4096 || dx>4096)throw std::runtime_error("Invalid horizontal scroll");
     const int dy=pair[1].toInt(&ok);if(!ok || dy< -4096 || dy>4096)throw std::runtime_error("Invalid vertical scroll");
     mnm::reconstruction::scrollTerrainCamera(camera,map.width,map.height,dx,dy);
    }
   }
   tiles=mnm::preview::worldTerrainTiles(map,camera);
   mapInfo={{"path",p.value("map")},{"width",qint64(map.width)},{"height",qint64(map.height)},{"layers",qint64(map.layers)},{"camera",QJsonArray{camera.column,camera.row,int(camera.span),int(camera.cutLevel)}},{"pan",QJsonArray{camera.f11,camera.f15}}};
   if(p.isSet("output"))mapInfo.insert("cells_sha256",mapCellHash(map));
   if(recipe)mapInfo.insert("recipe",recipeInfo);
   if(p.isSet("grid"))mapInfo.insert("assembly",assemblyInfo);
   if(p.isSet("initialize-terrain"))mapInfo.insert("terrain_geometry",geometryInfo);
   mapInfo.insert("recovered_camera",p.isSet("recovered-camera"));
   mapInfo.insert("scroll",QJsonArray::fromStringList(p.values("scroll")));
   mapInfo.insert("origin_fields",QJsonArray{camera.f41,camera.f45,camera.f49,camera.f4d,camera.f51,camera.f55});
  }else{
   tiles=mnm::preview::mapRegionTiles(map,{fields[0],fields[1],fields[2],fields[3],fields[4]},p.isSet("overlap"));
   mapInfo={{"path",p.value("map")},{"width",qint64(map.width)},{"height",qint64(map.height)},{"layers",qint64(map.layers)},{"region",p.value("region")}};
  }
 }else{
 if(p.isSet("region"))throw std::runtime_error("--region requires --map");
 for(const auto& value:p.value("definitions").split(',')){const auto id=value.toUInt(&ok);if(!ok || tiles.size()>=9)throw std::runtime_error("Expected 1..9 definition IDs");const int row=tiles.size()/3,col=tiles.size()%3;
  tiles.push_back({id,{row,col,0,p.isSet("overlap")?256:256+32*(col-row),p.isSet("overlap")?160:96+16*(col+row),0,0,0,0}});}
 }
 std::optional<mnm::reconstruction::PaletteShadingConfig> shading;
 if(p.isSet("palette-shading")){shading.emplace();const auto light=p.value("light").toInt(&ok);if(!ok || light< -127 || light>127)throw std::runtime_error("Light must lie in -127..127");for(auto& tile:tiles)tile.state.light=std::int8_t(light);}
 if(p.isSet("lighting-config")){
  if(!shading)throw std::runtime_error("Lighting config requires palette-shading");
  auto opened=store.open(p.value("lighting-config").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);
  auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::loadPaletteLightingFields(*file);if(auto* e=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);
  const auto& fields=std::get<mnm::assets::PaletteLightingFields>(loaded);*shading=mnm::reconstruction::applyPaletteLighting(*shading,{fields.lightCurve,fields.colourFactor,fields.lightPower,fields.colourPower});
 }
 if(p.isSet("preferences")){
  if(!shading)throw std::runtime_error("Preferences require palette-shading");
  auto opened=store.open(p.value("preferences").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);
  auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::loadTerrainPalettePreference(*file);if(auto* e=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);
  *shading=mnm::reconstruction::applyTerrainPalettePreference(*shading,std::get<mnm::assets::TerrainPalettePreference>(loaded).lightLevels);
  if(shading->count&(shading->count-1))throw std::runtime_error("TerrainLightLevels requires a supported power of two after original admission (2..256)");
 }
 if(!shading && p.isSet("light"))throw std::runtime_error("Light requires palette-shading");
 if((p.isSet("light-fixture") && (!p.isSet("terrain-lighting") || p.isSet("light-source"))) || (p.isSet("light-tick") && !p.isSet("light-fixture")))throw std::runtime_error("Light fixture requires terrain-lighting and excludes light-source; light-tick requires fixture");
 if(p.isSet("light-source") && !p.isSet("terrain-lighting"))throw std::runtime_error("Light source requires terrain-lighting");
 if(p.isSet("terrain-lighting")){
  if(!world || !shading || !p.isSet("lighting-config") || p.isSet("light"))throw std::runtime_error("Terrain lighting requires world, palette-shading and lighting-config; excludes uniform light");
  auto opened=store.open(p.value("lighting-config").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);
  auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::loadTerrainLightingFields(*file);if(auto* e=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(e->detail);
  const auto& fields=std::get<mnm::assets::TerrainLightingFields>(loaded);const auto config=mnm::reconstruction::applyTerrainLighting({},fields.ambientLight,fields.lightRamp);
  const auto hash=[](const auto& bytes){return QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(bytes.data()),bytes.size()),QCryptographicHash::Sha256).toHex());};
  if(p.isSet("light-fixture")){
   QFile file(p.value("light-fixture"));if(!file.open(QIODevice::ReadOnly) || file.size()>1024*1024)throw std::runtime_error("Cannot read bounded lighting fixture");
   const auto data=file.readAll();if(file.error()!=QFileDevice::NoError)throw std::runtime_error("Lighting fixture read failed");
   std::istringstream input(data.toStdString());const auto snapshots=mnm::preview::readLightingFixture(input);
   unsigned selected=unsigned(snapshots.size()-1);if(p.isSet("light-tick")){selected=p.value("light-tick").toUInt(&ok);if(!ok || selected>=snapshots.size())throw std::runtime_error("Light tick outside fixture");}
   mnm::reconstruction::TerrainLightingCycle cycle(lightShape[0],lightShape[1],lightShape[2],config);QJsonArray trace;
   for(unsigned i=0;i<=selected;++i){cycle.step(snapshots[i]);const auto& c=cycle.creatures();const auto& o=cycle.objects();QJsonArray hashes;for(const auto& buffer:cycle.field().buffers())hashes.append(hash(buffer));
    trace.append(QJsonObject{{"tick",qint64(i)},{"changed_input",snapshots[i].changed},{"changed_after",0},{"creature_phase",int(c.phase)},{"creature_remaining",qint64(c.remaining)},{"creature_quota",qint64(c.quota)},{"creature_cursor",int(c.cursor)},{"published_flag",c.published},{"object_phase",int(o.phase)},{"object_remaining",qint64(o.remaining)},{"object_quota",qint64(o.quota)},{"object_cursor",int(o.cursor)},{"object_active",o.active},{"object_requested",o.requested},{"buffer_sha256",hashes}});
   }
   const auto& field=cycle.field();for(auto& tile:tiles)tile.state.light=field.at(unsigned(tile.state.column),unsigned(tile.state.row),unsigned(tile.state.level));
   const auto& bytes=field.buffers()[0];mapInfo.insert("terrain_light_field",QJsonObject{{"ambient",config.ambient},{"ramp",config.ramp},{"bytes",qint64(bytes.size())},{"sha256",hash(bytes)},{"publication","recovered creature then static cycle"},{"fixture",p.value("light-fixture")},{"fixture_sha256",QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex())},{"selected_tick",int(selected)},{"fixture_ticks",int(snapshots.size())},{"trace",trace}});
  }else{
  mnm::reconstruction::TerrainLightField field(lightShape[0],lightShape[1],lightShape[2],config);
  for(const auto& request:p.values("light-source")){const auto values=request.split(',');if(values.size()!=4)throw std::runtime_error("Light source requires column,row,layer,size");std::array<unsigned,4> source{};for(unsigned i=0;i<4;++i){source[i]=values[int(i)].toUInt(&ok);if(!ok)throw std::runtime_error("Invalid light source integer");}field.stamp({source[0],source[1],source[2],source[3]});}
  field.publish();for(auto& tile:tiles)tile.state.light=field.at(unsigned(tile.state.column),unsigned(tile.state.row),unsigned(tile.state.level));
  const auto& bytes=field.buffers()[0];mapInfo.insert("terrain_light_field",QJsonObject{{"ambient",config.ambient},{"ramp",config.ramp},{"bytes",qint64(bytes.size())},{"sha256",QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(bytes.data()),bytes.size()),QCryptographicHash::Sha256).toHex())},{"sources",QJsonArray::fromStringList(p.values("light-source"))},{"publication","immediate controlled fixture"}});
  }
 }
 mnm::render::GlBlitter renderer;const auto result=mnm::preview::renderTerrain(renderer,std::get<mnm::assets::TerrainCatalog>(catalog),std::get<mnm::assets::Sprite>(sprite),tiles,{view,world?camera.cutLevel:1},p.isSet("visibility"),world,shading);
 if(p.isSet("output")){
  QByteArray raw;for(auto word:result.pixels.pixels){raw.append(char(word&255));raw.append(char((word>>8)&255));}
  const auto prefix=p.value("output");writeNew(prefix+".565",raw);QFile png(prefix+".png");if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly) || !result.image.save(&png,"PNG"))throw std::runtime_error("Cannot save new PNG");
  mapInfo.insert("palette_shading",bool(shading));if(shading){mapInfo.insert("palette_count",int(shading->count));mapInfo.insert("preferences",p.value("preferences"));if(!p.isSet("terrain-lighting"))mapInfo.insert("controlled_light",p.value("light").toInt());mapInfo.insert("palette_lighting",QJsonObject{{"light_curve",shading->intensityLevel},{"colour_factor",shading->saturationLevel},{"light_power",shading->intensityPower},{"colour_power",shading->saturationPower},{"config",p.value("lighting-config")}});}
  QJsonArray queue;for(const auto& item:result.queue){const auto& d=item.draw;queue.append(QJsonObject{{"tile",qint64(item.tile)},{"frame",qint64(d.frame)},{"role",qint64(d.role)},{"key",d.key},{"kind",d.kind},{"x",d.anchorX},{"y",d.anchorY},{"shade",d.shade}});}
  const auto rgba=result.image.convertToFormat(QImage::Format_RGBA8888);const auto rgbaHash=QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(rgba.constBits()),rgba.sizeInBytes()),QCryptographicHash::Sha256).toHex();
  QJsonArray owners;for(const auto& owner:result.owners)owners.append(QJsonArray{owner.flags8,owner.flags10});
  QJsonArray tileInfo;for(const auto& tile:tiles){const auto& state=tile.state;QJsonObject input{{"definition",qint64(tile.definition)},{"column",state.column},{"row",state.row},{"level",state.level},{"flags8",state.flags8},{"flags10",state.flags10}};if(p.isSet("terrain-lighting"))input.insert("light",state.light);if(world){input.insert("cell",qint64(tile.cell));input.insert("priority",state.priority);input.insert("x",state.anchorX);input.insert("y",state.anchorY);}tileInfo.append(input);}
  writeNew(prefix+".json",QJsonDocument(QJsonObject{{"view",qint64(view)},{"visibility",p.isSet("visibility")},{"map",mapInfo},{"tiles",tileInfo},{"queue",queue},{"owners",owners},{"rgba_sha256",QString::fromLatin1(rgbaHash)},{"live_validated",false},{"world",world},{"remaining_surfaces",qint64(renderer.stats().surfaces)}}).toJson());return 0;
 }
 QLabel widget;widget.setWindowTitle("Native terrain preview");widget.setPixmap(QPixmap::fromImage(result.image));widget.resize(512,256);widget.show();return app.exec();
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
