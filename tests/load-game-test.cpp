#include "load_game_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
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
  require(QDir().mkpath(root+"/CFG")&&QDir().mkpath(root+"/Interface/LoadGame/800x600"),"directories");
  write(root+"/CFG/interface screens text.cfg","[STRINGS]\nSTR_00=Fixture\nSTR_01=Load Game\nSTR_02=Fixture\nSTR_03=Fixture\nSTR_04=Fixture\nSTR_05=Fixture\nSTR_11=Cancel\nSTR_47=Load\nSTR_48=Loading...\n");
  const auto dir=root+"/Interface/LoadGame",config=dir+"/screen (Load Game).cfg";
  const QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_2]\nRect2=230,55,575,102\nFont=LARGE\nTextFlags=CENTRE\nText=1\n[EDITBOX_1]\nRect2=163,150,637,192\nFont=SMALL\n[LISTBOX_1]\nRect2=165,233,635,500\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=70,530,220,580\nFont=LARGE\nText=47\n[TEXTBUTTON_2]\nRect2=580,530,730,580\nFont=LARGE\nText=11\n");write(config,layout);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"image");
  LoadGameWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");widget.show();widget.focusSelection();app.processEvents();
  auto* list=widget.findChild<QListWidget*>("loadGameList");auto* edit=widget.findChild<QLineEdit*>("loadGameFileName");auto* load=widget.findChild<QPushButton*>("loadGameLoad");auto* cancel=widget.findChild<QPushButton*>("loadGameCancel");
  require(widget.findChild<QLabel*>("loadGameHeading")->text()=="Load Game"&&load->text()=="Load"&&cancel->text()=="Cancel"&&cancel->hasFocus(),"configured labels and empty focus");
  int loaded=0,cancelled=0;QString id;QObject::connect(&widget,&LoadGameWidget::loadRequested,&widget,[&](const QString& value){++loaded;id=value;});QObject::connect(&widget,&LoadGameWidget::cancelled,&widget,[&]{++cancelled;});
  load->click();require(loaded==0&&!load->isEnabled(),"empty guard");
  const QVector<LoadGameWidget::Save> saves{{"auto","Autosave"},{"campaign","Campaign"},{"battle","<b>Before battle</b>"}};
  require(widget.setSaves(saves,QString(),&error)&&widget.selectedSaveId().isEmpty()&&edit->text().isEmpty(),"no implicit choice");widget.focusSelection();require(list->hasFocus(),"list focus");
  list->setCurrentRow(0);require(edit->text()=="Autosave"&&load->isEnabled(),"selection populates filename");load->click();require(loaded==1&&id=="auto","opaque ID request");
  QKeyEvent down(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);QApplication::sendEvent(list,&down);require(widget.selectedSaveId()=="campaign"&&edit->text()=="Campaign","keyboard selection synchronization");
  QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(list,&enter);require(loaded==2&&id=="campaign","list Enter once");
  edit->setFocus();edit->setText("Unknown");QApplication::sendEvent(edit,&enter);load->click();require(widget.selectedSaveId().isEmpty()&&!load->isEnabled()&&loaded==2&&edit->text()=="Unknown","unknown typed name blocked and retained");
  edit->setText("Autosave");require(widget.selectedSaveId()=="auto"&&list->currentRow()==0,"typed exact name selects");QApplication::sendEvent(edit,&enter);require(loaded==3&&id=="auto","editor Enter once");
  QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(edit,&repeat);QApplication::sendEvent(list,&repeat);require(loaded==3,"Enter repeats suppressed");
  require(widget.setSaves({saves[2],saves[1],saves[0]},QString(),&error)&&widget.selectedSaveId()=="auto"&&list->currentRow()==2,"refresh stable ID preservation");
  require(widget.setSaves(saves,"battle",&error)&&edit->text()=="<b>Before battle</b>","explicit selection and literal names");
  const QVector<QVector<LoadGameWidget::Save>> invalid{{{"x","One"},{"x","Two"}},{{"x","Same"},{"y","Same"}},{{"","Empty ID"}},{{"x",""}},{{"x","Two\nlines"}},{{"x",QString(257,'x')}}};
  for(const auto& bad:invalid)require(!widget.setSaves(bad,QString(),&error)&&!error.isEmpty()&&widget.selectedSaveId()=="battle"&&list->count()==3,"invalid data transactional");
  require(!widget.setSaves(saves,"missing",&error)&&widget.selectedSaveId()=="battle","missing requested save rejected");
  require(widget.setSaves({saves[0]},QString(),&error)&&widget.selectedSaveId().isEmpty()&&edit->text().isEmpty()&&!load->isEnabled(),"removed selection cleared");
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(edit,&escape);require(cancelled==1,"Escape from edit");cancel->setFocus();QApplication::sendEvent(cancel,&enter);require(cancelled==2&&loaded==3,"Enter on Cancel");
  widget.resize(1200,600);app.processEvents();require(list->geometry()==QRect(365,233,470,267)&&edit->geometry()==QRect(363,150,474,42),"wide geometry");
  const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"artwork and letterbox");
  widget.resize(400,600);app.processEvents();require(edit->geometry()==QRect(82,225,237,21)&&list->font().pixelSize()==10,"scaled edit/list");
  write(config,QByteArray(layout).replace("Font=SMALL","Font=INVALID"));require(!widget.loadAssets(root,&error)&&!error.isEmpty()&&load->text()=="Load"&&list->count()==1,"transactional layout failure");write(config,layout);
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");
   for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  widget.hide();MenuPreview preview;require(preview.loadAssets(root,false,&error),"preview root");preview.show();app.processEvents();auto* main=preview.findChild<MainMenuWidget*>();main->findChild<QPushButton*>("mainMenuAction1")->click();app.processEvents();
  auto* selection=preview.findChild<LoadGameWidget*>();auto* stack=preview.findChild<QStackedWidget*>();require(selection&&stack->currentWidget()==selection&&selection->selectedSaveId()=="sample-autosave","Main Load opens preview");
  selection->findChild<QPushButton*>("loadGameLoad")->click();require(stack->currentWidget()==selection&&preview.statusBar()->currentMessage().contains("sample-autosave")&&preview.statusBar()->currentMessage().contains("adapter pending"),"load intent stays pending");
  selection->findChild<QPushButton*>("loadGameCancel")->click();require(stack->currentWidget()==main,"Cancel returns to Main");
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt background rejection");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Load Game test: %s\n",failure.what());return 1;}
}
