#include "scene.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool ok){if(!ok)throw std::runtime_error("Queue scene/original oracle mismatch");}
static assets::Sprite sprite(unsigned asset){
    assets::Sprite s;s.storage=assets::SpriteStorage::rgb565;
    for(unsigned frame=0;frame<(asset?1:4);++frame){assets::SpriteFrame f;f.width=f.height=3;
        f.originX=asset==0?2:asset==1?1:-1;f.originY=asset==0?1:asset==1?0:1;
        std::vector<std::uint16_t> pixels;for(unsigned p=0;p<9;++p)pixels.push_back(p==4?0:std::uint16_t(1000*(asset+1)+100*frame+p));
        f.pixels=pixels;f.opaqueMask={1,0,1,1,1,0,0,1,1};s.frames.push_back(f);}
    return s;
}
static assets::Animation animation(unsigned asset){
    assets::Animation a;
    for(unsigned i=0;i<(asset?1:4);++i){a.starts.push_back(a.records.size());
        std::array<std::uint32_t,9> data{};if(!asset){data[0]=2;data[1]=1;}
        a.records.push_back({0,int(asset?0:i),data});a.records.push_back({6,-1,{}});}
    a.starts.push_back(a.records.size());return a;
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try{
        QJsonArray cases;
        if(argc==2){QFile file(QString::fromLocal8Bit(argv[1]));require(file.open(QIODevice::ReadOnly));
            const auto doc=QJsonDocument::fromJson(file.readAll());require(doc.isObject());cases=doc.object()["scenes"].toArray();require(cases.size()==24);
        }else{
            // Distinct keys pin default queue fixture order without a sort oracle.
            cases.append(QJsonObject{{"view",0},{"positions",QJsonArray{QJsonArray{0,0,0,0},QJsonArray{32,0,0,0},QJsonArray{64,0,0,0},QJsonArray{96,0,0,0}}},
                {"order",QJsonArray{0,1,2,3,4,5,6,7,8,9,10,11}},{"depth_keys",QJsonArray{6,8,9,38,40,41,70,72,73,102,104,105}}});
        }
        render::GlBlitter renderer;unsigned frames=0;
        for(const auto& value:cases){const auto fixture=value.toObject();const unsigned view=fixture["view"].toInt();
            const auto body=sprite(0),left=sprite(1),right=sprite(2);
            const auto ani=animation(0),one=animation(1),two=animation(2);
            std::vector<preview::SpriteLayer> layers{{left,one,0,reconstruction::AttachmentPoint::first},{right,two,0,reconstruction::AttachmentPoint::second}};
            {
                preview::SpriteScene scene(renderer,body,ani,{0,1,2,3},false,{1,view},std::move(layers));
                const auto positions=fixture["positions"].toArray();require(positions.size()==4);
                for(unsigned actor=0;actor<4;++actor){const auto p=positions[actor].toArray();require(p.size()==4);scene.setAnchor(actor,256,190);
                    scene.setQueueInput(actor,{{p[0].toInt(),p[1].toInt(),p[2].toInt(),p[3].toInt()},{{6,8,9}}});}
                const auto order=fixture["order"].toArray(),keys=fixture["depth_keys"].toArray();require(order.size()==12 && keys.size()==12);
                for(unsigned tick=0;tick<3;++tick){if(tick)scene.advance();scene.present();auto expected=preview::SpriteScene::background();
                    if(tick<2){const auto queue=scene.drawQueue();require(queue.size()==12);
                        for(unsigned i=0;i<12;++i){const unsigned id=order[i].toInt(),actor=id/3,asset=id%3;
                            require(queue[i].actor==actor && queue[i].asset==asset && queue[i].key==keys[i].toInt());
                            const auto& source=asset==0?body:asset==1?left:right;const auto& f=source.frames[asset?0:actor];
                            const auto& pixels=std::get<std::vector<std::uint16_t>>(f.pixels);
                            // Independent CPU compositor: coordinates from fixture,
                            // order from original PE, no scene anchors/queue used.
                            const int x=256+(asset?0:2)-f.originX,y=190+(asset?0:1)-f.originY;
                            for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)if(f.opaqueMask[row*3+col])expected.pixels[(y+row)*512+x+col]=pixels[row*3+col];
                        }
                    }else require(scene.drawQueue().empty());
                    require(scene.read().pixels==expected.pixels);++frames;
                }
                bool rejected=false;try{scene.setQueueInput(0,{{0,0,-1,0},{{6,8,9}}});}catch(const std::invalid_argument&){rejected=true;}require(rejected);
            }
            require(renderer.stats().surfaces==0 && renderer.stats().pixels==0);
        }
        std::cout<<"{\"frames\":"<<frames<<",\"cases\":"<<cases.size()<<",\"all_match\":true}\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
