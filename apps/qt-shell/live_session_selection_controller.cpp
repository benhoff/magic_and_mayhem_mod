#include "live_session_selection_controller.hpp"
#include "../../protocols/include/mnm/menu_v16.h"
LiveSessionSelectionController::LiveSessionSelectionController(LiveMenuSession& session,MultiplayerGameSelectionWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&MultiplayerGameSelectionWidget::sessionSelected,&widget_,[this](const QString& id){bool ok=false;const auto row=id.toUInt(&ok);if(entered_&&ok&&row&&row<=quint32(names_.size()))session_.request(MNM_MENU_SESSION_JOIN,row);});
    QObject::connect(&widget_,&MultiplayerGameSelectionWidget::cancelled,&widget_,[this]{if(entered_)session_.request(MNM_MENU_SESSION_CANCEL);});
}
bool LiveSessionSelectionController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=21){entered_=false;names_.clear();return true;}if(!state.ready)return true;
    const auto& b=state.sessions;if(b.parent!=16||b.depth!=5||b.context!=2||b.transport<1||b.transport>3||b.busy||b.pending||b.actions!=3){if(error)*error="Unsupported session-selection caller or controls.";return false;}
    if(!entered_||names_!=b.names){QVector<MultiplayerGameSelectionWidget::Session> rows;for(int i=0;i<b.names.size();++i)rows.append({QString::number(i+1),b.names[i]});
        const auto selected=b.selected?QString::number(b.selected):QString();if(!widget_.setSessions(rows,selected,error))return false;names_=b.names;
    }
    entered_=true;return true;
}
