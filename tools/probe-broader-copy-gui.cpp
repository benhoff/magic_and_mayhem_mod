// Native-only main-thread GPU/diagnostic pressure with physical accessible UI.
#include "../renderer/commands.hpp"
#include "../renderer/command_copy_diagnostics.hpp"
#include "../apps/qt-shell/acceptance_capture.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
namespace {
void check(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
SurfaceCommand command(unsigned op,unsigned seq,std::initializer_list<unsigned> values){SurfaceCommand c;c.operation=op;c.version=4;c.sequence=seq;std::copy(values.begin(),values.end(),c.words.begin());return c;}
SurfaceCommand create(unsigned id,unsigned seq,const Image& image){auto c=command(1,seq,{id,unsigned(image.width),unsigned(image.height),16,0xf800,0x7e0,31});c.image=image;c.format={16,{0xf800,0x7e0,31}};return c;}
}
int main(int argc,char** argv){
 QSurfaceFormat fmt;fmt.setVersion(3,3);fmt.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(fmt);QApplication app(argc,argv);
 try{
  check(argc==2,"Owned output directory required");const auto out=QString::fromLocal8Bit(argv[1]);check(QDir().mkpath(out),"Cannot create output");
  qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",(out+"/capture").toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","copy-pipeline-GUI-pressure-v1");qputenv("MNM_ACCEPTANCE_NORMAL","1");qputenv("MNM_ACCEPTANCE_CAMPAIGN","1");qputenv("MNM_ACCEPTANCE_COMMAND_HISTORY","1");
  AcceptanceCapture commands("commands"),viewport("viewport");GlBlitter renderer;
  auto diagnostics=std::make_shared<CommandCopyDiagnostics>(out+"/copies",CommandCopyDiagnostics::Selection::AllCopies);CommandConsumerOptions options;options.copyDiagnostics=diagnostics;options.exportImages=true;CommandConsumer consumer(renderer,{},options);
  Image first{800,600,std::vector<std::uint32_t>(800*600)},second=first,expected=first;
  for(std::size_t i=0;i<first.pixels.size();++i){first.pixels[i]=(i*11+1)&65535;second.pixels[i]=first.pixels[i]^31;}
  consumer.submit(create(1,1,first));consumer.submit(create(2,2,expected));consumer.submit(create(3,3,second));unsigned seq=4,done=0,paints=0,pulses=0;bool during=false;QString failure;
  QWidget window;window.setWindowTitle("Magic & Mayhem Workshop");QVBoxLayout layout(&window);QLabel status("COPY progress 0");QPushButton pulse("Diagnostic pulse 0");layout.addWidget(&status);layout.addWidget(&pulse);window.resize(480,180);window.show();
  QObject::connect(&pulse,&QPushButton::clicked,&window,[&]{++pulses;during|=done>0&&done<600;pulse.setText("Diagnostic pulse "+QString::number(pulses));});
  QElapsedTimer elapsed;elapsed.start();QTimer timer;timer.setInterval(10);
  QObject::connect(&timer,&QTimer::timeout,&window,[&]{try{
   check(elapsed.elapsed()<60000,"Native GUI control deadline");if(!QFile::exists(out+"/go"))return;
   for(unsigned batch=0;batch<4&&done<600;++batch,++done){
    const auto& source=(done/100)%2?second:first;const auto sid=(done/100)%2?3u:1u;const unsigned x=(done%10)*40,y=((done/10)%10)*40;
    consumer.submit(command(3,seq++,{sid,2,0,0,40,40,x,y,0,0}));
    for(unsigned yy=0;yy<40;++yy)for(unsigned xx=0;xx<40;++xx)expected.pixels[(y+yy)*800+x+xx]=source.pixels[yy*800+xx];
    commands.append("fragment",{{"sequence",int(done)}},QByteArray(52,char(done)));
    if(done%10==0){const QByteArray plane(reinterpret_cast<const char*>(expected.pixels.data()),qsizetype(expected.pixels.size()*4));viewport.append("paint",{{"paint",int(++paints)},{"width",800},{"height",600}},plane);viewport.append("swap",{{"paint",int(paints)}});}
   }
   status.setText("COPY progress "+QString::number(done));
   if(done==600&&QFile::exists(out+"/finish")){timer.stop();app.quit();}
  }catch(const std::exception& error){failure=QString::fromLocal8Bit(error.what());timer.stop();app.exit(1);}});timer.start();const auto code=app.exec();
  check(code==0&&failure.isEmpty()&&done==600&&pulses>0&&during,"Native pressure/physical GUI response incomplete");
  consumer.submit(command(6,seq++,{2}));const auto actual=consumer.result().native;check(actual.size()==800*600*2,"Native plane extent");
  for(std::size_t i=0;i<expected.pixels.size();++i)check(qFromLittleEndian<quint16>(actual.constData()+2*i)==expected.pixels[i],"Actual GPU stencil output changed");
  consumer.submit(command(7,seq++,{1}));consumer.submit(command(7,seq++,{2}));consumer.submit(command(7,seq++,{3}));consumer.submit(command(8,seq++,{}));consumer.finish();diagnostics->finish();commands.append("ended",{});commands.finish();viewport.finish();
  QFile file(out+"/copies/report.json");check(file.open(QIODevice::ReadOnly),"COPY report missing");const auto report=QJsonDocument::fromJson(file.readAll()).object();check(report["success"].toBool()&&report["copies_seen"].toInt()==600&&report["copies_checked"].toInt()==600&&!report["truncated"].toBool(),"Incomplete actual COPY pressure diagnostics");
  const auto driver=renderer.driver();QJsonObject result{{"success",true},{"original_game_runs",0},{"actual_native_output_matches",true},{"COPYs",600},{"GUI_pulses",int(pulses)},{"GUI_pulse_during_pressure",during},{"elapsed_ms",double(elapsed.elapsed())},{"renderer",QString::fromStdString(driver.renderer)},{"vendor",QString::fromStdString(driver.vendor)},{"copy_diagnostic",report},{"native_sha256",QString::fromLatin1(QCryptographicHash::hash(actual,QCryptographicHash::Sha256).toHex())}};
  std::puts(QJsonDocument(result).toJson(QJsonDocument::Compact).constData());return 0;
 }catch(const std::exception& error){std::puts(QJsonDocument(QJsonObject{{"success",false},{"error",error.what()},{"original_game_runs",0}}).toJson(QJsonDocument::Compact).constData());return 1;}
}
