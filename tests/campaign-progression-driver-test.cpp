#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>

// Actual Qt event-loop/public-control regression. No process, bridge, game,
// renderer or original campaign behavior is executed by this fixture.
int main(int argc,char** argv)try{
    QApplication app(argc,argv);
    const QString test=argc>1?QString::fromLocal8Bit(argv[1]):"progression";
    const bool disabled=test=="disabled",wrongRealm=test=="wrong-realm";
    const QString mode=(disabled||wrongRealm)?"progression":test;
    qputenv("MNM_CAMPAIGN_PROGRESSION_CASE",mode.toUtf8());
    qputenv("MNM_CAMPAIGN_PROGRESSION_SIZE","800x600");
    QTemporaryDir temporary;if(!temporary.isValid())throw std::runtime_error("Temporary fixture output");
    QMainWindow window;auto* body=new QWidget(&window);auto* layout=new QVBoxLayout(body);
    window.setCentralWidget(body);
    LiveMenuSession session("/unused");QStringList clicks;bool entered=false,timedOut=false;
    MenuBridge::State state;state.ready=1;state.thread=40;state.generation=1;state.sequence=2;
    auto publish=[&](quint32 screen){state.screen=screen;++state.ack;state.sequence+=2;++state.generation;if(session.stateChanged)session.stateChanged(state);};
    auto add=[&](const QString& name,auto action){
        auto* button=new QPushButton(name,body);button->setObjectName(name);layout->addWidget(button);
        QObject::connect(button,&QPushButton::clicked,&window,[&,name,action]{clicks.append(name);action();});return button;
    };
    add("launchGame",[&]{state.ack=0;state.screen=3;session.stateChanged(state);})->setEnabled(!disabled);
    add("mainMenuAction3",[&]{publish(10);});
    add("preferencesRadio1",[]{});
    add("preferencesOk",[&]{publish(3);});
    add("mainMenuAction0",[&]{state.region.realm=wrongRealm?"Greek":"Celtic";state.region.region=1;publish(18);});
    add("regionEntryDifficulty1",[&]{state.region.difficulty=1;publish(18);});
    add("regionEntryEnter",[&]{entered=true;app.quit();});
    window.show();
    installCampaignProgressionTest(app,window,session,temporary.filePath("flow.json"),[]{return CampaignPresentationProbe{};});
    QTimer::singleShot(2000,&window,[&]{timedOut=true;app.exit(2);});
    const int code=app.exec();
    if(timedOut||session.running()||session.launcherProcessId())throw std::runtime_error("Missing bounded dispatch or unexpected process launch");
    if(disabled||wrongRealm){
        QFile report(temporary.filePath("flow.json"));if(!report.open(QIODevice::ReadOnly))throw std::runtime_error("Missing refusal report");
        const auto flow=QJsonDocument::fromJson(report.readAll()).object();
        const auto expected=disabled?"Public control unavailable: launchGame":"Unexpected original region identity";
        if(code!=1||flow["success"].toBool()||flow["error"].toString()!=expected||entered)throw std::runtime_error("Refusal was bypassed");
    }else{
        const QStringList expected{"launchGame","mainMenuAction3","preferencesRadio1","preferencesOk","mainMenuAction0","regionEntryDifficulty1","regionEntryEnter"};
        if(code||!entered||clicks!=expected)throw std::runtime_error("Public startup control sequence differs");
    }
    std::cout<<"{\"success\":true,\"original_game_executed\":false,\"controls\":"<<clicks.size()<<"}\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
