#include "mini_menu_widget.hpp"
#include "main_menu_widget.hpp"
#include "menu_preview.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>

static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static void write(const QString& path, const QByteArray& bytes) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "fixture write");
}
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    try {
        QTemporaryDir directory;
        require(directory.isValid(), "fixture directory");
        const auto root=directory.path();
        require(QDir().mkpath(root+"/CFG"), "strings directory");
        QByteArray strings("[STRINGS]\n");
        const std::array<const char*,8> labels{"Load Game", "Save Game", "Preferences", "Quit Game", "Cancel", "Preferences", "Quit Battle", "Cancel"};
        for(int i=0;i<8;++i) strings+=QString("STR_%1=%2\n").arg(i,2,10,QLatin1Char('0')).arg(QString::fromLatin1(labels[i])).toLatin1();
        write(root+"/CFG/interface screens text.cfg",strings);
        QImage image(800,600,QImage::Format_RGB32);image.fill(QColor(90,120,150));
        auto fixture=[&](const QString& dir,const QString& cfg,int count,bool text) {
            const auto full=root+"/Interface/"+dir;
            require(QDir().mkpath(full+"/800x600"), "screen directory");
            const auto format=dir=="MiniMenu"?"BMP":"JPG";
            const auto background=dir=="MiniMenu"?image.copy(0,0,600,400):image;
            require(background.save(full+"/800x600/Fixture 800-600."+format,format), "fixture image");
            QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n");
            if(text) layout+="[TEXT_1]\nRect2=100,75,700,125\nText=2\n";
            for(int i=0;i<count;++i) {
                const int top=i<5?180+50*i:230+50*(i-5);
                layout+=QString("[TEXTBUTTON_%1]\nRect2=250,%2,550,%3\nText=%4 ; label\n").arg(i+1).arg(top).arg(top+40).arg(i).toLatin1();
            }
            write(full+"/"+cfg,layout);return layout;
        };
        const auto layout=fixture("MiniMenu","screen (Mini Menu).cfg",8,false);
        fixture("MainScreen","screen (MainMenu).cfg",6,true);
        fixture("QuickBattleMainMenu","Screen (Quick Battle Main Menu).cfg",4,true);
        const auto config=root+"/Interface/MiniMenu/screen (Mini Menu).cfg";
        MiniMenuWidget menu;QString error;
        require(menu.loadAssets(root,&error) && error.isEmpty(), "valid Mini Menu assets");
        menu.show();app.processEvents();
        auto button=[&](int i){return menu.findChild<QPushButton*>(QString("miniMenuButton%1").arg(i+1));};
        int count=0;MiniMenuWidget::Action action=MiniMenuWidget::Action::Cancel;
        QObject::connect(&menu,&MiniMenuWidget::actionRequested,&menu,[&](auto value){action=value;++count;});
        const std::array<MiniMenuWidget::Action,8> expected{
            MiniMenuWidget::Action::LoadGame,MiniMenuWidget::Action::SaveGame,MiniMenuWidget::Action::Preferences,
            MiniMenuWidget::Action::QuitGame,MiniMenuWidget::Action::Cancel,MiniMenuWidget::Action::Preferences,
            MiniMenuWidget::Action::QuitBattle,MiniMenuWidget::Action::Cancel};
        for(int i=0;i<8;++i){
            require(button(i)->text()==QString::fromLatin1(labels[i]),"configured labels");
            require(button(i)->isVisible()==(i<5) && button(i)->isEnabled()==(i<5),"campaign control visibility");
            button(i)->click();
            require(count==qMin(i+1,5),"inactive battle controls emit nothing");
            if(i<5)require(action==expected[i],"campaign action semantics");
        }
        button(1)->setFocus();
        QKeyEvent down(QEvent::KeyPress,Qt::Key_Space,Qt::NoModifier),up(QEvent::KeyRelease,Qt::Key_Space,Qt::NoModifier);
        QApplication::sendEvent(button(1),&down);QApplication::sendEvent(button(1),&up);
        require(count==6 && action==MiniMenuWidget::Action::SaveGame,"Space activates save");
        menu.setMode(MiniMenuWidget::Mode::Battle);app.processEvents();
        require(menu.mode()==MiniMenuWidget::Mode::Battle && button(5)->hasFocus(),"mode switch focus");
        for(int i=0;i<8;++i){
            require(button(i)->isVisible()==(i>=5) && button(i)->isEnabled()==(i>=5),"battle control visibility");
            const int before=count;button(i)->click();
            require(count==before+(i>=5?1:0),"inactive campaign controls emit nothing");
            if(i>=5)require(action==expected[i],"battle action semantics");
        }
        QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
        QApplication::sendEvent(button(5),&escape);
        require(count==10 && action==MiniMenuWidget::Action::Cancel,"Escape from child");
        QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier,QString(),true);
        QApplication::sendEvent(button(5),&repeat);require(count==10,"Escape autorepeat suppressed");
        menu.resize(1200,600);app.processEvents();
        require(menu.contentRect()==QRect(200,0,800,600) && button(5)->geometry()==QRect(450,230,300,40),"wide layout");
        const auto captured=menu.grab().toImage();const auto color=captured.pixelColor(320,120);
        require(captured.pixelColor(20,20)==QColor(Qt::black) && captured.pixelColor(220,20)==QColor(Qt::black) && color==QColor(90,120,150),"centered panel and letterboxing");
        menu.resize(400,600);app.processEvents();
        require(button(5)->geometry()==QRect(125,265,150,20),"scaled battle layout");
        menu.setMode(MiniMenuWidget::Mode::Campaign);app.processEvents();
        require(button(0)->hasFocus() && button(5)->isHidden(),"switch back focus");
        write(config,QByteArray(layout).replace("250,380,550,420","250,380,900,420"));
        require(!menu.loadAssets(root,&error) && !error.isEmpty() && button(4)->text()=="Cancel","invalid rectangle transactional reload");
        write(config,layout);
        MenuPreview preview;require(preview.loadAssets(root,false,&error),"preview assets");
        menu.hide();preview.show();app.processEvents();
        for(auto mode:{MiniMenuWidget::Mode::Campaign,MiniMenuWidget::Mode::Battle}){
            require(preview.openMiniMenu(root,mode,&error),"preview opens requested mode");app.processEvents();
            auto* mini=preview.findChild<MiniMenuWidget*>();auto* stack=preview.findChild<QStackedWidget*>();
            require(stack->currentWidget()==mini && mini->mode()==mode,"requested mode visible");
            mini->findChild<QPushButton*>(mode==MiniMenuWidget::Mode::Campaign?"miniMenuButton4":"miniMenuButton7")->click();
            require(stack->currentWidget()==mini && preview.statusBar()->currentMessage().contains("adapter pending"),"quit intent does not close preview");
            auto* cancel=mini->findChild<QPushButton*>(mode==MiniMenuWidget::Mode::Campaign?"miniMenuButton5":"miniMenuButton8");
            if(mode==MiniMenuWidget::Mode::Campaign)cancel->click();
            else {QKeyEvent key(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(cancel,&key);}
            app.processEvents();
            require(stack->currentWidget()==preview.findChild<MainMenuWidget*>(),"Cancel and Escape return to main preview");
        }
        write(root+"/Interface/MiniMenu/800x600/Fixture 800-600.BMP","bad BMP");
        require(!menu.loadAssets(root,&error) && !error.isEmpty(),"corrupt image rejection");
        return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"Mini Menu test: %s\n",error.what());return 1;}
}
