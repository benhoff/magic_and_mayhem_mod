#include "resource-fixtures.hpp"
#include <QCoreApplication>
#include <iostream>
#include <thread>
using namespace mnm::assets;
using namespace resource_test;
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir directory;
    const auto root=std::filesystem::path(directory.path().toStdString());populate(root);
    const ResourceId id{ResourceKind::ui,"prepared"};ResourceManager manager(store(root));manager.bind(id,spr());
    auto request=manager.request(id);std::optional<PreparedResource> result;
    std::thread worker([&]{result.emplace(prepareResource(request));});worker.join();
    require(manager.stats().loads==0&&!manager.isResident(id),"Worker published manager state");
    require(result->sourceImage()==sprite(),"Preparation lost the verified encoded input");
    std::filesystem::remove(root/"body.spr");
    const auto& adopted=manager.adopt(std::move(*result));
    require(adopted.frameCount()==3&&adopted.revision>0&&manager.stats().loads==1,"Owned adoption reopened a removed asset");
    require(&manager.resident(id)==&adopted,"Resident-only access differs");
    write(root,"body.spr",sprite());
    const auto revision=adopted.revision;
    manager.unload(id);rejected([&]{manager.resident(id);});
    auto stale=prepareResource(manager.request(id));manager.unload(id);
    rejected([&]{manager.adopt(std::move(stale));});
    require(manager.stats().residentResources==0&&manager.stats().decodedBytes==0,"Stale adoption mutated residency");
    auto fresh=prepareResource(manager.request(id));
    ResourceManager foreign(store(root));foreign.bind(id,spr());rejected([&]{foreign.adopt(std::move(fresh));});
    require(manager.adopt(std::move(fresh)).revision>revision,"Failed foreign adoption consumed the valid result");
    manager.unloadAll();auto pending=prepareResource(manager.request(id));manager.unloadAll();
    rejected([&]{manager.adopt(std::move(pending));});
    std::size_t checkpoints=0;rejected([&]{prepareResource(manager.request(id),[&]{if(++checkpoints==3)throw std::runtime_error("Cancelled read");});});
    require(checkpoints==3&&!manager.isResident(id),"Cancelled preparation was adopted");
    ResourceLimits limits;limits.residentResources=1;ResourceManager bounded(store(root),limits);
    const ResourceId other{ResourceKind::ui,"other"};bounded.bind(id,spr());bounded.bind(other,spr());
    auto first=prepareResource(bounded.request(id));auto second=prepareResource(bounded.request(other));
    bounded.adopt(std::move(first));const auto bytes=bounded.stats().decodedBytes;
    rejected([&]{bounded.adopt(std::move(second));});
    require(bounded.stats().loads==1&&bounded.stats().decodedBytes==bytes,"Failed adoption changed budgets");
    bounded.unload(id);bounded.adopt(std::move(second));
    ResourceManager paired(store(root));auto recipe=spr();recipe.animation="body.ani";
    const ResourceId creature{ResourceKind::creature,"worker"};paired.bind(creature,recipe);
    auto pairRequest=paired.request(creature);std::thread pairWorker([&]{result.emplace(prepareResource(pairRequest));});pairWorker.join();
    require(paired.adopt(std::move(*result)).animation->records[0].argument==0,"Worker lost paired animation admission");
    const std::pair<ResourceImageFormat,const char*> formats[]={{ResourceImageFormat::bmp,"ui.bmp"},{ResourceImageFormat::pcx,"ui.pcx"},{ResourceImageFormat::jpeg,"ui.jpg"}};
    for(const auto& format:formats){const ResourceId bitmap{ResourceKind::ui,"bitmap/"+std::to_string(unsigned(format.first))};paired.bind(bitmap,{format.first,format.second,{},{},{}});
        auto bitmapRequest=paired.request(bitmap);std::thread bitmapWorker([&]{result.emplace(prepareResource(bitmapRequest));});bitmapWorker.join();
        require(paired.adopt(std::move(*result)).frameCount()==1,"Worker bitmap preparation failed");}
    std::cout<<"Worker preparation, file-free adoption, binding invalidation, foreign/stale refusal, cancellation and atomic budgets pass\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
