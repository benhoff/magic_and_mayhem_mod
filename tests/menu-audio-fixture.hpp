#pragma once
#include "audio-catalog-fixture.hpp"
#include "grimoire-fixtures.hpp"
namespace menuAudioFixture {
using audioFixture::check;
inline void save(const QString& path,const QByteArray& data){grimoire_fixture::write(path,data);}
inline void fixtures(const QString& root){
    check(QDir().mkpath(root+"/CFG"),"strings folder");
    QByteArray strings("[STRINGS]\n");for(int i=0;i<90;++i)strings+=QString("STR_%1=Fixture %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();
    save(root+"/CFG/interface screens text.cfg",strings);
    QImage background(800,600,QImage::Format_RGB32);background.fill(QColor(80,110,140));
    const auto image=[&](const QString& folder){check(QDir().mkpath(folder+"/800x600"),"menu folder");check(background.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"menu image");};
    const auto main=root+"/Interface/MainScreen",quick=root+"/Interface/QuickBattleMainMenu",prefs=root+"/Interface/BattleOptionsScreen";
    image(main);image(quick);image(prefs);
    QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=600,565,790,590\n");
    for(int i=0;i<6;++i)layout+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(260+i*45).arg(300+i*45).arg(i).toLatin1();
    save(main+"/screen (MainMenu).cfg",layout);
    layout="[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,75,700,125\nText=2\n";
    for(int i=0;i<4;++i)layout+=QString("[TEXTBUTTON_%1]\nRect2=200,%2,600,%3\nText=%4\n").arg(i+1).arg(200+i*70).arg(250+i*70).arg(i).toLatin1();
    save(quick+"/Screen (Quick Battle Main Menu).cfg",layout);
    layout="[GLOBALS]\nBackgroundFile=Fixture\n";
    for(int i=0;i<11;++i)layout+=QString("[TEXT_%1]\nRect2=100,%2,700,%3\nFont=%4\nTextFlags=LEFT\nText=%5\n").arg(i+1).arg(30+i*25).arg(55+i*25).arg(i==0?"LARGE":"SMALL").arg(i).toLatin1();
    for(int i=0;i<12;++i)layout+=QString("[RADIOBUTTON_%1]\nRect2=100,%2,500,%3\nFont=SMALL\nText=%4\n").arg(i+1).arg(240+i*15).arg(265+i*15).arg(i+12).toLatin1();
    for(int i=0;i<2;++i)layout+=QString("[SLIDERBAR_%1]\nRect2=100,%2,325,%3\nminValue=-10000\nmaxValue=0\n").arg(i+1).arg(145+i*50).arg(175+i*50).toLatin1();
    layout+="[TEXTBUTTON_1]\nRect2=40,530,280,570\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=530,530,760,570\nFont=LARGE\nText=11\n";
    save(prefs+"/screen (Battle Options).cfg",layout);grimoire_fixture::create(root);
    check(QDir().mkpath(root+"/Sounds"),"sounds folder");const auto sounds=std::filesystem::path(root.toStdString())/"Sounds";
    audioFixture::write(sounds/"Click.wav",audioFixture::wave(4000));audioFixture::write(sounds/"Page.wav",audioFixture::wave(-2000));
    audioFixture::profile(sounds,"[Sounds]\n822=Click\n830=Page\n840=Logical\n[Randomised]\n840=830\n[Optimisation]\nMaxSimultaneousSounds=4\n");
}
}
