// Replay publication state with independent owned CPU source history. Original
// World planes are the separately checked offline oracle's results. No native
// destination capture or player upload bytes enter this history.
#include "../compat/legacy/canvas_producers.hpp"
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../apps/qt-shell/profile_capture_codec.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtEndian>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <tuple>
using namespace mnm;
namespace {
void need(bool v,const char* message){if(!v)throw std::runtime_error(message);}
QByteArray readFile(const QString& path){QFile f(path);need(f.open(QIODevice::ReadOnly)&&f.size()<=32*1024*1024,"Bounded history input");return f.readAll();}
void writeFile(const QString& path,const QByteArray& data){QFile f(path);need(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(data)==data.size(),"New history output");}
unsigned word(const QByteArray& b,qsizetype at){need(at>=0&&at+4<=b.size(),"History WORD bounds");return qFromLittleEndian<quint32>(b.constData()+at);}
void put(QByteArray& b,unsigned v){char w[4];qToLittleEndian<quint32>(v,w);b.append(w,4);}
struct Plane {render::Image image;std::vector<std::uint8_t> known;};
class KnowledgeMirror final:public render::CanvasMirror {
public:
 std::map<unsigned,Plane> planes;
 void create(unsigned id,int w,int h)override{need(!planes.count(id),"Mirror creation identity");planes.emplace(id,Plane{{w,h,std::vector<std::uint32_t>(std::size_t(w)*h)},std::vector<std::uint8_t>(std::size_t(w)*h)});}
 void release(unsigned id)override{need(planes.erase(id)==1,"Mirror release identity");}
 void update(unsigned id,int x,int y,const render::Image& image)override{updateMasked(id,x,y,image,std::vector<std::uint8_t>(image.pixels.size(),1));}
 void updateMasked(unsigned id,int x,int y,const render::Image& image,const std::vector<std::uint8_t>& mask)override{
  auto& p=planes.at(id);need(x>=0&&y>=0&&x+image.width<=p.image.width&&y+image.height<=p.image.height&&mask.size()==image.pixels.size(),"Mirror patch extent");
  for(int row=0;row<image.height;++row){for(int col=0;col<image.width;++col){const auto at=std::size_t(row)*image.width+col,to=std::size_t(y+row)*p.image.width+x+col;
   if(mask[at]){p.image.pixels[to]=image.pixels[at];p.known[to]=1;}
  }}
 }
 void copy(unsigned source,unsigned destination,render::Rect r,int x,int y,std::optional<std::uint16_t> key)override{
  const auto s=planes.at(source);auto& d=planes.at(destination);const int w=r.right-r.left,h=r.bottom-r.top;
  need(r.left>=0&&r.top>=0&&r.right<=s.image.width&&r.bottom<=s.image.height&&x>=0&&y>=0&&x+w<=d.image.width&&y+h<=d.image.height,"Mirror copy extent");
  for(int row=0;row<h;++row){for(int col=0;col<w;++col){const auto from=std::size_t(r.top+row)*s.image.width+r.left+col,to=std::size_t(y+row)*d.image.width+x+col;
   if(!s.known[from]){d.known[to]=0;d.image.pixels[to]=0;}
   else if(!key||s.image.pixels[from]!=*key){d.image.pixels[to]=s.image.pixels[from];d.known[to]=1;}
  }}
 }
 render::Image readForCpu(unsigned id)override{
  auto image=planes.at(id).image;const auto& known=planes.at(id).known;
  // CanvasSequence copies back only pixels already known to its history. The
  // glyph replay separately requires every actually sampled pixel below.
  for(std::size_t i=0;i<known.size();++i){if(!known[i])image.pixels[i]=0;}
  return image;
 }
};
using Key=std::tuple<quint64,quint64,unsigned>;
struct Case {QByteArray row;std::array<unsigned,4> masks;bool used=false;};
struct ModelDiagnostics {
 QProcess& process;QString path;
 ~ModelDiagnostics(){
  if(process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(5000);}
  QFile file(path);if(file.open(QIODevice::WriteOnly|QIODevice::NewOnly))file.write(process.readAllStandardError());
 }
};
QByteArray receive(QProcess& p,qsizetype size){
 QByteArray result;
 while(result.size()<size){
  if(!p.bytesAvailable())need(p.waitForReadyRead(30000),"State model response deadline/exit");
  const auto part=p.read(size-result.size());need(!part.isEmpty(),"Truncated state model response");result.append(part);
 }
 return result;
}
}
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);need(argc==8,"Expected profile, assets, original World directory, joined cases, model service, original PE, new output");
 QDir out(argv[7]);need(!out.exists()&&QDir().mkpath(out.path()),"New caller history output");
 QFile casesInput(argv[4]);need(casesInput.open(QIODevice::ReadOnly)&&casesInput.size()<=300*1024*1024,"Bounded joined cases");
 const auto caseHeader=casesInput.read(16);need(caseHeader.size()==16&&caseHeader.left(8)=="MNMCSJ01"&&word(caseHeader,8)==1,"Joined case header");
 const unsigned caseCount=word(caseHeader,12);need(caseCount&&caseCount<=262144&&casesInput.size()==16+qint64(caseCount)*888,"Joined case extent");
 std::map<Key,Case> cases;
 for(unsigned i=0;i<caseCount;++i){const auto meta=casesInput.read(40),row=casesInput.read(848);need(meta.size()==40&&row.size()==848,"Joined case truncation");
  Key key{qFromLittleEndian<quint64>(meta.constData()),qFromLittleEndian<quint64>(meta.constData()+8),word(meta,16)};
  need(word(row,0)==word(meta,20)&&word(row,4)==1&&cases.emplace(key,Case{row,{word(meta,24),word(meta,28),word(meta,32),word(meta,36)}}).second,"Joined case identity");
 }
 auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
 const auto store=std::get<assets::AssetStore>(std::move(opened));KnowledgeMirror mirror;
 legacy::CanvasProducerReplay history([store](const auto& name){auto f=store.open(name);if(auto* e=std::get_if<assets::Error>(&f))throw std::runtime_error(e->detail);auto file=std::get<std::unique_ptr<assets::AssetFile>>(std::move(f));auto data=assets::readWhole(*file,16*1024*1024);if(auto* e=std::get_if<assets::Error>(&data))throw std::runtime_error(e->detail);return std::get<std::vector<std::uint8_t>>(std::move(data));},&mirror);
 QProcess model;model.setProgram(argv[5]);model.setArguments({argv[6]});model.start();need(model.waitForStarted(10000),"Model service startup");
 ModelDiagnostics diagnostics{model,out.filePath("model-stderr.log")};
 QByteArray hello("MNMCSR01",8);put(hello,1);put(hello,caseCount);need(model.write(hello)==hello.size(),"Model header write");need(receive(model,16)==QByteArray("MNMCSO01",8)+hello.mid(8),"Model response header");
 QFile results(out.filePath("model-results.bin"));need(results.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&results.write(QByteArray("MNMCSO01",8)+hello.mid(8))==16,"New state result stream");
 unsigned checked=0,glyphs=0,mismatches=0;std::array<unsigned,7> models{};
 auto check=[&](const legacy::CanvasProducer& c,quint64 g,quint64 source){
  const auto& f=c.fields;auto found=cases.find({g,source,f[1]});if(found==cases.end())return;
  auto& item=found->second;need(!item.used&&f[22]==word(item.row,16),"Repeated or foreign source caller");item.used=true;
  history.publishWrites();QByteArray pixels,known;
  if(f[2]==8){const auto& p=mirror.planes.at(f[3]);need(p.image.width==int(f[5])&&p.image.height==int(f[6]),"Glyph source plane dimensions");
   for(std::size_t i=0;i<p.known.size();++i){const auto value=p.known[i]?p.image.pixels[i]:0;need(value<=65535,"Glyph history WORD");pixels.append(char(value));pixels.append(char(value>>8));known.append(char(p.known[i]));}
   ++glyphs;
  }
  QByteArray cps;auto fields=f;fields[0]=96+c.bytes().size();fields[18]=c.bytes().size();for(auto v:fields)put(cps,v);cps.append(reinterpret_cast<const char*>(c.bytes().data()),c.bytes().size());
  QByteArray packet;put(packet,32+848+cps.size()+pixels.size()+known.size());put(packet,cps.size());put(packet,known.size());put(packet,known.size());for(auto v:item.masks)put(packet,v);
  packet.append(item.row);packet.append(cps);packet.append(pixels);packet.append(known);
  need(model.write(packet)==packet.size(),"Model query write");while(model.bytesToWrite())need(model.waitForBytesWritten(30000),"Model query deadline");
  const auto response=receive(model,380);need(word(response,0)==380&&word(response,4)==word(item.row,0)&&word(response,8)>0&&word(response,8)<models.size(),"Model result identity");
  for(unsigned i=3;i<8;++i){need(word(response,i*4)<=1,"Unknown model result flag");}
  const bool ok=word(response,12)&&word(response,16)&&word(response,20)&&word(response,24)&&word(response,28);mismatches+=!ok;
  need(results.write(response)==response.size(),"State result retention");++models[word(response,8)];++checked;
 };
 QFile profile(argv[1]);need(profile.open(QIODevice::ReadOnly)&&profile.size()<=16ll*1024*1024*1024,"Bounded source profile");
 const auto magic=profile.read(8);const bool prefix=magic=="MNMPROF4";need(prefix||magic=="MNMPROF3","Owned source profile format");
 auto codec=prefix?std::make_unique<profile::Decoder>():nullptr;quint64 worlds=0,chunks=0,blocks=0;
 while(!profile.atEnd()){
  const auto h=profile.read(prefix?profile::HeaderSize:32);need(h.size()==(prefix?profile::HeaderSize:32),"Profile header");
  const auto g=qFromLittleEndian<quint64>(h.constData()),chunk=qFromLittleEndian<quint64>(h.constData()+8);
  const unsigned s=word(h,16),r=word(h,20),n=word(h,24);
  need(g==worlds+1&&++blocks<=8192&&s>=64&&s<=16777216&&(chunk?r==0:r>=64&&r<=197184)&&n>4&&n<=s+r+(s+r)/(prefix?128:1000)+(prefix?1024:128),"Profile identities/extent");
  const auto packed=profile.read(n);need(packed.size()==n,"Profile body");const auto body=codec?codec->decode(h,packed,s+r):qUncompress(packed);need(body.size()==s+r,"Profile decompression");
  const auto stream=legacy::decodeCanvasProducers(std::vector<std::uint8_t>(body.begin(),body.begin()+s),false);
  if(chunk){need(chunk==++chunks,"Source prefix order");for(const auto& c:stream.operations){need(c.fields[2]!=11&&c.fields[2]!=12&&c.fields[2]!=13,"World in source prefix");check(c,g,chunk);history.apply(c);}continue;}
  const auto handoff=legacy::prepareRollingHandoff(stream,0,std::vector<std::uint8_t>(body.begin()+s,body.end()),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
  ++worlds;bool active=false,committed=false;unsigned canvas=0;
  for(const auto& c:handoff.stream.operations){const auto& f=c.fields;
   if(f[2]==11){need(!active,"Nested World");active=true;canvas=f[3];}
   else if(active&&(f[2]==9||f[2]==14||f[2]==26)){check(c,g,chunks+1);}
   else if(active&&f[2]==10&&f[3]==canvas){
    need(!committed,"Duplicate World oracle commit");const auto path=QDir(argv[3]).filePath(QString("world-%1.qz").arg(g,6,10,QChar('0')));const auto pixels=qUncompress(readFile(path));
    need(pixels.size()==qsizetype(f[5])*f[6]*2,"Original World plane extent");render::Image image{int(f[5]),int(f[6]),{}};image.pixels.reserve(std::size_t(f[5])*f[6]);
    for(qsizetype at=0;at<pixels.size();at+=2)image.pixels.push_back(qFromLittleEndian<quint16>(pixels.constData()+at));
    history.commitNativeWorld(canvas,image);committed=true;
   }else if(f[2]==12){need(active&&committed,"Incomplete World");active=false;}
   else{check(c,g,chunks+1);history.apply(c);}
  }
  need(!active&&committed,"Missing World oracle plane");
 }
 model.closeWriteChannel();need(model.waitForFinished(30000)&&model.exitStatus()==QProcess::NormalExit&&model.exitCode()==0,"Model service retirement");
 need(model.readAllStandardOutput().isEmpty(),"Trailing model result bytes");results.close();
 need(checked==caseCount&&std::all_of(cases.begin(),cases.end(),[](const auto& c){return c.second.used;}),"Missing state/source cases");
 QJsonArray byModel;for(auto count:models)byModel.append(int(count));
 writeFile(out.filePath("summary.json"),QJsonDocument(QJsonObject{{"success",mismatches==0},{"cases",int(checked)},{"glyphs",int(glyphs)},{"model_mismatches",int(mismatches)},{"models",byModel},{"worlds",QString::number(worlds)},{"source_chunks",QString::number(chunks)},{"native_destination_inputs",false},{"new_original_raster_comparison",false}}).toJson());
 return mismatches?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
