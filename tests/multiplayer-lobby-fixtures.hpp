#pragma once
#include <QDir>
#include <QFile>
#include <QImage>
#include <array>
#include <stdexcept>
namespace lobby_fixture {
inline void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
inline void write(const QString& path,const QByteArray& bytes){QFile file(path);require(file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size(),"fixture write");}
inline QString rect(int x,int y,int w,int h){return QString("%1,%2,%3,%4").arg(x).arg(y).arg(x+w).arg(y+h);}
struct Layouts {QString basePath,hostPath,joinPath;QByteArray base,host,join;};
inline Layouts create(const QString& root,const QImage& image) {
    const auto path=root+"/Interface/MultiplayerBattleSetup";require(QDir().mkpath(path+"/800x600")&&image.save(path+"/800x600/Fixture 800-600.JPG","JPG"),"lobby image");
    QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n");
    for (int i=0;i<40;++i) {
        QString r,text="\"0\"",alignment="LEFT";
        if(i<4){r=rect(75,25+i*75,225,25);text="55";}
        else if(i<8) r=rect(75,50+(i-4)*75,50,25);
        else if(i<12){r=rect(125,50+(i-8)*75,200,25);text="56";}
        else if(i==12){r=rect(400,25,375,50);text="\"\"";alignment="MIDDLE";}
        else if(i==13){r=rect(460,100,315,35);text="\"\"";alignment="MIDDLE";}
        else if(i<27){r=rect(400,150+(i-14)*25,150,25);text=QString::number(std::array<int,13>{33,34,58,59,36,37,38,60,61,62,63,64,65}[i-14]);}
        else r=rect(550,150+(i-27)*25,50,25);
        layout+=QString("[TEXT_%1]\nFont=SMALL\nRect2=%2\nTextflags=%3\nText=%4\n").arg(i+1).arg(r,alignment,text).toLatin1();
    }
    const std::array<int,17> minimum{50,100,0,30,0,0,0,0,1,0,0,0,0,0,0,0,0};
    const std::array<int,17> maximum{200,800,21,3000,7,7,7,240,20,15,50,50,30,50,50,50,50};
    const std::array<int,17> step{10,20,1,10,1,1,1,5,1,1,1,5,1,5,5,5,5};
    QByteArray host;
    for (int i=0;i<13;++i) host+=QString("[SLIDERBAR_%1]\nRect2=%2\nminValue=%3\nmaxValue=%4\nstep=%5\n").arg(i+1).arg(rect(600,150+i*25,175,25)).arg(minimum[i]).arg(maximum[i]).arg(step[i]).toLatin1();
    for (int i=0;i<4;++i) layout+=QString("[SLIDERBAR_%1]\nRect2=%2\nminValue=0\nmaxValue=50\nstep=5\n").arg(i+1).arg(rect(125,50+i*75,200,25)).toLatin1();
    for (int i=0;i<8;++i) layout+=QString("[STANDARDBUTTON_%1]\nRect2=%2\n").arg(i+1).arg(i<4?rect(325,25+i*75,50,50):rect(30,30+(i-4)*75,45,45)).toLatin1();
    for (int i=0;i<3;++i) host+=QString("[STANDARDBUTTON_%1]\nRect2=%2\n").arg(i+1).arg(rect(290,100+i*75,25,25)).toLatin1();
    layout+="[TEXTBUTTON_1]\nFont=SMALL\nRect2=400,490,575,525\nText=11\n[MESSAGELISTBOX_1]\nFont=SMALL\nRect2=25,325,375,525\n[EDITBOX_1]\nFont=SMALL\nRect2=25,540,774,575\nText=\"edit box1\"\n[TEXTBUTTON_3]\n[TEXTBUTTON_4]\n[TEXTBUTTON_5]\n[TEXTBUTTON_6]\n[TEXTBUTTON_7]\n[TEXTBUTTON_8]\n";
    host+="[TEXTBUTTON_1]\nFont=SMALL\nRect2=400,100,460,135\nText=52\n[TEXTBUTTON_2]\nFont=SMALL\nRect2=600,490,775,525\nText=53\n";
    const QByteArray join("[TEXTBUTTON_1]\nFont=SMALL\nRect2=600,490,775,525\nText=54\n");
    Layouts result{path+"/screen (MultiPlayer Battle Setup).cfg",path+"/screen (MultiPlayer Battle Create).cfg",path+"/screen (Multiplayer Battle Join).cfg",layout,host,join};
    write(result.basePath,layout);write(result.hostPath,host);write(result.joinPath,join);return result;
}
}
