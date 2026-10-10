#pragma once
#include <QtGlobal>
#include <functional>
class QApplication;
class QMainWindow;
class LiveMenuSession;
class QString;
struct CampaignPresentationProbe { quint64 frames=0; bool active=false; bool fallback=false; };
void installCampaignSmokeTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&,
                             std::function<CampaignPresentationProbe()>);
