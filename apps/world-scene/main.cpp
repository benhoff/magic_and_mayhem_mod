#include "scene.hpp"
#ifdef MNM_SCENE_WINDOW_TEST
#include "../../tests/scene-window-driver.hpp"
#endif
#include "playback.hpp"
#include "playback_controls.hpp"
#include "picking.hpp"
#include "scene_canvas.hpp"
#include "movement_controls.hpp"
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
#include <QPainter>
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
#ifdef MNM_SCENE_WINDOW_TEST
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
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
#ifdef MNM_SCENE_WINDOW_TEST
    p.addOption({"window-script","Bounded test-only window action script","file"});
    p.addOption({"window-output","New test-only artifact directory","directory"});
#endif
    p.process(app);
    if(!p.isSet("checkpoint") || !p.isSet("root")) throw std::invalid_argument("Specify --checkpoint and --root");
    auto state=mnm::game::readSnapshot(p.value("checkpoint").toStdString());
    if(!state.animation || !state.navigation) throw std::invalid_argument("Scene needs owned ANI and frozen navigation");
    auto navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation);
    const auto geometryFingerprint=navigation->binding().fingerprint;
    if(!(navigation->binding()==*state.navigation)) navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation,true);
    if(!(navigation->binding()==*state.navigation)) navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation,true,true);
    mnm::game::World world(0);world.restore(std::move(state));
    mnm::game::MovementSession session(std::move(world),navigation);
    auto configured=mnm::assets::AssetStore::create(p.value("root").toStdString());
    if(auto* error=std::get_if<mnm::assets::Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    mnm::assets::ResourceManager resources(std::move(store));
    const mnm::assets::ResourceId terrainId{mnm::assets::ResourceKind::terrain,"preview/terrain"};
    const mnm::assets::ResourceId creatureId{mnm::assets::ResourceKind::creature,"preview/creature"};
    resources.bind(terrainId,{mnm::assets::ResourceImageFormat::sprite,(p.value("realm")+"/Terrain.spr").toStdString(),{},(p.value("realm")+"/Terrain.ttd").toStdString(),{}});
    mnm::assets::ResourceRecipe creatureRecipe{mnm::assets::ResourceImageFormat::sprite,p.value("sprite").toStdString(),{},{},{}};
    creatureRecipe.animationBytes=session.world().state().animation->data;resources.bind(creatureId,creatureRecipe);
    const auto& terrainResource=resources.load(terrainId);const auto& creatureResource=resources.load(creatureId);
    const auto& terrain=std::get<mnm::assets::Sprite>(terrainResource.image);
    const auto& creature=std::get<mnm::assets::Sprite>(creatureResource.image);
    const auto& catalog=*terrainResource.terrainCatalog;const auto& animation=*creatureResource.animation;
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
        mnm::scene::validateMapGeometry(map,catalog,frozen,geometryFingerprint);
        camera.origin={int(map.width*16),int(map.height*16),0};
        for(unsigned z=0;z<map.layers;++z)for(unsigned y=0;y<map.height;++y)for(unsigned x=0;x<map.width;++x) {
            const auto& c=map.cell(x,y,z);
            if(c.definition && !(c.flags10&0x4000) && !(c.flags8&0x80)) {
                std::optional<mnm::game::Point> standing;
                if(z>0) standing=mnm::game::Point{int(x),int(y),int(z)};
                tiles.push_back({c.definition,{int(x*32),int(y*32),int(z*16+16)},c.flags8,c.flags10,standing});
            }
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
            tiles.push_back({number(p.value("definition"),65535),session.finePosition(probe),4,0,mnm::game::Point{int(col),int(row),int(z)}});
        }
        camera.origin={int(x*32+w*16),int(y*32+h*16),0};
    }
    mnm::render::GlBlitter renderer;
    mnm::render::SceneRenderer drawing(renderer,resources,{512,256,std::vector<std::uint32_t>(512*256,0x2124)});
    const auto draw=[&] {return mnm::scene::render(drawing,terrainId,creatureId,mnm::scene::compose(session,animation,catalog,tiles,camera,terrain.frames.size(),creature.frames.size()));};
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
            QJsonArray queue;for(const auto& d:frame.queue) {
                QJsonObject item{{"creature",d.creature},{"frame",qint64(d.frame)},{"x",d.x},{"y",d.y},{"key",d.key}};
                if(d.actor) {item.insert("slot",int(d.actor->slot));item.insert("generation",qint64(d.actor->generation));}
                if(d.standing) item.insert("cell",QJsonArray{d.standing->x,d.standing->y,d.standing->z});
                queue.append(item);
            }
            QJsonArray actors;for(unsigned slotId=0;slotId<session.world().state().slots.size();++slotId) {
                const auto& slot=session.world().state().slots[slotId];
                if(!slot.entity || slot.entity->cleaned || slot.entity->family!=mnm::game::Family::creature || !slot.entity->motion) continue;
                const auto fine=session.finePosition(*slot.entity);actors.append(QJsonObject{{"slot",int(slotId)},{"generation",qint64(slot.generation)},{"fine",QJsonArray{fine.x,fine.y,fine.z}}});
            }
            report.append(QJsonObject{{"tick",qint64(session.world().state().tick)},{"queue",queue},{"actors",actors}});
            if(i+1<frames) session.step();
        }
        writeNew(p.value("output")+".json",QJsonDocument(QJsonObject{{"policy",p.isSet("terrain-map")?"ordinary installed-map crop; diagnostic projection":"diagnostic cell-centred terrain fixture"},{"terrain_map",p.value("terrain-map")},{"view",int(camera.view)},{"frames",report}}).toJson());
        const auto result=mnm::game::writeSnapshot((p.value("output")+".mnms").toStdString(),session.world().state());
        if(!result.durable) throw std::runtime_error(result.detail);
        return 0;
    }
    if(p.isSet("frames")) throw std::invalid_argument("Frames requires output");
    QWidget window;window.setObjectName("nativeSceneWindow");window.setWindowTitle("Native movement scene");auto* layout=new QVBoxLayout(&window);
    auto* image=new mnm::scene::SceneCanvas;image->setObjectName("sceneCanvas");std::vector<mnm::scene::Draw> displayedQueue;auto* status=new QLabel;status->setObjectName("sceneStatus");layout->addWidget(image);layout->addWidget(status);
    mnm::scene::Orders orders;auto* controls=new mnm::scene::MovementControls;layout->addWidget(controls);
    layout->addWidget(new QLabel("Left-click terrain and use Spawn creature while paused. Left-click a creature, then right-click terrain to queue a move. Play runs at 10 ticks/second; Step works while paused. Target-cell fields also work."));
    auto* actions=new QHBoxLayout;layout->addLayout(actions);
    auto* playbackControls=new mnm::scene::PlaybackControls;auto* save=new QPushButton("Save checkpoint");save->setObjectName("saveCheckpoint");actions->addWidget(playbackControls);actions->addWidget(save);
