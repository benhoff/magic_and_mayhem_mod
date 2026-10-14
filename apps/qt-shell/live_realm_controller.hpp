#pragma once
#include "live_menu_session.hpp"
#include "realm_viewer_widget.hpp"
#include "realm_viewer_assets.hpp"
class LiveRealmController final {
public:
    LiveRealmController(LiveMenuSession&,RealmViewerWidget&);
    bool loadAssets(const QString&,QString* error=nullptr);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    RealmViewerWidget& widget_;
    std::array<QVector<mnm::ui::RealmCatalogRegion>,3> catalog_{};
    quint32 generation_=0;
};
