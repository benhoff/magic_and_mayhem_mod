#include "drawing_session.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <set>
#include <iostream>

namespace {
using namespace mnm::legacy;
QByteArray hash(const QByteArray &b){return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
void fields(const QJsonObject &o,std::initializer_list<const char *> names){
    QStringList allowed;for(auto name:names)allowed.append(QLatin1String(name));
    if(o.size()!=allowed.size())throw std::invalid_argument("Missing or extra native session field");
    for(auto i=o.begin();i!=o.end();++i)if(!allowed.contains(i.key()))throw std::invalid_argument("Unknown native session field");
}
unsigned number(const QJsonObject &o,const char *key,unsigned minimum,unsigned maximum){
    const auto v=o.value(QLatin1String(key));const auto n=v.toDouble(-1);
    if(!v.isDouble()||!std::isfinite(n)||n<minimum||n>maximum||n!=std::floor(n))
        throw std::invalid_argument("Invalid bounded native session number");
    return unsigned(n);
}
QJsonObject object(const QJsonValue &v){if(!v.isObject())throw std::invalid_argument("Expected native session object");return v.toObject();}
QJsonArray array(const QJsonValue &v){if(!v.isArray())throw std::invalid_argument("Expected native session array");return v.toArray();}
QString string(const QJsonValue &v){if(!v.isString()||v.toString().isEmpty())throw std::invalid_argument("Expected nonempty native session string");return v.toString();}
QByteArray fingerprint(const QJsonValue &v){
    const auto s=string(v).toLatin1();
    if(s.size()!=64)throw std::invalid_argument("Invalid native input hash");
    for(char c:s)if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))throw std::invalid_argument("Invalid native input hash");
    return s;
}
QByteArray load(const mnm::assets::AssetStore &store,const QString &name,std::int64_t limit){
    auto opened=store.open(name.toStdString());
    if(auto *e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);
    auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
    auto read=mnm::assets::readWhole(*file,limit);
    if(auto *e=std::get_if<mnm::assets::Error>(&read))throw std::runtime_error(e->detail);
    const auto bytes=std::get<std::vector<std::uint8_t>>(std::move(read));
    return {reinterpret_cast<const char *>(bytes.data()),qsizetype(bytes.size())};
}
void save(const QString &path,const QByteArray &bytes){
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)||file.write(bytes)!=bytes.size())
        throw std::runtime_error("Cannot create native session output");
}
DrawingBoundary boundary(const QJsonObject &o){
    fields(o,{"sequence","queues","canvas","width","height"});
    return {number(o,"sequence",1,1048576),number(o,"queues",0,120),number(o,"canvas",1,262144),
            number(o,"width",1,2048),number(o,"height",1,2048)};
}
DrawingStage stage(const QString &s){
    if(s=="scene")return DrawingStage::Scene;
    if(s=="menu")return DrawingStage::Menu;
    if(s=="loading")return DrawingStage::Loading;
    if(s=="world")return DrawingStage::World;
    if(s=="results")return DrawingStage::Results;
    throw std::invalid_argument("Unknown native session stage");
}
struct Segment {
    unsigned ordinal;QString label,path;QByteArray digest;
    DrawingStage stage;CanvasProducerStream stream;std::vector<DrawingBoundary> frames;
};
}
int main(int argc,char **argv){
    QGuiApplication app(argc,argv);
    const bool snapshots=argc==4&&std::string(argv[3])=="--source-snapshots";
    if(argc!=3&&!snapshots){std::cerr<<"Usage: mnm-drawing-session-replay INPUT_DIRECTORY NEW_OUTPUT_DIRECTORY [--source-snapshots]\n";return 2;}
    QJsonObject report{{"schema",1},{"success",false},{"original_execution",false},
        {"original_oracles_read",false},{"original_pixels_used_as_native_inputs",false},
        {"physical_presentation",false},{"live_replacement",false}};
    const QDir output(QString::fromLocal8Bit(argv[2]));bool ownsOutput=false;
    try{
        // Preflight owned, bounded input before creating any outputs.
        auto root=mnm::assets::AssetStore::create(std::filesystem::path(argv[1]));
        if(auto *e=std::get_if<mnm::assets::Error>(&root))throw std::runtime_error(e->detail);
        const auto store=std::get<mnm::assets::AssetStore>(std::move(root));
        const auto declaration=load(store,"drawing-session.json",4*1024*1024);
        const auto doc=QJsonDocument::fromJson(declaration);
        if(!doc.isObject())throw std::invalid_argument("Invalid native session declaration");
        const auto config=doc.object();fields(config,{"schema","session","assets","segments"});
        if(number(config,"schema",1,1)!=1)throw std::invalid_argument("Unsupported native session schema");
        const auto token=string(config.value("session"));bool valid=false;
        const auto session=token.toULongLong(&valid);
        if(!valid||!session||QString::number(session)!=token)throw std::invalid_argument("Invalid exact native session token");
        const auto assets=object(config.value("assets"));
        if(assets.size()>512)throw std::invalid_argument("Native asset binding budget exceeded");
        std::uint64_t assetBytes=0;
        auto source=[&](const std::string &name){
            const auto key=QString::fromStdString(name);const auto expected=fingerprint(assets.value(key));
            const auto bytes=load(store,key,32*1024*1024);
            if(hash(bytes)!=expected)throw std::invalid_argument("Changed native source asset");
            return std::vector<std::uint8_t>(bytes.begin(),bytes.end());
        };
        for(auto i=assets.begin();i!=assets.end();++i){assetBytes+=source(i.key().toStdString()).size();
            if(assetBytes>512u*1024*1024)throw std::invalid_argument("Native aggregate asset budget exceeded");}
        // WorldResources indexes installed SPR files independently of the
        // producer callback. Account for that complete input surface too.
        auto checkCatalogue=[&]{
            for(const auto &entry:std::filesystem::recursive_directory_iterator(store.root())){
                if(!entry.is_regular_file())continue;
                auto extension=entry.path().extension().string();
                std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return char(std::tolower(c));});
                if(extension!=".spr")continue;
                const auto key=QString::fromStdString(entry.path().lexically_relative(store.root()).generic_string());
                if(!assets.contains(key))throw std::invalid_argument("Unbound native World catalogue asset");
                source(key.toStdString());
            }
        };
        checkCatalogue();
        const auto entries=array(config.value("segments"));
        if(entries.isEmpty()||entries.size()>64)throw std::invalid_argument("Native segment budget exceeded");
        std::vector<Segment> segments;unsigned records=0,frames=0;std::uint64_t inputBytes=0;
        for(const auto &entry:entries){
            const auto o=object(entry);fields(o,{"segment","stage","producer","frames"});
            const auto producer=object(o.value("producer"));fields(producer,{"path","sha256"});
            Segment s{};s.ordinal=number(o,"segment",1,64);s.label=string(o.value("stage"));s.stage=stage(s.label);
            if(s.ordinal!=segments.size()+1)throw std::invalid_argument("Unordered native segments");
            s.path=string(producer.value("path"));s.digest=fingerprint(producer.value("sha256"));
            const auto raw=load(store,s.path,512u*1024*1024);inputBytes+=raw.size();
            if(inputBytes>1024u*1024*1024||hash(raw)!=s.digest)throw std::invalid_argument("Changed or oversized native producer input");
            s.stream=decodeCanvasProducers({raw.begin(),raw.end()},false);
            if(s.stream.operations.empty()||s.stream.operations.size()>1048576-records)throw std::invalid_argument("Native record budget exceeded");
            records+=s.stream.operations.size();
            for(const auto &c:s.stream.operations)if(c.fields[2]==9&&c.fields[21])
                throw std::invalid_argument("Native raster input contains observed result");
            unsigned previousSequence=0,previousQueues=0;std::set<unsigned> snapshotCanvases;
            for(const auto &f:array(o.value("frames"))){
                const auto b=boundary(object(f));
                if(++frames>8192||b.sequence<previousSequence||(!snapshots&&b.sequence==previousSequence)||b.queues<previousQueues||b.sequence>s.stream.operations.size())
                    throw std::invalid_argument("Unordered native frame boundaries");
                if(b.sequence!=previousSequence)snapshotCanvases.clear();
                if(!snapshotCanvases.insert(b.canvas).second)throw std::invalid_argument("Duplicate native source snapshot");
                previousSequence=b.sequence;previousQueues=b.queues;s.frames.push_back(b);
            }
            // A frame-less nonterminal segment represents explicit cancellation.
            // Otherwise no unpresented tail is silently discarded.
            if((!s.frames.empty()&&s.frames.back().sequence!=s.stream.operations.size())||
               (s.ordinal==unsigned(entries.size())&&s.frames.empty()))
                throw std::invalid_argument("Native segment lacks complete terminal frame");
            segments.push_back(std::move(s));
        }
        if(output.exists()||!QDir().mkpath(output.absolutePath()))throw std::invalid_argument("Native session output must be new");
        ownsOutput=true;report["terminal_state"]="failed";report["remaining_surfaces"]=QJsonValue::Null;
        mnm::render::GlBlitter renderer;
        try{
            if(snapshots){
                QJsonArray outputs;unsigned snapshotCount=0;
                for(const auto &s:segments){
                    DrawingScene scene(renderer,store,source,true);std::size_t next=0;
                    for(const auto &c:s.stream.operations){
                        report["source_sequence"]=int(c.fields[1]);report["source_operation"]=int(c.fields[2]);report["source_canvas"]=int(c.fields[3]);report["source_phase"]="apply";
                        scene.apply(c);
                        while(next<s.frames.size()&&s.frames[next].sequence==c.fields[1]){
                            const auto &b=s.frames[next++];
                            if(b.queues!=scene.queues())throw std::invalid_argument("Source snapshot queue changed");
                            report["source_phase"]="snapshot";report["source_canvas"]=int(b.canvas);
                            const auto image=scene.sourceSnapshot(b.canvas);
                            if(image.width!=int(b.width)||image.height!=int(b.height))throw std::invalid_argument("Source snapshot extent changed");
                            QByteArray bytes;bytes.reserve(qsizetype(image.pixels.size()*2));
                            for(auto p:image.pixels){bytes.append(char(p));bytes.append(char(p>>8));}
                            const auto name=QString("source-%1.565").arg(++snapshotCount,6,10,QChar('0'));
                            save(output.filePath(name),bytes);
                            outputs.append(QJsonObject{{"sequence",int(b.sequence)},{"canvas",int(b.canvas)},
                                {"width",int(b.width)},{"height",int(b.height)},
                                {"path",name},{"sha256",QString::fromLatin1(hash(bytes))}});
                        }
                    }
                    if(next!=s.frames.size())throw std::runtime_error("Native source snapshot not reached");
                }
                if(renderer.stats().surfaces)throw std::runtime_error("Native source snapshot resources remain");
                if(load(store,"drawing-session.json",4*1024*1024)!=declaration)throw std::runtime_error("Native declaration changed");
                for(const auto &s:segments)if(hash(load(store,s.path,512u*1024*1024))!=s.digest)throw std::runtime_error("Native producer changed");
                for(auto i=assets.begin();i!=assets.end();++i)source(i.key().toStdString());
                checkCatalogue();report["frames"]=outputs;report["native_source_snapshots"]=true;
                report["records"]=int(records);report["terminal_state"]="source_snapshots_complete";
                report["driver"]=QString::fromStdString(renderer.driver().renderer);report["success"]=true;
            }else{
            DrawingSession native(renderer,store,source);native.begin(session);QJsonArray outputs;
            for(const auto &s:segments){
                native.transition(session,s.ordinal,s.stage);std::size_t next=0;
                for(const auto &c:s.stream.operations){
                    native.apply(session,s.ordinal,c);
                    if(next<s.frames.size()&&s.frames[next].sequence==c.fields[1]){
                        const auto identity=native.publish(session,s.ordinal,native.frames()+1,s.frames[next++]);
                        const auto image=native.read(identity);native.presentGpu(identity);
                        QByteArray bytes;bytes.reserve(qsizetype(image.pixels.size()*2));
                        for(auto p:image.pixels){bytes.append(char(p));bytes.append(char(p>>8));}
                        const auto name=QString("frame-%1.565").arg(identity.presentation,6,10,QChar('0'));
                        save(output.filePath(name),bytes);
                        const auto &b=identity.source;
                        outputs.append(QJsonObject{{"session",token},{"segment",int(identity.segment)},
                            {"stage",s.label},{"presentation",int(identity.presentation)},{"sequence",int(b.sequence)},
                            {"queues",int(b.queues)},{"canvas",int(b.canvas)},{"width",int(b.width)},
                            {"height",int(b.height)},{"path",name},{"sha256",QString::fromLatin1(hash(bytes))}});
                    }
                }
                if(next!=s.frames.size())throw std::runtime_error("Native frame boundary not reached");
            }
            native.finish(session);report["terminal_state"]="finished";
            report["session"]=token;report["input_sha256"]=QString::fromLatin1(hash(declaration));
            report["segments"]=int(native.segments());report["records"]=int(native.records());
            report["frames"]=outputs;report["frames_published"]=int(native.frames());native.stop();
            if(renderer.stats().surfaces)throw std::runtime_error("Native session surfaces remain after shutdown");
            if(load(store,"drawing-session.json",4*1024*1024)!=declaration)throw std::runtime_error("Native declaration changed during execution");
            for(const auto &s:segments)if(hash(load(store,s.path,512u*1024*1024))!=s.digest)throw std::runtime_error("Native producer changed during execution");
            for(auto i=assets.begin();i!=assets.end();++i)source(i.key().toStdString());
            checkCatalogue();
            report["native_gpu_presentation_ready"]=true;
            report["driver"]=QString::fromStdString(renderer.driver().renderer);report["success"]=true;
            }
        }catch(...){report["remaining_surfaces"]=double(renderer.stats().surfaces);report["terminal_state"]="failed";throw;}
        report["remaining_surfaces"]=double(renderer.stats().surfaces);
    }catch(const std::exception &e){report["error"]=QString::fromUtf8(e.what());}
    if(ownsOutput)try{save(output.filePath("session-report.json"),QJsonDocument(report).toJson());}
        catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()<<'\n';
    return report.value("success").toBool()?0:1;
}
