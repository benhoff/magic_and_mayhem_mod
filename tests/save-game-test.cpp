#include "save_game_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <array>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes){QFile f(path);require(f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size(),"fixture write");}
static QMessageBox* dialog(QWidget& widget){for(auto* d:widget.findChildren<QMessageBox*>())if(d->isVisible())return d;return nullptr;}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  QTemporaryDir temporary;require(temporary.isValid(),"fixture directory");const auto root=temporary.path();
  require(QDir().mkpath(root+"/CFG")&&QDir().mkpath(root+"/Interface/SaveGame/800x600"),"directories");
  write(root+"/CFG/interface screens text.cfg","[STRINGS]\nSTR_00=Fixture\nSTR_01=Fixture\nSTR_02=Fixture\nSTR_03=Fixture\nSTR_04=Fixture\nSTR_05=Fixture\nSTR_11=Cancel\nSTR_75=Save Game\nSTR_83=Save\nSTR_94=Delete\n");
  const auto dir=root+"/Interface/SaveGame",config=dir+"/screen (Save Game).cfg";
  const QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_2]\nRect2=230,55,575,102\nFont=LARGE\nTextFlags=CENTRE\nText=75\n[EDITBOX_1]\nRect2=163,150,637,192\nFont=SMALL\n[LISTBOX_1]\nRect2=165,233,635,500\nFont=SMALL\n[TEXTBUTTON_1]\nRect2=70,530,220,580\nFont=LARGE\nText=83\n[TEXTBUTTON_2]\nRect2=580,530,730,580\nFont=LARGE\nText=11\n[TEXTBUTTON_3]\nRect2=325,530,475,580\nFont=LARGE\nText=94\n");write(config,layout);
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));require(image.save(dir+"/800x600/Fixture 800-600.JPG","JPG"),"image");
  SaveGameWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"valid assets");widget.show();widget.focusSelection();app.processEvents();
  auto* list=widget.findChild<QListWidget*>("saveGameList");auto* edit=widget.findChild<QLineEdit*>("saveGameFileName");auto* save=widget.findChild<QPushButton*>("saveGameSave");auto* remove=widget.findChild<QPushButton*>("saveGameDelete");auto* cancel=widget.findChild<QPushButton*>("saveGameCancel");
  require(widget.findChild<QLabel*>("saveGameHeading")->text()=="Save Game"&&save->text()=="Save"&&remove->text()=="Delete"&&cancel->hasFocus(),"configured labels and empty focus");
  int saved=0,deleted=0,cancelled=0;QString name,id;
  QObject::connect(&widget,&SaveGameWidget::saveRequested,&widget,[&](const QString& n,const QString& i){++saved;name=n;id=i;});QObject::connect(&widget,&SaveGameWidget::deleteRequested,&widget,[&](const QString& i){++deleted;id=i;});QObject::connect(&widget,&SaveGameWidget::cancelled,&widget,[&]{++cancelled;});
  save->click();remove->click();require(saved==0&&deleted==0&&!dialog(widget),"empty actions blocked");
  const QVector<SaveGameWidget::Save> saves{{"auto","Autosave"},{"campaign","Campaign"}};require(widget.setSaves(saves,"campaign",&error),"explicit selection");
  require(edit->text()=="Campaign"&&remove->isEnabled()&&save->isEnabled(),"selected filename");
  save->click();app.processEvents();auto* d=dialog(widget);require(d&&d->objectName()=="saveGameOverwriteConfirmation"&&d->defaultButton()==d->button(QMessageBox::No)&&saved==0,"overwrite waits for default-No confirmation");
  save->click();require(dialog(widget)==d,"duplicate requests use one dialog");d->button(QMessageBox::No)->click();app.processEvents();require(saved==0,"overwrite rejection");
  save->click();app.processEvents();dialog(widget)->button(QMessageBox::Yes)->click();app.processEvents();require(saved==1&&name=="Campaign"&&id=="campaign"&&list->count()==2,"overwrite intent only");
  remove->click();app.processEvents();require(dialog(widget)&&dialog(widget)->objectName()=="saveGameDeleteConfirmation"&&deleted==0,"delete confirmation");dialog(widget)->button(QMessageBox::No)->click();app.processEvents();require(deleted==0,"delete rejection");
  remove->click();app.processEvents();dialog(widget)->button(QMessageBox::Yes)->click();app.processEvents();require(deleted==1&&id=="campaign"&&list->count()==2,"delete intent leaves supplied model unchanged");
  save->click();app.processEvents();QPointer<QMessageBox> stale=dialog(widget);edit->setText("New name");require(!dialog(widget)&&remove->isEnabled()==false&&save->isEnabled(),"editing invalidates pending overwrite");if(stale)stale->done(QMessageBox::Yes);require(saved==1,"stale dialog cannot emit");
  save->click();require(saved==2&&name=="New name"&&id.isEmpty(),"new name emits without overwrite dialog");
  for(const QString& invalid:QStringList{"","   ","../file","folder/name","folder\\name","C:name",".",".."}){edit->setText(invalid);save->click();require(!save->isEnabled()&&saved==2,"invalid native name blocked");}
  edit->setText("Fresh");QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(edit,&enter);require(saved==3&&name=="Fresh","editor Enter once");QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true);QApplication::sendEvent(edit,&repeat);require(saved==3,"repeat ignored");
  require(widget.setSaves(saves,"auto",&error),"reselect");remove->click();app.processEvents();require(dialog(widget),"pending delete before refresh");require(widget.setSaves({saves[1]},QString(),&error)&&!dialog(widget)&&widget.selectedSaveId().isEmpty()&&edit->text().isEmpty(),"refresh removes selection and cancels dialog");
  const QVector<QVector<SaveGameWidget::Save>> invalidModels{{{"x","One"},{"x","Two"}},{{"x","Same"},{"y","Same"}},{{"x","bad/name"}},{{"x",QString(257,'x')}}};
  for(const auto& bad:invalidModels)require(!widget.setSaves(bad,QString(),&error)&&!error.isEmpty()&&list->count()==1,"invalid model transactional");
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(edit,&escape);require(cancelled==1,"Escape from edit");
  widget.resize(1200,600);app.processEvents();require(remove->geometry()==QRect(525,530,150,50)&&list->geometry()==QRect(365,233,470,267),"wide geometry");auto capture=widget.grab().toImage();require(capture.pixelColor(10,10)==QColor(Qt::black)&&qAbs(capture.pixelColor(210,10).blue()-150)<4,"background and bars");
  widget.resize(400,600);app.processEvents();require(edit->geometry()==QRect(82,225,237,21),"scaled edit");write(config,QByteArray(layout).replace("Font=SMALL","Font=BAD"));require(!widget.loadAssets(root,&error)&&save->text()=="Save"&&list->count()==1,"invalid layout transactional");write(config,layout);
  for(const auto& entry:std::array<std::pair<QString,QString>,2>{{{"MainScreen","screen (MainMenu).cfg"},{"QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg"}}}){
   const auto folder=root+"/Interface/"+entry.first;require(QDir().mkpath(folder+"/800x600"),"navigation directory");require(image.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"navigation image");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=150,50,650,100\nText=0\n");for(int i=0;i<6;++i){cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();}write(folder+"/"+entry.second,cfg);
  }
  const auto miniDir=root+"/Interface/MiniMenu";require(QDir().mkpath(miniDir+"/800x600"),"mini directory");require(image.copy(0,0,600,400).save(miniDir+"/800x600/Fixture 800-600.BMP","BMP"),"mini image");QByteArray mini("[GLOBALS]\nBackgroundFile=Fixture\n");for(int i=0;i<8;++i){mini+=QString("[TEXTBUTTON_%1]\nRect2=250,200,550,240\nText=0\n").arg(i+1).toLatin1();}write(miniDir+"/screen (Mini Menu).cfg",mini);
  widget.hide();MenuPreview preview;require(preview.loadAssets(root,false,&error)&&preview.openSaveGame(root,&error),"standalone preview");preview.show();app.processEvents();auto* screen=preview.findChild<SaveGameWidget*>();auto* stack=preview.findChild<QStackedWidget*>();screen->findChild<QLineEdit*>("saveGameFileName")->setText("Sample new save");screen->findChild<QPushButton*>("saveGameSave")->click();require(stack->currentWidget()==screen&&preview.statusBar()->currentMessage().contains("adapter pending"),"preview request pending");screen->findChild<QPushButton*>("saveGameCancel")->click();require(stack->currentWidget()==preview.findChild<MainMenuWidget*>(),"standalone Cancel to Main");
  require(preview.openMiniMenu(root,MiniMenuWidget::Mode::Campaign,&error),"campaign Mini Menu");preview.findChild<MiniMenuWidget*>()->findChild<QPushButton*>("miniMenuButton2")->click();require(stack->currentWidget()==screen,"Mini Save opens screen");screen->findChild<QPushButton*>("saveGameCancel")->click();require(stack->currentWidget()==preview.findChild<MiniMenuWidget*>(),"Cancel returns to caller Mini Menu");
  write(dir+"/800x600/Fixture 800-600.JPG","bad JPEG");require(!widget.loadAssets(root,&error),"bad background rejection");return 0;
 }catch(const std::exception& failure){std::fprintf(stderr,"Save Game test: %s\n",failure.what());return 1;}
}
