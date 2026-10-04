#pragma once
#include "menu-sprite-fixtures.hpp"
#include <QImage>
#include <QPainter>
namespace grimoire_fixture {
inline void write(const QString& path,const QByteArray& bytes){QFile file(path);if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size())throw std::runtime_error("Grimoire fixture write");}
inline QString create(const QString& root) {
 const auto folder=root+"/Interface/Grimoire";if(!QDir().mkpath(folder+"/800x600"))throw std::runtime_error("Grimoire folder");
 QByteArray cfg("[HEADER]\nValidConfig=TRUE\n[OPTIONS]\ntextSpace_ModifyX=0\ntextSpace_ModifyY=-4\n[CHAPTERS]\n");for(int i=0;i<8;++i)cfg+=QString("C%1_SORTED=%2\n").arg(i).arg(i>=4?"TRUE":"FALSE").toLatin1();
 cfg+="[PAGETURN_LEFT]\n800x600_X=75\n800x600_Y=533\n[PAGETURN_RIGHT]\n800x600_X=675\n800x600_Y=533\n[CLOSE_GRIMOIRE]\n800x600_X=773\n800x600_Y=238\n[TABS]\n";
 for(int i=0;i<8;++i)for(const auto& side:{QString("L"),QString("R")})cfg+=QString("800x600_%1%2X=%3\n800x600_%1%2Y=%4\n").arg(i+1).arg(side).arg(side=="L"?10:722).arg(22+i*65).toLatin1();
 write(folder+"/Grimoire.cfg",cfg);QByteArray tips("[HEADER]\nValidConfig=TRUE\n[STRINGS]\n");for(int i=0;i<9;++i)tips+=QString("STR_%1=Tip %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();write(folder+"/Grimoiretooltip.cfg",tips);
 QByteArray text("~[ header ~[ nested [more] ] ]\n");for(int i=0;i<8;++i)text+=QString("~C%1-000-\"Chapter %1\"\n~S02-001-\"Zulu\"\n~L1\nText section two.\n~NL\nSecond paragraph.\n~D \"Mana\" ignored synthetic stat payload\n~NP\nSecond text page.\n~S01-002-\"<b>Alpha</b>\"\n~L0\nAlpha body.\n").arg(i).toLatin1();write(folder+"/Grimoire.txt",text);
 QImage background(800,600,QImage::Format_RGB888);background.fill(QColor(130,110,80));if(!background.save(folder+"/800x600/Backdrop.JPG","JPG"))throw std::runtime_error("Grimoire background");
 QImage page(400,600,QImage::Format_RGB888);page.fill(QColor(0,0,255));{QPainter painter(&page);painter.fillRect(QRect(60,15,320,555),QColor(130,110,80));painter.fillRect(QRect(89,47,282,489),QColor(255,0,255));}
 if(!page.save(folder+"/800x600/GenericL01.JPG","JPG")||!page.save(folder+"/800x600/GenericR01.JPG","JPG"))throw std::runtime_error("Grimoire generic art");
 QImage companion(400,600,QImage::Format_RGB888);companion.fill(QColor(80,140,70));if(!companion.save(folder+"/800x600/C0S02P2_Companion.JPG","JPG"))throw std::runtime_error("companion artwork");
 menu_sprite_fixture::write(root,"Interface/Grimoire/800x600/icons.spr",34);menu_sprite_fixture::write(root,"Interface/Grimoire/Generic/PageTurns.spr",4);return folder;
}
}
