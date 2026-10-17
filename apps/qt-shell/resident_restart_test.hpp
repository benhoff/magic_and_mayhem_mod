#pragma once
#include "campaign_smoke_test.hpp"
#include <QJsonObject>
// Bounded opt-in actual-game acceptance. Production failure/restart code is unchanged.
void installResidentRestartTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&,
    std::function<CampaignPresentationProbe()>,std::function<QJsonObject()>);
