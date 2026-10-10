#include "../compat/legacy/canvas_world.hpp"
#include "../renderer/sprites/sprite.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <unistd.h>
using namespace mnm;
using namespace render;
using Clock=std::chrono::steady_clock;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F> static void reject(F action){bool refused=false;try{action();}catch(const std::invalid_argument&){refused=true;}require(refused,"Invalid input was accepted");}
template<class F> static void rejectUndefined(F action){bool refused=false;try{action();}catch(const std::runtime_error& e){require(std::string(e.what())=="Surface pixels are undefined; reload or overwrite required","Unexpected undefined-pixel refusal");refused=true;}require(refused,"Undefined pixels were accepted");}
static QByteArray load(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly),"Cannot open input");return f.readAll();}
static QString digest(const Image& image){QByteArray words;words.resize(image.pixels.size()*2);for(std::size_t i=0;i<image.pixels.size();++i){words[2*i]=char(image.pixels[i]);words[2*i+1]=char(image.pixels[i]>>8);}return QString::fromLatin1(QCryptographicHash::hash(words,QCryptographicHash::Sha256).toHex());}
static double elapsed(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
static double rss(){std::ifstream f("/proc/self/statm");std::uint64_t total=0,resident=0;f>>total>>resident;require(bool(f),"Cannot sample resident memory");return double(resident)*sysconf(_SC_PAGESIZE);}
static QJsonObject replay(GlBlitter& renderer,const legacy::CanvasProducerStream& stream,const assets::AssetStore& store,const QJsonArray& expected){
  QJsonArray timings;unsigned checkpoints=0;double cpuMs=0;unsigned inside=0;
  legacy::CanvasProducerReplay producer([&](const auto& name){auto opened=store.open(name);if(auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(error->detail);auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto read=assets::readWhole(*file,16*1024*1024);if(auto* error=std::get_if<assets::Error>(&read))throw std::runtime_error(error->detail);return std::get<std::vector<std::uint8_t>>(std::move(read));});
  legacy::CanvasWorld world(renderer,store);
  for(const auto& c:stream.operations){
    const auto& r=c.fields;auto started=Clock::now();producer.apply(c);if(world.active())cpuMs+=elapsed(started);
    if(r[2]==11){cpuMs=0;world.begin(c,producer.read(r[3]));}
    else if(world.active()){
      if(r[2]==9)world.append(c);
      else if(r[2]==10&&r[3]==world.canvas()){producer.commitNativeWorld(r[3],world.complete(producer.read(r[3])));++inside;}
      else if(r[2]==12){const auto& p=world.profile();require(p.identityFrames<=4096&&p.identityBytes<=16*1024*1024,"World identity cache exceeds bound");require(p.cacheSurfaces<=64,"World cache surface bound exceeded");timings.append(QJsonObject{{"queue",int(world.queue())},{"native_work_ms",cpuMs+p.adoptMs+p.prepareMs+p.submitMs+p.readbackMs+p.compareMs},{"gpu_submit_ms",p.submitMs},{"uploads",double(p.cacheUploads)},{"cache_frames",double(p.cacheFrames)},{"identity_bytes",double(p.identityBytes)}});world.end(c);}
      else require(r[2]==4||r[2]==10||r[2]==25,"Unsupported producer inside World");
    }
    if(r[2]==10){require(checkpoints<unsigned(expected.size()),"Unexpected checkpoint");const auto e=expected[int(checkpoints)].toObject();require(unsigned(e["sequence"].toInt())==r[1]&&unsigned(e["canvas"].toInt())==r[3]&&unsigned(e["oracle"].toInt())==r[14],"Checkpoint identity mismatch");const auto image=producer.read(r[3]);require(image.width==int(r[5])&&image.height==int(r[6])&&digest(image)==e["sha256"].toString(),"Checkpoint pixels changed");++checkpoints;}
  }
  require(!world.active()&&world.completedQueues()==stream.queues&&world.readbacks()==stream.queues,"Incomplete World replay");
  require(checkpoints==unsigned(expected.size())&&inside==stream.queues,"Incomplete checkpoint comparison");
  return {{"checkpoints",int(checkpoints)},{"queues",int(world.completedQueues())},{"frames",timings}};
}
// Independent scalar RGB565 arithmetic; snapshots are per draw, never per batch.
static void oracleDraw(Image& dest,const Image& source,const Image& mask,Rect rect,int x,int y,const SpriteComposite& op,const SpriteColourTable& palette){
  const auto before=dest.pixels;
  for(int row=0;row<rect.bottom-rect.top;++row)for(int col=0;col<rect.right-rect.left;++col){const auto s=(row+rect.top)*source.width+col+rect.left;if(!mask.pixels[s])continue;const auto at=(y+row)*dest.width+x+col;const auto v=palette[source.pixels[s]];auto& d=dest.pixels[at];
    switch(op.mode){
    case CompositeMode::copy:d=v;break;
    case CompositeMode::half:d=((v>>1)&0x7bef)+((d>>1)&0x7bef);break;
    case CompositeMode::quarterSource:{const auto h=(d>>1)&0x7bef;d=h+((h>>1)&0x7bef)+((v>>2)&0x39e7);break;}
    case CompositeMode::quarterDestination:{const auto h=(v>>1)&0x7bef;d=h+((h>>1)&0x7bef)+((d>>2)&0x39e7);break;}
    case CompositeMode::displace:d=before[at+op.rowOffsets[(y+row)%op.rowPeriod]];break;
    default:throw std::runtime_error("Unsupported scalar mode");
    }
  }
}
static QJsonObject adversarial(GlBlitter& renderer,unsigned seed,unsigned rounds){
  std::mt19937 random(seed);Image source{9,7,std::vector<std::uint32_t>(63)},mask{9,7,std::vector<std::uint32_t>(63)},oracle{64,48,std::vector<std::uint32_t>(3072)};
  for(auto& p:source.pixels){p=random()%256;}source.pixels[0]=0;
  for(auto& p:mask.pixels){p=random()%3!=0;}mask.pixels[0]=1;
  for(auto& p:oracle.pixels){p=random()&65535;}
  auto src=renderer.create(source,spriteFormat),coverage=renderer.create(mask,{8,{}}),dest=renderer.create(oracle,spriteFormat);
  unsigned draws=0,comparisons=0,refusals=0;std::array<unsigned,5> modes{};SpriteColourTable palette{};const auto context=QOpenGLContext::currentContext();
  for(unsigned round=0;round<rounds;++round){
    renderer.batch([&]{for(unsigned n=0;n<700;++n){
      if(n==0||n>=600){for(auto& p:palette)p=std::uint16_t(random());} // exceed both512 copies and64 owned palettes
      palette[0]=0;SpriteComposite op;op.mode=n<680?CompositeMode::copy:CompositeMode(random()%5);++modes[unsigned(op.mode)];op.rowPeriod=1+random()%16;for(auto& p:op.rowOffsets)p=random()%17;
      const int left=random()%8,top=random()%6;Rect rect{left,top,left+1+int(random()%(9-left)),top+1+int(random()%(7-top))};const int x=random()%32,y=random()%40;
      oracleDraw(oracle,source,mask,rect,x,y,op,palette);renderer.composite(src,dest,rect,x,y,coverage,op,&palette);++draws;
      if(n==50){source.pixels[0]=(source.pixels[0]+1)%256;renderer.update(src,0,0,source);} // source mutation must flush ordered copies
    }});
    require(QOpenGLContext::currentContext()==context,"Batch leaked caller GL context");require(renderer.read(dest).pixels==oracle.pixels,"Seeded mixed draws differ from scalar oracle");++comparisons;
    const Rect region{int(random()%20),int(random()%20),40,35};const std::array<std::uint16_t,3> add{std::uint16_t(random()),std::uint16_t(random()),std::uint16_t(random())};
    for(int y=region.top;y<region.bottom;++y)for(int x=region.left;x<region.right;++x){auto& d=oracle.pixels[y*64+x];d=(std::min(((d>>11)+add[0])&65535u,31u)<<11)|(std::min((((d>>5)&63)+add[1])&65535u,63u)<<5)|std::min(((d&31)+add[2])&65535u,31u);}
    renderer.additiveRect(dest,region,add);require(renderer.read(dest).pixels==oracle.pixels,"Seeded additive draw differs");++comparisons;
    auto unknown=renderer.allocate(64,48,spriteFormat);SpriteComposite half;half.mode=CompositeMode::half;rejectUndefined([&]{renderer.composite(src,unknown,{0,0,9,7},0,0,coverage,half,&palette);});++refusals;
    renderer.destroy(unknown);reject([&]{renderer.composite(dest,dest,{0,0,1,1},0,0,coverage,{},&palette);});++refusals;
    try{renderer.batch([&]{renderer.composite(src,dest,{0,0,1,1},0,0,coverage,{},&palette);oracleDraw(oracle,source,mask,{0,0,1,1},0,0,{},palette);throw std::runtime_error("fixture exception");});}catch(const std::runtime_error& e){require(std::string(e.what())=="fixture exception","Unexpected batch failure");}
    require(QOpenGLContext::currentContext()==context&&renderer.read(dest).pixels==oracle.pixels,"Exceptional batch lost order/context");++comparisons;
    renderer.invalidateContents(dest);rejectUndefined([&]{renderer.read(dest);});++refusals;renderer.update(dest,0,0,oracle);
    // Signed anchors, negative origins, corners and fully hidden sprites.
    Image words=source;for(auto& p:words.pixels)p=palette[p];PreparedSpriteFrame frame{words,mask,3,-2,{}};UploadedSpriteFrame sprite(renderer,std::move(frame));std::size_t budget=100000;while(!sprite.advanceUpload(budget)){}
    for(const auto pos:std::vector<std::pair<int,int>>{{-12,-12},{0,0},{3,-2},{64,48},{61,44},{35,20}}){const int left=pos.first-3,top=pos.second+2;for(int sy=0;sy<7;++sy)for(int sx=0;sx<9;++sx){const int x=left+sx,y=top+sy;if(x>=2&&x<62&&y>=1&&y<47&&mask.pixels[sy*9+sx])oracle.pixels[y*64+x]=palette[source.pixels[sy*9+sx]];}sprite.drawClipped(dest,pos.first,pos.second,{2,1,62,47});require(renderer.read(dest).pixels==oracle.pixels,"Clipped signed-anchor draw differs");++comparisons;}
    require(renderer.stats().scratchPixels<=2048u*2048u&&renderer.stats().maxCopyBatch<=512,"Scratch/copy bound exceeded");
  }
  renderer.destroy(src);renderer.destroy(coverage);renderer.destroy(dest);
  require(renderer.stats().maxCopyBatch==512,"Adversarial fixture did not reach copy batch boundary");
  QJsonArray modeCounts;for(auto count:modes){require(count>0,"Adversarial fixture omitted a blend mode");modeCounts.append(int(count));}
  return {{"seed",double(seed)},{"rounds",int(rounds)},{"draws",int(draws)},{"pixel_comparisons",int(comparisons)},{"refusals",int(refusals)},{"mode_counts",modeCounts},{"max_copy_batch",double(renderer.stats().maxCopyBatch)}};
}
int main(int argc,char** argv){
  QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QGuiApplication app(argc,argv);QJsonObject report{{"success",false}};
  if(argc!=8){std::cerr<<"Usage: native-render-soak INPUT ASSETS EXPECTED_JSON OUTPUT ITERATIONS SEED ROUNDS\n";return 2;}
  try{
    const auto raw=load(argv[1]);const auto stream=legacy::decodeCanvasProducers({raw.begin(),raw.end()});auto opened=assets::AssetStore::create(argv[2]);if(auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(error->detail);auto store=std::get<assets::AssetStore>(std::move(opened));
    const auto expected=QJsonDocument::fromJson(load(argv[3])).object()["checkpoints"].toArray();require(!expected.empty(),"Missing expected hashes");const unsigned iterations=std::stoul(argv[5]);require(iterations>=4&&iterations<=10000,"Invalid soak iteration bound");const unsigned rounds=std::stoul(argv[7]);require(rounds>=1&&rounds<=10000,"Invalid adversarial round bound");
    // Envelope mutations must refuse immediately, before source allocation.
    unsigned malformed=0;for(unsigned at=0;at<64;at+=4){auto bytes=std::vector<std::uint8_t>(raw.begin(),raw.begin()+64);bytes[at]^=128;reject([&]{legacy::decodeCanvasProducers(bytes,false);});++malformed;}
    GlBlitter renderer;const auto driver=renderer.driver();report["driver"]=QJsonObject{{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)},{"version",QString::fromStdString(driver.version)}};
    report["adversarial"]=adversarial(renderer,std::stoul(argv[6]),rounds);report["malformed_envelopes_refused"]=int(malformed);require(renderer.stats().surfaces==0,"Adversarial test leaked surfaces");
    QJsonArray sessions;for(unsigned i=0;i<iterations;++i){auto started=Clock::now();auto session=replay(renderer,stream,store,expected);require(renderer.stats().surfaces==0&&renderer.stats().pixels==0,"Session teardown leaked GPU surfaces");session["elapsed_ms"]=elapsed(started);session["rss_bytes"]=rss();session["scratch_pixels"]=double(renderer.stats().scratchPixels);session["scratch_allocations"]=double(renderer.stats().scratchAllocations);sessions.append(session);report["sessions"]=sessions;std::cout<<"Completed offline session "<<i+1<<"/"<<iterations<<std::endl;}
    report["success"]=true;report["original_pixels_used_as_native_inputs"]=false;report["remaining_surfaces"]=double(renderer.stats().surfaces);
  }catch(const std::exception& e){report["error"]=QString::fromLocal8Bit(e.what());std::cerr<<e.what()<<'\n';}
  QFile file(argv[4]);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)){std::cerr<<"Cannot create report\n";return 1;}file.write(QJsonDocument(report).toJson());return report["success"].toBool()?0:1;
}
