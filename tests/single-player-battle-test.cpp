#include "single_player_battle_widget.hpp"
#include "menu_preview.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* why) {if (!ok) throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes) {QFile file(path);require(file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size(),"fixture write");}
static QString rect(int x,int y,int w,int h) {return QString("%1,%2,%3,%4").arg(x).arg(y).arg(x+w).arg(y+h);}
int main(int argc,char** argv) {
 QApplication app(argc,argv);
 try {
    QTemporaryDir temporary;require(temporary.isValid(),"temporary directory");const auto root=temporary.path();
    QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));
    auto folder=[&](const QString& name) {
        const auto path=root+"/Interface/"+name;require(QDir().mkpath(path+"/800x600")&&image.save(path+"/800x600/Fixture 800-600.JPG","JPG"),"fixture image");return path;
    };
    require(QDir().mkpath(root+"/CFG"),"strings folder");
    QByteArray strings("[STRINGS]\nSTR_00=Menu\nSTR_01=Menu\nSTR_02=Menu\nSTR_03=Menu\nSTR_04=Menu\nSTR_05=Menu\nSTR_10=OK\nSTR_11=Cancel\nSTR_52=Map\nSTR_53=Start\nSTR_55=No Player\nSTR_66=Select Map\nSTR_87=Single Player Battle\n");
    const std::array<QString,13> names{"Mana","Health","Magic Items","Selection time","Law Talismans","Neutral Talismans","Chaos Talismans","Game Time","Lives","Places of Power","Mana Sprites","Artefacts","Control Limit"};
    for (int i=0;i<13;++i) strings+=QString("STR_%1=%2\n").arg(100+i).arg(names[i]).toLatin1();
    write(root+"/CFG/interface screens text.cfg",strings);
    const auto singleFolder=folder("SinglePlayerBattle");const auto config=singleFolder+"/screen (Single Player Battle).cfg";
    QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n");
    for (int i=0;i<40;++i) {
        QString r,text="\"0\"",alignment="LEFT";
        if(i<4){r=rect(75,25+i*75,225,25);text="55";}
        else if(i<8) r=rect(75,50+(i-4)*75,50,25);
        else if(i<12){r=rect(125,50+(i-8)*75,200,25);text="\"\"";}
        else if(i==12){r=rect(400,25,375,50);text="87";alignment="MIDDLE";}
        else if(i==13){r=rect(460,100,315,35);text="\"Large Medieval Forest\"";alignment="MIDDLE";}
        else if(i<27){r=rect(400,150+(i-14)*25,150,25);text=QString::number(100+i-14);}
        else r=rect(550,150+(i-27)*25,50,25);
        layout+=QString("[TEXT_%1]\nFont=SMALL\nRect2=%2\nTextflags=%3\nText=%4\n").arg(i+1).arg(r,alignment,text).toLatin1();
    }
    const std::array<int,17> minimum{50,100,0,30,0,0,0,0,1,0,0,0,0,0,0,0,0};
    const std::array<int,17> maximum{200,800,21,3000,7,7,7,240,20,15,50,50,30,50,50,50,50};
    const std::array<int,17> step{10,20,1,10,1,1,1,5,1,1,1,5,1,5,5,5,5};
    for (int i=0;i<17;++i) layout+=QString("[SLIDERBAR_%1]\nRect2=%2\nminValue=%3\nmaxValue=%4\nstep=%5\n").arg(i+1).arg(i<13?rect(600,150+i*25,175,25):rect(125,50+(i-13)*75,200,25)).arg(minimum[i]).arg(maximum[i]).arg(step[i]).toLatin1();
    const std::array<QString,4> standard{rect(325,25,50,50),rect(30,30,50,50),rect(290,175,25,25),rect(290,250,25,25)};
    for (int i=0;i<4;++i) layout+=QString("[STANDARDBUTTON_%1]\nRect2=%2\n").arg(i+1).arg(standard[i]).toLatin1();
    for (int i=0;i<6;++i) layout+=QString("[PICTUREBOX_%1]\nRect2=%2\n").arg(i+1).arg(i<3?rect(325,100+i*75,50,50):rect(30,105+(i-3)*75,45,45)).toLatin1();
    const std::array<QString,3> buttonRects{rect(400,490,175,35),rect(600,490,175,35),rect(400,100,60,35)};
    const std::array<int,3> buttonIds{11,53,52};
    for (int i=0;i<3;++i) layout+=QString("[TEXTBUTTON_%1]\nFont=SMALL\nRect2=%2\nText=%3\n").arg(i+1).arg(buttonRects[i]).arg(buttonIds[i]).toLatin1();
    write(config,layout);
    SinglePlayerBattleWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets including quoted literals");widget.show();app.processEvents();
    auto button=[&](const char* name){return widget.findChild<QPushButton*>(name);};
    auto slider=[&](int n){return widget.findChild<QSlider*>(QString("singlePlayerSlider%1").arg(n));};
    auto label=[&](int n){return widget.findChild<QLabel*>(QString("singlePlayerText%1").arg(n));};
    require(label(13)->text()=="Single Player Battle"&&label(1)->text()=="No Player"&&!label(9)->isVisible()&&button("singlePlayerMap")->text()=="Map","static string and empty overlay handling");
    require(!button("singlePlayerStart")->isEnabled()&&!slider(14)->isEnabled(),"empty model cannot start");
    int starts=0,cancels=0,maps=0,changed=-1,colour=-1,removed=-1;SinglePlayerBattleWidget::Setup submitted;
    QObject::connect(&widget,&SinglePlayerBattleWidget::startRequested,&widget,[&](const auto& setup){++starts;submitted=setup;});
    QObject::connect(&widget,&SinglePlayerBattleWidget::cancelled,&widget,[&]{++cancels;});QObject::connect(&widget,&SinglePlayerBattleWidget::mapRequested,&widget,[&]{++maps;});
    QObject::connect(&widget,&SinglePlayerBattleWidget::playerChangeRequested,&widget,[&](int slot){changed=slot;});QObject::connect(&widget,&SinglePlayerBattleWidget::colourChangeRequested,&widget,[&](int slot){colour=slot;});QObject::connect(&widget,&SinglePlayerBattleWidget::playerRemovalRequested,&widget,[&](int slot){removed=slot;});
    QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(slider(1),&enter);require(starts==0,"guarded keyboard start");
    SinglePlayerBattleWidget::Setup setup;setup.mapId="opaque-map";setup.mapName="<b>Sample map</b>";
    for(int i=0;i<4;++i) setup.players[i]={true,QString("Player %1").arg(i),"wizard","W1",QString::number(i),"Red",i*5};
    require(widget.setSetup(setup,&error)&&button("singlePlayerStart")->isEnabled()&&label(14)->text()==setup.mapName&&label(14)->textFormat()==Qt::PlainText,"valid model and literal map text");
    for(int i=0;i<17;++i){require(slider(i+1)->minimum()==minimum[i]&&slider(i+1)->maximum()==maximum[i]&&slider(i+1)->singleStep()==step[i],"configured ranges and steps");slider(i+1)->setValue(maximum[i]);require((i<13?label(28+i):label(5+i-13))->text()==QString::number(maximum[i]),"live numeric labels");}
    button("singlePlayerStart")->click();require(starts==1&&submitted.mapId=="opaque-map"&&submitted.values[3]==3000&&submitted.values[8]==20&&submitted.players[3].handicap==50,"typed complete setup request");
    slider(1)->setValue(56);require(slider(1)->value()==60&&widget.setup().values[0]==60,"pointer values snap to step");slider(14)->setValue(7);require(slider(14)->value()==5&&label(5)->text()=="5","handicap snap and label");
    QKeyEvent left(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);QApplication::sendEvent(slider(1),&left);require(widget.setup().values[0]==50,"native arrow step");
    QApplication::sendEvent(slider(1),&enter);require(starts==2,"Enter starts once from slider");QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(slider(1),&repeat);require(starts==2,"repeat suppressed");
    button("singlePlayerMap")->setFocus();QApplication::sendEvent(button("singlePlayerMap"),&enter);require(maps==1&&starts==2,"Enter respects Map focus");
    for(int i=0;i<4;++i){button(qPrintable(QString("singlePlayerPortrait%1").arg(i)))->click();button(qPrintable(QString("singlePlayerColour%1").arg(i)))->click();require(changed==i&&colour==i,"slot-specific portrait and colour intent");}
    button("singlePlayerRemove2")->click();require(removed==2,"first remove slot");button("singlePlayerRemove3")->click();require(removed==3,"second remove slot");
    require(widget.setSetup(setup,&error),"reset model");auto invalid=setup;invalid.values[0]=51;require(!widget.setSetup(invalid,&error)&&widget.setup().values[0]==50,"off-step model rejected without mutation");invalid=setup;invalid.players[0].handicap=55;require(!widget.setSetup(invalid,&error),"out of range handicap");invalid=setup;invalid.mapName.clear();require(!widget.setSetup(invalid,&error),"incomplete map rejected");invalid=setup;invalid.players[1].portraitId.clear();require(!widget.setSetup(invalid,&error),"incomplete active player rejected");
    auto draft=setup;draft.players[0].active=false;require(widget.setSetup(draft)&&!button("singlePlayerStart")->isEnabled(),"human required");draft=setup;for(int i=1;i<4;++i)draft.players[i].active=false;require(widget.setSetup(draft)&&!button("singlePlayerStart")->isEnabled()&&!button("singlePlayerColour2")->isEnabled()&&!button("singlePlayerRemove2")->isEnabled()&&button("singlePlayerPortrait2")->isEnabled(),"opponent guard and inactive slot controls");
    require(widget.setSetup(setup),"restore model");QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(slider(4),&escape);require(cancels==1,"Escape from slider");QApplication::sendEvent(button("singlePlayerCancel"),&enter);require(cancels==2&&starts==2,"Enter on Cancel");
    widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&slider(1)->geometry()==QRect(800,150,175,25),"wide geometry");const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,590).blue()-150)<4,"background and letterbox");widget.resize(400,600);app.processEvents();require(slider(1)->geometry()==QRect(300,225,88,13),"scaled geometry");
    for(const auto& broken:{QByteArray(layout).replace("step=10","step=0"),QByteArray(layout).replace("minValue=50","minValue=60"),QByteArray(layout).replace("600,150,775,175","600,150,900,175"),QByteArray(layout).replace("Text=87","Text=999")}){write(config,broken);require(!widget.loadAssets(root,&error)&&!error.isEmpty()&&widget.setup().mapId=="opaque-map"&&slider(1)->minimum()==50&&label(13)->text()=="Single Player Battle","asset rejection is transactional");}write(config,layout);
    for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
        const auto path=folder(entry.first);QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,50,700,100\nText=0\n");for(int i=0;i<6;++i)cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();write(path+"/"+entry.second,cfg);
    }
    const auto mapFolder=folder("MapSelectionScreen");const QByteArray mapLayout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=234,62,568,115\nFont=LARGE\nTextFlags=CENTRE\nText=66\n[LISTBOX_1]\nRect2=165,178,635,448\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=70,530,220,580\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=580,530,730,580\nFont=LARGE\nText=11\n");write(mapFolder+"/screen (Map Selection Screen).cfg",mapLayout);
    widget.hide();MenuPreview preview;require(preview.loadAssets(root,true,&error),"preview load");preview.show();app.processEvents();auto* stack=preview.findChild<QStackedWidget*>();auto* quick=preview.findChild<QuickBattleMenuWidget*>();quick->findChild<QPushButton*>("quickBattleAction2")->click();
    auto* single=qobject_cast<SinglePlayerBattleWidget*>(stack->currentWidget());require(single&&single->setup().mapId=="sample-forest"&&single->setup().players[0].name=="Sample player","Quick route and samples");single->findChild<QSlider*>("singlePlayerSlider1")->setValue(150);single->findChild<QPushButton*>("singlePlayerPortrait2")->click();single->findChild<QPushButton*>("singlePlayerColour0")->click();require(single->setup().players[2].portraitId=="sample-wizard-2"&&single->setup().players[0].colourText=="Blue","local sample selectors");single->findChild<QPushButton*>("singlePlayerRemove2")->click();require(!single->setup().players[2].active,"local removal");single->findChild<QPushButton*>("singlePlayerPortrait2")->click();require(single->setup().players[2].active,"sample restore");
    single->findChild<QPushButton*>("singlePlayerMap")->click();auto* mapsWidget=qobject_cast<MapSelectionWidget*>(stack->currentWidget());require(mapsWidget&&mapsWidget->selectedMapId()=="sample-forest","Map caller selection");mapsWidget->findChild<QListWidget*>("mapSelectionList")->setCurrentRow(1);mapsWidget->findChild<QPushButton*>("mapSelectionOk")->click();require(stack->currentWidget()==single&&single->setup().mapId=="sample-plains"&&single->setup().mapName=="Sample plains map"&&single->setup().values[0]==150&&single->findChild<QPushButton*>("singlePlayerMap")->hasFocus(),"Map OK updates caller and retains draft");
    single->findChild<QPushButton*>("singlePlayerMap")->click();require(mapsWidget->selectedMapId()=="sample-plains","reopen selects caller map");mapsWidget->findChild<QListWidget*>("mapSelectionList")->setCurrentRow(2);mapsWidget->findChild<QPushButton*>("mapSelectionCancel")->click();require(stack->currentWidget()==single&&single->setup().mapId=="sample-plains","Map Cancel keeps previous map");
    write(mapFolder+"/screen (Map Selection Screen).cfg","broken");single->findChild<QPushButton*>("singlePlayerMap")->click();require(stack->currentWidget()==single&&single->setup().values[0]==150&&preview.statusBar()->currentMessage().contains("failed"),"Map asset failure preserves caller");write(mapFolder+"/screen (Map Selection Screen).cfg",mapLayout);
    single->findChild<QPushButton*>("singlePlayerStart")->click();require(stack->currentWidget()==single&&preview.statusBar()->currentMessage().contains("sample-plains")&&preview.statusBar()->currentMessage().contains("adapter pending"),"Start intent pending");single->findChild<QPushButton*>("singlePlayerCancel")->click();require(stack->currentWidget()==quick&&quick->findChild<QPushButton*>("quickBattleAction2")->hasFocus(),"Cancel restores initiating focus");quick->findChild<QPushButton*>("quickBattleAction2")->click();require(stack->currentWidget()==single&&single->setup().mapId=="sample-plains"&&single->setup().values[0]==150,"draft survives reopen");
    require(preview.openMapSelection(root,&error),"Map from Single API");mapsWidget->findChild<QPushButton*>("mapSelectionCancel")->click();single->findChild<QPushButton*>("singlePlayerCancel")->click();require(preview.openMapSelection(root,&error),"standalone Map route");mapsWidget->findChild<QPushButton*>("mapSelectionCancel")->click();require(stack->currentWidget()==quick,"standalone Map return remains Quick");
    write(singleFolder+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt image rejected");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Single Player Battle test: %s\n",failure.what());return 1;}
}
