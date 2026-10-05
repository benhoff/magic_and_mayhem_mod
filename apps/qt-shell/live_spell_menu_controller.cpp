#include "live_spell_menu_controller.hpp"
#include <QPushButton>
LiveSpellMenuController::LiveSpellMenuController(LiveMenuSession& session,SpellboxWidget& widget):session_(session),widget_(widget){
    widget_.findChild<QPushButton*>("spellboxOK")->setText("Start battle");
    widget_.findChild<QPushButton*>("spellboxCancel")->setText("Reset edits");
    widget_.findChild<QPushButton*>("spellboxPreview")->hide();
    QObject::connect(&widget_,&SpellboxWidget::cancelled,&widget_,[this]{widget_.setInventory(inventory_);});
    QObject::connect(&widget_,&SpellboxWidget::loadoutAccepted,&widget_,[this](const SpellboxWidget::Request& request){
        std::array<int,63> assignments;assignments.fill(-1);
        for(const auto& a:request.assignments){bool ok=false;int position=a.talismanId.toInt(&ok);if(!ok||position<0||position>=63)return;
            if(!a.itemId.isEmpty()){int item=a.itemId.toInt(&ok);if(!ok)return;assignments[position]=item;}}
        if(!session_.finishSpells(assignments))widget_.setInventory(inventory_);
    });
}
bool LiveSpellMenuController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=7||!state.ready)return true;
    const auto& b=state.spells;SpellboxWidget::Inventory model;model.ownerId=QString::number(b.owner);
    for(int i=0;i<21;++i){if(!(b.offered&(1u<<i)))continue;SpellboxWidget::Item item;item.id=QString::number(i);item.name=b.items[i];item.artworkIndex=i;item.quantity=1;
        for(int a=0;a<3;++a){int at=i*3+a;item.spells[a]={QString::number(b.recipes[at]),b.names[at],b.recipes[at]+2};}
        model.items.append(item);
    }
    for(int a=0;a<3;++a)for(int i=0;i<b.counts[a];++i){int at=a*21+i;model.talismans.append({QString::number(at),static_cast<SpellboxWidget::Alignment>(a),b.assignments[at]>=0?QString::number(b.assignments[at]):QString()});}
    // The engine timer changes each second. Preserve the local draft when only
    // time changes; refresh on entry or actual owner/inventory/loadout changes.
    bool same=model.ownerId==inventory_.ownerId&&model.items.size()==inventory_.items.size()&&model.talismans.size()==inventory_.talismans.size();
    if(same)for(int i=0;i<model.items.size();++i){const auto& x=model.items[i];const auto& y=inventory_.items[i];same&=x.id==y.id&&x.name==y.name;for(int a=0;a<3;++a)same&=x.spells[a].id==y.spells[a].id&&x.spells[a].name==y.spells[a].name;}
    if(same)for(int i=0;i<model.talismans.size();++i)same&=model.talismans[i].id==inventory_.talismans[i].id&&model.talismans[i].itemId==inventory_.talismans[i].itemId;
    if(!same||!generation_){if(!widget_.setInventory(model,error))return false;inventory_=model;}
    generation_=state.generation;return true;
}
