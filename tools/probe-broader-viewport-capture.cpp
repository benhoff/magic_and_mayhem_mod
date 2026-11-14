// Exact retained viewport-prefix arrival replay; no game, GL or synthetic END.
#include "acceptance_capture.hpp"
#include <QCryptographicHash>
#include <QJsonParseError>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void need(bool value,const char* why){if(!value)throw std::runtime_error(why);}
struct Row{quint64 at;QString kind;QJsonObject metadata;QByteArray body;};
struct Capture{QJsonObject opening,closing;std::vector<Row> rows;quint64 raw=0;};
quint32 word(QFile& file){auto b=file.read(4);need(b.size()==4,"Truncated envelope word");return qFromLittleEndian<quint32>(b.constData());}
Capture read(const QString& path){
 QFile file(path);need(file.open(QIODevice::ReadOnly)&&file.size()<1024ll*1024*1024&&file.read(8)=="MNMACE01","Bounded capture required");Capture result;QByteArray previous;int previousIndex=-1;QJsonArray extent;quint32 index=0;quint64 time=0;
 while(!file.atEnd()){
  const auto m=word(file),n=word(file);need(m>0&&m<=65536&&n<=33u*1024*1024,"Envelope row bound");auto meta=file.read(m),packed=file.read(n);need(meta.size()==m&&packed.size()==n,"Truncated envelope row");QJsonParseError error;auto v=QJsonDocument::fromJson(meta,&error).object();need(error.error==QJsonParseError::NoError&&v["index"].toInt()==int(index++),"Metadata/FIFO index");const auto kind=v["kind"].toString();const auto stamp=v["host_us"].toString().toULongLong();need(stamp>=time,"Nonmonotonic input time");time=stamp;
  if(kind=="open"){need(index==1&&n==0&&v["role"]=="viewport"&&v["version"].toInt()==2,"Campaign viewport v2 required");result.opening=v;continue;}
  if(kind=="close"){need(n==0&&file.atEnd()&&v["prior_rows"].toInt()==int(index-1),"Terminal counters");result.closing=v;continue;}
  need(kind=="paint"||kind=="swap","Unexpected viewport row");QByteArray body=n?qUncompress(packed):QByteArray{};need(!n||body.size()==qFromBigEndian<quint32>(packed.constData()),"Compressed body extent");
  if(!body.isEmpty()){
   const QJsonArray size{v["width"],v["height"]};need(kind=="paint"&&qint64(v["width"].toInt())*v["height"].toInt()*4==body.size(),"Paint extent");
   if(v["capture_body_encoding"]=="xor-previous-paint-v1"){
    need(previous.size()==body.size()&&extent==size&&v["capture_body_base"].toInt()==previousIndex,"Delta predecessor");auto* p=body.data();const auto* old=previous.constData();for(qsizetype i=0;i<body.size();++i)p[i]=char(uchar(p[i])^uchar(old[i]));
   }else need(v["capture_body_encoding"]=="full-rgba-v1"&&v["capture_body_base"].toInt()==-1,"Unknown capture encoding");
   need(QCryptographicHash::hash(body,QCryptographicHash::Sha256).toHex()==v["capture_body_sha256"].toString().toLatin1(),"Raw paint checksum");previous=body;extent=size;previousIndex=v["index"].toInt();
  }
  result.raw+=body.size();need(result.raw<=8ull*1024*1024*1024&&result.rows.size()<16384,"Retained replay input bound");result.rows.push_back({stamp,kind,v,std::move(body)});
 }
 need(!result.opening.isEmpty()&&!result.rows.empty(),"Empty retained input");return result;
}
}
int main(int argc,char** argv)try{
 need(argc==3,"Expected immutable retained viewport file and fresh output directory");const auto input=read(QString::fromLocal8Bit(argv[1]));const auto directory=QString::fromLocal8Bit(argv[2]);need(!QFileInfo::exists(directory)&&QDir().mkpath(directory),"Fresh output required");qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","retained-viewport-prefix-control");qputenv("MNM_ACCEPTANCE_NORMAL","1");qputenv("MNM_ACCEPTANCE_CAMPAIGN","1");qunsetenv("MNM_ACCEPTANCE_COMMAND_HISTORY");qunsetenv("MNM_ACCEPTANCE_IMAGES");
 using Clock=std::chrono::steady_clock;const auto begin=Clock::now(),start=begin+std::chrono::milliseconds(200);qint64 late=0,append=0;
 {AcceptanceCapture capture("viewport");for(const auto& row:input.rows){const auto target=start+std::chrono::microseconds(row.at-input.rows.front().at);std::this_thread::sleep_until(target);auto now=Clock::now();late=std::max(late,qint64(std::chrono::duration_cast<std::chrono::microseconds>(now-target).count()));auto metadata=row.metadata;metadata.remove("index");metadata.remove("host_us");metadata.remove("kind");metadata["probe_original_host_us"]=QString::number(row.at);capture.append(row.kind,metadata,row.body);append=std::max(append,qint64(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-now).count()));}capture.finish();}
 const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-begin).count();const auto names=QDir(directory).entryList({"*.bin"},QDir::Files);need(names.size()==1,"One output required");const auto path=QDir(directory).filePath(names[0]);auto output=read(path);need(!output.closing.isEmpty()&&output.rows.size()<=input.rows.size(),"Closed replay prefix required");
 for(std::size_t i=0;i<output.rows.size();++i)need(output.rows[i].kind==input.rows[i].kind&&output.rows[i].body==input.rows[i].body&&output.rows[i].metadata["probe_original_host_us"].toString()==QString::number(input.rows[i].at),"Replay changed retained payload/order");
 const bool complete=output.closing["complete"].toBool()&&output.closing["error"].toString().isEmpty()&&output.rows.size()==input.rows.size();
 QJsonObject result{{"success",true},{"original_game_runs",0},{"normal_game_completion_claimed",false},{"old_capture_completed",false},{"synthetic_END_added",false},{"live_failure_cause_proved",false},{"input_rows",int(input.rows.size())},{"copied_rows",int(output.rows.size())},{"input_raw_bytes",double(input.raw)},{"input_has_close",!input.closing.isEmpty()},{"retained_prefix_replayed_completely",complete},{"every_written_payload_matches_input",true},{"opening",output.opening},{"closing",output.closing},{"elapsed_us",double(elapsed)},{"maximum_producer_lateness_us",double(late)},{"maximum_append_us",double(append)},{"wire",path},{"scope","Bounded isolated viewport recorder replay at the exact observed retained-prefix arrival intervals. A refusal diagnoses this replay only; no game, GL presentation, complete old capture, missing pixels/END or universal live cause/repair is inferred."}};std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
