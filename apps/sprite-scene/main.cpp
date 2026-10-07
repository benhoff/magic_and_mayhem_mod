#include "scene.hpp"
#include "attachment_recipe.hpp"
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
    parser.addOption({"attachment-mode-one","Use recovered mode-one recipe from installed effect configuration"});
    parser.addOption({"attachment-facing","ANI facing at attachment admission, independent of body controls","facing","0"});
    parser.addOption({"attachment-health","Creature health fixture for recovered attachment visibility","health","1"});
    parser.addOption({"attachment-remove-at","Remove mode-one attachment before this preview tick","tick"});
    parser.addOption({"attachment-reenter-at","Reenter mode one before this preview tick","tick"});
    parser.addOption({"tile-size","Creature footprint in tiles (1 or 2)","tiles","1"});
    parser.addOption({"placement-view","Recovered offset adjustment value, independent of world projection","view","0"});
    parser.addOption({"queue-position","Explicit per-actor queue x,y,height,priority; repeat once per actor (independent of pixels)","x,y,height,priority"});
    parser.addOption({"visibility","Apply the recovered SPR bitmask pass (normal draw kind; no terrain owners)"});
    parser.addOption({"visibility-expanded","Use the original expanded visibility viewport bounds; requires visibility"});
    parser.addOption({"overlap","Preview fixture: place all actor anchors at the canvas center"});
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
    const bool modeOne=parser.isSet("attachment-mode-one");
    const auto attachmentFacing=parser.value("attachment-facing").toUInt(&ok);if(!ok || attachmentFacing>7)throw std::runtime_error("Attachment facing must be 0..7");
    const auto attachmentHealth=parser.value("attachment-health").toInt(&ok);if(!ok)throw std::runtime_error("Invalid attachment health");
    auto scheduled=[&](const char* option){if(!parser.isSet(option))return 0u;const auto t=parser.value(option).toUInt(&ok);
        if(!modeOne || !ok || !t || t>ticks)throw std::runtime_error("Attachment transition outside preview limits");
        return t;};
    const auto removeAt=scheduled("attachment-remove-at"),reenterAt=scheduled("attachment-reenter-at");
    if(!modeOne && (parser.isSet("attachment-facing") || parser.isSet("attachment-health")))throw std::runtime_error("Attachment fixture options require mode one");
    if(modeOne && parser.isSet("layer"))throw std::runtime_error("Select the mode-one recipe or explicit layers");
    std::vector<mnm::preview::SceneQueueInput> queueInputs;
    const auto queueValues=parser.values("queue-position");
    if(!queueValues.empty() && std::size_t(queueValues.size())!=sequences.size())throw std::runtime_error("Supply one queue position per actor");
    for(const auto& value:queueValues){const auto parts=value.split(',');if(parts.size()!=4)throw std::runtime_error("Queue position needs x,y,height,priority");
        std::array<std::int32_t,4> numbers{};for(unsigned i=0;i<4;++i){numbers[i]=parts[i].toInt(&ok);if(!ok)throw std::runtime_error("Invalid queue coordinate");}
        if(numbers[2]<0)throw std::runtime_error("Queue height must be nonnegative");
        queueInputs.push_back({{numbers[0],numbers[1],numbers[2],numbers[3]},{{6,8,9}}});
    }
    if(parser.isSet("visibility-expanded") && !parser.isSet("visibility"))throw std::runtime_error("Expanded visibility needs --visibility");
    auto initialize=[&](mnm::preview::SpriteScene& scene){scene.setVisibility(parser.isSet("visibility"),parser.isSet("visibility-expanded"));for(std::size_t i=0;i<sequences.size();++i){
        scene.setCreatureHealth(i,attachmentHealth);if(!queueInputs.empty())scene.setQueueInput(i,queueInputs[i]);
        if(parser.isSet("overlap"))scene.setAnchor(i,256,190);
    }};
    auto change=[&](mnm::preview::SpriteScene& scene,unsigned tick){
        if(changeTick && tick==changeTick)scene.selectFacing(changeActor,changeFacing);
        for(std::size_t i=0;i<sequences.size();++i){if(removeAt && tick==removeAt)scene.setModeOneAttachment(i,false);if(reenterAt && tick==reenterAt)scene.setModeOneAttachment(i,true);}
    };
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString(),{"C:/MagicMayhem"});
    if(const auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    auto open=[&](const QString& path){auto value=store.open(path.toStdString());if(const auto* e=std::get_if<mnm::assets::Error>(&value))throw std::runtime_error(e->detail);return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(value));};
    mnm::assets::ResourceManager resources(store);
    auto load=[&](mnm::assets::ResourceId id,const QString& aniPath,const QString& overridePath){
        mnm::preview::SpriteLayer asset;
        auto file=open(aniPath);auto result=mnm::assets::loadAnimation(*file);file.reset();
        if(const auto* e=std::get_if<mnm::assets::AnimationError>(&result))throw std::runtime_error(e->detail);
        const auto animation=std::get<mnm::assets::Animation>(std::move(result));
        QString spritePath=overridePath;
        if(spritePath.isEmpty()){
            const auto& nameBytes=animation.spriteName;const auto end=std::find(nameBytes.begin(),nameBytes.end(),0);
            const std::string name(nameBytes.begin(),end);
            if(end==nameBytes.end() || name.empty() || name=="." || name==".." || name.find_first_of("/\\:")!=std::string::npos ||
               std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32 || c>126;}))throw std::runtime_error("ANI sprite basename requires an explicit sprite path");
            const auto slash=std::max(aniPath.lastIndexOf('/'),aniPath.lastIndexOf('\\'));
            spritePath=aniPath.left(slash+1)+QString::fromStdString(name);
        }
        resources.bind(id,{mnm::assets::ResourceImageFormat::sprite,spritePath.toStdString(),aniPath.toStdString(),{},{}});
        resources.load(id);asset.resource=std::move(id);return std::make_pair(std::move(asset),spritePath);
    };
    const mnm::assets::ResourceId body{mnm::assets::ResourceKind::creature,"preview/body"};
    auto primary=load(body,parser.value("ani"),parser.value("sprite"));const auto spritePath=primary.second;

    mnm::preview::ScenePlacement placement;placement.tileSizeXY=parser.value("tile-size").toUInt(&ok);
    if(!ok || placement.tileSizeXY<1 || placement.tileSizeXY>2)throw std::runtime_error("Tile size must be 1 or 2");
    placement.view=parser.value("placement-view").toUInt(&ok);if(!ok || placement.view>3)throw std::runtime_error("Placement view must be 0..3");
    std::vector<mnm::preview::SpriteLayer> layers;QJsonArray layerInputs;
    if(modeOne){
        const auto recipe=mnm::preview::loadModeOneRecipe(store,attachmentFacing);
        auto loaded=load({mnm::assets::ResourceKind::effect,"36"},QString::fromStdString(recipe.ani),{});loaded.first.sequence=recipe.selection.sequence;loaded.first.modeOne=true;
        layerInputs.append(QJsonObject{{"ani",QString::fromStdString(recipe.ani)},{"sprite",loaded.second},{"sequence",qint64(recipe.selection.sequence)},
            {"slot",1},{"recipe","mode-one"},{"config_entry",36},{"asset_index",qint64(recipe.selection.assetIndex)},{"admission_facing",qint64(attachmentFacing)}});
        layers.push_back(std::move(loaded.first));
    }
    if(parser.values("layer").size()>2)throw std::runtime_error("Select at most two layers");
    for(const auto& selection:parser.values("layer")){
        const auto fields=selection.split(',');if(fields.size()!=3)throw std::runtime_error("Layer needs ANI,sequence,slot");
        const auto sequence=fields[1].toUInt(&ok);if(!ok)throw std::runtime_error("Invalid layer sequence");
        const auto slot=fields[2].toUInt(&ok);if(!ok || (slot!=1 && slot!=2))throw std::runtime_error("Layer slot must be 1 or 2");
        auto loaded=load({mnm::assets::ResourceKind::effect,"preview/layer-"+std::to_string(layers.size())},fields[0],{});loaded.first.sequence=sequence;
        loaded.first.attachment=slot==1?mnm::reconstruction::AttachmentPoint::first:mnm::reconstruction::AttachmentPoint::second;
        layerInputs.append(QJsonObject{{"ani",fields[0]},{"sprite",loaded.second},{"sequence",qint64(sequence)},{"slot",int(slot)}});
        layers.push_back(std::move(loaded.first));
    }
    mnm::render::GlBlitter renderer;
    if(parser.isSet("export-dir")){
        const auto directory=parser.value("export-dir");if(QFile::exists(directory) || !QDir().mkdir(directory))throw std::runtime_error("Export directory must be new with an existing parent");
        QJsonArray frames;
        {
            mnm::preview::SpriteScene scene(renderer,resources,body,sequences,parser.isSet("loop"),placement,std::move(layers));
            initialize(scene);
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
                QJsonArray order;for(const auto& draw:scene.drawQueue())order.append(QJsonObject{{"actor",qint64(draw.actor)},{"asset",qint64(draw.asset)},{"frame",qint64(draw.frame)},{"key",draw.key},{"kind",draw.kind}});
                frames.append(QJsonObject{{"tick",int(tick)},{"draw_queue",order},{"actors",actors},{"layers",attached},{"native_sha256",hash(native)},
                    {"rgba_sha256",hash(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()))}});
            }
        }
        const auto stats=renderer.stats();
        if(stats.surfaces || stats.pixels)throw std::runtime_error("Scene resources leaked");
        const auto driver=renderer.driver();
        const auto bytes=QJsonDocument(QJsonObject{{"ani",parser.value("ani")},{"sprite",spritePath},{"loop_policy",parser.isSet("loop")},
            {"layer_inputs",layerInputs},{"tile_size_xy",qint64(placement.tileSizeXY)},{"placement_view",qint64(placement.view)},
            {"attachment_health",attachmentHealth},{"attachment_remove_at",qint64(removeAt)},{"attachment_reenter_at",qint64(reenterAt)},
            {"visibility_policy",parser.isSet("visibility")},{"visibility_expanded",parser.isSet("visibility-expanded")},
            {"queue_policy","NoCD depth/sort contract; synthetic world positions and biases 6/8/9; optional bitmask visibility, no terrain owners"},
            {"clock","one controller call per explicit preview tick; wall-clock interval is not recovered"},
            {"frames",frames},{"uploads",qint64(stats.uploads)},{"copies",qint64(stats.copies)},
            {"remaining_surfaces",int(stats.surfaces)},{"renderer",QString::fromStdString(driver.renderer)}}).toJson();
        writeNew(QDir(directory).filePath("report.json"),bytes);std::cout<<bytes.constData();return 0;
    }
    mnm::preview::SpriteScene scene(renderer,resources,body,sequences,parser.isSet("loop"),placement,std::move(layers));
    initialize(scene);
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
