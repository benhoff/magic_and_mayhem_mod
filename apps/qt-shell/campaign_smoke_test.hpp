#pragma once
#include <QtGlobal>
#include <functional>
#include <QString>
class QApplication;
class QMainWindow;
class LiveMenuSession;
class QString;
struct CampaignPresentationProbe { quint64 frames=0; bool active=false; bool fallback=false; quint64 paints=0; unsigned recoveries=0; QString error; QString renderer; };
void installCampaignSmokeTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&,
                             std::function<CampaignPresentationProbe()>);
