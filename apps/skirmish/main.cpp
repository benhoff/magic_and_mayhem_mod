#include "window.hpp"
#include "creature_runtime.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#ifdef MNM_SKIRMISH_WINDOW_TEST
#include "../../tests/skirmish-window-driver.hpp"
#endif
#ifdef MNM_CAMPAIGN_WINDOW_TEST
#include "../../tests/campaign-window-driver.hpp"
#endif
int main(int argc, char **argv) try {
#if defined(MNM_SKIRMISH_WINDOW_TEST) || defined(MNM_CAMPAIGN_WINDOW_TEST)
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
    QApplication app(argc, argv);
    QCommandLineParser p;
    p.addHelpOption();
    p.addOption({"installation", "Use installed Crocodile rules in native battles and rebind matching campaigns", "directory"});
    p.addOption({"asset-root", "Installed game data directory for map campaigns", "directory"});
    p.addOption({"mission-campaign", "Start the staged Plains mission and Cure reward campaign"});
    p.addOption({"map-campaign", "Start a native campaign on an installed Plains crop"});
    p.addOption({"campaign", "Start a new native campaign"});
    p.addOption({"load-campaign", "Load a native campaign checkpoint", "file"});
    p.addOption({"load", "Load a native skirmish checkpoint", "file"});
#if defined(MNM_SKIRMISH_WINDOW_TEST) || defined(MNM_CAMPAIGN_WINDOW_TEST)
    p.addOption({"test-script", "Bounded test-only window script", "file"});
    p.addOption({"test-output", "New test-only output directory", "directory"});
#endif
    p.process(app);
    auto creatureDriver = p.isSet("installation") ? mnm::skirmish::installedCreatureRuntime(p.value("installation").toStdString()) : nullptr;
    mnm::skirmish::Controller c(p.isSet("asset-root")?p.value("asset-root").toStdString():p.value("installation").toStdString(), creatureDriver);
    if (int(p.isSet("load")) + int(p.isSet("campaign")) + int(p.isSet("load-campaign")) + int(p.isSet("map-campaign")) + int(p.isSet("mission-campaign")) > 1)
        throw std::invalid_argument("Choose one startup mode");
    if (p.isSet("mission-campaign"))
        c.newMapCampaign({},true);
    if (p.isSet("map-campaign"))
        c.newMapCampaign();
    if (p.isSet("campaign"))
        c.newCampaign();
    if (p.isSet("load-campaign"))
        c.loadCampaign(p.value("load-campaign").toStdString());
    if (p.isSet("load"))
        c.load(p.value("load").toStdString());
    mnm::skirmish::Window window(c);
    window.show();
#ifdef MNM_SKIRMISH_WINDOW_TEST
    auto driver =
        mnm::skirmish::test::start(p.value("test-script"), p.value("test-output"), window, c);
#endif
#ifdef MNM_CAMPAIGN_WINDOW_TEST
    mnm::skirmish::test::campaignWindow(p.value("test-script"), p.value("test-output"), window, c);
#endif
    return app.exec();
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
