#include "scene.hpp"
#include <QApplication>
#include <QCommandLineParser>
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
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString(),{"C:/MagicMayhem"});
    if(const auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    auto open=[&](const QString& path){auto value=store.open(path.toStdString());if(const auto* e=std::get_if<mnm::assets::Error>(&value))throw std::runtime_error(e->detail);return std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(value));};
    auto file=open(parser.value("ani"));auto result=mnm::assets::loadAnimation(*file);file.reset();
    if(const auto* e=std::get_if<mnm::assets::AnimationError>(&result))throw std::runtime_error(e->detail);
    const auto animation=std::get<mnm::assets::Animation>(std::move(result));
    QString spritePath=parser.value("sprite");
    if(!parser.isSet("sprite")){
        const auto end=std::find(animation.spriteName.begin(),animation.spriteName.end(),0);
        const std::string name(animation.spriteName.begin(),end);
        if(end==animation.spriteName.end() || name.empty() || name=="." || name==".." || name.find_first_of("/\\:")!=std::string::npos ||
           std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32 || c>126;}))throw std::runtime_error("ANI sprite basename requires explicit --sprite override");
        const auto ani=parser.value("ani");const auto slash=std::max(ani.lastIndexOf('/'),ani.lastIndexOf('\\'));
        spritePath=ani.left(slash+1)+QString::fromStdString(name);
    }
    file=open(spritePath);auto decoded=mnm::assets::loadSprite(*file);file.reset();
    if(const auto* e=std::get_if<mnm::assets::SpriteError>(&decoded))throw std::runtime_error(e->detail);
    auto sprite=std::get<mnm::assets::Sprite>(std::move(decoded));
    mnm::render::GlBlitter renderer;
    if(parser.isSet("export-dir")){
        const auto directory=parser.value("export-dir");if(QFile::exists(directory) || !QDir().mkdir(directory))throw std::runtime_error("Export directory must be new with an existing parent");
        QJsonArray frames;
        {
            mnm::preview::SpriteScene scene(renderer,std::move(sprite),animation,sequences,parser.isSet("loop"));
            for(unsigned tick=0;tick<=ticks;++tick){
                if(tick){scene.advance();}
                const auto image=scene.present();const auto native=packed(scene.read());
                const auto stem=QString("frame-%1").arg(tick,3,10,QChar('0'));
                writeNew(QDir(directory).filePath(stem+".565"),native);
                QFile png(QDir(directory).filePath(stem+".png"));if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly) || !image.save(&png,"PNG") || !png.flush())throw std::runtime_error("Scene PNG write failed");
                QJsonArray actors;for(const auto& a:scene.actors())actors.append(QJsonObject{{"sequence",qint64(a.sequence)},
                    {"anchor_x",a.anchorX},{"anchor_y",a.anchorY},{"sprite",a.sprite?QJsonValue(qint64(*a.sprite)):QJsonValue(QJsonValue::Null)},{"event",a.event}});
                frames.append(QJsonObject{{"tick",int(tick)},{"actors",actors},{"native_sha256",hash(native)},
                    {"rgba_sha256",hash(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()))}});
            }
        }
        const auto stats=renderer.stats();
        if(stats.surfaces || stats.pixels)throw std::runtime_error("Scene resources leaked");
        const auto driver=renderer.driver();
        const auto bytes=QJsonDocument(QJsonObject{{"ani",parser.value("ani")},{"sprite",spritePath},{"loop_policy",parser.isSet("loop")},
            {"clock","one controller call per explicit preview tick; wall-clock interval is not recovered"},
            {"frames",frames},{"uploads",qint64(stats.uploads)},{"copies",qint64(stats.copies)},
            {"remaining_surfaces",int(stats.surfaces)},{"renderer",QString::fromStdString(driver.renderer)}}).toJson();
        writeNew(QDir(directory).filePath("report.json"),bytes);std::cout<<bytes.constData();return 0;
    }
    mnm::preview::SpriteScene scene(renderer,std::move(sprite),animation,sequences,parser.isSet("loop"));
    QWidget window;window.setWindowTitle("Native ANI/SPR scene preview");auto* layout=new QVBoxLayout(&window);
    auto* caption=new QLabel("Sequences "+parser.value("sequences")+" — presentation clock; "+QString::number(ticks)+" steps per run");layout->addWidget(caption);
    auto* image=new QLabel;layout->addWidget(image);auto* controls=new QHBoxLayout;layout->addLayout(controls);
    auto* pause=new QPushButton("Pause");controls->addWidget(pause);auto* pace=new QSpinBox;pace->setRange(10,1000);pace->setValue(interval);pace->setSuffix(" ms");controls->addWidget(new QLabel("Presentation interval"));controls->addWidget(pace);
    QTimer timer;timer.setInterval(interval);unsigned tick=0;
    auto refresh=[&]{image->setPixmap(QPixmap::fromImage(scene.present()));};refresh();
    QObject::connect(pace,&QSpinBox::valueChanged,&timer,[&](int value){timer.setInterval(value);});
    QObject::connect(pause,&QPushButton::clicked,&timer,[&]{if(timer.isActive()){timer.stop();pause->setText("Resume");}else if(tick<ticks){timer.start();pause->setText("Pause");}});
    bool failed=false;
    QObject::connect(&timer,&QTimer::timeout,&window,[&]{try{scene.advance();refresh();if(++tick==ticks){timer.stop();pause->setEnabled(false);caption->setText("Preview complete — "+QString::number(ticks)+" steps");if(parser.isSet("smoke-test"))app.quit();}}catch(const std::exception& e){timer.stop();caption->setText(QString::fromLocal8Bit(e.what()));pause->setEnabled(false);failed=true;if(parser.isSet("smoke-test"))app.quit();}});
    window.show();timer.start();const auto status=app.exec();return failed?2:status;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
