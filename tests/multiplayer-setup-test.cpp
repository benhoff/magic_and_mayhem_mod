#include "multiplayer_setup_widget.hpp"
#include "menu_preview.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
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
  QTemporaryDir temporary;require(temporary.isValid(),"fixture directory");const auto root=temporary.path();require(QDir().mkpath(root+"/CFG"),"strings directory");
  QByteArray strings("[STRINGS]\n");for(int i=0;i<100;++i)strings+=QString("STR_%1=Fixture %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();write(root+"/CFG/interface screens text.cfg",strings);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));
  auto fixture=[&](bool create){
   const auto folder=root+(create?"/Interface/SetMultiplayerScreen":"/Interface/JoinMultiplayerScreen");require(QDir().mkpath(folder+"/800x600"),"screen directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"background");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n");
   for(int i=0;i<(create?4:3);++i){const int top=(create?52:100)+i*60;cfg+=QString("[TEXT_%1]\nRect2=100,%2,700,%3\nFont=%4\nTextFlags=MIDDLE\nText=%5\n").arg(i+1).arg(top).arg(top+40).arg(i==0?"LARGE":"SMALL").arg(i).toLatin1();}
   for(int i=0;i<(create?2:1);++i)cfg+=QString("[EDITBOX_%1]\nRect2=160,%2,640,%3\nFont=LARGE\n").arg(i+1).arg(create?160+90*i:210).arg(create?200+90*i:250).toLatin1();
   for(int i=0;i<3;++i)cfg+=QString("[RADIOBUTTON_%1]\nRect2=275,%2,700,%3\nFont=SMALL\nTextFlags=LEFT\nText=%4\n").arg(i+1).arg((create?360:330)+30*i).arg((create?390:360)+30*i).arg(44+i).toLatin1();
   cfg+=QString("[TEXTBUTTON_1]\nRect2=75,%1,275,%2\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=525,%1,725,%2\nFont=LARGE\nText=11\n").arg(create?515:465).arg(create?570:515).toLatin1();
   const auto config=folder+(create?"/Screen (Set Multiplayer Game).cfg":"/Screen (Join Multiplayer Game).cfg");write(config,cfg);return std::make_pair(config,cfg);
  };
  const auto joinFixture=fixture(false),createFixture=fixture(true);
  const auto sessionFolder=root+"/Interface/MultiplayerGameSelect";
  require(QDir().mkpath(sessionFolder+"/800x600")&&image.save(sessionFolder+"/800x600/Fixture 800-600.JPG","JPG"),"session assets");
  write(sessionFolder+"/Screen (Multiplayer Game Selection).cfg","[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,82,700,132\nFont=LARGE\nTextFlags=MIDDLE\nText=41\n[LISTBOX_1]\nRect2=165,175,635,445\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=75,475,275,525\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=525,475,725,525\nFont=LARGE\nText=11\n");
  for(auto mode:{MultiplayerSetupWidget::Mode::Join,MultiplayerSetupWidget::Mode::Create}){
   const bool create=mode==MultiplayerSetupWidget::Mode::Create;MultiplayerSetupWidget widget(mode);QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid mode assets");widget.show();widget.focusFirstField();app.processEvents();
   auto* first=widget.findChild<QLineEdit*>("multiplayerEdit1");auto* second=widget.findChild<QLineEdit*>("multiplayerEdit2");auto* ok=widget.findChild<QPushButton*>("multiplayerOk");auto* cancel=widget.findChild<QPushButton*>("multiplayerCancel");auto radio=[&](int i){return widget.findChild<QRadioButton*>(QString("multiplayerTransport%1").arg(i));};
   require(first->hasFocus()&&second->isVisible()==create&&widget.findChild<QLabel*>("multiplayerText4")->isVisible()==create&&radio(2)->isChecked()&&radio(1)->text()=="Fixture 44"&&!ok->isEnabled(),"mode controls, labels, sample transport and empty guard");
   int submitted=0,cancelled=0;MultiplayerSetupWidget::Request received;QObject::connect(&widget,&MultiplayerSetupWidget::requestSubmitted,&widget,[&](const auto& value){++submitted;received=value;});QObject::connect(&widget,&MultiplayerSetupWidget::cancelled,&widget,[&]{++cancelled;});
   ok->click();require(submitted==0,"empty submission blocked");first->setText("   ");if(create)second->setText("Player");ok->click();require(submitted==0&&!ok->isEnabled(),"whitespace field blocked");
   MultiplayerSetupWidget::Form form;form.userName="  Player  ";form.gameName=create?"  Game  ":QString();require(widget.setForm(form,&error)&&ok->isEnabled(),"valid form");
   for(int i=1;i<=3;++i){radio(i)->click();int checked=0;for(int j=1;j<=3;++j)checked+=radio(j)->isChecked();require(checked==1,"exclusive transports");ok->click();require(submitted==i&&received.mode==mode&&int(received.transport)==i-1&&received.userName=="Player"&&received.gameName==(create?"Game":""),"typed trimmed request and mode mapping");}
   auto invalid=form;invalid.transport=MultiplayerSetupWidget::Transport(99);require(!widget.setForm(invalid,&error)&&!error.isEmpty()&&widget.form().transport==MultiplayerSetupWidget::Transport::NullModem&&widget.form().userName==form.userName,"invalid enum transactional");invalid=form;invalid.userName=QString(65,'x');require(!widget.setForm(invalid,&error)&&widget.form().userName==form.userName,"oversized name rejected");invalid=form;invalid.userName="Two\nlines";require(!widget.setForm(invalid,&error),"control characters rejected");
   if(!create){invalid=form;invalid.gameName="Hidden game";require(!widget.setForm(invalid,&error)&&second->text().isEmpty(),"Join has no game field leakage");}
   first->setFocus();QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(first,&enter);require(submitted==4,"editor Enter once");QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(first,&repeat);require(submitted==4,"repeat ignored");
   QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(radio(1),&escape);require(cancelled==1,"Escape from radio");cancel->setFocus();QApplication::sendEvent(cancel,&enter);require(cancelled==2&&submitted==4,"Enter on Cancel");
   const auto config=create?createFixture:joinFixture;write(config.first,QByteArray(config.second).replace("TextFlags=LEFT","TextFlags=BAD"));require(!widget.loadAssets(root,&error)&&widget.form().userName==form.userName&&radio(1)->text()=="Fixture 44","invalid assets transactional");write(config.first,config.second);require(widget.loadAssets(root,&error)&&widget.form().userName==form.userName,"reload preserves fields");
   widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&first->geometry()==QRect(360,create?160:210,480,40),"mode-specific wide geometry");const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and letterboxing");widget.resize(400,600);app.processEvents();require(first->geometry()==QRect(80,create?230:255,240,20)&&first->font().pixelSize()==13,"scaled inputs");
  }
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  MenuPreview preview;QString error;require(preview.loadAssets(root,true,&error),"preview assets");preview.show();app.processEvents();auto* quick=preview.findChild<QuickBattleMenuWidget*>();auto* stack=preview.findChild<QStackedWidget*>();
  for(int button:{1,0}){quick->findChild<QPushButton*>(QString("quickBattleAction%1").arg(button))->click();app.processEvents();auto* screen=qobject_cast<MultiplayerSetupWidget*>(stack->currentWidget());require(screen&&screen->mode()==(button==1?MultiplayerSetupWidget::Mode::Join:MultiplayerSetupWidget::Mode::Create)&&screen->form().userName=="Sample player","Quick navigation opens matching form");screen->findChild<QPushButton*>("multiplayerOk")->click();if(button==1){
    auto* selection=qobject_cast<MultiplayerGameSelectionWidget*>(stack->currentWidget());require(selection&&selection->selectedSessionId().isEmpty(),"Join opens sample sessions without implicit selection");
    selection->findChild<QListWidget*>("multiplayerGameSelectionList")->setCurrentRow(1);selection->findChild<QPushButton*>("multiplayerGameSelectionOk")->click();
    require(stack->currentWidget()==selection&&preview.statusBar()->currentMessage().contains("sample-island as Sample player"),"selected session reports retained Join context");
    selection->findChild<QPushButton*>("multiplayerGameSelectionCancel")->click();require(stack->currentWidget()==screen&&screen->findChild<QPushButton*>("multiplayerOk")->hasFocus(),"session Cancel returns to Join and restores focus");
   }else require(stack->currentWidget()==screen&&preview.statusBar()->currentMessage().contains("networking adapter pending"),"Create remains pending");screen->findChild<QLineEdit*>("multiplayerEdit1")->setText(button==1?"Remembered player":"Remembered game");screen->findChild<QPushButton*>("multiplayerCancel")->click();require(stack->currentWidget()==quick&&quick->findChild<QPushButton*>(QString("quickBattleAction%1").arg(button))->hasFocus(),"Cancel returns and restores initiating focus");quick->findChild<QPushButton*>(QString("quickBattleAction%1").arg(button))->click();require(screen->findChild<QLineEdit*>("multiplayerEdit1")->text()==(button==1?"Remembered player":"Remembered game"),"independent drafts survive reopening");screen->findChild<QPushButton*>("multiplayerCancel")->click();}
  require(preview.openMultiplayerGameSelection(root,&error),"standalone session route");
  auto* selection=preview.findChild<MultiplayerGameSelectionWidget*>();selection->findChild<QPushButton*>("multiplayerGameSelectionCancel")->click();
  auto* join=qobject_cast<MultiplayerSetupWidget*>(stack->currentWidget());require(join&&join->mode()==MultiplayerSetupWidget::Mode::Join,"standalone Cancel route");
  write(sessionFolder+"/Screen (Multiplayer Game Selection).cfg","bad layout");join->findChild<QPushButton*>("multiplayerOk")->click();
  require(stack->currentWidget()==join&&join->form().userName=="Remembered player"&&preview.statusBar()->currentMessage().contains("failed"),"asset failure keeps Join draft and caller");
  write(root+"/Interface/JoinMultiplayerScreen/800x600/Fixture 800-600.JPG","bad JPEG");MultiplayerSetupWidget damaged(MultiplayerSetupWidget::Mode::Join);require(!damaged.loadAssets(root,&error),"corrupt image rejection");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Multiplayer setup test: %s\n",failure.what());return 1;}
}
