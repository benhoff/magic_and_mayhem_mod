#include "spell_research_widget.hpp"
#include "menu_preview.hpp"
#include "grimoire-fixtures.hpp"
#include "realm-viewer-fixtures.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTextBrowser>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <cstdio>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void menus(const QString& root){
    QDir().mkpath(root+"/CFG");QByteArray strings("[STRINGS]\n");for(int i=0;i<8;++i)strings+=QString("STR_%1=Label %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();grimoire_fixture::write(root+"/CFG/interface screens text.cfg",strings);
    QImage image(800,600,QImage::Format_RGB888);image.fill(QColor(100,110,120));
    for(const auto& dir:{QString("MainScreen"),QString("QuickBattleMainMenu")}){QDir().mkpath(root+"/Interface/"+dir+"/800x600");require(image.save(root+"/Interface/"+dir+"/800x600/Fixture 800-600.JPG","JPG"),"menu JPEG");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,50,700,100\nText=0\n");for(int i=0;i<6;++i)cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();grimoire_fixture::write(root+"/Interface/"+dir+"/"+(dir=="MainScreen"?"screen (MainMenu).cfg":"Screen (Quick Battle Main Menu).cfg"),cfg);}
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    QTemporaryDir temp;require(temp.isValid(),"temporary root");const auto root=temp.path();grimoire_fixture::create(root);
    SpellResearchWidget widget;QString error;require(widget.loadAssets(root,&error),qPrintable(error));
    SpellResearchWidget::Catalog model;model.ownerId="opaque-owner";model.spells={{"available","<b>Available</b>","Plain <a href='x'>description</a>",true,false},{"learned","Known","Known description",true,true},{"locked","Locked","Locked description",false,false}};
    require(widget.setCatalog(model,&error),qPrintable(error));widget.show();app.processEvents();
    auto* list=widget.findChild<QListWidget*>("spellResearchSpells");auto* filter=widget.findChild<QLineEdit*>("spellResearchFilter");auto* request=widget.findChild<QPushButton*>("spellResearchRequest");auto* close=widget.findChild<QPushButton*>("spellResearchClose");auto* text=widget.findChild<QTextBrowser*>("spellResearchDescription");
    require(list&&filter&&request&&close&&text&&list->count()==3&&request->isEnabled(),"catalog and controls");require(widget.findChild<QLabel*>("spellResearchTitle")->textFormat()==Qt::PlainText&&text->toPlainText()==model.spells[0].description,"literal supplied labels/prose");
    int requests=0,selections=0,cancels=0;SpellResearchWidget::Request intent;QObject::connect(&widget,&SpellResearchWidget::researchRequested,&widget,[&](const auto& value){++requests;intent=value;});QObject::connect(&widget,&SpellResearchWidget::selectedSpellChanged,&widget,[&](const auto&){++selections;});QObject::connect(&widget,&SpellResearchWidget::cancelled,&widget,[&]{++cancels;});
    request->click();require(requests==1&&intent.ownerId=="opaque-owner"&&intent.spellId=="available"&&!widget.catalog().spells[0].learned,"typed request does not invent progression");
    require(widget.selectSpell("learned",&error)&&!request->isEnabled(),"learned research guard");request->click();require(requests==1,"learned request suppression");require(widget.selectSpell("locked",&error)&&!request->isEnabled(),"availability guard");
    filter->setText("missing");require(list->count()==0&&text->toPlainText().isEmpty()&&!request->isEnabled()&&widget.catalog().selectedSpellId=="locked","empty filter preserves stable selection");filter->clear();require(list->count()==3&&widget.catalog().selectedSpellId=="locked","clear filter restores catalog");
    require(widget.selectSpell("available",&error),"restore available selection");QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier),repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,{},true),escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(list,&enter);QApplication::sendEvent(list,&repeat);require(requests==2,"keyboard research and repeat guard");QApplication::sendEvent(filter,&escape);close->click();require(cancels==2,"Escape and close");
    for(int kind=0;kind<6;++kind){auto bad=model;if(kind==0)bad.ownerId.clear();if(kind==1)bad.spells[1].id=bad.spells[0].id;if(kind==2)bad.selectedSpellId="missing";if(kind==3)bad.spells[0].description=QString(65537,'x');if(kind==4)bad.spells[0].name="bad\nname";if(kind==5)bad.spells[0].description=QString(QChar(0));require(!widget.setCatalog(bad,&error)&&widget.catalog().selectedSpellId=="available","catalog validation rollback");}require(!widget.selectSpell("missing",&error)&&selections==3,"unknown selection rollback");
    SpellResearchWidget::Catalog empty;empty.ownerId="known-owner";require(widget.setCatalog(empty,&error)&&list->count()==0&&!request->isEnabled(),"empty catalog research guard");require(widget.setCatalog(model,&error),"restore catalog");
    widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&filter->font().pixelSize()==22,"wide geometry/font");widget.resize(400,300);app.processEvents();require(filter->font().pixelSize()==11&&qAbs(widget.grab().toImage().pixelColor(0,0).red()-130)<4,"half scale font/art");
    grimoire_fixture::write(root+"/Interface/Grimoire/800x600/Backdrop.JPG","broken JPEG");require(!widget.loadAssets(root,&error)&&widget.catalog().ownerId==model.ownerId,"asset rollback");grimoire_fixture::create(root);
    const auto sample=SpellResearchWidget::installedPreviewCatalog(root);require(sample.spells.size()==2&&sample.spells[0].name=="<b>Alpha</b>"&&sample.spells[1].description.contains("Second text page."),"installed spell chapter catalog/pages");
    widget.hide();realm_viewer_fixture::create(root);menus(root);MenuPreview preview;require(preview.loadAssets(root,false,&error)&&preview.openRealmViewer(root,&error),qPrintable(error));preview.show();app.processEvents();auto* viewer=preview.findChild<RealmViewerWidget*>();auto* stack=preview.findChild<QStackedWidget*>();auto* researchButton=viewer->findChild<QPushButton*>("realmViewerAuxiliary4");require(researchButton&&researchButton->isEnabled(),"original Realm Research button exposed");researchButton->click();auto* research=preview.findChild<SpellResearchWidget*>();require(research&&stack->currentWidget()==research,"Realm Research navigation");research->findChild<QPushButton*>("spellResearchClose")->click();require(stack->currentWidget()==viewer&&researchButton->hasFocus(),"Research return restores Realm focus");
    auto restricted=viewer->campaign();restricted.auxiliaryAvailable[4]=false;require(viewer->setCampaign(restricted,&error)&&!researchButton->isEnabled(),"caller Research availability");
    require(preview.openSpellResearch(root,&error)&&research->selectSpell(sample.spells[1].id,&error),"reopen Research");research->findChild<QPushButton*>("spellResearchClose")->click();require(preview.openSpellResearch(root,&error)&&research->catalog().selectedSpellId==sample.spells[1].id,"selection retained on reopen");research->findChild<QPushButton*>("spellResearchClose")->click();
    grimoire_fixture::write(root+"/Interface/Grimoire/800x600/Backdrop.JPG","broken JPEG");require(!preview.openSpellResearch(root,&error)&&stack->currentWidget()==viewer,"failed Research opening preserves caller");
    if(argc==2){const QString installedRoot=QString::fromLocal8Bit(argv[1]);SpellResearchWidget real;require(real.loadAssets(installedRoot,&error),qPrintable(error));const auto catalog=SpellResearchWidget::installedPreviewCatalog(installedRoot);require(!catalog.spells.isEmpty()&&real.setCatalog(catalog,&error),"installed research catalog");
        for(const auto& spell:catalog.spells)require(real.selectSpell(spell.id,&error)&&real.findChild<QTextBrowser*>("spellResearchDescription")->toPlainText()==spell.description,"installed spell prose traversal");
        require(real.property("menuFontSource").toString()=="SFT","installed Research font source");for(const auto size:{QSize(800,600),QSize(400,300),QSize(1200,600)}){real.resize(size);real.show();app.processEvents();for(auto* child:real.findChildren<QWidget*>())require(child->font().family().startsWith("MNM")&&!child->font().bold(),"Research control escaped font policy");}
        real.resize(800,600);require(real.selectSpell(catalog.spells.first().id,&error),"capture selection");app.processEvents();QDir().mkpath("working/tests/spell-research");require(real.grab().save("working/tests/spell-research/research.png"),"installed Research capture");std::printf("Traversed %lld installed spell descriptions and three font/layout sizes\n",static_cast<long long>(catalog.spells.size()));
    }
    std::puts("Spell Research catalog/filter/request guards, input, art/fonts, rollback and Realm navigation passed");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"Spell Research test: %s\n",e.what());return 1;}}
