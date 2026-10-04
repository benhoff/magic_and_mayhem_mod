#include "scene.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <iostream>
#include <stdexcept>

static QString hash(const QByteArray& bytes){return QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex();}
static QByteArray packed(const mnm::render::Image& image){QByteArray b;b.reserve(image.width*image.height*2);for(auto p:image.pixels){b.append(char(p));b.append(char(p>>8));}return b;}
static void writeNew(const QString& path,const QByteArray& bytes){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(f.errorString().toStdString());if(f.write(bytes)!=bytes.size() || !f.flush()){f.remove();throw std::runtime_error("Scene output write failed");}}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);QCommandLineParser parser;parser.addHelpOption();
    parser.setApplicationDescription("Bounded native ANI/SPR scene; forward recovered ticks, independent presentation clock");
    parser.addOption({"root","Installed asset root","directory"});
    parser.addOption({"ani","Version-5 ANI path","path"});
    parser.addOption({"sprite","Explicit paired SPR (default ANI sibling basename)","path"});
    parser.addOption({"sequences","1..4 numeric ANI sequence indices","indices","0,4"});
    parser.addOption({"ticks","Steps per preview run (1..512; exports max 64)","count","32"});
    parser.addOption({"interval","Presentation interval in milliseconds; independent of game clock","ms","100"});
    parser.addOption({"loop","Preview policy: restart stopped sequences on the next step"});
    parser.addOption({"facing-change","Apply one phase-preserving change at tick,actor,facing (numeric facing 0..7)","selection"});
    parser.addOption({"layer","Attach ANI,sequence,slot (slot 1 or 2); repeat at most twice","selection"});
    parser.addOption({"tile-size","Creature footprint in tiles (1 or 2)","tiles","1"});
    parser.addOption({"placement-view","Recovered offset adjustment value, independent of world projection","view","0"});
    parser.addOption({"smoke-test","Close the window after the bounded preview completes"});
    parser.addOption({"export-dir","Export ticks synchronously to a new directory instead of opening a window","directory"});
    parser.process(app);
    if(!parser.isSet("root") || !parser.isSet("ani"))throw std::runtime_error("Specify --root and --ani");
    bool ok=false;const auto ticks=parser.value("ticks").toUInt(&ok);
    if(!ok || !ticks || ticks>512 || (parser.isSet("export-dir") && ticks>64))throw std::runtime_error("Tick count outside preview/export limits");
    const auto interval=parser.value("interval").toInt(&ok);if(!ok || interval<10 || interval>1000)throw std::runtime_error("Presentation interval must be 10..1000 ms");
    std::vector<std::uint32_t> sequences;for(const auto& field:parser.value("sequences").split(',')){
        const auto n=field.toUInt(&ok);if(!ok)throw std::runtime_error("Invalid sequence index");sequences.push_back(n);}
    if(sequences.empty() || sequences.size()>4)throw std::runtime_error("Select 1..4 sequences");
    unsigned changeTick=0,changeActor=0,changeFacing=0;
    if(parser.isSet("facing-change")){
        const auto fields=parser.value("facing-change").split(',');
        if(fields.size()!=3)throw std::runtime_error("Facing change needs tick,actor,facing");
        auto number=[&](int i){const auto v=fields[i].toUInt(&ok);if(!ok)throw std::runtime_error("Invalid facing change number");return v;};
        changeTick=number(0);changeActor=number(1);changeFacing=number(2);
        if(!changeTick || changeTick>ticks || changeActor>=sequences.size() || changeFacing>=8)throw std::runtime_error("Facing change outside run limits");
    }
    auto change=[&](mnm::preview::SpriteScene& scene,unsigned tick){if(changeTick && tick==changeTick)scene.selectFacing(changeActor,changeFacing);};
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString(),{"C:/MagicMayhem"});
    if(const auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    auto open=[&](const QString& path){auto value=store.open(path.toStdString());if(const auto* e=std::get_if<mnm::assets::Error>(&value))throw std::runtime_error(e->detail);return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(value));};
    auto load=[&](const QString& aniPath,const QString& overridePath){
        mnm::preview::SpriteLayer asset;
        auto file=open(aniPath);auto result=mnm::assets::loadAnimation(*file);file.reset();
        if(const auto* e=std::get_if<mnm::assets::AnimationError>(&result))throw std::runtime_error(e->detail);
        asset.animation=std::get<mnm::assets::Animation>(std::move(result));
        QString spritePath=overridePath;
        if(spritePath.isEmpty()){
            const auto& nameBytes=asset.animation.spriteName;const auto end=std::find(nameBytes.begin(),nameBytes.end(),0);
            const std::string name(nameBytes.begin(),end);
            if(end==nameBytes.end() || name.empty() || name=="." || name==".." || name.find_first_of("/\\:")!=std::string::npos ||
               std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32 || c>126;}))throw std::runtime_error("ANI sprite basename requires an explicit sprite path");
            const auto slash=std::max(aniPath.lastIndexOf('/'),aniPath.lastIndexOf('\\'));
            spritePath=aniPath.left(slash+1)+QString::fromStdString(name);
        }
        file=open(spritePath);auto decoded=mnm::assets::loadSprite(*file);file.reset();
        if(const auto* e=std::get_if<mnm::assets::SpriteError>(&decoded))throw std::runtime_error(e->detail);
        asset.sprite=std::get<mnm::assets::Sprite>(std::move(decoded));return std::make_pair(std::move(asset),spritePath);
    };
    auto primary=load(parser.value("ani"),parser.value("sprite"));
    const auto animation=std::move(primary.first.animation);auto sprite=std::move(primary.first.sprite);const auto spritePath=primary.second;
    mnm::preview::ScenePlacement placement;placement.tileSizeXY=parser.value("tile-size").toUInt(&ok);
    if(!ok || placement.tileSizeXY<1 || placement.tileSizeXY>2)throw std::runtime_error("Tile size must be 1 or 2");
    placement.view=parser.value("placement-view").toUInt(&ok);if(!ok || placement.view>3)throw std::runtime_error("Placement view must be 0..3");
    std::vector<mnm::preview::SpriteLayer> layers;QJsonArray layerInputs;
    if(parser.values("layer").size()>2)throw std::runtime_error("Select at most two layers");
    for(const auto& selection:parser.values("layer")){
        const auto fields=selection.split(',');if(fields.size()!=3)throw std::runtime_error("Layer needs ANI,sequence,slot");
        const auto sequence=fields[1].toUInt(&ok);if(!ok)throw std::runtime_error("Invalid layer sequence");
        const auto slot=fields[2].toUInt(&ok);if(!ok || (slot!=1 && slot!=2))throw std::runtime_error("Layer slot must be 1 or 2");
        auto loaded=load(fields[0],{});loaded.first.sequence=sequence;
        loaded.first.attachment=slot==1?mnm::reconstruction::AttachmentPoint::first:mnm::reconstruction::AttachmentPoint::second;
        layerInputs.append(QJsonObject{{"ani",fields[0]},{"sprite",loaded.second},{"sequence",qint64(sequence)},{"slot",int(slot)}});
        layers.push_back(std::move(loaded.first));
    }
    mnm::render::GlBlitter renderer;
    if(parser.isSet("export-dir")){
        const auto directory=parser.value("export-dir");if(QFile::exists(directory) || !QDir().mkdir(directory))throw std::runtime_error("Export directory must be new with an existing parent");
        QJsonArray frames;
        {
            mnm::preview::SpriteScene scene(renderer,std::move(sprite),animation,sequences,parser.isSet("loop"),placement,std::move(layers));
            for(unsigned tick=0;tick<=ticks;++tick){
                if(tick){change(scene,tick);scene.advance();}
                const auto image=scene.present();const auto native=packed(scene.read());
                const auto stem=QString("frame-%1").arg(tick,3,10,QChar('0'));
                writeNew(QDir(directory).filePath(stem+".565"),native);
                QFile png(QDir(directory).filePath(stem+".png"));if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly) || !image.save(&png,"PNG") || !png.flush())throw std::runtime_error("Scene PNG write failed");
                QJsonArray actors;for(const auto& a:scene.actors())actors.append(QJsonObject{{"sequence",qint64(a.sequence)},
                    {"anchor_x",a.anchorX},{"anchor_y",a.anchorY},{"draw_anchor_x",a.drawAnchor?QJsonValue(a.drawAnchor->x):QJsonValue(QJsonValue::Null)},{"draw_anchor_y",a.drawAnchor?QJsonValue(a.drawAnchor->y):QJsonValue(QJsonValue::Null)},{"sprite",a.sprite?QJsonValue(qint64(*a.sprite)):QJsonValue(QJsonValue::Null)},{"event",a.event}});
                QJsonArray attached;for(const auto& l:scene.layers())attached.append(QJsonObject{{"actor",qint64(l.actor)},{"layer",qint64(l.layer)},{"sequence",qint64(l.sequence)},
                    {"sprite",l.sprite?QJsonValue(qint64(*l.sprite)):QJsonValue(QJsonValue::Null)},
                    {"draw_anchor_x",l.drawAnchor?QJsonValue(l.drawAnchor->x):QJsonValue(QJsonValue::Null)},
                    {"draw_anchor_y",l.drawAnchor?QJsonValue(l.drawAnchor->y):QJsonValue(QJsonValue::Null)},{"event",l.event}});
                frames.append(QJsonObject{{"tick",int(tick)},{"actors",actors},{"layers",attached},{"native_sha256",hash(native)},
                    {"rgba_sha256",hash(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()))}});
            }
        }
        const auto stats=renderer.stats();
        if(stats.surfaces || stats.pixels)throw std::runtime_error("Scene resources leaked");
        const auto driver=renderer.driver();
        const auto bytes=QJsonDocument(QJsonObject{{"ani",parser.value("ani")},{"sprite",spritePath},{"loop_policy",parser.isSet("loop")},
            {"layer_inputs",layerInputs},{"tile_size_xy",qint64(placement.tileSizeXY)},{"placement_view",qint64(placement.view)},
            {"clock","one controller call per explicit preview tick; wall-clock interval is not recovered"},
            {"frames",frames},{"uploads",qint64(stats.uploads)},{"copies",qint64(stats.copies)},
            {"remaining_surfaces",int(stats.surfaces)},{"renderer",QString::fromStdString(driver.renderer)}}).toJson();
        writeNew(QDir(directory).filePath("report.json"),bytes);std::cout<<bytes.constData();return 0;
    }
    mnm::preview::SpriteScene scene(renderer,std::move(sprite),animation,sequences,parser.isSet("loop"),placement,std::move(layers));
    QWidget window;window.setWindowTitle("Native ANI/SPR scene preview");auto* layout=new QVBoxLayout(&window);
    auto* caption=new QLabel("Sequences "+parser.value("sequences")+" — presentation clock; "+QString::number(ticks)+" steps per run");layout->addWidget(caption);
    auto* image=new QLabel;layout->addWidget(image);auto* controls=new QHBoxLayout;layout->addLayout(controls);
    auto* pause=new QPushButton("Pause");controls->addWidget(pause);auto* pace=new QSpinBox;pace->setRange(10,1000);pace->setValue(interval);pace->setSuffix(" ms");controls->addWidget(new QLabel("Presentation interval"));controls->addWidget(pace);
    QTimer timer;timer.setInterval(interval);unsigned tick=0;
    std::vector<QComboBox*> groupControls,facingControls;
    auto refresh=[&]{image->setPixmap(QPixmap::fromImage(scene.present()));const auto states=scene.actors();
        for(std::size_t i=0;i<groupControls.size();++i){const auto sequence=states[i].sequence;
            groupControls[i]->setCurrentIndex(groupControls[i]->findData(sequence-sequence%8));facingControls[i]->setCurrentIndex(sequence%8);}
    };refresh();
    // Widgets call semantic scene actions; no executable offsets or pointers.
    const auto groups=scene.directionalGroups();
    for(std::size_t actor=0;actor<sequences.size();++actor){
        auto* row=new QHBoxLayout;layout->addLayout(row);row->addWidget(new QLabel("Actor "+QString::number(actor)));
        auto* group=new QComboBox;for(auto base:groups)group->addItem("Group "+QString::number(base),base);
        const auto base=sequences[actor]-sequences[actor]%8;group->setCurrentIndex(group->findData(base));row->addWidget(group);
        auto* facing=new QComboBox;for(unsigned f=0;f<8;++f)facing->addItem("Facing "+QString::number(f));facing->setCurrentIndex(sequences[actor]%8);row->addWidget(facing);
        const bool supported=group->currentIndex()>=0;group->setEnabled(supported);facing->setEnabled(supported);
        groupControls.push_back(group);facingControls.push_back(facing);
        QObject::connect(group,&QComboBox::activated,&window,[&,actor,group,facing](int){try{
            scene.selectGroup(actor,group->currentData().toUInt(),facing->currentIndex());refresh();
            caption->setText("Group changed; animation restarted");
        }catch(const std::exception& e){const auto selected=scene.actors()[actor].sequence;group->setCurrentIndex(group->findData(selected-selected%8));facing->setCurrentIndex(selected%8);caption->setText(QString::fromLocal8Bit(e.what()));}});
        QObject::connect(facing,&QComboBox::activated,&window,[&,actor,facing](int){try{
            scene.selectFacing(actor,facing->currentIndex());refresh();caption->setText("Facing changed; animation progress retained");
        }catch(const std::exception& e){facing->setCurrentIndex(scene.actors()[actor].sequence%8);caption->setText(QString::fromLocal8Bit(e.what()));}});
    }
    QObject::connect(pace,&QSpinBox::valueChanged,&timer,[&](int value){timer.setInterval(value);});
    QObject::connect(pause,&QPushButton::clicked,&timer,[&]{if(timer.isActive()){timer.stop();pause->setText("Resume");}else if(tick<ticks){timer.start();pause->setText("Pause");}});
    bool failed=false;
    QObject::connect(&timer,&QTimer::timeout,&window,[&]{try{change(scene,tick+1);scene.advance();refresh();if(++tick==ticks){timer.stop();pause->setEnabled(false);caption->setText("Preview complete — "+QString::number(ticks)+" steps");if(parser.isSet("smoke-test"))app.quit();}}catch(const std::exception& e){timer.stop();caption->setText(QString::fromLocal8Bit(e.what()));pause->setEnabled(false);failed=true;if(parser.isSet("smoke-test"))app.quit();}});
    window.show();timer.start();const auto status=app.exec();return failed?2:status;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