#ifdef MNM_SCENE_WINDOW_TEST
    std::function<void()> afterTick;
#endif
    mnm::scene::Playback playback([&] {session.step();
#ifdef MNM_SCENE_WINDOW_TEST
        if(afterTick) afterTick();
#endif
    });
#ifdef MNM_SCENE_WINDOW_TEST
    std::shared_ptr<QObject> testDriver;
#endif
    const auto refresh=[&] {
        orders.synchronize(session.world().state());const auto frame=draw();auto pixels=QPixmap::fromImage(frame.image);
        if(orders.selected()) {
            QPainter highlight(&pixels);highlight.setPen(QPen(QColor(255,210,40),2));
            for(const auto& d:frame.queue) if(d.actor && *d.actor==*orders.selected()) {
                const auto& body=creature.frames.at(d.frame);
                highlight.drawRect(d.x-body.originX,d.y-body.originY,int(body.width),int(body.height));
            }
        }
        displayedQueue=frame.queue;image->present(pixels);controls->updateChoices(mnm::scene::creatureChoices(session.world().state()),orders.selected(),navigation->binding().dimensions);
        controls->updateSpawning(orders.placement(),playback.playing(),session.canSpawnCreature());
        status->setText(QString("Tick %1 · terrain scene · view %2 · %3").arg(session.world().state().tick).arg(camera.view).arg(playback.playing()?"Playing":"Paused"));
    };
    controls->onSpawn=[&] {
        try {orders.spawn(session,!playback.playing());refresh();status->setText("Creature spawned and selected. Right-click terrain to move it.");}
        catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    controls->onSelect=[&](std::optional<mnm::game::Handle> actor) {
        try {orders.select(session.world().state(),actor);refresh();}
        catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    controls->onMove=[&](mnm::game::Point target) {
        try {orders.move(session,target);refresh();status->setText("Move order queued. Play or Step to apply it.");}
        catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    controls->onStop=[&] {
        try {orders.stop(session);refresh();status->setText("Stop queued. Play or Step to apply it.");}
        catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    controls->onCancelQueuedMoves=[&] {
        try {const auto n=orders.cancelQueuedMoves(session);refresh();status->setText(QString("Removed %1 queued moves.").arg(n));}
        catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    image->onSelectAt=[&](int x,int y) {
        try {
            const auto actor=mnm::scene::pickActor(displayedQueue,terrain,creature,x,y);
            orders.selectPlacement(session.world().state(),actor?std::nullopt:mnm::scene::pickTerrain(displayedQueue,terrain,x,y));
            controls->onSelect(actor);
        }
        catch(const std::exception& e) {status->setText(e.what());}
    };
    image->onMoveAt=[&](int x,int y) {
        try {
            orders.synchronize(session.world().state());
            if(!orders.selected()) {refresh();status->setText("Select a creature first.");return;}
            const auto cell=mnm::scene::pickTerrain(displayedQueue,terrain,x,y);
            if(!cell) {status->setText("No movement cell at this point.");return;}
            controls->setTarget(*cell);controls->onMove(*cell);
        } catch(const std::exception& e) {refresh();status->setText(e.what());}
    };
    playback.onRefresh=refresh;
    playback.onPlaying=[&](bool playing) {playbackControls->updatePlaying(playing);refresh();};
    playback.onError=[&](const std::string& message) {status->setText(QString("Paused: %1").arg(QString::fromStdString(message)));};
    playbackControls->onPlay=[&] {playback.play();};
    playbackControls->onPause=[&] {playback.pause();};
    playbackControls->onStep=[&] {playback.step();};
    QObject::connect(save,&QPushButton::clicked,[&] {
        playback.pause();
        const auto path=QFileDialog::getSaveFileName(&window,"New checkpoint",{},"Native checkpoint (*.mnms)");if(path.isEmpty()) return;
        try {const auto result=mnm::game::writeSnapshot(path.toStdString(),session.world().state());if(!result.durable) throw std::runtime_error(result.detail);}
        catch(const std::exception& e) {QMessageBox::critical(&window,"Cannot save",e.what());}
    });
    refresh();window.show();
#ifdef MNM_SCENE_WINDOW_TEST
    if(!p.isSet("window-script") || !p.isSet("window-output")) throw std::invalid_argument("Window test requires script and output");
    testDriver=mnm::scene::test::startWindowScript(p.value("window-script"),p.value("window-output"),window,*image,session,draw,playback,afterTick);
#endif
    return app.exec();
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
