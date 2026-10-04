#include "preferences_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <array>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size(),"fixture write");}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  QTemporaryDir temporary;require(temporary.isValid(),"fixture directory");const auto root=temporary.path();
  require(QDir().mkpath(root+"/CFG")&&QDir().mkpath(root+"/Interface/BattleOptionsScreen/800x600"),"directories");
  QByteArray strings("[STRINGS]\n");for(int i=0;i<40;++i)strings+=QString("STR_%1=Fixture %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();write(root+"/CFG/interface screens text.cfg",strings);
  const auto dir=root+"/Interface/BattleOptionsScreen",config=dir+"/screen (Battle Options).cfg";
  QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n");
  for(int i=0;i<11;++i){const int top=i==0?30:100+i*25;layout+=QString("[TEXT_%1]\nRect2=100,%2,700,%3\nFont=%4\nTextFlags=%5\nText=%6\n").arg(i+1).arg(top).arg(top+25).arg(i==0?"LARGE":"SMALL").arg(i==0?"CENTRE":"LEFT").arg(i==1||i==4||i==7?QString():QString::number(i)).toLatin1();}
  for(int i=0;i<12;++i){const int left=i<6?100:500,top=240+(i%6)*25;layout+=QString("[RADIOBUTTON_%1]\nRect2=%2,%3,%4,%5\nFont=SMALL\nText=%6\n").arg(i+1).arg(left).arg(top).arg(left+300).arg(top+25).arg(i+12).toLatin1();}
  for(int i=0;i<2;++i)layout+=QString("[SLIDERBAR_%1]\nRect2=100,%2,325,%3\nminValue=-5000\nmaxValue=0\n").arg(i+1).arg(145+i*50).arg(175+i*50).toLatin1();
  layout+="[TEXTBUTTON_1]\nRect2=40,530,280,570\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=530,530,760,570\nFont=LARGE\nText=11\n";write(config,layout);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"image");
  PreferencesWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");widget.show();widget.focusFirstControl();app.processEvents();
  auto radio=[&](int i){return widget.findChild<QRadioButton*>(QString("preferencesRadio%1").arg(i));};auto slider=[&](int i){return widget.findChild<QSlider*>(QString("preferencesSlider%1").arg(i));};
  auto* ok=widget.findChild<QPushButton*>("preferencesOk");auto* cancel=widget.findChild<QPushButton*>("preferencesCancel");
  require(slider(1)->hasFocus()&&slider(1)->minimum()==-5000&&slider(2)->maximum()==0&&widget.findChild<QLabel*>("preferencesText2")->isHidden()&&radio(1)->text()=="Fixture 12","roles, bounds, blank headings and labels");
  const auto initial=widget.settings();int applied=0,cancelled=0;PreferencesWidget::Settings observed;
  QObject::connect(&widget,&PreferencesWidget::settingsApplied,&widget,[&](const auto& value){++applied;observed=value;});QObject::connect(&widget,&PreferencesWidget::cancelled,&widget,[&]{++cancelled;});
  const std::array<std::pair<int,int>,5> groups{{{1,2},{3,4},{5,7},{8,10},{11,12}}};
  for(int i=1;i<=12;++i){radio(i)->click();for(const auto& group:groups){int checked=0;for(int j=group.first;j<=group.second;++j)checked+=radio(j)->isChecked();require(checked==1,"each group remains exclusive and independent");}}
  slider(1)->setValue(-5000);slider(2)->setValue(0);const auto draft=widget.draftSettings();
  require(widget.settings()==initial&&!(draft==initial)&&draft.resolution==PreferencesWidget::Resolution::Low&&draft.animation==PreferencesWidget::Animation::Cut&&draft.dialogueSpeed==PreferencesWidget::Speed::Slow&&draft.gameSpeed==PreferencesWidget::Speed::Slow&&!draft.borderPicture&&applied==0,"draft model separate from accepted snapshot");
  cancel->click();require(cancelled==1&&widget.draftSettings()==initial&&widget.settings()==initial&&applied==0,"Cancel restores snapshot");
  radio(2)->click();slider(1)->setValue(-3500);const auto selected=widget.draftSettings();ok->click();require(applied==1&&observed==selected&&widget.settings()==selected,"OK commits full local model");
  radio(1)->click();slider(2)->setValue(-4000);QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(radio(1),&escape);require(cancelled==2&&widget.draftSettings()==selected,"Escape from radio restores last accepted");
  slider(1)->setFocus();QKeyEvent end(QEvent::KeyPress,Qt::Key_End,Qt::NoModifier);QApplication::sendEvent(slider(1),&end);require(slider(1)->value()==0,"keyboard slider bound");QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(slider(1),&enter);require(applied==2&&widget.settings().musicLevel==0,"Enter from slider applies once");
  QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(slider(1),&repeat);require(applied==2,"repeat ignored");
  const auto accepted=widget.settings();auto invalid=accepted;invalid.musicLevel=-5001;require(!widget.setSettings(invalid,&error)&&!error.isEmpty()&&widget.settings()==accepted&&widget.draftSettings()==accepted,"invalid slider snapshot transactional");invalid=accepted;invalid.gameSpeed=PreferencesWidget::Speed(99);require(!widget.setSettings(invalid,&error)&&widget.settings()==accepted,"invalid enum rejected");
  radio(2)->click();const auto beforeReload=widget.draftSettings();require(widget.loadAssets(root,&error)&&widget.draftSettings()==beforeReload&&widget.settings()==accepted,"asset reload preserves draft and accepted");write(config,QByteArray(layout).replace("maxValue=0","maxValue=-6000"));require(!widget.loadAssets(root,&error)&&widget.draftSettings()==beforeReload&&slider(1)->maximum()==0,"bad bounds transactional");write(config,layout);
  widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&slider(1)->geometry()==QRect(300,145,225,30),"wide layout");const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and bars");widget.resize(400,600);app.processEvents();require(slider(1)->geometry()==QRect(50,223,113,15)&&radio(1)->font().pixelSize()==10,"scaled controls");
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  const auto miniDir=root+"/Interface/MiniMenu";require(QDir().mkpath(miniDir+"/800x600"),"mini directory");require(image.copy(0,0,600,400).save(miniDir+"/800x600/Fixture 800-600.BMP","BMP"),"mini image");QByteArray mini("[GLOBALS]\nBackgroundFile=Fixture\n");for(int i=0;i<8;++i){mini+=QString("[TEXTBUTTON_%1]\nRect2=250,200,550,240\nText=0\n").arg(i+1).toLatin1();}write(miniDir+"/screen (Mini Menu).cfg",mini);
  widget.hide();MenuPreview preview;require(preview.loadAssets(root,false,&error),"preview assets");preview.show();app.processEvents();auto* main=preview.findChild<MainMenuWidget*>();main->findChild<QPushButton*>("mainMenuAction3")->click();app.processEvents();auto* screen=preview.findChild<PreferencesWidget*>();auto* stack=preview.findChild<QStackedWidget*>();require(screen&&stack->currentWidget()==screen,"Main opens Preferences");screen->findChild<QSlider*>("preferencesSlider1")->setValue(-4200);screen->findChild<QPushButton*>("preferencesOk")->click();require(stack->currentWidget()==main&&preview.statusBar()->currentMessage().contains("accepted locally"),"apply returns to caller");main->findChild<QPushButton*>("mainMenuAction3")->click();require(screen->draftSettings().musicLevel==-4200,"local settings survive reopening");screen->findChild<QSlider*>("preferencesSlider1")->setValue(-2500);screen->findChild<QPushButton*>("preferencesCancel")->click();require(screen->settings().musicLevel==-4200&&stack->currentWidget()==main,"preview rollback");
  for(auto mode:{MiniMenuWidget::Mode::Campaign,MiniMenuWidget::Mode::Battle}){require(preview.openMiniMenu(root,mode,&error),"Mini Menu open");auto* caller=preview.findChild<MiniMenuWidget*>();caller->findChild<QPushButton*>(mode==MiniMenuWidget::Mode::Campaign?"miniMenuButton3":"miniMenuButton6")->click();require(stack->currentWidget()==screen,"Mini preferences route");screen->findChild<QPushButton*>("preferencesCancel")->click();require(stack->currentWidget()==caller&&caller->mode()==mode,"return to matching Mini mode");}
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt image rejection");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Preferences test: %s\n",failure.what());return 1;}
}
