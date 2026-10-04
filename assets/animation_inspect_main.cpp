#include "animation.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);
    if(argc!=3)throw std::runtime_error("Usage: mnm-animation-inspect ROOT MANIFEST (array of paths)");
    auto configured=mnm::assets::AssetStore::create(argv[1],{"C:/MagicMayhem"});
    if(const auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));QFile input(argv[2]);
    if(!input.open(QIODevice::ReadOnly) || input.size()>1024*1024)throw std::runtime_error("Manifest input limit");
    QJsonParseError error;const auto document=QJsonDocument::fromJson(input.read(1024*1024+1),&error);
    if(error.error!=QJsonParseError::NoError || !document.isArray() || document.array().isEmpty() || document.array().size()>4096)
        throw std::runtime_error("Invalid manifest array");
    QJsonArray files;
    for(const auto& path:document.array()){
        if(!path.isString())throw std::runtime_error("Manifest paths must be strings");
        auto opened=store.open(path.toString().toStdString());
        if(const auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);
        auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
        auto parsed=mnm::assets::loadAnimation(*file);file.reset();
        if(const auto* e=std::get_if<mnm::assets::AnimationError>(&parsed)){
            files.append(QJsonObject{{"path",path},{"decoded",false},{"code",int(e->code)},{"offset",qint64(e->offset)}});continue;
        }
        const auto& a=std::get<mnm::assets::Animation>(parsed);QByteArray bytes;
        auto word=[&](std::uint32_t v){for(unsigned i=0;i<4;++i)bytes.append(char(v>>(i*8)));};
        for(const auto& r:a.records){word(r.opcode);word(static_cast<std::uint32_t>(r.argument));for(auto v:r.metadata)word(v);}
        QJsonArray starts;for(auto v:a.starts)starts.append(qint64(v));
        files.append(QJsonObject{{"path",path},{"decoded",true},{"version",int(a.version)},{"opaque_header",qint64(a.opaqueHeader)},
            {"starts",starts},{"records",qint64(a.records.size())},
            {"sprite_name_hex",QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(a.spriteName.data()),20).toHex())},
            {"records_sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"files",files}}).toJson().constData();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
