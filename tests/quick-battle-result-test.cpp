#include "quick_battle_result_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size(),"fixture write");}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  QTemporaryDir temporary;require(temporary.isValid(),"temporary directory");const auto root=temporary.path();
  require(QDir().mkpath(root+"/CFG"),"strings directory");
  const std::array<const char*,9> names{"Game Over","Player","Kills","Deaths","Handicap Bonus","Score","Spectate","Continue","Quit"};
  QByteArray strings("[STRINGS]\n");for(int i=0;i<9;++i)strings+=QString("STR_%1=%2\n").arg(i,2,10,QLatin1Char('0')).arg(QString::fromLatin1(names[i])).toLatin1();
  write(root+"/CFG/interface screens text.cfg",strings);
  const auto dir=root+"/Interface/QuickBattleEnd";require(QDir().mkpath(dir+"/800x600"),"result directory");
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"background");
  QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n");
  for(int i=0;i<26;++i){
   const int column=i<6?qMax(0,i-1):(i-6)/4,row=i<6?0:(i-6)%4+1;
   const int left=column==0?80:275+column*100,top=row==0?125:125+75*row;
   const auto r=i==0?QRect(150,50,500,50):QRect(left,top,column==0?295:100,row==0?60:50);
   layout+=QString("[TEXT_%1]\nRect2=%2,%3,%4,%5\nText=%6\nTextflags=%7\nFont=%8\n").arg(i+1).arg(r.left()).arg(r.top()).arg(r.x()+r.width()).arg(r.y()+r.height()).arg(i<6?QString::number(i):QString("\"\"" )).arg(column==0 && i>=6?"LEFT":"MIDDLE").arg(i>0 && i<6?"SMALL":"LARGE").toLatin1();
  }
  for(int i=0;i<4;++i)layout+=QString("[PICTUREBOX_%1]\nRect2=25,%2,75,%3\n").arg(i+1).arg(200+75*i).arg(250+75*i).toLatin1();
  for(int i=0;i<3;++i)layout+=QString("[TEXTBUTTON_%1]\nRect2=%2,525,%3,575\nText=%4\nFont=LARGE\n").arg(i+1).arg(i==2?425:150).arg(i==2?650:375).arg(i+6).toLatin1();
  const auto config=dir+"/screen (Quick Battle End).cfg";write(config,layout);
  QuickBattleResultWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");
  widget.show();app.processEvents();
  auto label=[&](int n){return widget.findChild<QLabel*>(QString("quickResultText%1").arg(n));};
  auto button=[&](int n){return widget.findChild<QPushButton*>(QString("quickResultAction%1").arg(n));};
  require(label(1)->text()=="Game Over" && label(5)->wordWrap() && label(5)->font().pixelSize()==22,"configured headings and font roles");
  require(label(7)->isHidden() && !button(0)->isEnabled() && !button(1)->isEnabled(),"empty model has no invented players or primary action");
  QuickBattleResultWidget::Results results;results.primaryAction=QuickBattleResultWidget::PrimaryAction::Continue;
  for(int i=0;i<4;++i){auto& p=results.players[i];p.active=true;p.name=QString("<b>Player %1</b>").arg(i+1);p.portraitText=QString("P%1").arg(i+1);p.kills=QString::number(10+i);p.deaths=QString::number(20+i);p.handicapBonus=QString::number(30+i);p.score=QString::number(40+i);}
  widget.setResults(results);app.processEvents();
  for(int i=0;i<4;++i){
   require(label(7+i)->text()==results.players[i].name && label(11+i)->text()==QString::number(10+i) && label(15+i)->text()==QString::number(20+i) && label(19+i)->text()==QString::number(30+i) && label(23+i)->text()==QString::number(40+i),"column-major player mapping");
   require(label(7+i)->textFormat()==Qt::PlainText && widget.findChild<QLabel*>(QString("quickResultPortrait%1").arg(i+1))->text()==QString("P%1").arg(i+1),"plain names and text portraits");
  }
  require(button(0)->isHidden() && !button(0)->isEnabled() && button(1)->isVisible() && button(1)->hasFocus() && button(0)->geometry()==button(1)->geometry(),"overlapping primary actions exclusive");
  int calls=0;auto action=QuickBattleResultWidget::Action::Quit;
  QObject::connect(&widget,&QuickBattleResultWidget::actionRequested,&widget,[&](auto a){++calls;action=a;});
  button(0)->click();require(calls==0,"unavailable Spectate blocked");button(1)->click();require(calls==1&&action==QuickBattleResultWidget::Action::Continue,"Continue intent");
  QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(button(1),&enter);require(calls==2&&action==QuickBattleResultWidget::Action::Continue,"Enter from child");
  results.primaryAction=QuickBattleResultWidget::PrimaryAction::Spectate;results.canQuit=false;results.players[3]={};widget.setResults(results);app.processEvents();
  require(button(0)->hasFocus() && button(1)->isHidden() && !button(2)->isEnabled() && label(10)->isHidden() && label(26)->text().isEmpty(),"state switch clears stale row and updates focus");
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(button(0),&escape);button(2)->click();button(1)->click();require(calls==2,"blocked Quit and hidden Continue");
  button(0)->click();require(calls==3&&action==QuickBattleResultWidget::Action::Spectate,"Spectate intent");
  QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(button(0),&repeat);require(calls==3,"repeat ignored");
  results.primaryAction=QuickBattleResultWidget::PrimaryAction::None;widget.setResults(results);require(widget.hasFocus(),"all unavailable focus fallback");
  QApplication::sendEvent(&widget,&enter);require(calls==3,"no primary action invented");
  results.canQuit=true;widget.setResults(results);QApplication::sendEvent(button(2),&escape);require(calls==4&&action==QuickBattleResultWidget::Action::Quit,"Escape emits available Quit");
  widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&button(2)->geometry()==QRect(625,525,225,50),"wide layout");
  const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and letterboxing");
  widget.resize(400,600);app.processEvents();require(button(2)->geometry()==QRect(213,413,113,25),"scaled layout");
  write(config,QByteArray(layout).replace("Textflags=MIDDLE","Textflags=BAD"));require(!widget.loadAssets(root,&error)&&!error.isEmpty()&&label(1)->text()=="Game Over"&&button(2)->isEnabled(),"transactional invalid reload");write(config,layout);
  // Supply independent native Main and Quick navigation fixtures.
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");
   QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");
   for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  widget.hide();MenuPreview preview;require(preview.loadAssets(root,false,&error),"navigation load");preview.show();app.processEvents();
  require(preview.openQuickBattleResults(root,QuickBattleResultWidget::PrimaryAction::Spectate,&error),"spectate preview");app.processEvents();
  auto* result=preview.findChild<QuickBattleResultWidget*>();auto* stack=preview.findChild<QStackedWidget*>();result->findChild<QPushButton*>("quickResultAction0")->click();
  require(stack->currentWidget()==result && preview.statusBar()->currentMessage().contains("adapter pending"),"Spectate remains pending");
  result->findChild<QPushButton*>("quickResultAction2")->click();require(stack->currentWidget()==preview.findChild<MainMenuWidget*>(),"Quit returns to Main preview");
  require(preview.openQuickBattleResults(root,QuickBattleResultWidget::PrimaryAction::Continue,&error),"continue preview");result->findChild<QPushButton*>("quickResultAction1")->click();require(stack->currentWidget()==preview.findChild<QuickBattleMenuWidget*>(),"Continue returns to Quick preview");
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt background rejection");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Quick Battle result test: %s\n",failure.what());return 1;}
}
