#include "scene_snapshot.hpp"
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>

static QByteArray read(const QString& path,qsizetype limit){QFile f(path);if(!f.open(QIODevice::ReadOnly)||f.size()>limit)throw std::runtime_error("Snapshot/bindings input unavailable or too large");auto b=f.readAll();if(f.error()!=QFileDevice::NoError)throw std::runtime_error("Input read failed");return b;}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QCommandLineParser p;p.addHelpOption();
    p.addOptions({{"root","Installed read-only asset root.","path"},{"snapshot","Closed immutable scene snapshot.","path"},
        {"bindings","Pinned explicit observation bitmap bindings.","path"},{"output","New output prefix outside the asset root.","prefix"},
        {"unshaded","Explicit diagnostic embedded-palette policy; original lighting remains separate."},
        {"supported-only","Explicit partial preview; retain gap counts for omitted visible records."}});p.process(app);
    for(const auto* option:{"root","snapshot","bindings","output","unshaded"})if(!p.isSet(option))throw std::invalid_argument("Specify --root --snapshot --bindings --output --unshaded");
    const auto snapshot=mnm::legacy::decodeSceneSnapshot(read(p.value("snapshot"),8*1024*1024));
    auto configured=mnm::assets::AssetStore::create(p.value("root").toStdString());if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));mnm::assets::ResourceManager resources(store);mnm::legacy::SnapshotResources bindings(store,resources);
    const auto doc=QJsonDocument::fromJson(read(p.value("bindings"),1024*1024));if(!doc.isArray()||doc.array().isEmpty()||doc.array().size()>256)throw std::invalid_argument("Bindings must be a bounded nonempty array");
    for(const auto& value:doc.array()){if(!value.isObject())throw std::invalid_argument("Invalid binding record");const auto o=value.toObject();
        if(o.size()!=3||!o.value("id").isString()||!o.value("sprite").isString()||!o.value("sha256").isString())throw std::invalid_argument("Binding requires only id/sprite/sha256 strings");
        bindings.add({mnm::assets::ResourceKind::ui,o.value("id").toString().toStdString()},o.value("sprite").toString().toStdString(),o.value("sha256").toString().toLatin1());}
    const auto display=mnm::legacy::snapshotDisplay(snapshot,bindings,p.isSet("supported-only"));
    const int width=snapshot.width?int(snapshot.width):640,height=snapshot.height?int(snapshot.height):512;
    const auto prefix=std::filesystem::absolute(p.value("output").toStdString()).lexically_normal();
    const auto root=std::filesystem::weakly_canonical(store.root());const auto parent=std::filesystem::weakly_canonical(prefix.parent_path());
    if(parent==root||std::mismatch(root.begin(),root.end(),parent.begin(),parent.end()).first==root.end())throw std::invalid_argument("Outputs must be outside asset root");
    for(const auto* suffix:{".png",".565",".json"})if(std::filesystem::exists(prefix.string()+suffix))throw std::invalid_argument("Outputs must be new");
    mnm::render::GlBlitter renderer;QJsonArray draws;QByteArray pixels;QImage image;
    {mnm::render::SceneRenderer scene(renderer,resources,{width,height,std::vector<std::uint32_t>(std::size_t(width)*height,0x1234)});
        scene.draw(display.draws);const auto native=scene.read();image=scene.present();pixels.reserve(width*height*2);
        for(auto pixel:native.pixels){pixels.append(char(pixel));pixels.append(char(pixel>>8));}
        for(const auto& d:display.draws)draws.append(QJsonObject{{"id",QString::fromStdString(d.resource.text())},{"frame",qint64(d.frame)},{"x",d.anchorX},{"y",d.anchorY}});
    }
    const auto stats=renderer.stats();if(stats.surfaces)throw std::runtime_error("Snapshot scene surface ownership leaked");
    QJsonArray gaps;for(const auto& g:display.gaps)gaps.append(QJsonObject{{"record",qint64(g.record)},{"kind",g.kind},{"token",qint64(g.token)},{"reason",QString::fromStdString(g.reason)}});
    if(!image.save(QString::fromStdString(prefix.string()+".png")))throw std::runtime_error("PNG export failed");
    QFile raw(QString::fromStdString(prefix.string()+".565"));if(!raw.open(QIODevice::NewOnly|QIODevice::WriteOnly)||raw.write(pixels)!=pixels.size()||!raw.flush())throw std::runtime_error("Native export failed");raw.close();
    const QJsonObject report{{"success",true},{"sequence",qint64(snapshot.sequence)},{"view",qint64(snapshot.view)},
        {"width",width},{"height",height},{"records",qint64(snapshot.records.size())},{"draws",draws},{"hidden",qint64(display.hidden)},
        {"unsupported",qint64(display.unsupported)},{"unmapped",qint64(display.unmapped)},{"gaps",gaps},{"shaded_records",qint64(display.shaded)},
        {"complete_mapping",display.complete()},{"original_pixels_compared",false},{"live_replacement",false},
        {"policy","Observed ordered anchors, embedded unshaded asset palettes, diagnostic 0x1234 background"},
        {"pixel_sha256",QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex())},{"remaining_surfaces",qint64(stats.surfaces)}};
    QFile output(QString::fromStdString(prefix.string()+".json"));const auto bytes=QJsonDocument(report).toJson();
    if(!output.open(QIODevice::NewOnly|QIODevice::WriteOnly)||output.write(bytes)!=bytes.size()||!output.flush())throw std::runtime_error("Report export failed");
    std::cout<<bytes.constData();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
