// Native-only diagnostic pressure. No game, original assets or live equivalence.
#include "../renderer/commands.hpp"
#include "../renderer/command_copy_diagnostics.hpp"
#include "../apps/qt-shell/acceptance_capture.hpp"
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <chrono>
#include <thread>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
SurfaceCommand cmd(unsigned op,unsigned seq,std::initializer_list<unsigned> values){SurfaceCommand c;c.operation=op;c.version=4;c.sequence=seq;std::copy(values.begin(),values.end(),c.words.begin());return c;}
SurfaceCommand create(unsigned id,unsigned seq,const Image& image){auto c=cmd(1,seq,{id,unsigned(image.width),unsigned(image.height),16,0xf800,0x7e0,31});c.image=image;c.format={16,{0xf800,0x7e0,31}};return c;}
}
int main(int argc,char** argv){
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QGuiApplication app(argc,argv);
 try{
  check(argc==2,"Owned output directory required");const auto out=QString::fromLocal8Bit(argv[1]);check(QDir().mkpath(out),"Cannot create output");
  qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",(out+"/capture").toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","copy-queue-native-pressure-v1");qputenv("MNM_ACCEPTANCE_NORMAL","1");qputenv("MNM_ACCEPTANCE_CAMPAIGN","1");qputenv("MNM_ACCEPTANCE_COMMAND_HISTORY","1");
  AcceptanceCapture commands("commands"),viewport("viewport");GlBlitter renderer;
  auto diagnostic=std::make_shared<CommandCopyDiagnostics>(out+"/copies",CommandCopyDiagnostics::Selection::AllCopies);
  CommandConsumerOptions options;options.copyDiagnostics=diagnostic;options.exportImages=true;CommandConsumer consumer(renderer,{},options);
  Image source{800,600,std::vector<std::uint32_t>(800*600)},blank=source;
  for(std::size_t i=0;i<source.pixels.size();++i)source.pixels[i]=(i*11+1)&65535;
  consumer.submit(create(1,1,source));consumer.submit(create(2,2,blank));
  // The parent uses the same copies.jsonl constructor marker as a real player.
  const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(20);
  while(!QFile::exists(out+"/go")){check(std::chrono::steady_clock::now()<until,"Parent scheduling gate missing");std::this_thread::sleep_for(std::chrono::milliseconds(2));}
  unsigned seq=3,paint=0;QByteArray plane(reinterpret_cast<const char*>(source.pixels.data()),qsizetype(source.pixels.size()*4));
  const auto started=std::chrono::steady_clock::now();
  for(unsigned i=0;i<600;++i){
   consumer.submit(cmd(3,seq++,{1,2,0,0,800,600,0,0,0,0}));
   commands.append("fragment",{{"sequence",int(i)}},QByteArray(52,char(i)));
   if(i%10==0){viewport.append("paint",{{"paint",int(++paint)},{"width",800},{"height",600}},plane);viewport.append("swap",{{"paint",int(paint)}});}
  }
  consumer.submit(cmd(6,seq++,{2}));const auto actual=consumer.result().native;check(actual.size()==800*600*2,"Native output extent");
  for(std::size_t i=0;i<source.pixels.size();++i)check(qFromLittleEndian<quint16>(actual.constData()+2*i)==source.pixels[i],"Actual GPU output changed");
  consumer.submit(cmd(7,seq++,{1}));consumer.submit(cmd(7,seq++,{2}));consumer.submit(cmd(8,seq++,{}));consumer.finish();diagnostic->finish();commands.append("ended",{});commands.finish();viewport.finish();
  QFile file(out+"/copies/report.json");check(file.open(QIODevice::ReadOnly),"COPY report missing");const auto copy=QJsonDocument::fromJson(file.readAll()).object();check(copy["ended"].toBool()&&copy["error"].toString().isEmpty()&&copy["copies_seen"].toInt()==600,"COPY report incomplete");
  const auto driver=renderer.driver();QJsonObject result{{"success",true},{"original_game_runs",0},{"actual_native_output_matches",true},{"renderer",QString::fromStdString(driver.renderer)},{"vendor",QString::fromStdString(driver.vendor)},{"copy_diagnostic",copy},{"COPYs",600},{"viewport_paints",int(paint)},{"elapsed_ms",double(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count())},{"native_sha256",QString::fromLatin1(QCryptographicHash::hash(actual,QCryptographicHash::Sha256).toHex())}};
  std::puts(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());return 0;
 }catch(const std::exception& error){std::puts(QJsonDocument(QJsonObject{{"success",false},{"error",error.what()},{"original_game_runs",0}}).toJson(QJsonDocument::Compact).constData());return 1;}
}
