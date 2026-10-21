#include "palette_binding.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <thread>
using namespace mnm;
using namespace resource_test;
static assets::ProfileSnapshot profile(const std::string& text){return assets::ProfileSnapshot::parse(std::vector<std::uint8_t>(text.begin(),text.end()));}
static Bytes twoPalettes(){
 auto bytes=sprite(true);bytes.insert(bytes.begin()+24+768,768,0);word(bytes,4,bytes.size());word(bytes,16,2);
 bytes[24+768+2*3+2]=255;constexpr unsigned base=36+2*768;word(bytes,base+53+28,1);return bytes;
}
int main(int argc,char** argv)try{
 QGuiApplication application(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Scene palette fixture root");const auto root=std::filesystem::path(directory.path().toStdString());write(root,"indexed-two.spr",twoPalettes());write(root,"body.spr",sprite());
 const auto defaults=scene::paletteEffectProfile(profile(""));require(defaults.stoneReducer==.5&&defaults.metalSpecalValue==80&&defaults.redTintReducer==.25,"Original profile defaults");
 auto configured=scene::paletteEffectProfile(profile("[STONE_EFFECT]\nReducer= +.25tail\nSpecalValue=1\nDarkest=9\n[METAL_EFFECT]\nContrast=200\nBlueTint=-3\n[REDTINT_EFFECT]\nReducer=+.5suffix\n"));require(configured.stoneReducer==.25&&configured.stoneSpecalValue==1&&configured.stoneDarkest==9&&configured.metalContrast==200&&configured.metalBlueTint==-3&&configured.redTintReducer==.5,"Owned profile key/prefix decoding");
 require(scene::paletteEffectProfile(profile("[METAL_EFFECT]\nSpecalValue=not-a-number\n")).metalSpecalValue==0,"Original nonnumeric atof zero");
 for(const auto& value:{"nan","inf","-inf","1001","-1001","1e9999"})rejected([&]{scene::paletteEffectProfile(profile(std::string("[REDTINT_EFFECT]\nReducer=")+value+"\n"));});
 write(root,"effects.ini",Bytes{'[','S','T','O','N','E','_','E','F','F','E','C','T',']','\n','D','a','r','k','e','s','t','=','1','1','\n'});
 assets::ResourceManager resources(store(root));const assets::ResourceId indexed{assets::ResourceKind::effect,"palette-test"},second{assets::ResourceKind::effect,"palette-second"},direct{assets::ResourceKind::creature,"direct"};auto animated=spr("indexed-two.spr");animated.animationBytes=animation();auto directAnimated=spr();directAnimated.animationBytes=animation();resources.bind(indexed,animated);resources.bind(second,animated);resources.bind(direct,directAnimated);
 auto assetStore=store(root);auto opened=assetStore.open("effects.ini");require(std::holds_alternative<std::unique_ptr<assets::AssetFile>>(opened),"Plain effect profile open");auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto loaded=assets::loadProfile(*file);require(std::holds_alternative<assets::ProfileSnapshot>(loaded),"Plain effect profile load");require(scene::paletteEffectProfile(std::get<assets::ProfileSnapshot>(loaded)).stoneDarkest==11,"Plain effect profile production");
 scene::ScenePaletteBindings bindings(resources);rejected([&]{bindings.bind(indexed);});resources.load(indexed);resources.load(second);resources.load(direct);bindings.bind(indexed);bindings.bind(second);rejected([&]{bindings.bind(direct);});
 const auto& visual=resources.resident(indexed);const auto& spriteAsset=std::get<assets::Sprite>(visual.image);const auto sources=scene::paletteSources(spriteAsset);require(sources.size()==2&&sources[0][1][0]==255&&sources[1][2][2]==255,"Per-palette asset colour identities");reconstruction::PaletteEffectChain expected(sources);
 render::GlBlitter renderer;const render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};unsigned compared=0;
 {
  render::SceneRenderer sceneRenderer(renderer,resources,background);
  for(unsigned mode=0;mode<10;++mode){
   const auto ordinal=bindings.append(indexed,mode,{},configured);const auto target=expected.append(mode,{},configured);require(ordinal==target&&ordinal.has_value(),"Palette append ordinal binding");
   for(unsigned frame=0;frame<2;++frame){render::SceneDraw draw{indexed,frame,2,0};bindings.apply(draw,*ordinal,0);const auto colours=expected.colours(*ordinal,frame,0);require(draw.colours==colours,"Per-frame palette selection");const auto before=renderer.stats();sceneRenderer.draw({draw});const auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Palette draw imported/read back pixels");auto pixels=background.pixels;pixels[6]=colours[0];pixels[7]=colours[frame?2:1];require(sceneRenderer.read().pixels==pixels,"Native GPU palette values/coverage differ");++compared;}
  }
  render::SceneDraw invalid{indexed,99,2,0};invalid.colours=render::SpriteColourTable{};auto beforeColours=invalid.colours;rejected([&]{bindings.apply(invalid,0,0);});require(invalid.colours==beforeColours,"Bad frame partially changed draw");invalid.frame=0;rejected([&]{bindings.apply(invalid,999,0);});require(invalid.colours==beforeColours,"Bad ordinal partially changed draw");
  require(bindings.append(second,7)==0,"Independent resource palette ordinal");render::SceneDraw other{second,0,2,0};bindings.apply(other,0,0);require(other.colours.has_value(),"Heterogeneous resource colour binding");
  scene::PaletteBindingLimits smallLimits;smallLimits.bytes=8192;scene::ScenePaletteBindings small(resources,smallLimits);small.bind(indexed);const auto prior=small.retainedBytes();rejected([&]{small.append(indexed,5,{},configured);});require(small.retainedBytes()==prior,"Global palette budget partially published");require(small.append(indexed,0,{2,1,1,2,2})==0,"Budget recovery append");render::SceneDraw recovered{indexed,0,2,0};small.apply(recovered,0,0);require(recovered.colours->at(1)==0xf800,"Rejected metal append changed source RGB");
  bool rejectedThread=false;std::thread foreign([&]{try{bindings.clear();}catch(const std::exception&){rejectedThread=true;}});foreign.join();require(rejectedThread&&bindings.retainedBytes()>0,"Palette service owner-thread lifetime");
  const auto oldRevision=resources.resident(indexed).revision;resources.unload(indexed);rejected([&]{bindings.apply(invalid,0,0);});require(invalid.colours==beforeColours,"Unloaded palette changed draw");auto replacement=twoPalettes();replacement[24+3]=0;replacement[24+4]=255;write(root,"indexed-two.spr",replacement);resources.load(indexed);require(resources.resident(indexed).revision!=oldRevision,"Resource revision failed to advance");rejected([&]{bindings.apply(invalid,0,0);});rejected([&]{bindings.append(indexed,0);});bindings.bind(indexed);require(bindings.append(indexed,0,{2,1,1,2,2})==0,"Rebound palette append");render::SceneDraw fresh{indexed,0,2,0};bindings.apply(fresh,0,0);require(fresh.colours->at(1)==0x07e0,"Revision rebind retained old RGB");sceneRenderer.draw({fresh});auto pixels=background.pixels;pixels[6]=0;pixels[7]=0x07e0;require(sceneRenderer.read().pixels==pixels,"Native cache reused retired palette revision");++compared;
  bindings.retire(indexed);rejected([&]{bindings.apply(fresh,0,0);});bindings.retire(indexed);bindings.clear();require(!bindings.retainedBytes(),"Palette service retirement charge");
 }
 resources.unloadAll();const auto driver=renderer.driver();const QJsonObject result{{"success",true},{"gpu_frames",int(compared)},{"modes",10},{"profile_keys",9},{"revision_and_budget_controls_passed",true},{"driver",QJsonObject{{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)},{"version",QString::fromStdString(driver.version)}}}};std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
