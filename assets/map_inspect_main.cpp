#include "map.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);if(argc<3)throw std::runtime_error("Usage: mnm-map-inspect ROOT MAP_PATH [MAP_PATH ...]");
 auto configured=AssetStore::create(argv[1]);if(auto* e=std::get_if<Error>(&configured))throw std::runtime_error(e->detail);auto store=std::get<AssetStore>(std::move(configured));
 for(int arg=2;arg<argc;++arg){auto opened=store.open(argv[arg]);if(auto* e=std::get_if<Error>(&opened))throw std::runtime_error(e->detail);auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadMap(*file);if(auto* e=std::get_if<PersistenceError>(&decoded))throw std::runtime_error(e->detail);const auto map=std::get<MapAsset>(std::move(decoded));
  QByteArray bytes;const auto word=[&](std::uint32_t value,unsigned n){for(unsigned i=0;i<n;++i)bytes.append(char(value>>(8*i)));};word(6,4);word(map.width,4);word(map.height,4);word(map.layers,4);word(map.width*map.height,4);word(map.cells.size(),4);
  QJsonArray metadata;for(auto value:map.metadata){word(value,4);metadata.append(qint64(value));}
  for(const auto& cell:map.cells){word(cell.definition,2);for(auto value:cell.references)word(value,2);word(cell.flags8,2);word(cell.flags10,2);}
  const auto hash=QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex();
  std::cout<<QJsonDocument(QJsonObject{{"path",QString::fromLocal8Bit(argv[arg])},{"width",qint64(map.width)},{"height",qint64(map.height)},{"layers",qint64(map.layers)},{"cells",qint64(map.cells.size())},{"metadata",metadata},{"decoded_sha256",QString::fromLatin1(hash)}}).toJson(QJsonDocument::Compact).constData()<<'\n';
 }
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
