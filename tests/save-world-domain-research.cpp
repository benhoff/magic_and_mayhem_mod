// Read-only research: distinguish native safety limits from byte-grammar gaps.
#include "../assets/persistence.hpp"
#include "../assets/save_world.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
template<class T>T checked(std::variant<T,Error> value){
    if(auto* e=std::get_if<Error>(&value))throw std::runtime_error(e->detail);
    return std::get<T>(std::move(value));
}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);
    if(argc!=3)throw std::runtime_error("Expected game root and save path");
    auto store=checked(AssetStore::create(argv[1]));auto file=checked(store.open(argv[2]));
    auto value=loadSavedGame(*file);
    if(auto* e=std::get_if<PersistenceError>(&value))throw std::runtime_error(e->detail);
    auto saved=std::get<SavedGame>(std::move(value));
    QJsonObject out{{"save_version",int(saved.version)},{"world_marker",int(saved.worldMarker)},
        {"world_bytes",qint64(saved.worldTail.size())},{"realm",QString::fromStdString(saved.realm.name)},
        {"default_record_limit",65536},{"expanded_record_limit",524288}};
    for(unsigned phase=0;phase<2;++phase){
        WorldLimits limits;if(phase)limits.records=524288;
        auto decoded=decodeSavedWorld(saved,limits);QJsonObject result;
        if(auto* error=std::get_if<PersistenceError>(&decoded)){
            result={{"parsed",false},{"error",QString::fromStdString(error->detail)},{"offset",qint64(error->offset)},{"error_code",int(error->code)}};
        }else{
            const auto& world=std::get<SavedWorld>(decoded);QJsonArray dimensions;
            for(const auto& block:world.blocks)if(block.name=="descriptor"){
                if(block.offset+24>world.raw.size())throw std::runtime_error("Descriptor extent");
                for(unsigned i=0;i<6;++i){const auto* p=world.raw.data()+block.offset+i*4;dimensions.append(qint64(p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24));}
                break;
            }
            result={{"parsed",true},{"map_path",QString::fromLatin1(world.mapPath.data(),qsizetype(world.mapPath.size()))},
                {"counter",qint64(world.counter)},{"blocks",qint64(world.blocks.size())},{"descriptor_first_six_words",dimensions}};
        }
        out[phase?"expanded":"default"]=result;
    }
    std::cout<<QJsonDocument(out).toJson().constData();return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
