#include "sprite.hpp"
#include <QColor>
#include <QGuiApplication>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

using namespace mnm;
using namespace mnm::render;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F> static void rejected(F action){bool fail=false;try{action();}catch(const std::runtime_error&){fail=true;}require(fail,"Invalid sprite operation accepted");}
static void checkColors(const QImage& actual,const Image& expected){
    for(int y=0;y<expected.height;++y)for(int x=0;x<expected.width;++x){
        const auto p=expected.pixels[std::size_t(y)*expected.width+x];
        require(actual.pixelColor(x,y)==QColor((((p>>11)&31)<<3)|(p>>13),(((p>>5)&63)<<2)|((p>>9)&3),((p&31)<<3)|((p>>2)&7)),"Sprite presentation differs from RGB565 reference");
    }
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try{
        GlBlitter renderer;std::mt19937 random(0x535052);unsigned draws=0;
        for(bool indexed:{true,false})for(unsigned sample=0;sample<32;++sample){
            assets::Sprite sprite;sprite.storage=indexed?assets::SpriteStorage::indexed8:assets::SpriteStorage::rgb565;
            if(indexed){
                sprite.palettes.resize(4);
                for(auto& palette:sprite.palettes){
                    for(auto& color:palette)color={std::uint8_t(random()),std::uint8_t(random()),std::uint8_t(random())};
                    palette[0]={0,0,0};
                }
            }
            assets::SpriteFrame f;f.width=7;f.height=5;f.originX=-11;f.originY=13;
            f.opaqueMask.resize(35);
            std::vector<std::uint8_t> indices(35);std::vector<std::uint16_t> words(35);
            Image expected{11,9,std::vector<std::uint32_t>(99,0x1234)};
            for(std::size_t i=0;i<35;++i){
                f.opaqueMask[i]=std::uint8_t(random()%2);indices[i]=std::uint8_t(random());words[i]=std::uint16_t(random());
            }
            f.opaqueMask[0]=1;indices[0]=0;words[0]=0; // Opaque zero must overwrite background.
            f.opaqueMask[1]=0;indices[1]=0;words[1]=0;
            const unsigned palette=sample%4;SpriteColourTable colours{};
            const bool override=indexed && sample%2;for(auto& word:colours)word=std::uint16_t(random());
            for(std::size_t i=0;i<35;++i)if(f.opaqueMask[i]){
                auto pixel=std::uint32_t(words[i]);
                if(indexed){const auto c=sprite.palettes[palette][indices[i]];
                    pixel=(std::uint32_t(c.red>>3)<<11)|(std::uint32_t(c.green>>2)<<5)|(c.blue>>3);}
                if(override)pixel=colours[indices[i]];
                expected.pixels[(i/7+2)*11+i%7+2]=pixel;
            }
            if(indexed){f.paletteIndex=palette;f.pixels=indices;}else f.pixels=words;
            sprite.frames.push_back(f);
            const auto destination=renderer.create({11,9,std::vector<std::uint32_t>(99,0x1234)},spriteFormat);
            {
                UploadedSpriteFrame uploaded(renderer,sprite,0,override?&colours:nullptr);sprite={};colours.fill(0);
                const auto before=renderer.stats();uploaded.draw(destination,-9,15);const auto after=renderer.stats();
                require(after.uploads==before.uploads && after.nativeReadbacks==before.nativeReadbacks && after.copies==before.copies+1,"Draw did not retain GPU textures");
                require(renderer.read(destination).pixels==expected.pixels,"Sprite mask/origin draw differs from CPU reference");
                checkColors(renderer.present(destination),expected);
                rejected([&]{uploaded.draw(destination,-12,15);});
                rejected([&]{uploaded.draw(destination,std::numeric_limits<int>::max(),15);});
                rejected([&]{uploaded.draw(destination,-9,19);});
                require(renderer.read(destination).pixels==expected.pixels,"Rejected sprite placement modified output");
            }
            require(renderer.stats().surfaces==1,"Uploaded sprite surfaces leaked");
            renderer.destroy(destination);++draws;
        }
        assets::Sprite empty;empty.storage=assets::SpriteStorage::rgb565;
        assets::SpriteFrame f;f.pixels=std::vector<std::uint16_t>{};empty.frames.push_back(f);
        {UploadedSpriteFrame uploaded(renderer,empty,0);require(uploaded.empty(),"Empty frame changed");uploaded.draw(0,INT32_MIN,INT32_MAX);}
        require(renderer.stats().surfaces==0,"Empty frame allocated surfaces");
        rejected([&]{UploadedSpriteFrame uploaded(renderer,empty,1);});
        empty.frames[0].width=1;rejected([&]{UploadedSpriteFrame uploaded(renderer,empty,0);});
        assets::Sprite direct;direct.storage=assets::SpriteStorage::rgb565;
        f.width=2;f.height=1;f.pixels=std::vector<std::uint16_t>{0,0xffff};f.opaqueMask={1,0};direct.frames={f};
        direct.frames[0].opaqueMask[0]=2;rejected([&]{UploadedSpriteFrame uploaded(renderer,direct,0);});
        direct.frames[0]=f;direct.frames[0].paletteIndex=0;rejected([&]{UploadedSpriteFrame uploaded(renderer,direct,0);});
        direct.frames[0]=f;direct.frames[0].originX=INT32_MIN;
        {UploadedSpriteFrame uploaded(renderer,direct,0);rejected([&]{uploaded.draw(0,INT32_MAX,0);});}
        direct.frames[0]=f;SpriteColourTable override{};bool refused=false;try{UploadedSpriteFrame uploaded(renderer,direct,0,&override);}catch(const std::invalid_argument&){refused=true;}require(refused,"Direct pixels accepted a palette override");
        // Failure while allocating the second surface must release the first.
        std::vector<SurfaceId> handles;for(unsigned i=0;i<63;++i)handles.push_back(renderer.create({1,1,{0}},{8,{}}));
        rejected([&]{UploadedSpriteFrame uploaded(renderer,direct,0);});
        require(renderer.stats().surfaces==63,"Failed upload leaked its first texture");
        for(auto id:handles)renderer.destroy(id);
        // Mask coordinate crops, key conjunction, and reset on an ordinary copy.
        const auto source=renderer.create({3,2,{1,0,3,4,5,6}},spriteFormat);
        const auto mask=renderer.create({3,2,{0,1,1,1,0,1}},{8,{}});
        const auto dest=renderer.create({2,2,{9,9,9,9}},spriteFormat);
        renderer.copy(source,dest,{1,0,3,2},0,0,std::nullopt,mask);
        require(renderer.read(dest).pixels==std::vector<std::uint32_t>{0,3,9,6},"Cropped mask used destination coordinates");
        renderer.update(dest,0,0,{2,2,{9,9,9,9}});
        renderer.copy(source,dest,{1,0,3,2},0,0,3,mask);
        require(renderer.read(dest).pixels==std::vector<std::uint32_t>{0,9,9,6},"Key/mask conjunction differs");
        renderer.update(dest,0,0,{2,2,{9,9,9,9}});
        renderer.copy(source,dest,{1,0,3,2},0,0);
        require(renderer.read(dest).pixels==std::vector<std::uint32_t>{0,3,5,6},"Mask state leaked into regular copy");
        rejected([&]{renderer.copy(source,dest,{1,0,3,2},0,0,std::nullopt,dest);});
        const auto small=renderer.create({1,1,{1}},{8,{}});
        rejected([&]{renderer.copy(source,dest,{1,0,3,2},0,0,std::nullopt,small);});
        renderer.destroy(small);renderer.destroy(mask);
        rejected([&]{renderer.copy(source,dest,{1,0,3,2},0,0,std::nullopt,mask);});
        renderer.destroy(source);renderer.destroy(dest);
        require(renderer.stats().surfaces==0 && renderer.stats().pixels==0,"Sprite test resources leaked");
        std::cout<<draws<<" sprite mask/origin draws and RGB565 presentations match; ownership, limits and lifetimes pass\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
