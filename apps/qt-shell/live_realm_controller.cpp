#include "live_realm_controller.hpp"
#include "../../protocols/include/mnm/menu_v19.h"
#include <QPushButton>
#include <algorithm>
#include <exception>
LiveRealmController::LiveRealmController(LiveMenuSession& session,RealmViewerWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&RealmViewerWidget::regionRequested,&widget_,[this](const auto& request){
        bool valid=false;const auto id=request.regionId.section(':',1).toUInt(&valid);
        if(valid)session_.request(MNM_MENU_REALM_ACTIVATE,id);
    });
    QObject::connect(&widget_,&RealmViewerWidget::auxiliaryRequested,&widget_,[this](auto action){
        if(action==RealmViewerWidget::AuxiliaryAction::Character)session_.request(MNM_MENU_REALM_CHARACTER);
        else if(action==RealmViewerWidget::AuxiliaryAction::Grimoire)session_.request(MNM_MENU_REALM_GRIMOIRE);
        else if(action==RealmViewerWidget::AuxiliaryAction::Spellbox)session_.request(MNM_MENU_REALM_SPELLBOX);
    });
}
bool LiveRealmController::loadAssets(const QString& root,QString* error){
    try {auto catalog=mnm::ui::loadRealmCatalog(root);if(!widget_.loadAssets(root,error))return false;catalog_=std::move(catalog);return true;}
    catch(const std::exception& e){if(error)*error=QString::fromLocal8Bit(e.what());return false;}
}
bool LiveRealmController::present(const MenuBridge::State& state,QString* error){
    const bool ready=state.screen==4&&state.ready&&session_.realmMenusEnabled;
    const bool gainedReady=ready&&!widget_.isEnabled();
    widget_.setEnabled(ready);
    if(state.screen!=4){generation_=0;return true;}
    if(!session_.realmMenusEnabled){if(error)*error="Realm integration disabled.";return false;}
    if(!ready||state.generation==generation_){if(gainedReady)widget_.focusFirstControl();return true;}
    const auto& b=state.realm;
    if(b.kind<1||b.kind>3||b.visible<1||b.visible>19||b.rows.size()!=int(b.visible)){if(error)*error="Invalid Realm projection.";return false;}
    const int realm=int(b.kind)-1;const std::array<QString,3> names{"Celtic","Greek","Medieval"};
    RealmViewerWidget::Campaign c;c.id="original-campaign";c.name=names[realm];c.realm=RealmViewerWidget::Realm(realm);
    c.regionLimits={19,19,19};c.realmAvailable={false,false,false};c.realmAvailable[realm]=true;
    c.auxiliaryAvailable={bool(b.actions&1),bool(b.actions&4),bool(b.actions&2)&&session_.characterMenusEnabled,false,false};
    const auto previous=widget_.campaign();
    for(const auto& row:b.rows){
        RealmViewerWidget::Region r;r.id=QString("%1:%2").arg(names[realm]).arg(row.region);r.artworkNumber=int(row.region);
        r.name=int(row.region)<=catalog_[realm].size()?catalog_[realm][int(row.region)-1].name:QString("Region %1").arg(row.region);
        r.flagPosition={std::clamp(int(row.x)-24,0,752),std::clamp(int(row.y)-26,0,548)};
        r.flagArtworkIndex=-1;r.available=b.available&(1u<<row.region);r.auxiliaryAvailable={false,false,false};c.regions[realm].append(r);
        if(previous.selectedRegionIds[realm]==r.id)c.selectedRegionIds[realm]=r.id;
    }
    if(c.selectedRegionIds[realm].isEmpty()&&b.location)c.selectedRegionIds[realm]=QString("%1:%2").arg(names[realm]).arg(b.location);
    if(!widget_.setCampaign(c,error))return false;
    // Realm has no recovered direct Cancel-to-Main callback.
    widget_.findChild<QPushButton*>("realmViewerCancel")->setEnabled(false);
    generation_=state.generation;if(gainedReady)widget_.focusFirstControl();return true;
}
