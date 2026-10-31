// Oracle-side history only. This executable runs after the actual player exits.
// Native GPU outputs and original destination captures are never its inputs.
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../compat/legacy/canvas_producers.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>
using namespace mnm;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
QByteArray read(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly)&&f.size()<=32*1024*1024,"Bounded reference input");return f.readAll();}
void save(const QString& path,const QByteArray& b){QFile f(path);require(f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(b)==b.size(),"New reference output");}
void put(QByteArray& b,unsigned value){char w[4];qToLittleEndian<quint32>(value,w);b.append(w,4);}
QByteArray encode(const render::Image& image){QByteArray b;for(auto p:image.pixels){require(p<=65535,"Reference WORD");b.append(char(p));b.append(char(p>>8));}return b;}
}
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);require(argc==6,"Expected source capture, assets, original executor, pinned PE, new output");
 QDir output(argv[5]);require(!output.exists()&&QDir().mkpath(output.path()),"New reference directory");
 auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);
 const auto store=std::get<assets::AssetStore>(std::move(opened));
 legacy::CanvasProducerReplay history([store](const auto& name){auto opened=store.open(name);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);auto f=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto bytes=assets::readWhole(*f,16*1024*1024);if(auto* e=std::get_if<assets::Error>(&bytes))throw std::runtime_error(e->detail);return std::get<std::vector<std::uint8_t>>(std::move(bytes));});
 QFile input(argv[1]);require(input.open(QIODevice::ReadOnly)&&input.size()<=256ll*1024*1024,"Bounded capture");
 require(input.read(8)=="MNMPROF2","Owned independent World capture requires complete MNMPROF2");
 quint64 generation=0;QJsonArray worlds;std::uint64_t totalDraws=0;
 while(!input.atEnd()){
  const auto header=input.read(20);require(header.size()==20,"Source header");
  const auto g=qFromLittleEndian<quint64>(header.constData());const auto s=qFromLittleEndian<quint32>(header.constData()+8),r=qFromLittleEndian<quint32>(header.constData()+12),n=qFromLittleEndian<quint32>(header.constData()+16);
  require(g==++generation&&g<=512&&s>=16&&s<=16777216&&r>=64&&r<=197184&&n>4&&n<=s+r+(s+r)/1000+128,"Ordered bounded source");
  const auto packed=input.read(n);require(packed.size()==n&&qFromBigEndian<quint32>(packed.constData())==s+r,"Source compressed length");
  const auto data=qUncompress(packed);require(data.size()==s+r,"Source decompression");
  auto bytes=[&](unsigned at,unsigned size){return std::vector<std::uint8_t>(data.begin()+at,data.begin()+at+size);};
  const auto source=legacy::decodeCanvasProducers(bytes(0,s),false);
  const auto handoff=legacy::prepareRollingHandoff(source,0,bytes(s,r),1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
  unsigned canvas=0,width=0,height=0,count=0,raster=0;bool active=false;QByteArray draws("MNMSCHK1",8),before;
  for(const auto& c:handoff.stream.operations){const auto& f=c.fields;
   if(f[2]==11){require(!active,"Nested reference World");active=true;canvas=f[3];width=f[5];height=f[6];before=encode(history.read(canvas));put(draws,width);put(draws,height);put(draws,0);}
   else if(active&&(f[2]==9||f[2]==14||f[2]==26)){
    put(draws,f[2]==9?handoff.batch.descriptors.at(raster++)[1]:0);auto fields=f;fields[0]=96+c.bytes().size();fields[18]=c.bytes().size();for(auto value:fields)put(draws,value);draws.append(reinterpret_cast<const char*>(c.bytes().data()),c.bytes().size());++count;
   }else if(active&&f[2]==10&&f[3]==canvas){
    qToLittleEndian<quint32>(count,draws.data()+16);
    const auto stem=QString("world-%1").arg(g,6,10,QChar('0'));
    const auto request=output.filePath(stem+"-draws.bin"),baseline=output.filePath(stem+"-before.565"),result=output.filePath(stem+".565");save(request,draws);save(baseline,before);
    QProcess original;original.setProgram(argv[3]);original.setArguments({argv[4],request,baseline,result});original.start();require(original.waitForStarted(10000)&&original.waitForFinished(30000)&&original.exitStatus()==QProcess::NormalExit&&original.exitCode()==0,"Independent original raster execution failed");
    const auto words=read(result);require(words.size()==qsizetype(width)*height*2,"Original full World plane");
    render::Image image{int(width),int(height),{}};image.pixels.reserve(width*height);for(qsizetype at=0;at<words.size();at+=2)image.pixels.push_back(qFromLittleEndian<quint16>(words.constData()+at));
    // Only the oracle history receives original results. The actual player has
    // already exited; neither source stream nor native renderer is rewritten.
    history.commitNativeWorld(canvas,image);save(output.filePath(stem+".qz"),qCompress(words,3));
    worlds.append(QJsonObject{{"generation",QString::number(g)},{"width",int(width)},{"height",int(height)},{"draws",int(count)},{"rasters",int(raster)},{"canvas",int(canvas)},{"path",stem+".qz"}});totalDraws+=count;
    require(QFile::remove(request)&&QFile::remove(baseline)&&QFile::remove(result),"Temporary original reference retirement");
   }else if(f[2]==12){require(active,"Unentered reference return");active=false;}
   else history.apply(c);
  }
  require(!active&&worlds.size()==int(g),"Every World needs exactly one original result");
 }
 require(generation>0,"Empty original history");
 save(output.filePath("worlds.json"),QJsonDocument(QJsonObject{{"schema",1},{"success",true},{"worlds",worlds},{"draws",QString::number(totalDraws)},{"native_player_pixels_read",false},{"original_destination_capture_read",false},{"reference_history","Owned CPU producer inputs and original raster results chained continuously from initial creation"}}).toJson());
 std::cout<<generation<<" independently reconstructed Worlds\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
