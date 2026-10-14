#pragma once
#include <QtGlobal>
#include <functional>
#include <QString>
class QApplication;
class QMainWindow;
class LiveMenuSession;
class QString;
struct CampaignPresentationProbe { quint64 frames=0; bool active=false; bool fallback=false; quint64 paints=0; unsigned recoveries=0; QString error; QString renderer; quint64 presentationPixels=0;quint64 sourceGeneration=0; };
void installCampaignSmokeTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&,
                             std::function<CampaignPresentationProbe()>);

void installNativeHealingSmokeTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&,std::function<CampaignPresentationProbe()>);

// Repainting cached Worlds is display progress; a resident Quit-No resume must
// also pass the two-credit captured-source backlog and import fresh Worlds.
inline bool campaignResumeConfirmed(const CampaignPresentationProbe& p,quint64 frames,quint64 source,bool resident){
    return p.active && p.frames>=frames && p.frames-frames>=3 &&
        (!resident || (p.sourceGeneration>source && p.sourceGeneration-source>=3));
}
