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
        mnm::assets::Sprite directional;directional.storage=mnm::assets::SpriteStorage::rgb565;
        mnm::assets::Animation groups;
        for(unsigned f=0;f<32;++f){mnm::assets::SpriteFrame p;p.width=p.height=1;p.pixels=std::vector<std::uint16_t>{std::uint16_t(f+1)};p.opaqueMask={1};directional.frames.push_back(p);}
        for(unsigned s=0;s<16;++s){groups.starts.push_back(groups.records.size());groups.records.push_back({0,int(s*2),{}});groups.records.push_back({0,int(s*2+1),{}});groups.records.push_back({6,-1,{}});}
        groups.starts.push_back(groups.records.size());
        {
            mnm::preview::SpriteScene scene(renderer,std::move(directional),groups,{0},false);
            require(scene.directionalGroups()==std::vector<std::uint32_t>({0,8}),"Directional group validation differs");
            scene.advance();scene.advance();scene.selectFacing(0,7);
            require(scene.actors()[0].sprite==15,"Facing change restarted animation");
            scene.present();require(scene.read().pixels[190*512+256]==16,"Facing change rendered wrong SPR");
            scene.selectGroup(0,8,3);require(scene.actors()[0].sprite==22,"Group change did not restart");
            bool rejected=false;try{scene.selectFacing(0,8);}catch(const std::runtime_error&){rejected=true;}
            require(rejected && scene.actors()[0].sprite==22,"Invalid selection mutated actor");
        }
        require(renderer.stats().surfaces==0,"Directional scene leaked surfaces");
        auto makeSprite=[](unsigned colour,int ox,int oy){mnm::assets::Sprite result;result.storage=mnm::assets::SpriteStorage::rgb565;
            for(unsigned i=0;i<2;++i){mnm::assets::SpriteFrame f;f.width=f.height=1;f.originX=ox;f.originY=oy;f.pixels=std::vector<std::uint16_t>{std::uint16_t(colour?colour+i:0)};f.opaqueMask={1};result.frames.push_back(f);}return result;};
        auto makeAnimation=[](std::array<std::uint32_t,9> first,std::array<std::uint32_t,9> second){mnm::assets::Animation a;a.starts={0,3};a.records={{0,0,first},{0,1,second},{6,-1,{}}};return a;};
        const auto parent=makeAnimation({std::uint32_t(-3),std::uint32_t(-4),0,0,0,10,std::uint32_t(-20),30,std::uint32_t(-40)},
                                        {std::uint32_t(-5),std::uint32_t(-6),0,0,0,std::uint32_t(-10),std::uint32_t(-21),std::uint32_t(-30),std::uint32_t(-41)});
        const auto first=makeAnimation({5,std::uint32_t(-6),0,0,0,0,0,0,0},{7,std::uint32_t(-8),0,0,0,0,0,0,0});
        const auto second=makeAnimation({std::uint32_t(-9),10,0,0,0,0,0,0,0},{std::uint32_t(-11),12,0,0,0,0,0,0,0});
        for(unsigned view=0;view<4;++view){
            std::vector<mnm::preview::SpriteLayer> layers;
            layers.push_back({makeSprite(0,1,2),first,0,mnm::reconstruction::AttachmentPoint::first});
            layers.push_back({makeSprite(202,-1,-2),second,0,mnm::reconstruction::AttachmentPoint::second});
            mnm::preview::SpriteScene scene(renderer,makeSprite(11,3,4),parent,{0},false,{2,view},std::move(layers));
            const int shiftX=view==1?32:(view==3?-32:0),shiftY=view==2?-32:((view==1 || view==3)?-16:0);
            for(unsigned tick=0;tick<4;++tick){if(tick)scene.advance();scene.present();auto expected=mnm::preview::SpriteScene::background();
                const bool active=tick<3;const bool next=tick==2;
                if(active){const int bx=256+(next?-5:-3)+shiftX-3,by=190+(next?-6:-4)+shiftY-4;
                    const int lx=256+(next?-10:10)+shiftX+(next?7:5)-1,ly=190+(next?-21:-20)+shiftY+(next?-8:-6)-2;
                    const int rx=256+(next?-30:30)+shiftX+(next?-11:-9)+1,ry=190+(next?-41:-40)+shiftY+(next?12:10)+2;
                    expected.pixels[by*512+bx]=next?12:11;expected.pixels[ly*512+lx]=0;expected.pixels[ry*512+rx]=next?203:202;
                    const auto state=scene.layers();require(state.size()==2 && state[0].drawAnchor->x==lx+1 && state[1].drawAnchor->y==ry-2,"Layer anchors differ");
                }else require(!scene.layers()[0].sprite && !scene.layers()[1].sprite,"Stopped parent kept attachments visible");
                require(scene.read().pixels==expected.pixels,"ANI offsets, independent origins, layer asset cache or opaque zero differ");
            }
        }
        require(renderer.stats().surfaces==0,"Layered scenes leaked surfaces");
        std::cout<<"80 four-actor scenes match; 128 distinct uploads, cache eviction, restart and cleanup pass\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
