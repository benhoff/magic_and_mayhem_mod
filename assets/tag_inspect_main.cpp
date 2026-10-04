#include "tag.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-tag-inspect ROOT PATH.tag");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadTag(*file);file.reset();
    if(const auto* error=std::get_if<TagError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& asset=std::get<TagAsset>(decoded);QJsonArray entries;
    for(const auto& entry:asset.entries) {
        const auto name=tagEntryName(entry);
        const QByteArray raw(reinterpret_cast<const char*>(entry.nameBytes.data()),8);
        entries.append(QJsonObject{{"name",QString::fromLatin1(name.data(),static_cast<qsizetype>(name.size()))},
            {"name_bytes_hex",QString::fromLatin1(raw.toHex())},{"occurrence",qint64(entry.occurrence)}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(asset.sourceBytes)},
        {"record_count",qint64(asset.entries.size())},{"entries",entries}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
