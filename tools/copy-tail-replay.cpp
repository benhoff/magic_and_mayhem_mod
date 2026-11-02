// Bounded offline fixtures only. Never connected to a game/session channel.
#include "blit.hpp"
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
QByteArray read(QFile& f,qint64 n){auto b=f.read(n);require(b.size()==n,"Truncated fixture");return b;}
unsigned word(const QByteArray& b,unsigned at){require(at*4+4<=unsigned(b.size()),"Short fixture word");return qFromLittleEndian<quint32>(b.constData()+at*4);}
Image image(const QByteArray& b,unsigned width,unsigned height){Image out{int(width),int(height),{}};require(b.size()==qint64(width)*height*4,"Fixture plane extent");for(qsizetype at=0;at<b.size();at+=4){const auto p=qFromLittleEndian<quint32>(b.constData()+at);require(p<=65535,"Fixture WORD overflow");out.pixels.push_back(p);}return out;}
QByteArray bytes(const Image& im){QByteArray b;for(auto p:im.pixels){char w[4];qToLittleEndian<quint32>(p,w);b.append(w,4);}return b;}
QString hash(const QByteArray& b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
}
int main(int argc,char** argv)try{
 QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QGuiApplication app(argc,argv);
 require(argc==2||argc==3,"Expected fixture and optional --skip-copy control");const bool skip=argc==3;require(!skip||QString(argv[2])=="--skip-copy","Unknown control");
 QFile input(argv[1]);require(input.open(QIODevice::ReadOnly)&&input.size()<=2ll*1024*1024*1024,"Bounded fixture input");require(read(input,8)=="MNMCPT01","Fixture version");
 GlBlitter renderer;unsigned count=0,previous=0;
 while(true){
  const auto n=word(read(input,4),0);if(!n)break;
  require(++count<=1024&&n>=72&&n<=72+2*2048*2048*4,"Fixture case budget");const auto b=read(input,n);
  const auto seq=word(b,0),sw=word(b,1),sh=word(b,2),dw=word(b,3),dh=word(b,4);
  require(seq>previous&&sw&&sh&&dw&&dh&&sw<=2048&&sh<=2048&&dw<=2048&&dh<=2048,"Fixture identity/extent");previous=seq;
  require(n==72+std::uint64_t(sw)*sh*4+std::uint64_t(dw)*dh*4,"Fixture body extent");
  unsigned w[13];for(unsigned i=0;i<13;++i)w[i]=word(b,5+i);
  require(w[0]&&w[1]&&w[0]!=w[1]&&w[2]<w[4]&&w[3]<w[5]&&w[4]<=sw&&w[5]<=sh&&w[6]<=dw&&w[7]<=dh&&w[4]-w[2]<=dw-w[6]&&w[5]-w[3]<=dh-w[7]&&w[8]<=1&&!w[10]&&!w[11]&&!w[12]&&(w[8]||!w[9])&&w[9]<=65535,"Fixture COPY geometry/key");
  const auto source=b.mid(72,sw*sh*4),before=b.mid(72+sw*sh*4);
  const auto src=renderer.create(image(source,sw,sh),{16,{0xf800,0x7e0,31}}),dst=renderer.create(image(before,dw,dh),{16,{0xf800,0x7e0,31}});
  if(!skip)renderer.copy(src,dst,{int(w[2]),int(w[3]),int(w[4]),int(w[5])},int(w[6]),int(w[7]),w[8]?std::optional<std::uint32_t>(w[9]):std::nullopt);
  const auto actual=bytes(renderer.read(dst));renderer.destroy(src);renderer.destroy(dst);require(renderer.stats().surfaces==0,"Replay surfaces leaked");
  std::cout<<QJsonDocument(QJsonObject{{"sequence",int(seq)},{"width",int(dw)},{"height",int(dh)},{"source_sha256",hash(source)},{"before_sha256",hash(before)},{"actual_sha256",hash(actual)}}).toJson(QJsonDocument::Compact).constData()<<'\n';
 }
 require(input.atEnd()&&count,"Trailing or empty fixture");
 std::cerr<<QJsonDocument(QJsonObject{{"success",true},{"cases",int(count)},{"renderer",QString::fromStdString(renderer.driver().renderer)},{"remaining_surfaces",0},{"offline_only",true},{"skip_copy_control",skip}}).toJson(QJsonDocument::Compact).constData()<<'\n';
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
