#include "realm_viewer_assets.hpp"
#include "asset_file.hpp"
#include "fp.hpp"
#include <stdexcept>
namespace mnm::ui {
std::array<QVector<RealmCatalogRegion>,3> loadRealmCatalog(const QString& root) {
    auto created=assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if(auto* error=std::get_if<assets::Error>(&created))throw std::runtime_error(error->detail);
    const auto& store=std::get<assets::AssetStore>(created);
    auto open=[&](const QString& path){auto result=store.open(path.toStdString());if(auto* error=std::get_if<assets::Error>(&result))throw std::runtime_error(path.toStdString()+": "+error->detail);return std::move(std::get<std::unique_ptr<assets::AssetFile>>(result));};
    const std::array<QString,3> realms{"Celtic","Greek","Medieval"};const std::array<int,3> counts{8,12,16};std::array<QVector<RealmCatalogRegion>,3> catalog;
    for(int r=0;r<3;++r)for(int n=1;n<=counts[r];++n){
        const auto prefix="Interface/RealmViewer/Generic/"+realms[r];const auto suffix=QString("_%1").arg(n,2,10,QLatin1Char('0'));
        auto file=open(prefix+"_Region"+suffix+".txt");auto bytes=assets::readWhole(*file,1024);
        if(auto* error=std::get_if<assets::Error>(&bytes))throw std::runtime_error(error->detail);
        const auto& data=std::get<std::vector<std::uint8_t>>(bytes);QString name=QString::fromLatin1(reinterpret_cast<const char*>(data.data()),qsizetype(data.size())).trimmed();
        if(name.isEmpty()||name.size()>256)throw std::runtime_error("Invalid Realm Viewer region name");
        for(const auto c:name)if(c.unicode()<32||c.unicode()==127)throw std::runtime_error("Invalid Realm Viewer region name character");
        file=open(prefix+"_FlagPath"+suffix+".FP");assets::FpLimits limits;limits.inputBytes=64*1024;limits.points=4096;limits.decodedBytes=32*1024;
        auto fp=assets::loadFp(*file,limits);if(auto* error=std::get_if<assets::FpError>(&fp))throw std::runtime_error(error->detail);
        // First stored flag position is an explicit native preview choice. No
        // path animation, randomized slot reservation or ownership inference.
        const auto pos=std::get<assets::FpAsset>(fp).flagPositions[0];
        if(pos.x<0||pos.x>752||pos.y<0||pos.y>548)throw std::runtime_error("Realm Viewer flag position outside canvas");
        catalog[r].push_back({n,name,{pos.x,qMax(0,pos.y-52)}});
    }
    return catalog;
}
}
