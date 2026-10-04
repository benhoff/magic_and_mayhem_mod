#include "menu_fonts.hpp"
#include "sft.hpp"
#include "main_menu_widget.hpp"
#include "load_game_widget.hpp"
#include "save_game_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include "mini_menu_widget.hpp"
#include "battle_result_widget.hpp"
#include "quick_battle_result_widget.hpp"
#include "map_selection_widget.hpp"
#include "preferences_widget.hpp"
#include "multiplayer_setup_widget.hpp"
#include "multiplayer_game_selection_widget.hpp"
#include "single_player_battle_widget.hpp"
#include "multiplayer_lobby_widget.hpp"
#include "region_entry_widget.hpp"
#include "character_screen_widget.hpp"
#include "grimoire_widget.hpp"
#include "spellbox_widget.hpp"
#include "realm_viewer_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QLineEdit>
#include <QHelpEvent>
#include <QPainterPath>
#include <QRawFont>
#include <QTemporaryDir>
#include <QToolTip>
#include <cstdio>
#include <stdexcept>

static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static mnm::assets::SftFont fixture(){
    mnm::assets::SftFont font;font.ascent=3;font.descent=1;font.rowCount=4;font.metricGlyphCount=223;
    font.rowMetrics.resize(4*223,{0,2});font.glyphs.frames.resize(223);
    for(auto& f:font.glyphs.frames){f.width=2;f.height=3;f.originX=-1;f.originY=2;f.opaqueMask={1,1,1,0,1,1};}
    return font;
}
static void inspect(QWidget& widget){
    require(widget.property("menuFontSource").toString()=="SFT","installed screen did not select SFT");
    for(auto* child:widget.findChildren<QWidget*>()){
        const auto font=child->font();
        require(font.family().startsWith("MNM"),qPrintable(QString("%1/%2/%3 escaped shared font: %4").arg(widget.metaObject()->className()).arg(child->metaObject()->className()).arg(child->objectName()).arg(font.family())));
        require(!font.bold(),"control uses synthetic bold");
        require(QRawFont::fromFont(font).isValid(),"control font is invalid");
    }
}
static void sizes(QWidget& widget){
    for(const auto size:{QSize(800,600),QSize(400,300),QSize(1200,600)}){
        widget.resize(size);widget.show();qApp->processEvents();inspect(widget);
    }
}
template<class Widget> static void installed(const QString& root){
    Widget widget;QString error;require(widget.loadAssets(root,&error),qPrintable(error));sizes(widget);
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    auto font=fixture();const auto bytes=mnm::ui::menuFontData(font,"MNMFixture");
    QRawFont raw(bytes,4,QFont::PreferNoHinting);require(raw.isValid(),"Qt rejected outline fixture");
    const auto glyphs=raw.glyphIndexesForString(QString::fromLatin1(" A_\xff"));
    require(glyphs==QList<quint32>({1,34,64,224}),"Latin-1 glyph mapping");
    require(raw.glyphIndexesForString(QString::fromUtf8("\xe2\x98\x83"))[0]==0,"unsupported Unicode maps to missing glyph");
    require(raw.pathForGlyph(glyphs[1]).boundingRect()==QRectF(1,-2,2,3),"signed origin/baseline conversion");
    require(raw.pathForGlyph(glyphs[1]).contains(QPointF(1.5,-.5)),"opaque mask lost filled pixel");
    require(!raw.pathForGlyph(glyphs[1]).contains(QPointF(2.5,-.5)),"opaque mask hole lost");
    require(raw.pathForGlyph(1).isEmpty(),"space should be invisible");
    require(raw.advancesForGlyphIndexes({34})[0].x()==4,"shared profile advance/tracking");
    font.ascent=0;bool rejected=false;try{(void)mnm::ui::menuFontData(font,"Bad");}catch(const std::exception&){rejected=true;}require(rejected,"invalid metrics accepted");
    QTemporaryDir empty;QWidget menu;mnm::ui::installMenuFonts(&menu,mnm::ui::loadMenuFonts(empty.path()));
    require(menu.property("menuFontSource").toString()=="fallback","absent catalog fallback is not explicit");
    require(mnm::ui::menuFont(&menu,mnm::ui::MenuFontRole::Body,.5).pixelSize()==11,"shared scale policy");
    QDir().mkpath(empty.path()+"/Sprites");QFile broken(empty.path()+"/Sprites/body text 800.sft");require(broken.open(QIODevice::WriteOnly),"create corrupt catalog");broken.write("bad");broken.close();
    rejected=false;try{(void)mnm::ui::loadMenuFonts(empty.path());}catch(const std::exception&){rejected=true;}require(rejected,"corrupt catalog silently fell back");
    if(argc==2){const QString root=QString::fromLocal8Bit(argv[1]);
        installed<MainMenuWidget>(root);installed<QuickBattleMenuWidget>(root);installed<MiniMenuWidget>(root);
        installed<QuickBattleResultWidget>(root);installed<MapSelectionWidget>(root);installed<LoadGameWidget>(root);installed<SaveGameWidget>(root);
        installed<PreferencesWidget>(root);installed<MultiplayerGameSelectionWidget>(root);installed<SinglePlayerBattleWidget>(root);
        installed<RegionEntryWidget>(root);installed<CharacterScreenWidget>(root);installed<GrimoireWidget>(root);installed<SpellboxWidget>(root);installed<RealmViewerWidget>(root);
        QString error;
        for(auto mode:{MultiplayerSetupWidget::Mode::Join,MultiplayerSetupWidget::Mode::Create}){MultiplayerSetupWidget w(mode);require(w.loadAssets(root,&error),qPrintable(error));sizes(w);}
        for(auto mode:{MultiplayerLobbyWidget::Mode::Host,MultiplayerLobbyWidget::Mode::Join}){MultiplayerLobbyWidget w(mode);require(w.loadAssets(root,&error),qPrintable(error));sizes(w);}
        for(auto outcome:{BattleResultWidget::Outcome::Victory,BattleResultWidget::Outcome::Defeat}){BattleResultWidget w;require(w.loadAssets(root,outcome,&error),qPrintable(error));sizes(w);}
        auto fonts=mnm::ui::loadMenuFonts(root);require(fonts==mnm::ui::loadMenuFonts(root),"registered fonts were not reused");
        auto first=std::make_unique<QWidget>();QWidget second;mnm::ui::installMenuFonts(first.get(),fonts);mnm::ui::installMenuFonts(&second,fonts);fonts.reset();first.reset();
        require(QRawFont::fromFont(mnm::ui::menuFont(&second,mnm::ui::MenuFontRole::Body)).isValid(),"shared font lifetime");
        QLineEdit edit(&second);edit.setText(QString::fromUtf8("save_01 \xe2\x98\x83"));edit.setCursorPosition(3);edit.insert("X");require(edit.text().startsWith("savXe_01"),"native font broke text editing");
        QHelpEvent tip(QEvent::ToolTip,QPoint(0,0),edit.mapToGlobal(QPoint(0,0)));QApplication::sendEvent(&edit,&tip);require(QToolTip::font().family()==mnm::ui::menuFont(&second,mnm::ui::MenuFontRole::Tooltip).family(),"tooltip escaped font role");
        MainMenuWidget screenshot;require(screenshot.loadAssets(root,&error),qPrintable(error));screenshot.show();app.processEvents();require(screenshot.grab().save("working/tests/menu-fonts/main.png"),"save installed capture");
        for(const auto name:{"load","preferences","grimoire","lobby"}){
            std::unique_ptr<QWidget> capture;
            if(QString(name)=="load"){auto w=std::make_unique<LoadGameWidget>();require(w->loadAssets(root,&error),qPrintable(error));capture=std::move(w);}
            if(QString(name)=="preferences"){auto w=std::make_unique<PreferencesWidget>();require(w->loadAssets(root,&error),qPrintable(error));capture=std::move(w);}
            if(QString(name)=="grimoire"){auto w=std::make_unique<GrimoireWidget>();require(w->loadAssets(root,&error),qPrintable(error));capture=std::move(w);}
            if(QString(name)=="lobby"){auto w=std::make_unique<MultiplayerLobbyWidget>(MultiplayerLobbyWidget::Mode::Host);require(w->loadAssets(root,&error),qPrintable(error));capture=std::move(w);}
            capture->show();app.processEvents();require(capture->grab().save(QString("working/tests/menu-fonts/%1.png").arg(name)),"save font capture");
        }
        std::puts("All 21 installed menu variants use SFT fonts across controls and sizes");
    }
    std::puts("Menu font glyph mapping, masks, origins, metrics, bounds and fallback passed");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
