#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include "menu_preview.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
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
        const auto root = directory.path();
        const auto quickDir = root + "/Interface/QuickBattleMainMenu";
        const auto mainDir = root + "/Interface/MainScreen";
        require(QDir().mkpath(quickDir + "/800x600") && QDir().mkpath(mainDir + "/800x600") && QDir().mkpath(root + "/CFG"), "fixture paths");
        QImage background(800,600,QImage::Format_RGB32);
        background.fill(QColor(80,110,140));
        require(background.save(quickDir + "/800x600/Fixture 800-600.JPG", "JPG") &&
                background.save(mainDir + "/800x600/Fixture 800-600.JPG", "JPG"), "fixture images");
        QByteArray layout("[GLOBALS]\nBackgroundFile=\"Fixture\"\n[TEXT_1]\nRect2=100,75,700,125\nText=2\n");
        const std::array<int,4> ids{76,41,77,11};
        const std::array<int,4> top{200,275,350,475};
        for (int i=0;i<4;++i)
            layout += QString("[TEXTBUTTON_%1]\nRect2=200,%2,600,%3\nText=%4 ; label\n").arg(i+1).arg(top[i]).arg(top[i]+50).arg(ids[i]).toLatin1();
        const auto config = quickDir + "/Screen (Quick Battle Main Menu).cfg";
        write(config, layout);
        QByteArray mainLayout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=600,565,790,590\n");
        for (int i=0;i<6;++i) mainLayout += QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(260+i*45).arg(300+i*45).arg(i).toLatin1();
        write(mainDir + "/screen (MainMenu).cfg",mainLayout);
        QByteArray strings("[STRINGS]\nSTR_76=Create Multiplayer Game\nSTR_41=Join Multiplayer Game\nSTR_77=Create Single Player Game\nSTR_11=Cancel\n");
        for(int i=0;i<6;++i) strings += QString("STR_%1=Main %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();
        const auto textPath = root + "/CFG/interface screens text.cfg";
        write(textPath,strings);
        QuickBattleMenuWidget menu;
        QString error;
        require(menu.loadAssets(root,&error) && error.isEmpty(),"valid quick battle assets");
        menu.show(); app.processEvents();
        auto button = [&](int i) { return menu.findChild<QPushButton*>(QString("quickBattleAction%1").arg(i)); };
        require(button(0)->text()=="Create Multiplayer Game" && button(2)->text()=="Create Single Player Game", "label IDs");
        require(menu.findChild<QLabel*>()->text()=="Main 2", "heading ID from strings");
        int count=0;
        QuickBattleMenuWidget::Action action=QuickBattleMenuWidget::Action::Cancel;
        QObject::connect(&menu,&QuickBattleMenuWidget::actionRequested,&menu,[&](auto value){action=value;++count;});
        for(int i=0;i<4;++i){button(i)->click();require(count==i+1 && int(action)==i,"four semantic actions");}
        button(2)->setFocus();
        QKeyEvent spaceDown(QEvent::KeyPress,Qt::Key_Space,Qt::NoModifier), spaceUp(QEvent::KeyRelease,Qt::Key_Space,Qt::NoModifier);
        QApplication::sendEvent(button(2),&spaceDown); QApplication::sendEvent(button(2),&spaceUp);
        require(count==5 && action==QuickBattleMenuWidget::Action::CreateSinglePlayer,"Space activation");
        QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
        QApplication::sendEvent(button(2),&escape);
        require(count==6 && action==QuickBattleMenuWidget::Action::Cancel,"Escape from child button");
        menu.resize(1200,600);app.processEvents();
        require(menu.contentRect()==QRect(200,0,800,600) && button(0)->geometry()==QRect(400,200,400,50),"wide geometry");
        const auto captured=menu.grab().toImage();
        const auto color=captured.pixelColor(220,20);
        require(captured.pixelColor(20,20)==QColor(Qt::black) && qAbs(color.blue()-140)<4,"background and letterbox");
        menu.resize(400,600);app.processEvents();
        require(button(0)->geometry()==QRect(100,250,200,25),"scaled geometry");
        write(textPath,QByteArray(strings).replace("STR_77=Create Single Player Game", "STR_78=Wrong ID"));
        require(!menu.loadAssets(root,&error) && !error.isEmpty() && button(2)->text()=="Create Single Player Game","missing label preserves old state");
        write(textPath,strings);
        write(config,QByteArray(layout).replace("100,75,700,125","100,75,900,125"));
        require(!menu.loadAssets(root,&error) && !error.isEmpty(),"invalid heading bounds");
        write(config,layout);
        MenuPreview preview;
        require(preview.loadAssets(root,false,&error),"preview asset load");
        menu.hide();preview.show();app.processEvents();
        auto* main=preview.findChild<MainMenuWidget*>();
        auto* quick=preview.findChild<QuickBattleMenuWidget*>();
        auto* stack=preview.findChild<QStackedWidget*>();
        require(stack->currentWidget()==main,"initial main menu");
        main->findChild<QPushButton*>("mainMenuAction2")->click();app.processEvents();
        require(stack->currentWidget()==quick && quick->findChild<QPushButton*>("quickBattleAction0")->hasFocus(),"quick navigation and focus");
        quick->findChild<QPushButton*>("quickBattleAction3")->click();app.processEvents();
        require(stack->currentWidget()==main && main->findChild<QPushButton*>("mainMenuAction2")->hasFocus(),"cancel returns focus");
        require(preview.loadAssets(root,true,&error) && stack->currentWidget()==quick,"direct quick preview");
        auto* first=quick->findChild<QPushButton*>("quickBattleAction0");
        QKeyEvent escapePreview(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
        QApplication::sendEvent(first,&escapePreview);app.processEvents();
        require(stack->currentWidget()==main,"Escape returns to main menu");
        write(quickDir+"/800x600/Fixture 800-600.JPG","bad image");
        require(!menu.loadAssets(root,&error) && !error.isEmpty(),"corrupt JPEG");
        return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"Quick Battle test: %s\n",error.what());return 1;}
}
