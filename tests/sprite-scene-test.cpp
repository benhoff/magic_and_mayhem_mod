#include "scene.hpp"
#include <QGuiApplication>
#include <QColor>
#include <iostream>
#include <stdexcept>
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try{
        mnm::render::GlBlitter renderer;mnm::assets::Sprite sprite;sprite.storage=mnm::assets::SpriteStorage::rgb565;
        mnm::assets::Animation animation;
        for(unsigned i=0;i<128;++i){mnm::assets::SpriteFrame f;f.width=f.height=1;f.pixels=std::vector<std::uint16_t>{std::uint16_t(i+1)};f.opaqueMask={1};sprite.frames.push_back(f);}
        for(unsigned group=0;group<4;++group){animation.starts.push_back(animation.records.size());
            for(unsigned f=0;f<32;++f)animation.records.push_back({0,int(group*32+f),{}});
            animation.records.push_back({6,-1,{}});}
        animation.starts.push_back(animation.records.size());
        {
            mnm::preview::SpriteScene scene(renderer,std::move(sprite),animation,{0,1,2,3},true);
            for(unsigned tick=0;tick<80;++tick){
                if(tick){scene.advance();}
                const auto image=scene.present();auto expected=mnm::preview::SpriteScene::background();
                const auto phase=tick%34;const bool active=phase!=33;const auto frame=phase?phase-1:0;
                const auto actors=scene.actors();
                for(unsigned g=0;g<4;++g){const auto id=g*32+frame;const auto x=int((g+1)*512/5);
                    require(actors[g].sprite==(active?std::optional<std::uint32_t>(id):std::nullopt),"Scene restart/selection policy differs");
                    if(active){expected.pixels[190*512+x]=id+1;const auto p=id+1;
                        require(image.pixelColor(x,190)==QColor(((p>>11)&31)*255/31,((p>>5)&63)*255/63,(p&31)*255/31),"Scene presentation marker differs");}
                }
                require(scene.read().pixels==expected.pixels,"Scene retained trails or composed wrong markers");
                require(renderer.stats().surfaces<=50,"Scene cache exceeded 24 owned uploads");
            }
            require(renderer.stats().uploads>128*2,"Cache eviction path was not exercised");
        }
        require(renderer.stats().surfaces==0 && renderer.stats().pixels==0,"Scene textures leaked");
        std::cout<<"80 four-actor scenes match; 128 distinct uploads, cache eviction, restart and cleanup pass\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
