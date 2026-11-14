// Replay retained recorder input only. This is not an engine/game/history proof.
#include "acceptance_capture.hpp"
#include <QJsonParseError>
#include <QCryptographicHash>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
struct Input {quint64 at;QString kind;QJsonObject metadata;QByteArray body;};
quint32 word(QFile& file){const auto bytes=file.read(4);require(bytes.size()==4,"Input word");return qFromLittleEndian<quint32>(bytes.constData());}
QByteArray digest(const std::vector<Input>& rows){QCryptographicHash hash(QCryptographicHash::Sha256);for(const auto& row:rows){char size[8];qToLittleEndian<quint64>(row.body.size(),size);hash.addData(QByteArrayView(size,8));hash.addData(row.body);}return hash.result().toHex();}
}
int main(int argc,char** argv)try{
 require(argc==3,"Expected retained-input fixture and new output directory");
 const QString inputPath=QString::fromLocal8Bit(argv[1]),directory=QString::fromLocal8Bit(argv[2]);
 require(!QFileInfo::exists(directory)&&QDir().mkpath(directory),"New owned output required");
 QFile input(inputPath);require(input.open(QIODevice::ReadOnly)&&input.size()<2ll*1024*1024*1024&&input.read(8)=="MNMCRP01","Input envelope");
 const auto count=word(input);require(count>0&&count<65535,"Input row bound");std::vector<Input> rows;rows.reserve(count);quint64 previous=0;quint64 rawBytes=0;
 for(quint32 i=0;i<count;++i){
  const auto m=word(input),n=word(input);require(m>0&&m<=65536&&n<=32u*1024*1024,"Input row extent");
  const auto metadataBytes=input.read(m);require(metadataBytes.size()==m,"Input metadata bytes");QJsonParseError parse;auto metadata=QJsonDocument::fromJson(metadataBytes,&parse).object();require(parse.error==QJsonParseError::NoError,"Input JSON");
  const auto at=metadata["host_us"].toString().toULongLong();const auto kind=metadata["kind"].toString();require(at>=previous&&(kind=="fragment"||kind=="cpu-read"),"Input order/kind");previous=at;
  auto body=input.read(n);require(body.size()==n,"Input body");rawBytes+=n;require(rawBytes<=1536ull*1024*1024,"Replay input budget");
  metadata.remove("index");metadata.remove("host_us");metadata.remove("kind");metadata["probe_original_host_us"]=QString::number(at);rows.push_back({at,kind,std::move(metadata),std::move(body)});
 }
 require(input.atEnd(),"Extra input bytes");input.close();const auto expected=digest(rows);
 qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","retained-prefix-control");qputenv("MNM_ACCEPTANCE_NORMAL","1");qputenv("MNM_ACCEPTANCE_COMMAND_HISTORY","1");qunsetenv("MNM_ACCEPTANCE_CAMPAIGN");qunsetenv("MNM_ACCEPTANCE_IMAGES");
 using Clock=std::chrono::steady_clock;const auto begin=Clock::now(),start=begin+std::chrono::milliseconds(200);qint64 late=0,maxAppend=0;
 {AcceptanceCapture capture("commands");
  for(const auto& row:rows){
   const auto target=start+std::chrono::microseconds(row.at-rows.front().at);std::this_thread::sleep_until(target);const auto now=Clock::now();late=std::max(late,qint64(std::chrono::duration_cast<std::chrono::microseconds>(now-target).count()));
   capture.append(row.kind,row.metadata,row.body);maxAppend=std::max(maxAppend,qint64(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-now).count()));
  }
  capture.finish();
 }
 const auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-begin).count();const auto names=QDir(directory).entryList({"*.bin"},QDir::Files);require(names.size()==1,"One replay wire required");const auto output=QDir(directory).filePath(names.front());QFile wire(output);require(wire.open(QIODevice::ReadOnly)&&wire.read(8)=="MNMACE01","Output envelope");
 QJsonObject opening,closing;quint32 index=0,matched=0;quint64 actualRaw=0;QCryptographicHash actual(QCryptographicHash::Sha256);
 while(!wire.atEnd()){
  const auto m=word(wire),n=word(wire);require(m>0&&m<=65536&&n<=33u*1024*1024,"Output row extent");const auto encoded=wire.read(m);require(encoded.size()==m,"Output metadata bytes");auto metadata=QJsonDocument::fromJson(encoded).object();require(metadata["index"].toInt()==int(index++),"Output FIFO index");const auto packed=wire.read(n);require(packed.size()==n,"Output payload bytes");auto body=n?qUncompress(packed):QByteArray{};require(!n||body.size()==qFromBigEndian<quint32>(packed.constData()),"Output compressed extent");
  if(metadata["kind"]=="open"){require(index==1,"Nested output open");opening=metadata;continue;}
  if(metadata["kind"]=="close"){require(wire.atEnd(),"Nonterminal output close");closing=metadata;continue;}
  require(matched<rows.size()&&metadata["kind"].toString()==rows[matched].kind&&body==rows[matched].body,"Output payload changed/order lost");metadata.remove("index");metadata.remove("host_us");metadata.remove("kind");require(metadata==rows[matched].metadata,"Output source metadata changed");char size[8];qToLittleEndian<quint64>(body.size(),size);actual.addData(QByteArrayView(size,8));actual.addData(body);actualRaw+=body.size();++matched;
 }
 require(!opening.isEmpty()&&!closing.isEmpty()&&closing["prior_rows"].toInt()==int(index-1)&&closing["raw_bytes"].toString().toULongLong()==actualRaw,"Output terminal counters");
 const bool complete=closing["complete"].toBool();const bool success=complete&&closing["error"].toString().isEmpty()&&matched==rows.size()&&actual.result().toHex()==expected;
 QJsonObject result{{"success",success},{"original_game_runs",0},{"normal_game_completion_claimed",false},{"synthetic_END_added",false},{"input_rows",int(rows.size())},{"copied_rows",int(matched)},{"input_raw_bytes",double(rawBytes)},{"expected_payload_sha256",QString::fromLatin1(expected)},{"actual_copied_payload_sha256",QString::fromLatin1(actual.result().toHex())},{"every_written_row_matches_input",true},{"opening",opening},{"closing",closing},{"elapsed_us",double(elapsed)},{"maximum_producer_lateness_us",double(late)},{"maximum_append_us",double(maxAppend)},{"wire",output},{"wire_bytes",double(wire.size())},{"scope","Isolated recorder replay of exact retained incomplete command-capture prefix with its observed arrival timing. Copying the prefix completely is not complete original-game history, a synthesized END, a pixel comparison or player acceptance."}};
 std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return success?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
