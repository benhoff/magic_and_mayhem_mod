#include "scene.hpp"
#include "scene-resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
template<class F> static void refused(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}require(failed,"Scene accepted a retired decoded resource generation");}
static assets::Sprite sprite(){assets::Sprite s;s.storage=assets::SpriteStorage::rgb565;assets::SpriteFrame f;f.width=f.height=1;f.pixels=std::vector<std::uint16_t>{0x1234};f.opaqueMask={1};s.frames.push_back(f);return s;}
static assets::Animation animation(){assets::Animation a;a.starts={0,3};a.records={{0,0,{}},{0,0,{}},{6,-1,{}}};return a;}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);render::GlBlitter renderer;
 {
  test::SceneResources resources;const auto id=resources.add("body",sprite(),animation());
  preview::SpriteScene scene(renderer,resources.manager,id,{0},false);scene.present();const auto image=scene.read().pixels;
  const auto before=scene.actors()[0].event;
  resources.manager.unload(id);
  refused([&]{(void)scene.sprite();});refused([&]{(void)scene.directionalGroups();});refused([&]{scene.advance();});refused([&]{scene.present();});
  require(scene.read().pixels==image && scene.actors()[0].event==before,"Rejected retired body changed phase or GPU output");
  resources.manager.load(id);
  refused([&]{(void)scene.sprite();});refused([&]{scene.selectGroup(0,0,0);});refused([&]{scene.selectFacing(0,0);});refused([&]{scene.present();});
  require(scene.read().pixels==image,"Reloaded generation silently replaced a scene binding");
 }
 require(renderer.stats().surfaces==0,"Retired body leaked scene storage");
 {
  test::SceneResources resources;const auto body=resources.add("body",sprite(),animation());const auto child=resources.add("child",sprite(),animation());
  preview::SpriteScene scene(renderer,resources.manager,body,{0},false,{},{{child,0,reconstruction::AttachmentPoint::first,true}});scene.present();const auto image=scene.read().pixels;
  resources.manager.unload(child);refused([&]{scene.advance();});refused([&]{(void)scene.drawQueue();});refused([&]{scene.present();});
  require(scene.read().pixels==image,"Retired layer changed accepted output");
  resources.manager.load(child);refused([&]{(void)scene.drawQueue();});refused([&]{scene.present();});
  resources.manager.unloadAll();refused([&]{(void)scene.sprite();});
 }
 require(renderer.stats().surfaces==0 && !renderer.stats().pixels,"Retired scene resources leaked GPU storage");
 std::cout<<"Retired/reloaded body and layer generations refuse before phase/GPU changes; cleanup passes\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
