#include "live_storage_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v13.h"
#include <QLineEdit>
LiveStorageMenuController::LiveStorageMenuController(LiveMenuSession& session,LoadGameWidget& load,SaveGameWidget& save):session_(session),load_(load),save_(save){
    save_.setOriginalOverwriteConfirmation(true);
    save_.findChild<QLineEdit*>("saveGameFileName")->setMaxLength(MNM_MENU_V13_NAME_SIZE-1);
    QObject::connect(&load_,&LoadGameWidget::loadRequested,&load_,[this](const QString& id){bool ok=false;auto row=id.toUInt(&ok);if(ok)session_.requestStorage(MNM_MENU_STORAGE_ACCEPT,row);});
    QObject::connect(&load_,&LoadGameWidget::cancelled,&load_,[this]{session_.requestStorage(MNM_MENU_STORAGE_CANCEL);});
    QObject::connect(&save_,&SaveGameWidget::saveRequested,&save_,[this](const QString& name,const QString&){session_.requestStorage(MNM_MENU_STORAGE_ACCEPT,0,name);});
    QObject::connect(&save_,&SaveGameWidget::deleteRequested,&save_,[this](const QString& id){bool ok=false;auto row=id.toUInt(&ok);if(ok)session_.requestStorage(MNM_MENU_STORAGE_DELETE,row);});
    QObject::connect(&save_,&SaveGameWidget::cancelled,&save_,[this]{session_.requestStorage(MNM_MENU_STORAGE_CANCEL);});
}
bool LiveStorageMenuController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_LOAD_SCREEN&&state.screen!=MNM_MENU_SAVE_SCREEN){generation_=0;return true;}
    if(!session_.storageMenusEnabled){if(error)*error="Save/Load integration disabled.";return false;}
    if(!state.ready||state.generation==generation_)return true;
    const auto& b=state.storage;const auto selected=b.selected?QString::number(b.selected):QString();
    if(state.screen==MNM_MENU_LOAD_SCREEN){
        QVector<LoadGameWidget::Save> rows;for(int i=0;i<b.names.size();++i)rows.append({QString::number(i+1),b.names[i]});
        if(!load_.setSaves(rows,selected,error))return false;
        if(!b.selected)load_.findChild<QLineEdit*>("loadGameFileName")->setText(b.fileName);
    }else{
        QVector<SaveGameWidget::Save> rows;for(int i=0;i<b.names.size();++i)rows.append({QString::number(i+1),b.names[i]});
        if(!save_.setSaves(rows,selected,error))return false;
        save_.findChild<QLineEdit*>("saveGameFileName")->setText(b.fileName);
    }
    generation_=state.generation;return true;
}
