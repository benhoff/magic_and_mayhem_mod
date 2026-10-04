#include "battle_result_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QKeyEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
static void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
static void write(const QString& path,const QByteArray& bytes) {
    QFile f(path); require(f.open(QIODevice::WriteOnly) && f.write(bytes)==bytes.size(),"fixture write");
}
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    try {
        QTemporaryDir temporary; require(temporary.isValid(),"temporary directory"); const auto root=temporary.path();
        require(QDir().mkpath(root+"/CFG") && QDir().mkpath(root+"/Interface/BattleEnd/800x600"),"fixture directories");
        write(root+"/CFG/interface screens text.cfg","[STRINGS]\nSTR_07=Achievement\nSTR_08=Experience Points\nSTR_09=Total\nSTR_10=OK\nSTR_93=Rating:\n");
        QByteArray layout("[GLOBAL]\n");
        for (int i=1;i<=21;++i) {
            const int top=20+20*i;
            const QString text=i==1?"7":i==2?"8":i==17?"9":i==19?"/  0":i==21?"93":"";
            layout+=QString("[TEXT_%1]\nRect2=50,%2,750,%3\nText=%4\nTextFlags=%5\nFont=%6\n").arg(i).arg(top).arg(top+20).arg(text).arg(i%2?"LEFT":"RIGHT").arg(i==17 || i==21?"SMALL":"LARGE").toLatin1();
        }
        layout+="[TEXTBUTTON_1]\nRect2=325,545,475,590\nText=10\nFont=LARGE\n";
        for (const QString& name:{QString("Victory"),QString("Defeat")}) {
            write(root+"/Interface/BattleEnd/screen (Battle End - "+name+").cfg",layout);
            QImage image(800,600,QImage::Format_RGB32); image.fill(name=="Victory"?QColor(90,120,150):QColor(150,120,90));
            require(image.save(root+"/Interface/BattleEnd/800x600/Battle "+name+" Screen 800-600.JPG","JPG"),"fixture background");
        }
        BattleResultWidget widget; QString error;
        require(widget.loadAssets(root,BattleResultWidget::Outcome::Victory,&error) && error.isEmpty(),"victory load without BackgroundFile");
        widget.show(); widget.focusContinue(); app.processEvents();
        auto text=[&](int i){return widget.findChild<QLabel*>(QString("battleResultText%1").arg(i));};
        auto* button=widget.findChild<QPushButton*>("battleResultContinue");
        require(text(1)->text()=="Achievement" && text(2)->text()=="Experience Points" && button->text()=="OK","fixed configured labels");
        require(text(3)->isHidden() && text(18)->isHidden(),"no invented result values");
        BattleResultWidget::Results results; results.rewards[0]={"<b>Literal reward</b>","120"}; results.rewards[6]={"Seventh","7"};
        results.totalPoints="127"; results.maximumPoints="300"; widget.setResults(results);
        require(text(3)->text()==results.rewards[0].achievement && text(3)->textFormat()==Qt::PlainText && text(16)->text()=="7","all rewards and plain text");
        require(text(18)->text()=="127" && text(19)->text()=="/  300" && text(17)->isVisible() && text(21)->isHidden(),"totals presentation");
        require(text(2)->alignment().testFlag(Qt::AlignRight) && text(17)->font().pixelSize()==20,"alignment and font roles");
        int calls=0; QObject::connect(&widget,&BattleResultWidget::continueRequested,&widget,[&]{++calls;});
        button->click(); require(calls==1,"OK intent");
        for (int key:{Qt::Key_Return,Qt::Key_Escape}) {
            QKeyEvent event(QEvent::KeyPress,key,Qt::NoModifier); QApplication::sendEvent(button,&event);
        }
        require(calls==3,"keyboard intent from child");
        QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,QString(),true); QApplication::sendEvent(button,&repeat);
        require(calls==3,"repeat ignored");
        require(widget.loadAssets(root,BattleResultWidget::Outcome::Defeat,&error),"defeat reload");
        require(widget.outcome()==BattleResultWidget::Outcome::Defeat && text(4)->text()=="120","data preserved across outcome reload");
        results={}; results.summary="Sample defeat"; results.rating="Sample rating"; results.title="Defeat!"; results.totalPoints="999"; widget.setResults(results);
        require(text(3)->isHidden() && text(16)->text().isEmpty() && text(20)->isVisible() && text(21)->isVisible() && text(21)->text()=="Rating: Sample rating" && text(17)->isHidden() && text(18)->isHidden(),"stale rewards cleared and rating excludes totals");
        widget.resize(1200,600); app.processEvents();
        require(widget.contentRect()==QRect(200,0,800,600) && button->geometry()==QRect(525,545,150,45),"letterbox geometry");
        auto capture=widget.grab().toImage(); auto pixel=capture.pixelColor(210,10);
        require(capture.pixelColor(10,10)==QColor(Qt::black) && qAbs(pixel.red()-150)<4 && qAbs(pixel.blue()-90)<4,"outcome background and black bars");
        widget.resize(400,600); app.processEvents(); require(button->geometry()==QRect(163,423,75,23),"scaled geometry");
        write(root+"/Interface/BattleEnd/screen (Battle End - Victory).cfg",QByteArray(layout).replace("TextFlags=RIGHT","TextFlags=INVALID"));
        require(!widget.loadAssets(root,BattleResultWidget::Outcome::Victory,&error) && !error.isEmpty() && widget.outcome()==BattleResultWidget::Outcome::Defeat && text(20)->text()=="Sample defeat","transactional rejection");
        write(root+"/Interface/BattleEnd/screen (Battle End - Victory).cfg",layout);
        widget.hide(); MenuPreview preview; require(preview.openBattleResults(root,BattleResultWidget::Outcome::Victory,&error),"preview opens");
        preview.show(); app.processEvents(); auto* stack=preview.findChild<QStackedWidget*>(); auto* result=preview.findChild<BattleResultWidget*>();
        require(stack->currentWidget()==result && result->findChild<QPushButton*>("battleResultContinue")->hasFocus(),"result focus");
        result->findChild<QPushButton*>("battleResultContinue")->click();
        require(stack->currentWidget()==preview.findChild<MainMenuWidget*>(),"continue returns to native main preview");
        write(root+"/Interface/BattleEnd/800x600/Battle Victory Screen 800-600.JPG","bad JPEG");
        require(!widget.loadAssets(root,BattleResultWidget::Outcome::Victory,&error),"corrupt background rejected");
        return 0;
    } catch (const std::exception& failure) { std::fprintf(stderr,"Battle result test: %s\n",failure.what()); return 1; }
}
