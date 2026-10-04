#pragma once
#include "menu-sprite-fixtures.hpp"
#include <QImage>
#include <QFileInfo>
#include <QMap>
namespace realm_viewer_fixture {
inline void write(const QString& path,const QByteArray& bytes){QFile file(path);if(!QDir().mkpath(QFileInfo(path).path())||!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size())throw std::runtime_error("Realm fixture write");}
inline QByteArray fp(int x,int y){QByteArray data(132,0);data[16]=1;data[20]=2;data[0]='.';data[1]='F';data[2]='P';data[4]=2;for(int i=0;i<4;++i){data[84+i]=char(unsigned(x)>>(8*i));data[88+i]=char(unsigned(y)>>(8*i));data[116+i]=char(unsigned(x)>>(8*i));data[120+i]=char(unsigned(y)>>(8*i));data[124+i]=char(unsigned(x+10)>>(8*i));data[128+i]=char(unsigned(y+10)>>(8*i));}return data;}
inline void put(QByteArray& b,int at,unsigned value,int size=4){for(int i=0;i<size;++i)b[at+i]=char(value>>(8*i));}
inline QByteArray pcx(const QRect& rect,bool border){
 static QMap<QString,QByteArray> cache;const auto key=QString("%1,%2,%3,%4,%5").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()).arg(border);if(cache.contains(key))return cache[key];
 QByteArray data(128,0);data[0]=10;data[1]=5;data[2]=1;data[3]=8;put(data,8,799,2);put(data,10,599,2);data[65]=1;put(data,66,800,2);
 for(int y=0;y<600;++y){QByteArray row(800,0);for(int x=0;x<800;++x)if(rect.contains(x,y)&&!(x==rect.center().x()&&y==rect.center().y()))row[x]=1;
  for(int x=0;x<800;){int end=x+1;while(end<800&&row[end]==row[x]&&end-x<63)++end;data.append(char(0xc0|(end-x)));data.append(row[x]);x=end;}}
 data.append(char(12));QByteArray palette(768,0);palette[2]=char(255);palette[3]=char(255);palette[4]=char(border?220:0);data+=palette;cache[key]=data;return data;
}
inline QByteArray ani(){
 constexpr int sequences=51,records=5+50*3;QByteArray b(44+(sequences+1)*4+records*44,0);b.replace(0,4,QByteArray("ANI\0",4));put(b,4,unsigned(b.size()));put(b,8,records);put(b,12,5);put(b,20,sequences+1);b.replace(24,9,"Flags.spr");
 for(int i=0;i<=sequences;++i)put(b,44+i*4,i?5+(i-1)*3:0);
 auto record=[&](int i,int opcode,int argument){const int at=44+(sequences+1)*4+i*44;put(b,at,opcode);put(b,at+4,unsigned(argument));};
 record(0,1,1);record(1,0,0);record(2,0,1);record(3,4,-3);record(4,6,-1);
 for(int i:{1,2}){put(b,44+(sequences+1)*4+i*44+8,unsigned(-2));put(b,44+(sequences+1)*4+i*44+12,unsigned(-47));}
 for(int s=1;s<sequences;++s){const int i=5+(s-1)*3;record(i,0,2);record(i+1,4,-1);record(i+2,6,-1);}return b;
}
inline void create(const QString& root){
 const auto base=root+"/Interface/RealmViewer";write(base+"/realmviewtooltip.cfg","[HEADER]\nValidConfig=TRUE\n[STRINGS]\nSTR_00=Portmanteau\nSTR_01=Grimoire\nSTR_02=Character Improvement\nSTR_03=Options\n");
 const std::array<QString,3> names{"Celtic","Greek","Medieval"};const std::array<int,3> counts{8,12,16};
 for(int r=0;r<3;++r){QImage map(800,600,QImage::Format_RGB888);map.fill(QColor(80+r*50,100,120));if(!QDir().mkpath(base+"/800x600")||!map.save(base+"/800x600/"+names[r]+"_Map.bmp","BMP"))throw std::runtime_error("Realm map fixture");
  for(int n=1;n<=counts[r];++n){const auto prefix=base+"/Generic/"+names[r];const auto suffix=QString("_%1").arg(n,2,10,QLatin1Char('0'));write(prefix+"_Region"+suffix+".txt",QString("%1 region %2").arg(names[r]).arg(n).toLatin1());write(prefix+"_FlagPath"+suffix+".FP",fp(100+n*25,200+r*80));const QRect shape(100+n*25,300+r*60,18,18);write(prefix+"_Silhouette"+suffix+".pcx",pcx(shape,false));write(base+"/800x600/"+names[r]+"_Border"+suffix+".pcx",pcx(shape,true));}
 }
 write(base+"/Generic/flags.ani",ani());
 menu_sprite_fixture::write(root,"Interface/RealmViewer/Generic/flags.spr",89);menu_sprite_fixture::write(root,"Interface/RealmViewer/Generic/RlmBtn.spr",12);menu_sprite_fixture::shared(root);
}
}
