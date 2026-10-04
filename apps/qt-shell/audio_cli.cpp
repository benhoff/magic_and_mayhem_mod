#include "audio_cli.hpp"
#include "audio_session.hpp"
#include <QComboBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMediaDevices>
#include <QPushButton>
#include <QSaveFile>
#include <QVBoxLayout>
#include <cstdio>
namespace r=mnm::reconstruction::audio;
void addAudioOptions(QCommandLineParser& p){
    p.addOption({"audio-catalog","Preview the reconstructed audio manager without a game.","sounds-directory"});
    p.addOption({"audio-preflight","Check catalog WAV data without opening an audio device."});
    p.addOption({"audio-path-policy","Source filenames: literal or dequote-missing-leaf.","policy","literal"});
    p.addOption({"audio-map","Map ID used for source classifications and permanent preload.","id","1"});
    p.addOption({"audio-sound","Initially select a catalog sound or randomized group.","id"});
    p.addOption({"audio-report","Write catalog preflight and startup results as JSON.","file"});
}
namespace {
QJsonDocument report(const r::CatalogPreflight& p,const QString& error={},unsigned rate=0){
    QJsonArray sources,groups;
    for(const auto& s:p.sources)sources.append(QJsonObject{{"id",s.id},{"path",QString::fromStdString(s.path)},
        {"playable",s.playable},{"pcmBytes",qint64(s.pcmBytes)},{"durationMs",qint64(s.durationMs)},{"diagnostic",QString::fromStdString(s.diagnostic)}});
    for(const auto& g:p.groups){QJsonArray members,resolved;for(auto id:g.members)members.append(id);for(auto id:g.resolved)resolved.append(id);
        groups.append(QJsonObject{{"id",g.id},{"members",members},{"resolved",resolved},{"playable",g.playable},{"diagnostic",QString::fromStdString(g.diagnostic)}});}
    return QJsonDocument(QJsonObject{{"version",1},{"gameLaunched",false},{"policy",p.policy==r::NativeSourcePathPolicy::literal?"literal":"dequote-missing-leaf"},
        {"catalogValid",p.valid()},{"catalogError",QString::fromStdString(p.diagnostic)},
        {"simultaneousLimit",qint64(p.simultaneousLimit)},{"sources",sources},{"groups",groups},{"outputRate",qint64(rate)},{"startupError",error}});
}
bool save(const QString& path,const QJsonDocument& document){
    const auto bytes=document.toJson();
    if(path.isEmpty()){std::fwrite(bytes.constData(),1,std::size_t(bytes.size()),stdout);return true;}
    QSaveFile file(path);return file.open(QIODevice::WriteOnly) && file.write(bytes)==bytes.size() && file.commit();
}
}
int runAudio(QApplication& app,const QCommandLineParser& p){
    const auto policyText=p.value("audio-path-policy");
    if(policyText!="literal" && policyText!="dequote-missing-leaf")return 2;
    const auto policy=policyText=="literal"?r::NativeSourcePathPolicy::literal:r::NativeSourcePathPolicy::dequoteMissingLeaf;
    bool ok=false;const auto map=p.value("audio-map").toUInt(&ok);if(!ok)return 2;
    int selected=0;if(p.isSet("audio-sound")){selected=p.value("audio-sound").toInt(&ok);if(!ok || selected<=0)return 2;}
    if(p.isSet("audio-preflight")){
        auto made=mnm::assets::AssetStore::create(p.value("audio-catalog").toStdString());
        if(const auto* error=std::get_if<mnm::assets::Error>(&made)){std::fprintf(stderr,"%s\n",error->detail.c_str());return 2;}
        const auto preflight=r::preflightCatalog(std::get<mnm::assets::AssetStore>(std::move(made)),policy);
        if(!save(p.value("audio-report"),report(preflight)))return 8;
        if(!preflight.valid())return 2;
        // Report all gaps; a nonzero status makes missing assets useful in automation.
        for(const auto& s:preflight.sources)if(!s.playable)return 3;
        for(const auto& g:preflight.groups)if(!g.playable)return 3;
        return 0;
    }
    AudioSession session(makeQtSessionOutput(QMediaDevices::defaultAudioOutput()));
    const bool started=session.start(p.value("audio-catalog"),map,policy);
    if(!save(p.value("audio-report"),report(session.preflight(),session.lastError(),session.outputRate())))return 8;
    if(!started){std::fprintf(stderr,"%s\n",session.lastError().toUtf8().constData());return 4;}
    QWidget window;window.setWindowTitle("Native audio catalog — "+policyText);
    auto* layout=new QVBoxLayout(&window);auto* choices=new QComboBox(&window);
    const auto add=[&](int id,const QString& kind,bool playable){choices->addItem(QString("%1 %2 — %3").arg(kind).arg(id).arg(playable?"ready":"unavailable"),id);};
    for(const auto& s:session.preflight().sources)add(s.id,"Sound",s.playable);
    for(const auto& g:session.preflight().groups)add(g.id,"Group",g.playable);
    if(selected){const auto index=choices->findData(selected);if(index<0)return 2;choices->setCurrentIndex(index);}
    auto* status=new QLabel("Ready. Missing entries are listed in the preflight report.",&window);
    auto* play=new QPushButton("Play",&window);auto* stop=new QPushButton("Stop session",&window);auto* restart=new QPushButton("Restart session",&window);
    layout->addWidget(choices);layout->addWidget(play);layout->addWidget(stop);layout->addWidget(restart);layout->addWidget(status);
    QObject::connect(play,&QPushButton::clicked,&window,[&]{status->setText(session.play(choices->currentData().toInt())?"Playing":session.lastError());});
    QObject::connect(stop,&QPushButton::clicked,&window,[&]{session.stop();status->setText("Stopped; queued PCM discarded");});
    QObject::connect(restart,&QPushButton::clicked,&window,[&]{status->setText(session.start(p.value("audio-catalog"),map,policy)?"Ready":session.lastError());});
    window.resize(520,200);window.show();return app.exec();
}
