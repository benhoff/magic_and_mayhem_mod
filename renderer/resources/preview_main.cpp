#include "resource_cache.hpp"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <limits>
#include <stdexcept>

int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QCommandLineParser parser;parser.addHelpOption();
    parser.setApplicationDescription("Native stable resource recipe preview; explicit bindings, no game launch");
    parser.addOption({"root","Installed asset root","directory"});
    parser.addOption({"kind","creature, terrain, effect or ui","kind","ui"});
    parser.addOption({"id","Stable lowercase semantic name","name"});
    parser.addOption({"image","Image request relative to root","path"});
    parser.addOption({"format","sprite, bmp, pcx or jpeg","format","sprite"});
    parser.addOption({"animation","Optional paired ANI request","path"});
    parser.addOption({"catalog","Optional terrain TTD request","path"});
    parser.addOption({"sequence","Optional ANI sequence binding","index"});
    parser.addOption({"frame","Image frame to draw","index","0"});
    parser.addOption({"output","New PNG path outside asset root","file"});parser.process(app);
    for(const auto* key:{"root","id","image","output"})if(!parser.isSet(key))throw std::invalid_argument(std::string("Required --")+key);
    const auto number=[&](const char* key){bool ok=false;const auto value=parser.value(key).toUInt(&ok);if(!ok)throw std::invalid_argument(std::string("Invalid --")+key);return value;};
    const auto kind=parser.value("kind"),format=parser.value("format");
    mnm::assets::ResourceId id;id.name=parser.value("id").toStdString();
    if(kind=="creature")id.kind=mnm::assets::ResourceKind::creature;else if(kind=="terrain")id.kind=mnm::assets::ResourceKind::terrain;
    else if(kind=="effect")id.kind=mnm::assets::ResourceKind::effect;else if(kind!="ui")throw std::invalid_argument("Unknown kind");
    mnm::assets::ResourceRecipe recipe;recipe.image=parser.value("image").toStdString();
    if(format=="bmp")recipe.format=mnm::assets::ResourceImageFormat::bmp;else if(format=="pcx")recipe.format=mnm::assets::ResourceImageFormat::pcx;
    else if(format=="jpeg")recipe.format=mnm::assets::ResourceImageFormat::jpeg;else if(format!="sprite")throw std::invalid_argument("Unknown format");
    if(parser.isSet("animation"))recipe.animation=parser.value("animation").toStdString();
    if(parser.isSet("catalog"))recipe.terrainCatalog=parser.value("catalog").toStdString();
    if(parser.isSet("sequence"))recipe.sequence=number("sequence");
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString());
    if(const auto* error=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    const auto output=std::filesystem::weakly_canonical(parser.value("output").toStdString());
    const auto relative=output.lexically_relative(store.root());
    if(relative.empty() || *relative.begin()!="..")throw std::invalid_argument("Output must lie outside asset root");
    mnm::assets::ResourceManager resources(std::move(store));resources.bind(id,recipe);const auto& loaded=resources.load(id);
    const auto frame=number("frame");if(frame>=loaded.frameCount())throw std::out_of_range("Frame outside resource");
    std::uint32_t width=0,height=0;std::int32_t ox=0,oy=0;
    std::visit([&](const auto& image){using T=std::decay_t<decltype(image)>;
        if constexpr(std::is_same_v<T,mnm::assets::Sprite>){const auto& f=image.frames[frame];width=f.width;height=f.height;ox=f.originX;oy=f.originY;}
        else{width=image.width;height=image.height;}
    },loaded.image);
    if(width>2046 || height>2046 || ox==INT32_MAX || oy==INT32_MAX)throw std::runtime_error("Frame exceeds preview bounds");
    mnm::render::GlBlitter renderer;
    const auto canvas=renderer.create({int(width+2),int(height+2),std::vector<std::uint32_t>(std::size_t(width+2)*(height+2),0x1234)},mnm::render::spriteFormat);
    {
        mnm::render::ResourceCache cache(renderer,resources);cache.draw(id,frame,canvas,ox+1,oy+1);
        const auto uploads=renderer.stats().uploads;cache.draw(id,frame,canvas,ox+1,oy+1);
        if(renderer.stats().uploads!=uploads)throw std::runtime_error("Resident draw unexpectedly uploaded");
        const auto image=renderer.present(canvas);QFile png(QString::fromStdString(output.string()));
        if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(png.errorString().toStdString());
        if(!image.save(&png,"PNG") || !png.flush()){png.remove();throw std::runtime_error("Failed to save preview");}
        cache.retire(id);
    }
    renderer.destroy(canvas);
    if(renderer.stats().surfaces || resources.stats().decodedBytes)throw std::runtime_error("Preview resource retirement incomplete");
    std::cout<<QJsonDocument(QJsonObject{{"ok",true},{"id",QString::fromStdString(id.text())},{"frame",int(frame)},
        {"decoded_loads",qint64(resources.stats().loads)},{"surfaces",0},{"decoded_bytes",0}}).toJson().constData();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
