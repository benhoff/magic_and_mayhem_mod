#include "realm_viewer_visuals.hpp"
#include "asset_file.hpp"
#include "pcx.hpp"
#include "fp.hpp"
#include "animation.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::ui {
bool RealmRegionVisual::contains(const QPoint& p) const {
    return maskRect.contains(p)&&mask.constScanLine(p.y()-maskRect.y())[p.x()-maskRect.x()]!=0;
}
const MenuSpriteFrame& realmFlagFrame(const RealmFlagAnimation& clip,quint64 tick){
    if(clip.frames.isEmpty()||clip.duration<1)throw std::runtime_error("Empty Realm Viewer flag clip");
    auto at=clip.loop?tick%quint64(clip.duration):qMin(tick,quint64(clip.duration-1));
    for(const auto& f:clip.frames){if(at<quint64(f.ticks))return f.sprite;at-=f.ticks;}
    return clip.frames.last().sprite;
}
RealmViewerVisuals loadRealmViewerVisuals(const QString& root,const MenuSpriteSheet& flags){
    auto created=assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if(auto* error=std::get_if<assets::Error>(&created))throw std::runtime_error(error->detail);
    const auto& store=std::get<assets::AssetStore>(created);
    auto open=[&](const QString& path){auto opened=store.open(path.toStdString());if(auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(path.toStdString()+": "+error->detail);return std::move(std::get<std::unique_ptr<assets::AssetFile>>(opened));};
    auto pcx=[&](const QString& path){auto file=open(path);assets::PcxLimits limits;limits.inputBytes=2*1024*1024;limits.width=800;limits.height=600;limits.pixels=480000;auto result=assets::loadPcx(*file,limits);if(auto* error=std::get_if<assets::PcxError>(&result))throw std::runtime_error(path.toStdString()+": "+error->detail);auto image=std::move(std::get<assets::PcxImage>(result));if(image.width!=800||image.height!=600||image.originX||image.originY)throw std::runtime_error("Invalid Realm Viewer PCX canvas");return image;};
    RealmViewerVisuals result;const std::array<QString,3> names{"Celtic","Greek","Medieval"};const std::array<int,3> counts{8,12,16};
    for(int r=0;r<3;++r)for(int n=1;n<=counts[r];++n){
        const auto suffix=QString("_%1").arg(n,2,10,QLatin1Char('0'));const QString prefix="Interface/RealmViewer/";
        const auto shape=pcx(prefix+"Generic/"+names[r]+"_Silhouette"+suffix+".pcx");
        const auto border=pcx(prefix+"800x600/"+names[r]+"_Border"+suffix+".pcx");
        QImage mask(800,600,QImage::Format_Grayscale8),overlay(800,600,QImage::Format_ARGB32);if(mask.isNull()||overlay.isNull())throw std::runtime_error("Realm Viewer PCX allocation failed");
        QRect maskRect,borderRect;
        for(int y=0;y<600;++y){auto* m=mask.scanLine(y);auto* b=reinterpret_cast<QRgb*>(overlay.scanLine(y));for(int x=0;x<800;++x){const auto i=std::size_t(y)*800+x;const auto bit=shape.indices[i];if(bit>1)throw std::runtime_error("Realm Viewer silhouette is not binary");m[x]=bit;if(bit)maskRect|=QRect(x,y,1,1);const auto& rgb=border.palette[border.indices[i]];if(rgb==std::array<std::uint8_t,3>{0,0,255})b[x]=0;else{b[x]=qRgb(rgb[0],rgb[1],rgb[2]);borderRect|=QRect(x,y,1,1);}}}
        if(maskRect.isEmpty()||borderRect.isEmpty())throw std::runtime_error("Empty Realm Viewer region shape/border");
        RealmRegionVisual region;region.maskRect=maskRect;region.borderRect=borderRect;region.mask=mask.copy(maskRect);region.border=overlay.copy(borderRect);
        if(region.mask.isNull()||region.border.isNull())throw std::runtime_error("Realm Viewer crop allocation failed");
        auto file=open(prefix+"Generic/"+names[r]+"_FlagPath"+suffix+".FP");assets::FpLimits limits;limits.inputBytes=64*1024;limits.points=4096;limits.decodedBytes=32*1024;
        auto decoded=assets::loadFp(*file,limits);if(auto* error=std::get_if<assets::FpError>(&decoded))throw std::runtime_error(error->detail);
        const auto& fp=std::get<assets::FpAsset>(decoded);
        for(unsigned path=0;path<fp.pathCount;++path){QVector<QPoint> points;for(unsigned i=0;i<fp.pointCounts[path];++i){const auto& p=fp.points[fp.pointOffsets[path]+i];if(p.x<0||p.x>=800||p.y<0||p.y>=600)throw std::runtime_error("Realm Viewer path point outside canvas");points.push_back({p.x,p.y});}region.paths.push_back(std::move(points));}
        result.regions[r].push_back(std::move(region));
    }
    auto file=open("Interface/RealmViewer/Generic/flags.ani");assets::AnimationLimits limits;limits.inputBytes=64*1024;limits.records=1024;limits.sequences=128;
    auto decoded=assets::loadAnimation(*file,limits);if(auto* error=std::get_if<assets::AnimationError>(&decoded))throw std::runtime_error(error->detail);
    const auto& ani=std::get<assets::Animation>(decoded);const auto zero=std::find(ani.spriteName.begin(),ani.spriteName.end(),0);
    const auto spriteName=QString::fromLatin1(reinterpret_cast<const char*>(ani.spriteName.data()),int(zero-ani.spriteName.begin()));
    if(spriteName.compare("flags.spr",Qt::CaseInsensitive)!=0)throw std::runtime_error("Unexpected Realm Viewer ANI sprite association");
    result.animations.resize(int(ani.starts.size()-1));
    for(unsigned sequence=0;sequence+1<ani.starts.size();++sequence){
        // Only the green flag and first walking figure are native preview roles.
        // Unused raw ANI sequences retain loader ownership without playback.
        if(sequence!=0&&sequence!=50)continue;
        RealmFlagAnimation clip;int delay=0;const auto first=ani.starts[sequence],end=ani.starts[sequence+1];
        for(unsigned at=first;at<end;++at){const auto& record=ani.records[at];
            if(record.opcode==1){if(record.argument<0||record.argument>32)throw std::runtime_error("Realm Viewer animation delay outside limits");delay=record.argument;}
            else if(record.opcode==0){if(record.argument<0||record.argument>=flags.size())throw std::runtime_error("Realm Viewer ANI sprite index outside catalog");auto frame=flags[record.argument];const auto dx=std::int32_t(record.metadata[0]),dy=std::int32_t(record.metadata[1]);if(dx< -128||dx>128||dy< -128||dy>128)throw std::runtime_error("Realm Viewer ANI displacement outside limits");frame.origin-=QPoint(dx,dy);clip.frames.push_back({frame,delay+1});clip.duration+=delay+1;}
            else if(record.opcode==4){const auto target=std::int64_t(at)+record.argument;if(target!=first||at+2!=end||ani.records[at+1].opcode!=6)throw std::runtime_error("Unsupported Realm Viewer animation loop");clip.loop=true;}
            else if(record.opcode==6&&at+1!=end)throw std::runtime_error("Premature Realm Viewer animation stop");
            else if(record.opcode!=6)throw std::runtime_error("Unsupported Realm Viewer animation opcode");
        }
        if(clip.frames.isEmpty())throw std::runtime_error("Empty Realm Viewer flag sequence");
        result.animations[int(sequence)]=std::move(clip);
    }
    if(result.animations.isEmpty())throw std::runtime_error("Missing Realm Viewer flag sequences");
    return result;
}
}
