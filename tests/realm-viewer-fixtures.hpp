#pragma once
#include "menu-sprite-fixtures.hpp"
#include <QImage>
#include <QFileInfo>
namespace realm_viewer_fixture {
inline void write(const QString& path,const QByteArray& bytes){QFile file(path);if(!QDir().mkpath(QFileInfo(path).path())||!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size())throw std::runtime_error("Realm fixture write");}
inline QByteArray fp(int x,int y){QByteArray data(116,0);data[0]='.';data[1]='F';data[2]='P';data[4]=2;for(int i=0;i<4;++i){data[84+i]=char(unsigned(x)>>(8*i));data[88+i]=char(unsigned(y)>>(8*i));}return data;}
inline void create(const QString& root){
 const auto base=root+"/Interface/RealmViewer";write(base+"/realmviewtooltip.cfg","[HEADER]\nValidConfig=TRUE\n[STRINGS]\nSTR_00=Portmanteau\nSTR_01=Grimoire\nSTR_02=Character Improvement\nSTR_03=Options\n");
 const std::array<QString,3> names{"Celtic","Greek","Medieval"};const std::array<int,3> counts{8,12,16};
 for(int r=0;r<3;++r){QImage map(800,600,QImage::Format_RGB888);map.fill(QColor(80+r*50,100,120));if(!QDir().mkpath(base+"/800x600")||!map.save(base+"/800x600/"+names[r]+"_Map.bmp","BMP"))throw std::runtime_error("Realm map fixture");
  for(int n=1;n<=counts[r];++n){const auto prefix=base+"/Generic/"+names[r];const auto suffix=QString("_%1").arg(n,2,10,QLatin1Char('0'));write(prefix+"_Region"+suffix+".txt",QString("%1 region %2").arg(names[r]).arg(n).toLatin1());write(prefix+"_FlagPath"+suffix+".FP",fp(100+n*25,200+r*80));}
 }
 menu_sprite_fixture::write(root,"Interface/RealmViewer/Generic/flags.spr",89);menu_sprite_fixture::write(root,"Interface/RealmViewer/Generic/RlmBtn.spr",12);menu_sprite_fixture::shared(root);
}
}
