#include "map_selection_widget.hpp"
#include "menu_preview.hpp"
#include "quick_battle_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
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
  require(QDir().mkpath(root+"/CFG")&&QDir().mkpath(root+"/Interface/MapSelectionScreen/800x600"),"directories");
  write(root+"/CFG/interface screens text.cfg","[STRINGS]\nSTR_00=Fixture\nSTR_01=Fixture\nSTR_02=Fixture\nSTR_03=Fixture\nSTR_04=Fixture\nSTR_05=Fixture\nSTR_10=OK\nSTR_11=Cancel\nSTR_49=Map Selection\n");
  const auto dir=root+"/Interface/MapSelectionScreen";
  const QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=234,62,568,115\nFont=LARGE\nTextFlags=CENTRE\nText=49\n[LISTBOX_1]\nRect2=165,178,635,448\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=70,530,220,580\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=580,530,730,580\nFont=LARGE\nText=11\n");
  const auto config=dir+"/screen (Map Selection Screen).cfg";write(config,layout);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"image");
  MapSelectionWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");
  widget.show();widget.focusSelection();app.processEvents();auto* list=widget.findChild<QListWidget*>("mapSelectionList");auto* ok=widget.findChild<QPushButton*>("mapSelectionOk");auto* cancel=widget.findChild<QPushButton*>("mapSelectionCancel");
  require(widget.findChild<QLabel*>("mapSelectionHeading")->text()=="Map Selection"&&ok->text()=="OK"&&cancel->text()=="Cancel"&&cancel->hasFocus(),"configured labels and empty focus");
  int confirmed=0,cancelled=0;QString id;QObject::connect(&widget,&MapSelectionWidget::mapSelected,&widget,[&](const QString& value){++confirmed;id=value;});QObject::connect(&widget,&MapSelectionWidget::cancelled,&widget,[&]{++cancelled;});
  ok->click();require(confirmed==0&&!ok->isEnabled(),"empty selection guard");
  const QVector<MapSelectionWidget::Map> maps{{"forest","Same name"},{"plains","Same name"},{"island","<b>Island</b>"}};
  require(widget.setMaps(maps,QString(),&error)&&list->count()==3&&widget.selectedMapId().isEmpty()&&!ok->isEnabled(),"no implicit first selection");
  widget.focusSelection();require(list->hasFocus(),"list focus");
  list->setCurrentRow(0);ok->click();require(confirmed==1&&id=="forest","stable ID confirmation");
  QKeyEvent down(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);QApplication::sendEvent(list,&down);require(widget.selectedMapId()=="plains","keyboard selection");
  QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(list,&enter);require(confirmed==2&&id=="plains","Enter confirms once");
  QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(list,&repeat);require(confirmed==2,"list Enter repeat suppressed");
  require(widget.setMaps({maps[2],maps[1],maps[0]},QString(),&error)&&widget.selectedMapId()=="plains"&&list->currentRow()==1,"refresh preserves ID");
  require(widget.setMaps(maps,"island",&error)&&widget.selectedMapId()=="island"&&list->currentItem()->text()=="<b>Island</b>","explicit selection and literal item text");
  for(const auto& invalid:QVector<QVector<MapSelectionWidget::Map>>{{{"x","One"},{"x","Two"}},{{"","Empty ID"}},{{"x",""}}})require(!widget.setMaps(invalid,QString(),&error)&&!error.isEmpty()&&widget.selectedMapId()=="island"&&list->count()==3,"invalid model transactional");
  require(!widget.setMaps(maps,"missing",&error)&&widget.selectedMapId()=="island","unknown explicit ID rejected");
  require(widget.setMaps({maps[0]},QString(),&error)&&widget.selectedMapId().isEmpty()&&!ok->isEnabled(),"removed selection cleared");
  list->setCurrentRow(0);list->clearSelection();ok->click();require(widget.selectedMapId().isEmpty()&&confirmed==2&&!ok->isEnabled(),"current item without selection cannot confirm");
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(list,&escape);require(cancelled==1,"Escape from list");
  cancel->setFocus();QApplication::sendEvent(cancel,&enter);require(cancelled==2&&confirmed==2,"Enter on Cancel");
  widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&list->geometry()==QRect(365,178,470,270),"wide layout");
  const auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and letterbox");
  widget.resize(400,600);app.processEvents();require(list->geometry()==QRect(83,239,235,135)&&list->font().pixelSize()==11,"scaled list and font");
  write(config,QByteArray(layout).replace("165,178,635,448","165,178,900,448"));require(!widget.loadAssets(root,&error)&&!error.isEmpty()&&list->count()==1&&ok->text()=="OK","invalid layout transactional");write(config,layout);
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");
   QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");
   for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  widget.hide();MenuPreview preview;require(preview.loadAssets(root,false,&error)&&preview.openMapSelection(root,&error),"preview load");preview.show();app.processEvents();auto* selection=preview.findChild<MapSelectionWidget*>();auto* stack=preview.findChild<QStackedWidget*>();
  require(stack->currentWidget()==selection&&selection->selectedMapId()=="sample-forest","sample preview selected");selection->findChild<QPushButton*>("mapSelectionOk")->click();require(stack->currentWidget()==preview.findChild<QuickBattleMenuWidget*>()&&preview.statusBar()->currentMessage().contains("sample-forest"),"OK preview route with selected ID");
  require(preview.openMapSelection(root,&error),"reopen preview");selection->findChild<QPushButton*>("mapSelectionCancel")->click();require(stack->currentWidget()==preview.findChild<QuickBattleMenuWidget*>(),"Cancel preview route");
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"corrupt background");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Map Selection test: %s\n",failure.what());return 1;}
}
