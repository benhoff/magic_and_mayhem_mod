#include "multiplayer_game_selection_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size(),"fixture write");}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  QTemporaryDir temporary;require(temporary.isValid(),"fixture directory");const auto root=temporary.path();
  require(QDir().mkpath(root+"/CFG")&&QDir().mkpath(root+"/Interface/MultiplayerGameSelect/800x600"),"directories");
  write(root+"/CFG/interface screens text.cfg","[STRINGS]\nSTR_00=Fixture\nSTR_01=Fixture\nSTR_02=Fixture\nSTR_03=Fixture\nSTR_04=Fixture\nSTR_05=Fixture\nSTR_10=OK\nSTR_11=Cancel\nSTR_66=Select Game\n");
  const auto dir=root+"/Interface/MultiplayerGameSelect";
  const QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,82,700,132\nFont=LARGE\nTextFlags=MIDDLE\nText=66\n[LISTBOX_1]\nRect2=165,175,635,445\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=75,475,275,525\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=525,475,725,525\nFont=LARGE\nText=11\n");
  const auto config=dir+"/Screen (Multiplayer Game Selection).cfg";write(config,layout);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"image");
  MultiplayerGameSelectionWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");
  widget.show();widget.focusSelection();app.processEvents();auto* list=widget.findChild<QListWidget*>("multiplayerGameSelectionList");auto* ok=widget.findChild<QPushButton*>("multiplayerGameSelectionOk");auto* cancel=widget.findChild<QPushButton*>("multiplayerGameSelectionCancel");
  require(widget.findChild<QLabel*>("multiplayerGameSelectionHeading")->text()=="Select Game"&&ok->text()=="OK"&&cancel->text()=="Cancel"&&cancel->hasFocus(),"configured labels and empty focus");
  int confirmed=0,cancelled=0;QString id;QObject::connect(&widget,&MultiplayerGameSelectionWidget::sessionSelected,&widget,[&](const QString& value){++confirmed;id=value;});QObject::connect(&widget,&MultiplayerGameSelectionWidget::cancelled,&widget,[&]{++cancelled;});
  ok->click();require(confirmed==0&&!ok->isEnabled(),"empty selection guard");
  const QVector<MultiplayerGameSelectionWidget::Session> sessions{{"forest","Same name"},{"plains","Same name"},{"island","<b>Island</b>"}};
  require(widget.setSessions(sessions,QString(),&error)&&list->count()==3&&widget.selectedSessionId().isEmpty()&&!ok->isEnabled(),"no implicit first selection");
  widget.focusSelection();require(list->hasFocus(),"list focus");
  list->setCurrentRow(0);ok->click();require(confirmed==1&&id=="forest","stable ID confirmation");
  QKeyEvent down(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);QApplication::sendEvent(list,&down);require(widget.selectedSessionId()=="plains","keyboard selection");
  QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(list,&enter);require(confirmed==2&&id=="plains","Enter confirms once");
  QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(list,&repeat);require(confirmed==2,"list Enter repeat suppressed");
  require(widget.setSessions({sessions[2],sessions[1],sessions[0]},QString(),&error)&&widget.selectedSessionId()=="plains"&&list->currentRow()==1,"refresh preserves ID");
  require(widget.setSessions(sessions,"island",&error)&&widget.selectedSessionId()=="island"&&list->currentItem()->text()=="<b>Island</b>","explicit selection and literal item text");
  for(const auto& invalid:QVector<QVector<MultiplayerGameSelectionWidget::Session>>{{{"x","One"},{"x","Two"}},{{"","Empty ID"}},{{"x",""}}})require(!widget.setSessions(invalid,QString(),&error)&&!error.isEmpty()&&widget.selectedSessionId()=="island"&&list->count()==3,"invalid model transactional");
  require(!widget.setSessions(sessions,"missing",&error)&&widget.selectedSessionId()=="island","unknown explicit ID rejected");
  require(widget.setSessions({sessions[0]},QString(),&error)&&widget.selectedSessionId().isEmpty()&&!ok->isEnabled(),"removed selection cleared");
  list->setCurrentRow(0);list->clearSelection();ok->click();require(widget.selectedSessionId().isEmpty()&&confirmed==2&&!ok->isEnabled(),"current item without selection cannot confirm");
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(list,&escape);require(cancelled==1,"Escape from list");
  cancel->setFocus();QApplication::sendEvent(cancel,&enter);require(cancelled==2&&confirmed==2,"Enter on Cancel");
  widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&list->geometry()==QRect(365,175,470,270),"wide layout");
  const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and letterbox");
  widget.resize(400,600);app.processEvents();require(list->geometry()==QRect(83,238,235,135)&&list->font().pixelSize()==11,"scaled list and font");
  write(config,QByteArray(layout).replace("165,175,635,445","165,175,900,445"));require(!widget.loadAssets(root,&error)&&!error.isEmpty()&&list->count()==1&&ok->text()=="OK","invalid layout transactional");write(config,layout);
  list->setCurrentRow(0);list->itemActivated(list->currentItem());require(confirmed==3&&id=="forest","activation confirms current selected ID");
  require(widget.setSessions({},QString(),&error)&&list->count()==0&&!ok->isEnabled()&&widget.selectedSessionId().isEmpty(),"empty refresh clears selection");
  widget.focusSelection();require(cancel->hasFocus(),"empty refresh focuses Cancel");QApplication::sendEvent(widget.findChild<QListWidget*>("multiplayerGameSelectionList"),&enter);require(confirmed==3,"empty Enter cannot confirm");
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt background");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Select Game test: %s\n",failure.what());return 1;}
}
