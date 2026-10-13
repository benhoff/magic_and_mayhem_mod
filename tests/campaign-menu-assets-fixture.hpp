#pragma once
#include "menu-sprite-fixtures.hpp"
#include <QImage>
#include <array>
namespace campaign_menu_fixture_assets {
inline void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
inline void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size(),"fixture write");}
// Synthetic layout/art, matching the widget roles tested by character-screen-test.
// No installed or immutable original assets are consumed.
inline void create(const QString& root){
require(QDir().mkpath(root+"/CFG"),"strings folder");
    QByteArray strings("[STRINGS]\n");for(int i=0;i<100;++i){const auto text=i==10?"OK":i==11?"Cancel":i==31?"Cornelius":i==32?"Experience Points":i==33?"Mana":i==34?"Health":i==35?"Control Limit":i==36?"Law Talismans":i==37?"Neutral Talismans":i==38?"Chaos Talismans":i==39?"Cost:":"Label";strings+=QString("STR_%1=%2\n").arg(i,2,10,QLatin1Char('0')).arg(text).toLatin1();}write(root+"/CFG/interface screens text.cfg",strings);
    QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));const auto folder=root+"/Interface/CharacterScreen";require(QDir().mkpath(folder+"/800x600")&&image.save(folder+"/800x600/CharacterScreen.JPG","JPG"),"character image");
    QImage face(400,300,QImage::Format_RGB888);face.fill(QColor(0,0,255));
    for(int i=0;i<3;++i){face.fill(QColor(0,0,255));for(int y=80;y<120;++y)for(int x=80;x<120;++x)face.setPixelColor(x,y,QColor(180+i*20,130-i*20,80));require(face.save(folder+QString("/800x600/WizardFace%1.JPG").arg(i),"JPG"),"face JPEG");}
    const std::array<QString,3> textureNames{"Mana Bar.BMP","Health Bar.BMP","Control Limit Bar.BMP"};
    QImage texture(700,20,QImage::Format_RGB888);texture.fill(QColor(80,80,80));
    for(int y=0;y<20;++y)for(int x=0;x<350;++x)texture.setPixelColor(x,y,QColor(170,140,60));
    for(const auto& name:textureNames)require(texture.save(folder+"/800x600/"+name,"BMP"),"bar BMP");
    menu_sprite_fixture::shared(root);menu_sprite_fixture::write(root,"Interface/CharacterScreen/800x600/sprites.spr",12);
    const std::array<QString,17> rectangles{"0,52,800,92","650,60,790,110","0,215,260,265","0,265,260,315","0,315,260,365","0,365,260,415","0,415,260,465","0,465,260,515","650,122,790,172","650,190,790,215","690,215,750,265","690,265,750,315","690,315,750,365","690,365,750,415","690,415,750,465","690,465,750,515","225,125,575,160"};
    const std::array<QString,6> variables{"VAR_MANACOST","VAR_HEALTHCOST","VAR_CONTROLCOST","VAR_LAWCOST","VAR_NEUTRALCOST","VAR_CHAOSCOST"};
    QByteArray layout("[GLOBALS]\nBackgroundFile=CharacterScreen\n");
    for(int i=0;i<17;++i){const auto text=i<8?QString::number(31+i):i==8?QString("VAR_EXPERIENCE"):i==9?QString("39"):i==16?QString("\"\""):variables[i-10];layout+=QString("[TEXT_%1]\nFont=%2\nRect2=%3\nText=%4\nTextFlags=%5\n").arg(i+1).arg(i==1||i==9?"SMALL":"LARGE").arg(rectangles[i],text,i>=2&&i<=7?"RIGHT":"CENTRE").toLatin1();}
    const std::array<int,3> maximum{200,800,40};
    for(int i=0;i<3;++i){layout+=QString("[STATBAR_%1]\nRect2=270,%2,620,%3\nminValue=0\nmaxValue=%4\nText=%8\n[SPRITEBAR_%1]\nRect2=270,%5,0,0\nminValue=0\nmaxValue=7\nSpriteIndexes=%6,%7\n").arg(i+1).arg(230+i*50).arg(250+i*50).arg(maximum[i]).arg(360+i*50).arg(i).arg(i+3).arg(textureNames[i]).toLatin1();}
    for(int i=0;i<12;++i)layout+=QString("[STANDARDBUTTON_%1]\nRect2=%2,%3,%4,%5\nSpriteIndexes=%6\n").arg(i+1).arg(i%2?750:660).arg(225+(i/2)*50).arg(i%2?780:690).arg(255+(i/2)*50).arg(i%2?"9,10,11":"6,7,8").toLatin1();
    layout+="[TEXTBUTTON_1]\nFont=LARGE\nRect2=40,530,280,570\nText=10\n[TEXTBUTTON_2]\nFont=LARGE\nRect2=530,530,760,570\nText=11\n[STEPS_COSTS]\nMana_IncrementAmount=5\nHealth_IncrementAmount=20\nControl_IncrementAmount=1\nTalisman_IncrementAmount=1\nMana_InitialCost=0\nMana_LinearComponent=.6\nMana_ExponentialComponent=1.3\n";
    const auto config=folder+"/screen (Character Screen).cfg";write(config,layout);
    const auto regionPath=root+"/Interface/RegionEntry";require(QDir().mkpath(regionPath+"/800x600")&&image.save(regionPath+"/800x600/Celtic_Region_01.JPG","JPG"),"region image");QByteArray regionLayout("[GLOBAL]\n[TEXT_1]\nRect2=50,40,525,75\nFont=LARGE\nTextFlags=LEFT\nText=Current Region\n[TEXTBUTTON_1]\nRect2=50,510,325,560\nFont=LARGE\nText=78\n[TEXTBUTTON_2]\nRect2=475,510,750,560\nFont=LARGE\nText=11\n");for(int i=0;i<4;++i)regionLayout+=QString("[RADIOBUTTON_%1]\nRect2=%2,90,%3,115\nFont=SMALL\nText=%4\n").arg(i+1).arg(50+i*175).arg(225+i*175).arg(79+i).toLatin1();const std::array<int,3> first{3,6,0};for(int i=0;i<3;++i)regionLayout+=QString("[STANDARDBUTTON_%1]\nRect2=%2,25,%3,85\nSpriteIndexes=%4,%5,%6\n").arg(i+1).arg(585+i*65).arg(645+i*65).arg(first[i]).arg(first[i]+1).arg(first[i]+2).toLatin1();write(regionPath+"/screen (Region Entry).cfg",regionLayout);

    const std::array<QString,3> realmNames{"Celtic","Greek","Medieval"};
    for(const auto& realm:realmNames)for(int n=1;n<=2;++n)
        require(image.save(regionPath+QString("/800x600/%1_Region_%2.JPG").arg(realm).arg(n,2,10,QLatin1Char('0')),"JPG"),"all fixture Region art");
}
}
