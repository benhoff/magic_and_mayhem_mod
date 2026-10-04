#include "spellbox_widget.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "menu-sprite-fixtures.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryDir>
#include <cstdio>
static void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static void write(const QString& path,const QByteArray& bytes){QFile file(path);require(file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size(),"fixture write");}
static SpellboxWidget::Inventory inventory(){
    SpellboxWidget::Inventory inv;inv.ownerId="owner";
    SpellboxWidget::Item one;one.id="opaque-item";one.name="<b>Item</b>";one.artworkIndex=22;one.quantity=1;
    one.spells={SpellboxWidget::Spell{"law-spell","Law spell",94},SpellboxWidget::Spell{"neutral-spell","Neutral spell",23},SpellboxWidget::Spell{}};
    inv.items.push_back(one);auto two=one;two.id="second-item";two.name="Second item";two.artworkIndex=0;two.quantity=2;two.spells[2]={"chaos-spell","Chaos spell",10};inv.items.push_back(two);
    inv.talismans={{"law-slot",SpellboxWidget::Alignment::Law,{}},{"neutral-slot",SpellboxWidget::Alignment::Neutral,{}},{"chaos-slot",SpellboxWidget::Alignment::Chaos,{}}};return inv;
}
int main(int argc,char**argv){QApplication app(argc,argv);try{
    QTemporaryDir temp;require(temp.isValid(),"temporary root");const auto root=temp.path(),folder=root+"/Interface/SpellBox";require(QDir().mkpath(folder+"/800x600"),"folder");
    QImage background(800,600,QImage::Format_RGB888);background.fill(QColor(100,70,40));require(background.save(folder+"/800x600/Portmanteau.bmp","BMP"),"BMP fixture");
    const QByteArray tips("[HEADER]\nValidConfig=TRUE\n[STRINGS]\nSTR_04=Talisman Of Law\nSTR_05=Talisman Of Neutrality\nSTR_06=Talisman Of Chaos\n");write(folder+"/Spellboxtooltip.cfg",tips);
    menu_sprite_fixture::write(root,"Interface/SpellBox/800x600/Talisman.spr",95);menu_sprite_fixture::write(root,"Interface/SpellBox/800x600/mitems.spr",23);
    SpellboxWidget widget;QString error;require(widget.loadAssets(root,&error)&&error.isEmpty(),"bounded assets load");widget.show();app.processEvents();
    auto button=[&](const char* name){auto* b=widget.findChild<QPushButton*>(name);require(b!=nullptr,"button exists");return b;};
    require(!button("spellboxOK")->isEnabled()&&!button("spellboxAssign")->isEnabled()&&!button("spellboxPreview")->isEnabled(),"empty state guarded");
    auto inv=inventory();require(widget.setInventory(inv,&error)&&error.isEmpty(),"supplied inventory");widget.focusFirstControl();require(button("spellboxItem0")->hasFocus(),"initial focus");
    require(button("spellboxItem0")->accessibleName()=="<b>Item</b>"&&widget.findChild<QLabel*>("spellboxDetail")->textFormat()==Qt::PlainText,"literal supplied labels");
    int previews=0,accepts=0,cancels=0;SpellboxWidget::Preview preview;SpellboxWidget::Request request;
    QObject::connect(&widget,&SpellboxWidget::spellPreviewRequested,&widget,[&](const auto& p){preview=p;++previews;});QObject::connect(&widget,&SpellboxWidget::loadoutAccepted,&widget,[&](const auto& r){request=r;++accepts;});QObject::connect(&widget,&SpellboxWidget::cancelled,&widget,[&]{++cancels;});
    QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier),repeat(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier,{},true),escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
    QApplication::sendEvent(button("spellboxPreview"),&enter);QApplication::sendEvent(button("spellboxPreview"),&repeat);require(previews==1&&preview.ownerId=="owner"&&preview.itemId=="opaque-item"&&preview.talismanId=="law-slot"&&preview.spellId=="law-spell","typed preview and repeat guard");
    button("spellboxAssign")->click();require(widget.draftRequest().assignments[0].itemId=="opaque-item"&&widget.inventory().talismans[0].itemId.isEmpty()&&button("spellboxItem0")->toolTip().contains("0 available"),"local assignment and quantity accounting");
    require(widget.assignItem("opaque-item","law-slot",&error),"idempotent assignment");button("spellboxTalisman1")->click();require(!button("spellboxAssign")->isEnabled()&&!widget.assignItem("opaque-item","neutral-slot",&error),"exhausted item guarded");
    button("spellboxTalisman2")->click();require(!button("spellboxAssign")->isEnabled()&&!button("spellboxPreview")->isEnabled()&&!widget.assignItem("opaque-item","chaos-slot",&error),"missing supplied spell guarded");
    require(!widget.assignItem("unknown","law-slot",&error)&&!widget.removeItem("unknown",&error),"unknown IDs rejected");
    button("spellboxTalisman0")->click();button("spellboxRemove")->click();require(widget.draftRequest().assignments[0].itemId.isEmpty()&&button("spellboxItem0")->toolTip().contains("1 available"),"remove restores copy");
    require(widget.assignItem("opaque-item","law-slot",&error)&&widget.assignItem("second-item","law-slot",&error)&&widget.assignItem("opaque-item","neutral-slot",&error)&&widget.assignItem("second-item","chaos-slot",&error),"replacement restores prior item and independent alignments");
    button("spellboxOK")->click();require(accepts==1&&request.assignments.size()==3&&request.assignments[0].spellId=="law-spell"&&request.assignments[2].spellId=="chaos-spell"&&widget.inventory().talismans[2].itemId=="second-item","accept contains stable IDs and supplied spell IDs");
    require(widget.removeItem("chaos-slot",&error),"draft remove");QApplication::sendEvent(button("spellboxItem1"),&escape);require(cancels==1&&widget.draftRequest().assignments[2].itemId=="second-item","Escape restores accepted snapshot");
    for(int kind=0;kind<9;++kind){auto bad=inv;switch(kind){case 0:bad.ownerId.clear();break;case 1:bad.items[1].id=bad.items[0].id;break;case 2:bad.items[0].artworkIndex=23;break;case 3:bad.items[0].quantity=0;break;case 4:bad.items[0].spells[0].artworkIndex=95;break;case 5:bad.talismans[0].alignment=SpellboxWidget::Alignment(3);break;case 6:bad.talismans[0].itemId="unknown";break;case 7:bad.talismans[0].itemId=bad.talismans[1].itemId="opaque-item";break;case 8:bad.items[0].name="line\nbreak";break;}require(!widget.setInventory(bad,&error)&&!error.isEmpty()&&widget.draftRequest().assignments[2].itemId=="second-item","invalid inventory transactional");}
    auto tooMany=inv;for(int i=0;i<7;++i)tooMany.talismans.push_back({QString("extra%1").arg(i),SpellboxWidget::Alignment::Law,{}});require(!widget.setInventory(tooMany,&error),"seven per alignment bound");
    write(folder+"/Spellboxtooltip.cfg",QByteArray(tips).replace("TRUE","FALSE"));require(!widget.loadAssets(root,&error)&&widget.draftRequest().assignments[2].itemId=="second-item","config rollback");write(folder+"/Spellboxtooltip.cfg",tips);
    menu_sprite_fixture::write(root,"Interface/SpellBox/800x600/Talisman.spr",94);require(!widget.loadAssets(root,&error),"sprite count rollback");menu_sprite_fixture::write(root,"Interface/SpellBox/800x600/Talisman.spr",95);
    write(folder+"/800x600/Portmanteau.bmp","invalid BMP");require(!widget.loadAssets(root,&error)&&widget.inventory().ownerId=="owner","image rollback");require(background.save(folder+"/800x600/Portmanteau.bmp","BMP")&&widget.loadAssets(root,&error),"restore assets");
    widget.resize(1200,600);app.processEvents();require(widget.contentRect()==QRect(200,0,800,600)&&button("spellboxTalisman0")->geometry()==QRect(398,7,84,86)&&button("spellboxItem0")->geometry()==QRect(490,93,84,86),"wide placement");const auto captured=widget.grab().toImage();require(captured.pixelColor(10,10)==QColor(Qt::black)&&captured.pixelColor(600,590)==QColor(100,70,40),"native BMP and letterbox");require(captured.pixelColor(494,97)==QColor(100,70,40),"exhausted ingredient leaves its shelf empty");
    widget.resize(400,600);app.processEvents();require(button("spellboxItem0")->geometry()==QRect(145,197,42,43),"scaled native positions");
    // Click-to-carry and hover are draft previews until an explicit drop.
    widget.resize(800,600);app.processEvents();require(widget.setInventory(inv,&error),"pointer model reset");
    require(widget.findChild<QLabel*>("spellboxRecipe1")->toolTip().contains("Law spell")&&widget.findChild<QLabel*>("spellboxRecipe2")->toolTip().contains("Neutral spell")&&widget.findChild<QLabel*>("spellboxRecipe3")->toolTip().contains("No supplied spell"),"selected ingredient shows all three recipes in header");
    auto* ghost=widget.findChild<QLabel*>("spellboxCarriedArtwork");require(ghost&&!ghost->isVisible(),"no ingredient carried on open");
    auto* ingredient=button("spellboxItem0");const QPoint sample=ingredient->pos()+QPoint(4,4);
    const auto shelfBefore=widget.grab().toImage().pixelColor(sample);
    QMouseEvent held(QEvent::MouseButtonPress,QPointF(ingredient->rect().center()),QPointF(ingredient->mapToGlobal(ingredient->rect().center())),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(ingredient,&held);
    require(widget.grab().toImage().pixelColor(sample)==QColor(100,70,40)&&widget.draftRequest().assignments[0].itemId.isEmpty(),"held ingredient disappears from shelf without assignment");
    QMouseEvent released(QEvent::MouseButtonRelease,QPointF(-10,-10),QPointF(ingredient->mapToGlobal(QPoint(-10,-10))),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(ingredient,&released);
    require(widget.grab().toImage().pixelColor(sample)==shelfBefore&&ingredient->toolTip().contains("1 available"),"release restores ingredient without consuming inventory");
    button("spellboxItem0")->click();require(ghost->isVisible(),"click picks up ingredient artwork");
    auto move=[&](QWidget* target,QPoint point){QMouseEvent event(QEvent::MouseMove,QPointF(point),QPointF(target->mapToGlobal(point)),Qt::NoButton,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(target,&event);};
    auto* law=button("spellboxTalisman0");move(law,law->rect().center());
    require(law->toolTip().contains("Law spell")&&widget.draftRequest().assignments[0].itemId.isEmpty(),"hover shows resulting spell without assigning");
    require(ghost->geometry().contains(widget.mapFromGlobal(law->mapToGlobal(law->rect().center()))),"ingredient follows pointer");
    move(&widget,{700,550});require(law->toolTip().contains("empty"),"leaving talisman restores empty artwork");
    move(law,law->rect().center());law->click();require(!ghost->isVisible()&&widget.draftRequest().assignments[0].itemId=="opaque-item"&&law->toolTip().contains("Law spell"),"drop commits transformed talisman");
    law->click();require(ghost->isVisible(),"pick up assigned spell as ingredient");
    auto* neutral=button("spellboxTalisman1");move(neutral,neutral->rect().center());neutral->click();
    require(widget.draftRequest().assignments[0].itemId.isEmpty()&&widget.draftRequest().assignments[1].itemId=="opaque-item"&&!ghost->isVisible(),"move assigned copy without extra inventory");
    neutral->click();QMouseEvent shelf(QEvent::MouseButtonPress,QPointF(700,550),QPointF(widget.mapToGlobal(QPoint(700,550))),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&widget,&shelf);
    require(widget.draftRequest().assignments[1].itemId.isEmpty()&&!ghost->isVisible()&&button("spellboxItem0")->toolTip().contains("1 available"),"empty shelf returns carried copy");
    button("spellboxItem0")->click();QApplication::sendEvent(button("spellboxItem0"),&escape);require(!ghost->isVisible()&&cancels==1,"Escape deselects carried item before cancelling screen");
    button("spellboxItem0")->click();widget.hide();require(!ghost->isVisible(),"hidden screen drops pointer overlay");widget.show();app.processEvents();
    // Exercise maximum supported model and all installed frame indices with fixtures.
    auto full=inv;full.items.clear();full.talismans.clear();for(int i=0;i<30;++i){auto it=inv.items[1];it.id=QString("full-item-%1").arg(i);it.artworkIndex=i%23;it.quantity=21;for(int a=0;a<3;++a)it.spells[a].artworkIndex=(i*3+a)%95;full.items.push_back(it);}for(int a=0;a<3;++a)for(int t=0;t<7;++t)full.talismans.push_back({QString("full-slot-%1-%2").arg(a).arg(t),SpellboxWidget::Alignment(a),{}});require(widget.setInventory(full,&error)&&widget.findChildren<QPushButton*>().size()==56,"bounded maximum grid");require(widget.setInventory(inv,&error),"shrink grid");
    require(QDir().mkpath(root+"/CFG"),"strings dir");QByteArray strings("[STRINGS]\n");for(int i=0;i<100;++i)strings+=QString("STR_%1=Label %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();write(root+"/CFG/interface screens text.cfg",strings);menu_sprite_fixture::shared(root);
    QImage image(800,600,QImage::Format_RGB888);image.fill(QColor(90,120,150));
    for(const auto& dir:{QString("MainScreen"),QString("QuickBattleMainMenu"),QString("RegionEntry")})require(QDir().mkpath(root+"/Interface/"+dir+"/800x600"),"menu folders");
    for(const auto& dir:{QString("MainScreen"),QString("QuickBattleMainMenu")}){require(image.save(root+"/Interface/"+dir+"/800x600/Fixture 800-600.JPG","JPG"),"menu art");QByteArray cfg("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,50,700,100\nText=0\n");for(int i=0;i<6;++i)cfg+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(200+i*50).arg(240+i*50).arg(i).toLatin1();write(root+"/Interface/"+dir+"/"+(dir=="MainScreen"?"screen (MainMenu).cfg":"Screen (Quick Battle Main Menu).cfg"),cfg);}
    require(image.save(root+"/Interface/RegionEntry/800x600/Celtic_Region_01.JPG","JPG"),"region art");QByteArray regionCfg("[TEXT_1]\nFont=LARGE\nTextFlags=LEFT\nRect2=50,40,525,75\nText=Region\n[TEXTBUTTON_1]\nFont=LARGE\nRect2=50,510,325,560\nText=78\n[TEXTBUTTON_2]\nFont=LARGE\nRect2=475,510,750,560\nText=11\n");for(int i=0;i<4;++i)regionCfg+=QString("[RADIOBUTTON_%1]\nFont=SMALL\nRect2=%2,90,%3,115\nText=%4\n").arg(i+1).arg(50+i*175).arg(225+i*175).arg(79+i).toLatin1();const std::array<int,3> first{3,6,0};for(int i=0;i<3;++i)regionCfg+=QString("[STANDARDBUTTON_%1]\nRect2=%2,25,%3,85\nSpriteIndexes=%4,%5,%6\n").arg(i+1).arg(585+i*65).arg(645+i*65).arg(first[i]).arg(first[i]+1).arg(first[i]+2).toLatin1();write(root+"/Interface/RegionEntry/screen (Region Entry).cfg",regionCfg);
    widget.hide();MenuPreview shell;require(shell.loadAssets(root,false,&error)&&shell.openRegionEntry(root,&error),"Region navigation setup");shell.show();app.processEvents();auto* region=shell.findChild<RegionEntryWidget*>();auto* stack=shell.findChild<QStackedWidget*>();region->findChild<QRadioButton*>("regionEntryDifficulty3")->click();region->findChild<QPushButton*>("regionEntryAuxiliary1")->click();auto* box=qobject_cast<SpellboxWidget*>(stack->currentWidget());require(box,"Region Spellbox route");box->findChild<QPushButton*>("spellboxAssign")->click();box->findChild<QPushButton*>("spellboxOK")->click();require(stack->currentWidget()==region&&region->region().difficulty==RegionEntryWidget::Difficulty::Wizard&&region->findChild<QPushButton*>("regionEntryAuxiliary1")->hasFocus(),"accept restores caller difficulty and icon focus");region->findChild<QPushButton*>("regionEntryAuxiliary1")->click();require(box->draftRequest().assignments[0].itemId=="sample-item-0","accepted draft survives reopen");box->findChild<QPushButton*>("spellboxRemove")->click();box->findChild<QPushButton*>("spellboxCancel")->click();require(stack->currentWidget()==region&&box->draftRequest().assignments[0].itemId=="sample-item-0","cancel restores accepted loadout and caller");write(folder+"/800x600/Portmanteau.bmp","bad BMP");region->findChild<QPushButton*>("regionEntryAuxiliary1")->click();require(stack->currentWidget()==region&&shell.statusBar()->currentMessage().contains("failed"),"failed opening preserves caller");require(background.save(folder+"/800x600/Portmanteau.bmp","BMP"),"restore navigation image");region->findChild<QPushButton*>("regionEntryCancel")->click();require(shell.openSpellbox(root,&error),"standalone route");box->findChild<QPushButton*>("spellboxCancel")->click();require(stack->currentWidget()==shell.findChild<MainMenuWidget*>(),"standalone returns Main");return 0;
}catch(const std::exception& e){fprintf(stderr,"Spellbox test: %s\n",e.what());return 1;}}
