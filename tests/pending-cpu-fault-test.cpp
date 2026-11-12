#include "pending_cpu_fault_test.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <thread>
#include <cstdio>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void file(const QString& path,const QByteArray& bytes){QSaveFile f(path);check(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size()&&f.commit(),"fixture file");}
int main(int argc,char** argv)try{
 QCoreApplication app(argc,argv);QTemporaryDir dir;check(dir.isValid(),"temporary directory");
 for(unsigned variant=0;variant<4;++variant){
  const auto root=dir.filePath(QString::number(variant));QDir().mkpath(root+"/game");QDir().mkpath(root+"/lock-capture");
  const auto path=root+"/commands";QFile channel(path);check(channel.open(QIODevice::ReadWrite),"channel file");check(channel.resize(MNM_RENDER_COMMANDS_V2_SIZE),"channel resize");
  auto* map=channel.map(0,channel.size());check(map,"channel map");memset(map,0,64);memcpy(map,MNM_RENDER_COMMANDS_V3_MAGIC,8);auto* ring=reinterpret_cast<uint32_t*>(map);ring[2]=3;ring[3]=MNM_RENDER_COMMANDS_V2_SIZE;ring[4]=501;
  mnm_ring_writer producer{};check(mnm_ring_writer_bind(&producer,map,MNM_RENDER_COMMANDS_V2_SIZE),"producer bind");
  QByteArray raw(MNM_CRJ_BYTES,0);auto* h=reinterpret_cast<uint32_t*>(raw.data());memcpy(h,"MNMCRJ01",8);h[2]=1;h[3]=128;h[4]=128;h[5]=MNM_CRJ_CAPACITY;h[6]=h[7]=1;h[11]=501;
  auto* row=h+32;row[0]=1;row[1]=MNM_CRJ_REQUEST;row[2]=99;row[4]=501;row[5]=1;row[6]=3;row[14]=0x1234;row[15]=2;row[17]=100;row[18]=1;row[19]=800;row[20]=600;row[21]=3;row[23]=7;
  if(variant==2)row[18]=2; // Matching-ticket journal row is required.
  file(root+"/lock-capture/cpu-reader-journal-v1.bin",raw);
  mnm::render::SurfaceCommand command;command.version=5;command.operation=21;command.sequence=10;command.words={2,1,0,501,800,600,3,0,7,0};
  mnm::render::CommandResult result;result.commandStreamVersion=5;
  PendingCpuFaultTest fault(path,501);check(fault.arm()&&!fault.arm(),"fault can arm only once");
  if(variant==3){bool refused=false;try{fault.observe(command,result);}catch(const std::exception&){refused=true;}check(refused&&mnm_ring_load(producer.map+6)==1,"GUI injection accepted");channel.unmap(map);continue;}
  bool refused=false;
  std::thread runner([&]{
   try{fault.observe(command,result);}catch(const std::exception&){refused=true;}
  });
  if(variant<2){
   QElapsedTimer wait;wait.start();while(mnm_ring_load(producer.map+6)!=3&&wait.elapsed()<2000)QThread::msleep(1);
   check(mnm_ring_load(producer.map+6)==3,"worker fault missing");
   if(variant==1){const uint32_t d[16]={0x574d4e4d,1,1,0,99,0,0,0,1,1,0,0,0,3,0,0};file(root+"/game/native-cpu-reader-wait.bin",QByteArray(reinterpret_cast<const char*>(d),sizeof(d)));}
   const uint32_t d[8]={0x434d4e4d,1,variant?13u:7u,0x1234,1,1,100,501};file(root+"/game/native-cpu-transfer-refusal.bin",QByteArray(reinterpret_cast<const char*>(d),sizeof(d)));
  }
  runner.join();
  if(variant==2){check(refused&&mnm_ring_load(producer.map+6)==1&&!QFileInfo::exists(fault.recordPath()),"wrong original ticket accepted");}
  else{
   check(!refused,"matching worker fault refused");fault.finish(&result,0);
   QFile report(fault.recordPath());check(report.open(QIODevice::ReadOnly),"fault report");const auto q=QJsonDocument::fromJson(report.readAll()).object();
   check(q["finalized"].toBool()&&q["original_refusal"].toArray().size()==8,"final failure record incomplete");
   file(path+".cpu-reply",QByteArray("late reply"));bool lateRefused=false;try{fault.finish(&result,0);}catch(const std::exception&){lateRefused=true;}check(lateRefused,"late reply accepted");
  }
  channel.unmap(map);
 }
 std::puts("{\"success\":true,\"worker_cases\":2,\"original_wait_kinds\":2,\"wrong_ticket_refused\":true,\"gui_injection_refused\":true,\"late_reply_refused\":true,\"scope\":\"Synthetic worker-thread adapter protocol controls; no original-game execution\"}");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
