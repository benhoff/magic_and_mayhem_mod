#pragma once
#include "../../renderer/blit.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <cmath>
#include <functional>
#include <map>
#include <set>

// Diagnostic application adapter. Receives only native-derived RGB565 source
// snapshots and an input-only transfer plan. Original comparison planes stay
// outside this process. The renderer owns every composed pixel and history copy.
class RecordedComposition final {
public:
    using Load=std::function<QByteArray(const QString&,std::uint64_t)>;
    RecordedComposition(mnm::render::GlBlitter& renderer,const QJsonObject& plan,Load load)
        :gl_(renderer),load_(std::move(load)) {
        fields(plan,{"schema","width","height","sources","frames"});
        require(integer(plan["schema"],1,1)==1,"Composition schema");
        width_=integer(plan["width"],1,2048);height_=integer(plan["height"],1,2048);
        require(plan["sources"].isArray() && plan["frames"].isArray(),"Composition lists");
        frames_=plan["frames"].toArray();const auto sources=plan["sources"].toArray();
        require(!frames_.empty() && frames_.size()<=8192 && sources.size()<=8192,"Composition frame/source budget");
        std::uint64_t total=0;
        for(const auto& v:sources){
            require(v.isObject(),"Composition source object");const auto s=v.toObject();fields(s,{"id","path","sha256","width","height"});
            const auto id=integer(s["id"],1,8192);require(!sources_.count(id),"Duplicate composition source");
            Source source{s["path"].toString(),s["sha256"].toString().toLatin1(),integer(s["width"],1,2048),integer(s["height"],1,2048)};
            require(!source.path.isEmpty() && source.hash.size()==64,"Composition source identity");
            const auto bytes=load_(source.path,std::uint64_t(source.width)*source.height*2);total+=std::uint64_t(bytes.size());
            require(bytes.size()==qsizetype(source.width)*source.height*2 && digest(bytes)==source.hash && total<=2ull*1024*1024*1024,"Composition source hash/byte budget");
            sources_.emplace(id,std::move(source));
        }
        std::set<unsigned> used;
        for(qsizetype i=0;i<frames_.size();++i){
            require(frames_[i].isObject(),"Composition frame object");const auto f=frames_[i].toObject();
            const auto index=integer(f["index"],1,8192);require(index==unsigned(i)+1,"Composition presentation sequence");
            if(f["kind"]=="fill"){
                fields(f,{"index","kind","colour"});integer(f["colour"],0,65535);continue;
            }
            fields(f,{"index","kind","source_kind","source","view_rect","source_rect","destination_rect","key"});
            require(f["kind"]=="copy","Unknown composition operation");
            const auto source=integer(f["source"],1,8192);unsigned sw=width_,sh=height_;
            if(f["source_kind"]=="snapshot"){
                require(sources_.count(source),"Missing composition source");const auto& s=sources_.at(source);sw=s.width;sh=s.height;
                sourceLast_[source]=index;used.insert(source);
            }else{
                require(f["source_kind"]=="history" && source<index,"Missing/future composition history");historyLast_[source]=index;
            }
            const auto view=rect(f["view_rect"],sw,sh),src=rect(f["source_rect"],sw,sh),dst=rect(f["destination_rect"],width_,height_);
            require(src.left>=view.left && src.top>=view.top && src.right<=view.right && src.bottom<=view.bottom &&
                src.right-src.left==dst.right-dst.left && src.bottom-src.top==dst.bottom-dst.top,"Composition rectangle contract");
            if(!f["key"].isNull())integer(f["key"],0,65535);
        }
        require(used.size()==sources_.size(),"Unused composition source");
    }
    ~RecordedComposition(){
        for(const auto& s:cached_)gl_.destroy(s.second);
        for(const auto& s:history_)gl_.destroy(s.second);
        if(primary_)gl_.destroy(primary_);
    }
    unsigned width() const{return width_;}unsigned height() const{return height_;}
    QJsonArray frames() const{return frames_;}
    mnm::render::SurfaceId next(unsigned index){
        require(index==next_ && index<=unsigned(frames_.size()),"Composition next frame identity");
        if(!primary_){primary_=blank(width_,height_);gl_.invalidateContents(primary_);}
        const auto f=frames_[int(index)-1].toObject();
        if(f["kind"]=="fill")gl_.update(primary_,0,0,{int(width_),int(height_),std::vector<std::uint32_t>(std::size_t(width_)*height_,integer(f["colour"],0,65535))});
        else{
            const auto id=integer(f["source"],1,8192);mnm::render::SurfaceId source=0;
            if(f["source_kind"]=="history"){require(history_.count(id),"Retired composition history");source=history_.at(id);}
            else{
                if(!cached_.count(id)){
                    const auto& s=sources_.at(id);const auto bytes=load_(s.path,std::uint64_t(s.width)*s.height*2);
                    require(digest(bytes)==s.hash,"Composition source changed during execution");
                    mnm::render::Image image{int(s.width),int(s.height),{}};image.pixels.reserve(std::size_t(s.width)*s.height);
                    for(qsizetype p=0;p<bytes.size();p+=2)image.pixels.push_back(std::uint8_t(bytes[p])|(std::uint32_t(std::uint8_t(bytes[p+1]))<<8));
                    budget(image.pixels.size());cached_[id]=gl_.create(image,format());
                }
                source=cached_.at(id);
            }
            const auto src=rect(f["source_rect"],2048,2048),dst=rect(f["destination_rect"],width_,height_);
            const auto key=f["key"].isNull()?std::optional<std::uint32_t>{}:integer(f["key"],0,65535);
            gl_.copy(source,primary_,src,dst.left,dst.top,key);
        }
        if(historyLast_.count(index)){
            const auto saved=blank(width_,height_);gl_.copy(primary_,saved,{0,0,int(width_),int(height_)},0,0);history_[index]=saved;
        }
        for(auto it=cached_.begin();it!=cached_.end();)if(sourceLast_.at(it->first)==index){gl_.destroy(it->second);it=cached_.erase(it);}else ++it;
        for(auto it=history_.begin();it!=history_.end();)if(historyLast_.at(it->first)==index){gl_.destroy(it->second);it=history_.erase(it);}else ++it;
        ++next_;return primary_;
    }
    void verify() const{
        for(const auto& item:sources_){const auto& s=item.second;require(digest(load_(s.path,std::uint64_t(s.width)*s.height*2))==s.hash,"Composition source changed after execution");}
    }
private:
    struct Source{QString path;QByteArray hash;unsigned width,height;};
    static mnm::render::PixelFormat format(){return {16,{0xf800,0x7e0,31}};}
    static QByteArray digest(const QByteArray& b){return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
    static void require(bool ok,const char* why){if(!ok)throw std::invalid_argument(why);}
    static void fields(const QJsonObject& o,std::initializer_list<const char*> names){
        require(o.size()==int(names.size()),"Composition missing/extra field");for(const auto* n:names)require(o.contains(QLatin1String(n)),"Composition field identity");
    }
    static unsigned integer(const QJsonValue& v,unsigned low,unsigned high){
        const auto n=v.toDouble(-1);require(v.isDouble() && std::isfinite(n) && n>=low && n<=high && n==std::floor(n),"Composition number");return unsigned(n);
    }
    static mnm::render::Rect rect(const QJsonValue& v,unsigned width,unsigned height){
        require(v.isArray() && v.toArray().size()==4,"Composition rectangle");const auto a=v.toArray();
        const auto x=integer(a[0],0,width),y=integer(a[1],0,height),r=integer(a[2],0,width),b=integer(a[3],0,height);
        require(x<r && y<b,"Composition rectangle extent");return {int(x),int(y),int(r),int(b)};
    }
    void budget(std::size_t pixels) const{require(gl_.stats().pixels+pixels<=16777216 && gl_.stats().surfaces<64,"Composition live resource budget");}
    mnm::render::SurfaceId blank(unsigned w,unsigned h){budget(std::size_t(w)*h);return gl_.create({int(w),int(h),std::vector<std::uint32_t>(std::size_t(w)*h)},format());}
    mnm::render::GlBlitter& gl_;Load load_;QJsonArray frames_;unsigned width_=0,height_=0,next_=1;
    mnm::render::SurfaceId primary_=0;std::map<unsigned,Source> sources_;std::map<unsigned,unsigned> sourceLast_,historyLast_;
    std::map<unsigned,mnm::render::SurfaceId> cached_,history_;
};
