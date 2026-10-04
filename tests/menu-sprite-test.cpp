#include "menu_sprites.hpp"
#include "menu-sprite-fixtures.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <QEnterEvent>
#include <cstdio>
static void require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
int main(int argc,char** argv) {
 QApplication app(argc,argv);
 try {
    QTemporaryDir temporary;require(temporary.isValid(),"temporary root");const auto root=temporary.path();
    menu_sprite_fixture::write(root,"states.spr",3);const auto sheet=mnm::ui::loadMenuSprites(root,"states.spr");
    require(sheet.size()==3&&sheet[0].origin==QPoint(-1,-2),"owned frames and signed origins");
    require(sheet[0].image.pixelColor(0,0).alpha()==0&&sheet[0].image.pixelColor(1,0)==QColor(Qt::black)&&sheet[0].image.pixelColor(2,0)==QColor(Qt::red),"mask preserves opaque zero and RGB565 channels");
    const auto reject=[&](const QString& path){try{(void)mnm::ui::loadMenuSprites(root,path);return false;}catch(const std::exception&){return true;}};
    require(reject("../states.spr")&&reject("missing.spr"),"bounded asset paths");
    QFile file(root+"/indexed.spr");const auto data=menu_sprite_fixture::sprites(3,true);require(file.open(QIODevice::WriteOnly)&&file.write(data)==data.size(),"indexed fixture");file.close();
    const auto indexed=mnm::ui::loadMenuSprites(root,"indexed.spr");require(indexed[0].image.pixelColor(2,0)==QColor(220,100,30)&&indexed[0].image.pixelColor(1,0)==QColor(Qt::black),"indexed palette and opaque zero");
    bool invalid=false;try{(void)mnm::ui::spriteStates(sheet,1);}catch(const std::exception&){invalid=true;}require(invalid,"out-of-range state triplet rejected");
    auto empty=sheet;empty[1].image=QImage();invalid=false;try{(void)mnm::ui::spriteStates(empty,0);}catch(const std::exception&){invalid=true;}require(invalid,"empty state rejected");
    auto malformed=menu_sprite_fixture::sprites(3);const int at=24+3*4;malformed[at+12]=char(129);malformed[at+13]=0;malformed[at+14]=0;malformed[at+15]=0;
    QFile origin(root+"/origin.spr");require(origin.open(QIODevice::WriteOnly)&&origin.write(malformed)==malformed.size(),"origin fixture");origin.close();require(reject("origin.spr"),"unbounded origin rejected before UI use");
    mnm::ui::SpriteButton button("Fallback");button.setSprites(mnm::ui::spriteStates(sheet,0),QSize(6,6));button.resize(60,60);button.show();button.clearFocus();app.processEvents();
    auto colour=[&]{return button.grab().toImage().pixelColor(35,25);};
    // Keep the pointer away; hover must not affect the baseline.
    QEvent leave(QEvent::Leave);QApplication::sendEvent(&button,&leave);require(colour()==QColor(Qt::red),"normal state renders at anchor minus origin");
    QEnterEvent enter(QPointF(30,30),QPointF(30,30),QPointF(30,30));QApplication::sendEvent(&button,&enter);require(colour()==QColor(Qt::green),"hover state");
    button.setDown(true);require(colour()==QColor(Qt::blue),"pressed state");button.setDown(false);button.setEnabled(false);require(colour()!=QColor(Qt::green)&&colour()!=QColor(Qt::blue),"disabled renders dimmed normal");button.setEnabled(true);QApplication::sendEvent(&button,&leave);
    int clicks=0;QObject::connect(&button,&QPushButton::clicked,&button,[&]{++clicks;});button.click();require(clicks==1&&button.text()=="Fallback","semantic button and text accessibility retained");
    button.resize(30,30);require(button.grab().toImage().pixelColor(22,20)==QColor(Qt::red),"scaled anchor and pixels");
    mnm::ui::TalismanBar bar;bar.setSprites(sheet[0],sheet[1]);bar.setCounts(1,7);bar.resize(350,50);bar.show();app.processEvents();const auto capture=bar.grab().toImage();require(capture.pixelColor(3,2)==QColor(Qt::red)&&capture.pixelColor(53,2)==QColor(Qt::green)&&bar.text()=="1 / 7","active and inactive repeated talismans");
    invalid=false;try{bar.setCounts(0,1000000);}catch(const std::exception&){invalid=true;}require(invalid&&bar.text()=="1 / 7","unbounded repeated icons rejected");
    require(file.open(QIODevice::WriteOnly)&&file.write("bad")==3,"corrupt SPR");file.close();require(reject("indexed.spr")&&indexed[0].image.pixelColor(2,0)==QColor(220,100,30),"failed reload and source-independent output");
    return 0;
 } catch(const std::exception& error){std::fprintf(stderr,"Menu sprite test: %s\n",error.what());return 1;}
}
