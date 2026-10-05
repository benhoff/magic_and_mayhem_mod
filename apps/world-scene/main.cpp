#include "scene.hpp"
#include "map_navigation.hpp"
#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QBuffer>
#include <QCommandLineParser>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
namespace {
void writeNew(const QString& path,const QByteArray& data) {
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly) || f.write(data)!=data.size() || !f.flush())
        throw std::runtime_error("Cannot create scene output: "+path.toStdString());
}
unsigned number(const QString& s,unsigned maximum) {
    bool ok=false;const auto n=s.toUInt(&ok);
    if(!ok || n>maximum) throw std::invalid_argument("Scene numeric option out of bounds");
    return n;
}
}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);QCommandLineParser p;p.addHelpOption();
    p.addOption({"checkpoint","Native terrain-motion checkpoint with owned ANI","file"});
    p.addOption({"root","Installed read-only asset root","directory"});
    p.addOption({"realm","Diagnostic terrain realm","path","Realms/Celtic/Forest"});
    p.addOption({"sprite","Explicit creature SPR paired with checkpoint ANI","path","Creatures/RedCap.spr"});
    p.addOption({"definition","Explicit diagnostic TTD body ID","id","5"});
    p.addOption({"terrain-map","Local ordinary MAP geometry payload matching frozen navigation","file"});
    p.addOption({"region","Frozen grid x,y,standing-layer,width,height (width/height 1..8)","fields","0,0,1,8,4"});
    p.addOption({"view","Diagnostic orientation 0..3","index","0"});
    p.addOption({"ticks","Advance before first frame (0..4096)","count","0"});
    p.addOption({"frames","Export consecutive frames (1..64)","count","1"});
    p.addOption({"output","New output prefix; exports PNG/RGB565/JSON and exits","prefix"});
    p.process(app);
    if(!p.isSet("checkpoint") || !p.isSet("root")) throw std::invalid_argument("Specify --checkpoint and --root");
    auto state=mnm::game::readSnapshot(p.value("checkpoint").toStdString());
    if(!state.animation || !state.navigation) throw std::invalid_argument("Scene needs owned ANI and frozen navigation");
    auto decoded=mnm::assets::decodeAnimation(state.animation->data);
    if(auto* error=std::get_if<mnm::assets::AnimationError>(&decoded)) throw std::runtime_error(error->detail);
    auto animation=std::get<mnm::assets::Animation>(std::move(decoded));
    auto navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation);
    mnm::game::World world(0);world.restore(std::move(state));
    mnm::game::MovementSession session(std::move(world),navigation);
    auto configured=mnm::assets::AssetStore::create(p.value("root").toStdString());
    if(auto* error=std::get_if<mnm::assets::Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    const auto open=[&](QString path) {
        auto result=store.open(path.toStdString());
        if(auto* error=std::get_if<mnm::assets::Error>(&result)) throw std::runtime_error(error->detail);
        return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(result));
    };
    const auto sprite=[&](QString path) {
        auto file=open(path);auto result=mnm::assets::loadSprite(*file);
        if(auto* error=std::get_if<mnm::assets::SpriteError>(&result)) throw std::runtime_error(error->detail);
        return std::get<mnm::assets::Sprite>(std::move(result));
    };
    auto terrain=sprite(p.value("realm")+"/Terrain.spr"),creature=sprite(p.value("sprite"));
    auto ttd=open(p.value("realm")+"/Terrain.ttd");auto loaded=mnm::assets::loadTerrainCatalog(*ttd);
    if(auto* error=std::get_if<mnm::assets::TerrainCatalogError>(&loaded)) throw std::runtime_error(error->detail);
    const auto catalog=std::get<mnm::assets::TerrainCatalog>(std::move(loaded));
    std::vector<mnm::scene::Tile> tiles;
    mnm::scene::Camera camera;camera.view=number(p.value("view"),3);camera.x=256;camera.y=160;
    if(p.isSet("terrain-map")) {
        if(p.isSet("region") || p.isSet("definition"))throw std::invalid_argument("Terrain map excludes diagnostic region/definition");
        const auto readLocal=[](const QString& path,qint64 maximum){
            QFile f(path);if(!f.open(QIODevice::ReadOnly) || f.size()<0 || f.size()>maximum)throw std::runtime_error("Cannot read bounded geometry/navigation input");
            const auto b=f.read(maximum+1);if(f.error()!=QFileDevice::NoError || b.size()>maximum)throw std::runtime_error("Geometry/navigation read failed");
            return mnm::assets::Bytes(b.begin(),b.end());
        };
        auto loaded=mnm::assets::decodeMapPayload(readLocal(p.value("terrain-map"),76+4096*12));
        if(auto* error=std::get_if<mnm::assets::PersistenceError>(&loaded))throw std::runtime_error(error->detail);
        const auto map=std::get<mnm::assets::MapAsset>(std::move(loaded));
        const auto frozen=readLocal(QString::fromStdString(session.world().state().map),64*1024*1024);
        mnm::scene::validateMapGeometry(map,catalog,frozen,navigation->binding().fingerprint);
        camera.origin={int(map.width*16),int(map.height*16),0};
        for(unsigned z=0;z<map.layers;++z)for(unsigned y=0;y<map.height;++y)for(unsigned x=0;x<map.width;++x) {
            const auto& c=map.cell(x,y,z);
            if(c.definition && !(c.flags10&0x4000) && !(c.flags8&0x80))
                tiles.push_back({c.definition,{int(x*32),int(y*32),int(z*16+16)},c.flags8,c.flags10});
        }
    } else {
        const auto values=p.value("region").split(',');
        if(values.size()!=5) throw std::invalid_argument("Region needs x,y,standing-layer,width,height");
        const auto x=number(values[0],127),y=number(values[1],127),z=number(values[2],31),w=number(values[3],8),h=number(values[4],8);
        const auto dimensions=navigation->binding().dimensions;
        if(!z || !w || !h || x+w>unsigned(dimensions.x) || y+h>unsigned(dimensions.y) || z>=unsigned(dimensions.z))
            throw std::invalid_argument("Diagnostic region outside frozen grid");
        for(unsigned row=y;row<y+h;++row) for(unsigned col=x;col<x+w;++col) {
            mnm::game::Entity probe;probe.x=col;probe.y=row;probe.z=z;
            probe.motion=mnm::game::CreatureMotion{};probe.motion->terrainMotion=true;
            tiles.push_back({number(p.value("definition"),65535),session.finePosition(probe)});
        }
        camera.origin={int(x*32+w*16),int(y*32+h*16),0};
    }
    mnm::render::GlBlitter renderer;
    const auto draw=[&] {return mnm::scene::render(renderer,terrain,creature,mnm::scene::compose(session,animation,catalog,tiles,camera,terrain.frames.size(),creature.frames.size()));};
    for(unsigned i=0,n=number(p.value("ticks"),4096);i<n;++i) session.step();
    if(p.isSet("output")) {
        const auto frames=number(p.value("frames"),64);if(!frames) throw std::invalid_argument("Frames must be positive");
        QJsonArray report;
        for(unsigned i=0;i<frames;++i) {
            const auto frame=draw();const auto prefix=p.value("output")+QString("-%1").arg(i,3,10,QChar('0'));
            QByteArray pixels;pixels.reserve(512*256*2);
            for(auto pixel:frame.pixels.pixels) {pixels.append(char(pixel&255));pixels.append(char((pixel>>8)&255));}
            writeNew(prefix+".565",pixels);
            QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);
            if(!frame.image.save(&buffer,"PNG")) throw std::runtime_error("Cannot encode PNG");
            writeNew(prefix+".png",png);
            QJsonArray queue;for(const auto& d:frame.queue) queue.append(QJsonObject{{"creature",d.creature},{"frame",qint64(d.frame)},{"x",d.x},{"y",d.y},{"key",d.key}});
            QJsonArray actors;for(const auto& slot:session.world().state().slots) if(slot.entity) {const auto fine=session.finePosition(*slot.entity);actors.append(QJsonObject{{"fine",QJsonArray{fine.x,fine.y,fine.z}}});}
            report.append(QJsonObject{{"tick",qint64(session.world().state().tick)},{"queue",queue},{"actors",actors}});
            if(i+1<frames) session.step();
        }
        writeNew(p.value("output")+".json",QJsonDocument(QJsonObject{{"policy",p.isSet("terrain-map")?"ordinary installed-map crop; diagnostic projection":"diagnostic cell-centred terrain fixture"},{"terrain_map",p.value("terrain-map")},{"view",int(camera.view)},{"frames",report}}).toJson());
        const auto result=mnm::game::writeSnapshot((p.value("output")+".mnms").toStdString(),session.world().state());
        if(!result.durable) throw std::runtime_error(result.detail);
        return 0;
    }
    if(p.isSet("frames")) throw std::invalid_argument("Frames requires output");
    QWidget window;window.setWindowTitle("Native movement scene");auto* layout=new QVBoxLayout(&window);
    auto* image=new QLabel;auto* status=new QLabel;layout->addWidget(image);layout->addWidget(status);
    auto* actions=new QHBoxLayout;layout->addLayout(actions);
    auto* step=new QPushButton("Step");auto* save=new QPushButton("Save checkpoint");actions->addWidget(step);actions->addWidget(save);
    const auto refresh=[&] {const auto frame=draw();image->setPixmap(QPixmap::fromImage(frame.image));status->setText(QString("Tick %1 · terrain scene · view %2").arg(session.world().state().tick).arg(camera.view));};
    QObject::connect(step,&QPushButton::clicked,[&] {try {session.step();refresh();} catch(const std::exception& e) {QMessageBox::critical(&window,"Cannot step",e.what());}});
    QObject::connect(save,&QPushButton::clicked,[&] {
        const auto path=QFileDialog::getSaveFileName(&window,"New checkpoint",{},"Native checkpoint (*.mnms)");if(path.isEmpty()) return;
        try {const auto result=mnm::game::writeSnapshot(path.toStdString(),session.world().state());if(!result.durable) throw std::runtime_error(result.detail);}
        catch(const std::exception& e) {QMessageBox::critical(&window,"Cannot save",e.what());}
    });
    refresh();window.show();return app.exec();
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
