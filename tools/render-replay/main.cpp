#include "../../compat/legacy/world_rolling_session.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include "gl_viewport.hpp"
#include "profile_capture_codec.hpp"
#include <QApplication>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <QPushButton>
#include <QShortcut>
#include <QSpinBox>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <chrono>
#include <iostream>

using namespace mnm;
namespace {
void require(bool okay,const char* why){if(!okay)throw std::runtime_error(why);}
std::uint64_t word(const QByteArray& b,int at,int n=4){
    require(at>=0&&at+n<=b.size(),"Truncated replay word");std::uint64_t v=0;
    for(int i=0;i<n;++i)v|=std::uint64_t(std::uint8_t(b[at+i]))<<(8*i);
    return v;
}
QByteArray read(QFile& f,qint64 size){auto b=f.read(size);require(b.size()==size,"Truncated replay tape");return b;}
struct Event {std::uint64_t generation,chunk;unsigned kind;legacy::CanvasProducerStream stream;};
class Tape {
    QFile file_;
    std::vector<qint64> offsets_;
    std::unique_ptr<mnm::profile::Decoder> decoder_;
    unsigned decodedNext_=0;
public:
    unsigned worlds=0;
    explicit Tape(const QString& path):file_(path){
        require(file_.open(QIODevice::ReadOnly)&&file_.size()<=513ll*1024*1024,"Replay tape unavailable or too large");
        require(read(file_,8)=="MNMREP02","Replay tape version");const auto h=read(file_,8);
        const auto count=word(h,0);worlds=unsigned(word(h,4));require(count&&count<=8192&&worlds&&worlds<=count,"Replay tape counts");
        std::uint64_t completed=0,chunks=0,raw=0;unsigned last=0;
        for(unsigned i=0;i<count;++i){
            offsets_.push_back(file_.pos());const auto r=read(file_,84);
            const auto generation=word(r,0,8),chunk=word(r,8,8),s=word(r,16),q=word(r,20),packed=word(r,24),kind=word(r,80);last=unsigned(kind);
            require(generation==completed+1&&generation<=8192&&s>=16&&s<=16777216,"Replay event order/extent");
            if(kind==0){require(chunk==++chunks&&!q,"Replay source-history order");}
            else{require((kind==1||kind==2)&&!chunk&&(kind==1?(q>=64&&q<=197184):q==0),"Replay World request");++completed;}
            const auto size=s+q;raw+=size;
            require(raw<=16ull*1024*1024*1024&&packed>=5&&packed<=size+size/128+1024,"Replay compressed extent");
            require(file_.pos()+qint64(packed)<=file_.size()&&file_.seek(file_.pos()+qint64(packed)),"Replay payload extent");
        }
        require(completed==worlds&&last!=0&&file_.pos()==file_.size(),"Replay incomplete prefix or trailing data");
        // Complete admission before creating renderer state. No captured pointer
        // is invoked; the existing owned-source decoder validates each packet.
        for(unsigned i=0;i<offsets_.size();++i)(void)event(i);
    }
    unsigned size()const{return unsigned(offsets_.size());}
    Event event(unsigned index){
        require(index<offsets_.size(),"Replay event index");
        if(!decoder_||decodedNext_!=index){decoder_=std::make_unique<mnm::profile::Decoder>();decodedNext_=0;}
        QByteArray h,body;
        for(;decodedNext_<=index;++decodedNext_){
            require(file_.seek(offsets_[decodedNext_]),"Replay event position");h=read(file_,84);const auto packed=read(file_,qint64(word(h,24)));
            body=decoder_->decode(h.left(80),packed,unsigned(word(h,16)+word(h,20)));
        }
        const auto source=unsigned(word(h,16));std::vector<std::uint8_t> bytes(body.begin(),body.begin()+source);const auto kind=unsigned(word(h,80));
        auto stream=kind==2?legacy::decodeRollingQueue(bytes):legacy::decodeCanvasProducers(bytes,false);
        if(kind==1){std::vector<std::uint8_t> query(body.begin()+source,body.end());stream=legacy::prepareRollingHandoff(stream,0,query,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false).stream;}
        return {word(h,0,8),word(h,8,8),kind,std::move(stream)};
    }
};
class Engine {
public:
    render::GlBlitter renderer;
    legacy::WorldRollingSession world;
    explicit Engine(const assets::AssetStore& store,QOpenGLContext* share=nullptr):renderer(share),world(renderer,store,false,true,true,true){}
    render::GpuFrame consume(const Event& e){
        const auto before=renderer.stats();render::GpuFrame frame;
        if(e.kind==0)world.consumeSource(e.stream);else frame=world.consumeResident(e.generation,e.stream);
        require(renderer.stats().nativeReadbacks==before.nativeReadbacks&&renderer.stats().rgbaReadbacks==before.rgbaReadbacks&&!world.worldReadbacks(),"Ordinary replay read back pixels");
        return frame;
    }
    void synchronize(){renderer.batch([]{
        auto* gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
        require(gl,"Replay completion context");gl->glFinish();
    });}
};
QString pixels(const render::Image& image){
    QCryptographicHash hash(QCryptographicHash::Sha256);QByteArray row(8+qsizetype(image.pixels.size())*4,0);
    const auto put=[&](qsizetype at,std::uint32_t value){for(unsigned k=0;k<4;++k)row[at+k]=char(value>>(8*k));};
    put(0,unsigned(image.width));put(4,unsigned(image.height));for(std::size_t i=0;i<image.pixels.size();++i)put(8+qsizetype(i)*4,image.pixels[i]);hash.addData(row);return QString::fromLatin1(hash.result().toHex());
}
QJsonObject run(Tape& tape,const assets::AssetStore& store,unsigned first,unsigned last,unsigned repeats,bool verify){
    QJsonArray runs;std::vector<QString> expected;QString driver;
    for(unsigned repeat=0;repeat<repeats;++repeat){
        QJsonArray frames;std::uint64_t diagnosticReads=0;double submitTotal=0;QString finalHash;
        {Engine engine(store);driver=QString::fromStdString(engine.renderer.driver().renderer);
            for(unsigned i=0;i<tape.size();++i){
                auto event=tape.event(i);if(event.generation>last)break;
                if(event.kind!=0&&event.generation==first)engine.synchronize();
                const auto before=engine.renderer.stats();const auto start=std::chrono::steady_clock::now();engine.consume(event);const auto submitted=std::chrono::steady_clock::now();
                if(event.kind==0||event.generation<first)continue;
                const double submit=std::chrono::duration<double,std::milli>(submitted-start).count();submitTotal+=submit;
                engine.synchronize();
                QJsonObject row{{"generation",double(event.generation)},{"native_submit_ms",submit},{"native_completion_ms",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()},{"draw_calls",double(engine.renderer.stats().drawCalls-before.drawCalls)},{"texture_upload_calls",double(engine.renderer.stats().textureUploadCalls-before.textureUploadCalls)}};
                if(verify||event.generation==last){
                    finalHash=pixels(engine.world.diagnosticReadResident(engine.renderer));++diagnosticReads;
                    row["completion_with_diagnostic_ms"]=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();row["sha256"]=finalHash;
                    const auto at=std::size_t(event.generation-first);
                    if(verify){if(!repeat)expected.push_back(finalHash);else if(at>=expected.size()||expected[at]!=finalHash)throw std::runtime_error("Deterministic replay differs at frame "+std::to_string(event.generation)+", actual "+finalHash.toStdString());}
                }
                frames.append(row);
            }
            require(engine.world.completed()==last,"Replay did not complete selected range");
        }
        runs.append(QJsonObject{{"frames",frames},{"native_submit_total_ms",submitTotal},{"diagnostic_readbacks",double(diagnosticReads)},{"final_sha256",finalHash}});
    }
    return {{"success",true},{"renderer",driver},{"first_frame",int(first)},{"last_frame",int(last)},{"history_restored_from",1},{"repeats",int(repeats)},{"pixel_repeat_comparison",verify},{"ordinary_readbacks",0},{"runs",runs},{"scope","Ordered native World replay; each repeat has fresh renderer/producer state. Native submission excludes decoding, I/O and diagnostic readback; native_completion_ms includes explicit serial GPU drain, not GPU busy time. completion_with_diagnostic additionally includes readback/hash work. Fixed input is not deterministic gameplay, original pixel equivalence, live FPS, Qt pacing, HUD/audio or scanout proof."}};
}
class Viewer final:public QWidget {
    Tape& tape_;const assets::AssetStore& store_;GlViewport* viewport_;QLabel* label_;QSpinBox* position_;QTimer timer_;QElapsedTimer clock_;
    std::unique_ptr<Engine> engine_;unsigned cursor_=0,current_=0,target_=1,last_;double period_,next_=0;bool playing_=true,closed_=false;unsigned smoke_,restarts_=0,diagnostics_=0;QString firstEnd_;
    void reset(){viewport_->setGpuFrame({});engine_.reset();engine_=std::make_unique<Engine>(store_,viewport_->context());cursor_=current_=0;}
    void seek(unsigned frame){playing_=false;target_=frame;viewport_->setGpuFrame({});if(engine_&&frame<=current_)reset();}
    void tick(){try{
        if(!viewport_->ready())return;
        if(!engine_)reset();
        if(!target_){if(!playing_||clock_.elapsed()<next_)return;if(current_==last_){playing_=false;return;}target_=current_+1;}
        const auto frameBegin=clock_.elapsed();auto event=tape_.event(cursor_++);auto frame=engine_->consume(event);
        if(event.kind==0)return;
        current_=unsigned(event.generation);
        label_->setText(QString("Frame %1 / %2%3").arg(current_).arg(last_).arg(current_<target_?" — seeking":""));position_->setValue(int(current_));
        if(current_>=target_){
            viewport_->setGpuFrame(std::move(frame),current_,current_);target_=0;next_=frameBegin+period_;
            if(smoke_&&current_==last_){
                const auto hash=pixels(engine_->world.diagnosticReadResident(engine_->renderer));++diagnostics_;
                if(smoke_>1&&!restarts_){firstEnd_=hash;++restarts_;seek(1);playing_=true;return;}
                require(smoke_<2||hash==firstEnd_,"Viewer backward seek changed final pixels");closed_=true;QTimer::singleShot(100,this,[this]{close();});
            }
        }
    }catch(const std::exception& e){timer_.stop();label_->setText(QString::fromUtf8(e.what()));std::cerr<<e.what()<<'\n';QApplication::exit(1);}}
public:
    Viewer(Tape& tape,const assets::AssetStore& store,unsigned first,unsigned last,double fps,unsigned smoke):tape_(tape),store_(store),last_(last),period_(1000/fps),smoke_(smoke){
        setWindowTitle("Magic & Mayhem — World renderer replay");auto* layout=new QVBoxLayout(this);viewport_=new GlViewport(this);layout->addWidget(viewport_,1);
        auto* controls=new QHBoxLayout;layout->addLayout(controls);auto* play=new QPushButton("Play / pause",this);auto* previous=new QPushButton("Previous",this);auto* next=new QPushButton("Next",this);auto* restart=new QPushButton("Restart",this);
        for(auto* button:{play,previous,next,restart})controls->addWidget(button);
        label_=new QLabel(this);controls->addWidget(label_);position_=new QSpinBox(this);position_->setRange(1,int(last));controls->addWidget(position_);
        const auto toggle=[this]{playing_=!playing_;next_=clock_.elapsed();};const auto back=[this]{seek(std::max(1u,current_>0?current_-1:1));};const auto forward=[this]{seek(std::min(last_,current_+1));};
        connect(play,&QPushButton::clicked,this,toggle);connect(previous,&QPushButton::clicked,this,back);connect(next,&QPushButton::clicked,this,forward);connect(restart,&QPushButton::clicked,this,[this]{seek(1);});connect(position_,&QSpinBox::editingFinished,this,[this]{seek(unsigned(position_->value()));});
        for(const auto& binding:std::initializer_list<std::pair<int,std::function<void()>>>{{Qt::Key_Space,toggle},{Qt::Key_Left,back},{Qt::Key_Right,forward},{Qt::Key_Home,[this]{seek(1);}}}){auto* shortcut=new QShortcut(QKeySequence(binding.first),this);connect(shortcut,&QShortcut::activated,this,binding.second);}
        target_=first;resize(900,720);clock_.start();connect(&timer_,&QTimer::timeout,this,[this]{tick();});timer_.start(1);
    }
    ~Viewer()override{timer_.stop();viewport_->setGpuFrame({});engine_.reset();}
    QJsonObject report()const{return {{"success",!smoke_||closed_},{"viewed_through_frame",int(current_)},{"history_restored_from",1},{"seek_restarts",int(restarts_)},{"diagnostic_readbacks",int(diagnostics_)},{"ordinary_readbacks",0},{"scope","World-only viewer; Space pauses, Left/Right step, Home restarts. Seeking rebuilds producer/canvas history from frame1. Explicit smoke controls alone read diagnostic pixels."}};}
};
}
int main(int argc,char** argv)try{
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QApplication app(argc,argv);
    require(argc==8||argc==9,"Usage: mnm-world-replay TAPE ASSETS view|verify|benchmark FIRST LAST REPEATS FPS [SMOKE]");
    Tape tape(QString::fromLocal8Bit(argv[1]));const auto parse=[](const char* s){bool ok=false;const auto v=QString::fromLocal8Bit(s).toUInt(&ok);require(ok,"Invalid replay integer");return v;};
    const auto first=parse(argv[4]),last=parse(argv[5]),repeats=parse(argv[6]);bool ok=false;const double fps=QString::fromLocal8Bit(argv[7]).toDouble(&ok);require(ok&&fps>=1&&fps<=1000&&first&&first<=last&&last<=tape.worlds&&repeats>=1&&repeats<=8,"Replay range/rate bounds");
    auto opened=assets::AssetStore::create(argv[2]);if(const auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);const auto store=std::get<assets::AssetStore>(std::move(opened));
    const QString mode=QString::fromLocal8Bit(argv[3]);QJsonObject result;
    if(mode=="view"){Viewer viewer(tape,store,first,last,fps,argc==9?parse(argv[8]):0);viewer.show();const int code=app.exec();result=viewer.report();std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return code;}
    require(mode=="verify"||mode=="benchmark","Unknown replay mode");require(mode!="verify"||repeats>=2,"Verification requires at least two fresh replays");result=run(tape,store,first,last,repeats,mode=="verify");std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cout<<QJsonDocument(QJsonObject{{"success",false},{"error",QString::fromUtf8(e.what())}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 1;}
